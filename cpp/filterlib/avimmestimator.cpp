//
// Created by Felix on 25.05.2020.
//

#include "avimmestimator.h"
#include "avimmkalmanfilter.h"
#include "avimmextendedkalmanfilter.h"
#include "utils/avimmconfigparser.h"

AVIMMEstimator::AVIMMEstimator(const Vector& initial_state)
{
    m_mode_probabilities       = AVIMMConfigContainer::singleton().mode_probabilities;
    m_markov_transition_matrix = AVIMMConfigContainer::singleton().markov_transition_matrix;
    m_filter_types             = AVIMMConfigContainer::singleton().filter_types;
    
    m_mode_probabilities_matrix = Matrix::Zero(m_markov_transition_matrix.rows(), m_markov_transition_matrix.cols());
    
    // Initialize IMM Subfilters
    initializeSubfilters(initial_state);
    
    // Perform initial probability calculation and set IMM state
    calculateModeProbabilityMatrix();
    calculateIMMState();
    m_last_calculation = QDateTime::currentDateTimeUtc();
    m_previous_data = m_data;
    
    // Always have this on false, this can be set by the unittesthelper classe to enable special funtionality only needed for test running
    m_test_run = false;
}

//--------------------------------------------------------------------------

AVIMMEstimator::~AVIMMEstimator()
{
    cleanup_unique_ptr(m_filters);
}

//--------------------------------------------------------------------------

void AVIMMEstimator::initializeSubfilters(const Vector& initial_state)
{
    for (const auto& sub_filter_config : AVIMMConfigContainer::singleton().filters)
    {
        switch(AVIMMConfigContainer::singleton().filter_type_map[sub_filter_config->getConfigMapKey()])
        {
            case KalmanFilter: {
                // Initialize all matrices with a dt=0.0;
                Matrix F = AVIMMConfigParser::singleton().calculateTimeDependentMatrices(sub_filter_config->transition_matrix);
                Matrix P = AVIMMConfigParser::singleton().calculateTimeDependentMatrices(sub_filter_config->covariance_matrix);
                Matrix Q = AVIMMConfigParser::singleton().calculateTimeDependentMatrices(sub_filter_config->process_noise_matrix);
                Matrix B = AVIMMConfigParser::singleton().calculateTimeDependentMatrices(sub_filter_config->input_control_matrix);
                Matrix H = AVIMMConfigParser::singleton().calculateTimeDependentMatrices(sub_filter_config->measurement_control_matrix);
                Matrix R = AVIMMConfigParser::singleton().calculateTimeDependentMatrices(sub_filter_config->measurement_uncertainty_matrix);
                m_filters.push_back(std::make_unique<AVIMMKalmanFilter>(initial_state, F, P, H, Q, R, B, sub_filter_config->getConfigMapKey()));
                break;
            }
            case ExtendedKalmanFilter: {
                // Initialize all matrices with a dt=0.0;
                Matrix F = AVIMMConfigParser::singleton().calculateTimeDependentMatrices(sub_filter_config->transition_matrix);
                Matrix P = AVIMMConfigParser::singleton().calculateTimeDependentMatrices(sub_filter_config->covariance_matrix);
                Matrix Q = AVIMMConfigParser::singleton().calculateTimeDependentMatrices(sub_filter_config->process_noise_matrix);
                Matrix B = AVIMMConfigParser::singleton().calculateTimeDependentMatrices(sub_filter_config->input_control_matrix);
                Matrix H = AVIMMConfigParser::singleton().calculateTimeDependentMatrices(sub_filter_config->measurement_control_matrix);
                Matrix R = AVIMMConfigParser::singleton().calculateTimeDependentMatrices(sub_filter_config->measurement_uncertainty_matrix);
                Matrix J = AVIMMConfigParser::singleton().calculateTimeDependentMatrices(sub_filter_config->jacobi_matrix);
                m_filters.push_back(std::make_unique<AVIMMExtendedKalmanFilter>(initial_state, F, P, H, Q, R, B, J, sub_filter_config->getConfigMapKey()));
                break;
            }
            default:
                assert(("Invalid Filtertype!", false));
        }
    }
}

//--------------------------------------------------------------------------

void AVIMMEstimator::predictAndUpdate(const Vector &z, const Matrix &R_in, const Vector &u)
{
    Matrix R = R_in;
    prepare();
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
        if (R == DEFAULT_MATRIX)
        {
            R = AVIMMConfigParser::singleton().calculateTimeDependentMatrices(
                    AVIMMConfigContainer::singleton().filters[filter->getFilterKey()]->measurement_uncertainty_matrix);
        }
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

void AVIMMEstimator::calculateModeProbabilityMatrix()
{
    m_c = m_mode_probabilities * m_markov_transition_matrix;
    
    for (int i = 0; i < m_markov_transition_matrix.cols(); i++)
        for (int j = 0; j < m_markov_transition_matrix.rows(); j++)
        {
            m_mode_probabilities_matrix(i, j) = (m_markov_transition_matrix.coeff(i, j) * m_mode_probabilities[i])/m_c[j];
        }
}

//--------------------------------------------------------------------------

void AVIMMEstimator::calculateIMMState()
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

void AVIMMEstimator::calculateMixedStates()
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

void AVIMMEstimator::calculateModeProbabilities()
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

Vector AVIMMEstimator::expandVector(const Vector &x)
{
    Vector new_x = x;
    auto requested_size = REQUESTED_SIZE;
    auto vector_size = x.size();
    if (vector_size == requested_size)
        return new_x;
    
    // Fill with one until the requested size has been reached.
    for (int i = vector_size; i < requested_size; i++)
        new_x << 1.0;
    
    new_x = AVIMMConfigContainer::singleton().expansion_matrix * new_x;
    return new_x;
}

//--------------------------------------------------------------------------

Matrix AVIMMEstimator::expandMatrix(const Matrix &M)
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
    new_M = AVIMMConfigContainer::singleton().expansion_matrix * ones_M * AVIMMConfigContainer::singleton().expansion_matrix.transpose();
    return new_M;
}

//--------------------------------------------------------------------------

Matrix AVIMMEstimator::expandCovariance(const Matrix &M)
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
    new_M = AVIMMConfigContainer::singleton().expansion_matrix_covariance * ones_M * AVIMMConfigContainer::singleton().expansion_matrix_covariance.transpose();
    return new_M;
}


//--------------------------------------------------------------------------

Vector AVIMMEstimator::shrinkVector(const Vector &x, int dim)
{
    Vector new_x = x;
    if (dim == REQUESTED_SIZE)
        return new_x;
    
    new_x = AVIMMConfigContainer::singleton().shrinking_matrix * x;
    return new_x;
}

//--------------------------------------------------------------------------

Matrix AVIMMEstimator::shrinkMatrix(const Matrix &M, int dim)
{
    Matrix new_M = M;
    if (dim == REQUESTED_SIZE)
        return new_M;
    
    new_M = AVIMMConfigContainer::singleton().shrinking_matrix * M * AVIMMConfigContainer::singleton().shrinking_matrix.transpose();
    return new_M;
}

//--------------------------------------------------------------------------

void AVIMMEstimator::prepare()
{
    // Save data into previous data struct to preserves the previous state and covariance
    m_previous_data = m_data;
    
    // Save time of calculation and get time delta since last calculation
    if (!m_test_run)
        m_now = QDateTime::currentDateTimeUtc();
    
    float time_delta = static_cast<float>(m_last_calculation.msecsTo(m_now) / 1000);
    
    // Calculate all time depended matrices
    for (auto& filter: m_filters)
    {
        const QString filter_key = filter->getFilterKey();
        
        filter->getData().F = AVIMMConfigParser::singleton().calculateTimeDependentMatrices(
                AVIMMConfigContainer::singleton().filters[filter_key]->transition_matrix, time_delta);
        filter->getData().P = AVIMMConfigParser::singleton().calculateTimeDependentMatrices(
                AVIMMConfigContainer::singleton().filters[filter_key]->covariance_matrix, time_delta);
        filter->getData().H = AVIMMConfigParser::singleton().calculateTimeDependentMatrices(
                AVIMMConfigContainer::singleton().filters[filter_key]->measurement_control_matrix, time_delta);
        filter->getData().Q = AVIMMConfigParser::singleton().calculateTimeDependentMatrices(
                AVIMMConfigContainer::singleton().filters[filter_key]->process_noise_matrix, time_delta);
        filter->getData().R = AVIMMConfigParser::singleton().calculateTimeDependentMatrices(
                AVIMMConfigContainer::singleton().filters[filter_key]->measurement_uncertainty_matrix, time_delta);
        filter->getData().B = AVIMMConfigParser::singleton().calculateTimeDependentMatrices(
                AVIMMConfigContainer::singleton().filters[filter_key]->input_control_matrix, time_delta);
    }
    
    // Save now as las calculation step
    m_last_calculation = m_now;
}
