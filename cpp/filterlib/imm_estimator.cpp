//
// Created by Felix on 25.05.2020.
//

#include "imm_estimator.h"
#include "imm_kalman_filter.h"
#include "imm_extended_kalman_filter.h"

IMMEstimator::IMMEstimator()
{
    m_imm_config               = Config::instance();
    m_mode_probabilities       = m_imm_config.getModeProbabilities();
    m_markov_transition_matrix = m_imm_config.getMarkovTransitionMatrix();
    m_filter_types             = m_imm_config.getFilterType();
    
    m_mode_probabilities_matrix = Matrix::Zero(m_markov_transition_matrix.rows(), m_markov_transition_matrix.cols());

    // Initialize IMM Subfilters
    initializeSubfilters();
    
    // Perform initial probability calculation and set IMM state
    calculateModeProbabilityMatrix();
    calculateIMMState();
    m_previous_data = m_data;
}

//--------------------------------------------------------------------------

IMMEstimator::~IMMEstimator()
{
    cleanup_unique_ptr(m_filters);
}

//--------------------------------------------------------------------------

void IMMEstimator::initializeSubfilters()
{
    for (const auto& filter_type : m_filter_types)
    {
        switch (filter_type) {
            case KalmanFilter:
            {
                SubFilterConfig config = m_imm_config.getFilterConfigs()[KalmanFilter];
                Matrix F = config.m_transition_matrix;
                Matrix P = config.m_covariance_matrix;
                Matrix Q = config.m_process_noise_matrix;
                Matrix B = config.m_input_control_matrix;
                Matrix H = config.m_measurement_control_matrix;
                Matrix R = config.m_measurement_uncertainty_matrix;
                Vector initialState = Vector::Zero(F.rows(), 1);
                m_filters.push_back(std::make_unique<IMMKalmanFilter>(initialState, F, P, H, Q, R, B));
            }
            case ExtendedKalmanFilter:
            {
                SubFilterConfig config = m_imm_config.getFilterConfigs()[ExtendedKalmanFilter];
                Matrix F = config.m_transition_matrix;
                Matrix P = config.m_covariance_matrix;
                Matrix J = m_imm_config.createUnityMatrix(F.rows()); // TODO: figure out how to use inheritance correctly here
                Matrix Q = config.m_process_noise_matrix;
                Matrix B = config.m_input_control_matrix;
                Matrix H = config.m_measurement_control_matrix;
                Matrix R = config.m_measurement_uncertainty_matrix;
                Vector initialState = Vector::Zero(F.rows(), 1);
                m_filters.push_back(std::make_unique<IMMExtendedKalmanFilter>(initialState, F, P, H, Q, R, B, J));
            }
            assert(("Invalid Filtertype!", false));
        }
    }
}

//--------------------------------------------------------------------------

void IMMEstimator::predictAndUpdate(const Vector &z, const Matrix &R, const Vector &u)
{
    // Save data into previous data struct to preserves the previous state and covariance
    m_previous_data = m_data;
    calculateMixedStates();
    
    // Predict each filter
    int i = 0;
    for (const auto& filter: m_filters)
    {
        // Shrink filter state to correct size, this allows for subfilters with only a subset of the IMM state
        filter->getData().x = shrinkVector(m_mixed_states[i], filter->getData().x.size());
        filter->getData().P = shrinkMatrix(m_mixed_covariances[i], filter->getData().x.size());
        filter->predict(u);
        i++;
    }
    
    // Calculate the IMM state after prediction of each filter has finished
    calculateIMMState();
    m_data.x_prior = m_data.x;
    m_data.P_prior = m_data.P;
    
    // Update each filter
    for (const auto& filter: m_filters)
    {
        // Shrink measurement z and measurement variance R, this allows for subfilters with only a subset of the IMM state
        Vector z_shrunk = shrinkVector(z, filter->getData().x.size());
        Matrix R_shrunk = shrinkMatrix(R, filter->getData().x.size());
        filter->update(z, R);
    }
    
    // Recalculate Probabilities after update step to be prepared for the next calculation step
    calculateModeProbabilities();
    calculateModeProbabilityMatrix();
    calculateIMMState();
    m_data.x_post = m_data.x;
    m_data.P_post = m_data.P;
}

//--------------------------------------------------------------------------

void IMMEstimator::calculateModeProbabilityMatrix()
{
    m_c = m_mode_probabilities * m_markov_transition_matrix;
    
    for (int i = 0; i < m_markov_transition_matrix.cols(); i++)
        for (int j = 0; j < m_markov_transition_matrix.rows(); j++)
        {
            m_mode_probabilities_matrix(i, j) = (m_markov_transition_matrix.coeff(i, j) * m_mode_probabilities[i])/m_c[j];
        }
}

//--------------------------------------------------------------------------

void IMMEstimator::calculateIMMState()
{
    Vector x = Vector::Zero(m_data.x.rows(), m_data.x.cols());
    int i = 0;
    for (const auto &filter : m_filters) {
        auto probability = m_mode_probabilities[i];
        const Vector& filter_state_expanded = expandVector(filter->getData().x);
        x += filter_state_expanded * probability;
        i++;
    }
    
    Matrix P = Matrix::Zero(m_data.P.rows(), m_data.P.cols());
    i = 0;
    for (const auto &filter : m_filters) {
        auto probability = m_mode_probabilities[i];
        const Vector& filter_state_expanded = expandVector(filter->getData().x);
        const Matrix& filter_covariance_expanded = expandCovariance(filter->getData().P);
        const Vector& state_diff = filter_state_expanded - m_data.x;
        P += probability * ((state_diff * state_diff.transpose()) + filter_covariance_expanded);
        i++;
    }
    
    m_data.x = x;
    m_data.P = P;
}

//--------------------------------------------------------------------------

void IMMEstimator::calculateMixedStates()
{
    std::vector<Vector> xs;
    std::vector<Matrix> Ps;
    
    for (int j = 0; j < m_mode_probabilities_matrix.cols(); j++)
    {
        Vector col = m_mode_probabilities.col(j);
        
        Vector x = Vector::Zero(m_data.x.rows(), m_data.x.cols());
        int i = 0;
        for (const auto &filter : m_filters) {
            auto probability = col[i];
            const Vector& filter_state_expanded = expandVector(filter->getData().x);
            x += filter_state_expanded * probability;
            i++;
        }
        xs.push_back(x);
        
        Matrix P = Matrix::Zero(m_data.P.rows(), m_data.P.cols());
        i = 0;
        for (const auto &filter : m_filters) {
            auto probability = col[i];
            const Vector& filter_state_expanded = expandVector(filter->getData().x);
            const Matrix& filter_covariance_expanded = expandCovariance(filter->getData().P);
            const Vector& state_diff = filter_state_expanded - m_data.x;
            P += probability * ((state_diff * state_diff.transpose()) + filter_covariance_expanded);
            i++;
        }
        Ps.push_back(P);
    }
    m_mixed_states = xs;
    m_mixed_covariances = Ps;
}

//--------------------------------------------------------------------------

void IMMEstimator::calculateModeProbabilities()
{
    Vector probabilities(m_filters.size(), 1);
    int i = 0;
    auto sum = 0.0;
    for (const auto& filter : m_filters) {
        double probability = m_c[i] * filter->getLikelihood();
        probabilities << probability;
        sum += probability;
        i++;
    }
    probabilities /= sum;
    m_mode_probabilities = probabilities;
}

//--------------------------------------------------------------------------

Vector IMMEstimator::expandVector(const Vector &x)
{
    Vector new_x = x;
    auto requested_size = REQUESTED_SIZE;
    auto vector_size = x.size();
    if (vector_size == requested_size)
        return new_x;
    
    // Fill with one until the requested size has been reached.
    for (int i = vector_size; i < requested_size; i++)
        new_x << 1.0;
    
    new_x = m_imm_config.getExpansionMatrix() * new_x;
    return new_x;
}

//--------------------------------------------------------------------------

Matrix IMMEstimator::expandMatrix(const Matrix &M)
{
    Matrix new_M = M;
    auto requested_cols = REQUESTED_SIZE;
    auto requested_rows = REQUESTED_SIZE;
    auto matrix_cols = M.cols();
    auto matrix_rows = M.rows();
    
    if (requested_cols == matrix_cols && requested_rows == matrix_rows)
        return new_M;
    
    Matrix ones_M = Matrix::Ones(requested_rows ,requested_cols);
    for (int i = 0; i < matrix_rows; i++)
        for (int j = 0; j < matrix_cols; j++)
            ones_M(i,j) = new_M.coeff(i, j);
    new_M = m_imm_config.getExpansionMatrix() * ones_M * m_imm_config.getExpansionMatrix().transpose();
    return new_M;
}

//--------------------------------------------------------------------------

Matrix IMMEstimator::expandCovariance(const Matrix &M)
{
    Matrix new_M = M;
    auto requested_cols = REQUESTED_SIZE;
    auto requested_rows = REQUESTED_SIZE;
    auto matrix_cols = M.cols();
    auto matrix_rows = M.rows();
    
    if (requested_cols == matrix_cols && requested_rows == matrix_rows)
        return new_M;
    
    Matrix ones_M = Matrix::Ones(requested_rows ,requested_cols);
    for (int i = 0; i < matrix_rows; i++)
        for (int j = 0; j < matrix_cols; j++)
            ones_M(i,j) = new_M.coeff(i, j);
    new_M = m_imm_config.getExpansionMatrixCovariance() * ones_M * m_imm_config.getExpansionMatrixCovariance().transpose();
    return new_M;
}


//--------------------------------------------------------------------------

Vector IMMEstimator::shrinkVector(const Vector &x, int dim)
{
    Vector new_x = x;
    if (dim == REQUESTED_SIZE)
        return new_x;
    
    new_x = m_imm_config.getShrinkingMatrix() * x;
    return new_x;
}

//--------------------------------------------------------------------------

Matrix IMMEstimator::shrinkMatrix(const Matrix &M, int dim)
{
    Matrix new_M = M;
    if (dim == REQUESTED_SIZE)
        return new_M;
    
    new_M = m_imm_config.getShrinkingMatrix() * M * m_imm_config.getShrinkingMatrix().transpose();
    return new_M;
}