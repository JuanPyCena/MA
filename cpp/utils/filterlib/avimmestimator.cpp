//
// Created by Felix on 25.05.2020.
//

#include "avimmestimator.h"
#include "avimmfilterbase.h"
#include "avimmkalmanfilter.h"
#include "avimmextendedkalmanfilter.h"

AVIMMEstimator::AVIMMEstimator(const Vector& initial_state)
{
    m_config = AVIMMAirportConfigs::singleton().getIMMConfigData(initial_state);
    // Take those from the first sufilter since those are the same for both
    m_mode_probabilities       = m_config.initial_mode_probabilities;
    m_markov_transition_matrix = m_config.markov_transition_matrix;
    m_filter_types             = AVIMMStaticConfigContainer::singleton().filter_types;
    
    m_mode_probabilities_matrix = Matrix::Zero(m_markov_transition_matrix.rows(), m_markov_transition_matrix.cols());
    
    // Initialize IMM Subfilters
    m_data.x = initial_state;
    m_data.P = Matrix::Zero(initial_state.rows(), initial_state.rows());
    initializeSubfilters(initial_state);
    
    // Perform initial probability calculation and set IMM state
    calculateModeProbabilityMatrix(m_mode_probabilities_matrix);
    calculateIMMState(m_data.x, m_data.P);
    m_last_calculation = QDateTime::currentDateTimeUtc();
    m_previous_data = m_data;
    
    // Always have this on false, this can be set by the unittesthelper classes to enable special funtionality only needed for test running
    m_test_run = false;
}

//--------------------------------------------------------------------------

AVIMMEstimator::~AVIMMEstimator()
{
    cleanup_ptr_container(m_filters);
}

//--------------------------------------------------------------------------

void AVIMMEstimator::initializeSubfilters(const Vector& initial_state)
{
    for (auto sub_filter_config_key : m_config.sub_filter_config_keys)
    {
        switch(AVIMMStaticConfigContainer::singleton().filter_type_map[sub_filter_config_key])
        {
            case KalmanFilter: {
                // Initialize all matrices with a dt=0.0;
                Matrix F = AVIMMConfigParser::singleton().calculateTimeDependentMatrices(m_config.F_map[sub_filter_config_key]);
                Matrix P = AVIMMConfigParser::singleton().calculateTimeDependentMatrices(m_config.P_map[sub_filter_config_key]);
                Matrix Q = AVIMMConfigParser::singleton().calculateTimeDependentMatrices(m_config.Q_map[sub_filter_config_key]);
                Matrix B = AVIMMConfigParser::singleton().calculateTimeDependentMatrices(m_config.B_map[sub_filter_config_key]);
                Matrix H = AVIMMConfigParser::singleton().calculateTimeDependentMatrices(m_config.H_map[sub_filter_config_key]);
                Matrix R = AVIMMConfigParser::singleton().calculateTimeDependentMatrices(m_config.R_map[sub_filter_config_key]);
                auto* filter = new AVIMMKalmanFilter(initial_state, F, P, H, Q, R, B, sub_filter_config_key);
                m_filters.push_back(filter);
                break;
            }
            case ExtendedKalmanFilter: {
                // Initialize all matrices with a dt=0.0;
                Matrix F = AVIMMConfigParser::singleton().calculateTimeDependentMatrices(m_config.F_map[sub_filter_config_key]);
                Matrix P = AVIMMConfigParser::singleton().calculateTimeDependentMatrices(m_config.P_map[sub_filter_config_key]);
                Matrix Q = AVIMMConfigParser::singleton().calculateTimeDependentMatrices(m_config.Q_map[sub_filter_config_key]);
                Matrix B = AVIMMConfigParser::singleton().calculateTimeDependentMatrices(m_config.B_map[sub_filter_config_key]);
                Matrix H = AVIMMConfigParser::singleton().calculateTimeDependentMatrices(m_config.H_map[sub_filter_config_key]);
                Matrix R = AVIMMConfigParser::singleton().calculateTimeDependentMatrices(m_config.R_map[sub_filter_config_key]);
                Matrix J = AVIMMConfigParser::singleton().calculateTimeDependentMatrices(m_config.J_map[sub_filter_config_key]);
                auto* filter = new AVIMMExtendedKalmanFilter(initial_state, F, P, H, Q, R, B, J, sub_filter_config_key);
                m_filters.push_back(filter);
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
    calculateMixedStates(m_mixed_states, m_mixed_covariances);
    
    // Predict each filter
    int i = 0;
    for (const auto &filter: m_filters)
    {
        // Shrink filter state to correct size, this allows for subfilters with only a subset of the IMM state
        filter->getData().x = shrinkVector(m_mixed_states[i], filter->getData().x.size());
        filter->getData().P = shrinkMatrix(m_mixed_covariances[i], filter->getData().x.size());
        filter->predict(u);
        auto data = filter->getData();
        data.x = filter->getData().x_prior;
        data.P = filter->getData().P_prior;
        filter->setData(data);
        i++;
    }
    
    // Calculate the IMM state after prediction of each filter has finished
    calculateIMMState(m_data.x, m_data.P);
    m_data.x_prior = m_data.x;
    m_data.P_prior = m_data.P;
    
    // Update each filter
    for (const auto &filter: m_filters)
    {
        if (R == DEFAULT_MATRIX)
        {
            R = AVIMMConfigParser::singleton().calculateTimeDependentMatrices(
                    m_config.R_map[filter->getFilterKey()]);
        }
        // Shrink measurement z and measurement variance R, this allows for subfilters with only a subset of the IMM state
        Vector z_shrunk = shrinkVector(z, filter->getData().x.size());
        Matrix R_shrunk = shrinkMatrix(R, filter->getData().x.size());
        filter->update(z, R);
    }
    
    // Recalculate Probabilities after update step to be prepared for the next calculation step
    calculateModeProbabilities(m_mode_probabilities);
    calculateModeProbabilityMatrix(m_mode_probabilities_matrix);
    calculateIMMState(m_data.x, m_data.P);
    
    m_data.x_post     = m_data.x;
    m_data.P_post     = m_data.P;
    m_data.time_stamp = QDateTime::currentDateTimeUtc();
}

//--------------------------------------------------------------------------

std::pair<Vector, Matrix> AVIMMEstimator::extrapolate(const Vector &u)
{
    prepare(true);
    
    // Calculated mixed states
    std::vector<Vector> mixed_xs;
    std::vector<Matrix> mixed_Ps;
    
    calculateMixedStates(mixed_xs, mixed_Ps);
    
    // Predict each filter
    int i = 0;
    for (const auto &filter: m_filters)
    {
        // Shrink filter state to correct size, this allows for subfilters with only a subset of the IMM state
        filter->getData().x = shrinkVector(mixed_xs[i], filter->getData().x.size());
        filter->getData().P = shrinkMatrix(mixed_Ps[i], filter->getData().x.size());
        filter->predict(u);
        auto data = filter->getData();
        data.x = filter->getData().x_prior;
        data.P = filter->getData().P_prior;
        filter->setData(data);
        i++;
    }
    
    // Calculate the IMM state and covariance after prediction of each filter has finished
    Vector x_extrapolated = Vector::Zero(m_data.x.rows(), m_data.x.cols());
    Matrix P_extrapolated = Matrix::Zero(m_data.P.rows(), m_data.P.cols());
    
    calculateIMMState(x_extrapolated, P_extrapolated);
    
    return std::make_pair(x_extrapolated, P_extrapolated);
}

//--------------------------------------------------------------------------

void AVIMMEstimator::calculateModeProbabilityMatrix(Matrix& mode_probability_matrix)
{
    Matrix probability_matrix = Matrix::Zero(m_mode_probabilities_matrix.rows(), m_mode_probabilities_matrix.cols());
    m_c = m_markov_transition_matrix * m_mode_probabilities;
    
    for (int i = 0; i < m_markov_transition_matrix.cols(); i++)
        for (int j = 0; j < m_markov_transition_matrix.rows(); j++)
            probability_matrix(i, j) = (m_markov_transition_matrix.coeff(i, j) * m_mode_probabilities[i])/m_c[j];
    
    mode_probability_matrix = probability_matrix;
}

//--------------------------------------------------------------------------

void AVIMMEstimator::calculateIMMState(Vector& imm_state, Matrix& imm_covariance)
{
    Vector x = Vector::Zero(m_data.x.rows(), m_data.x.cols());
    int i = 0;
    for (const auto &filter : m_filters)
    {
        auto probability = m_mode_probabilities[i];
        const Vector& filter_state_expanded = expandVector(filter->getData().x);
        x += filter_state_expanded * probability;
        x = AVIMMFilterBase::zeroSmallElements(x);
        i++;
    }
    
    Matrix P = Matrix::Zero(m_data.P.rows(), m_data.P.cols());
    i = 0;
    for (const auto &filter : m_filters)
    {
        auto probability = m_mode_probabilities[i];
        const Vector& filter_state_expanded = expandVector(filter->getData().x);
        const Matrix& filter_covariance_expanded = expandCovariance(filter->getData().P);
        const Vector& state_diff = filter_state_expanded - m_data.x;
        P += probability * ((state_diff * state_diff.transpose()) + filter_covariance_expanded);
        P = AVIMMFilterBase::zeroSmallElements(P);
        i++;
    }
    
    imm_state = x;
    imm_covariance = P;
}

//--------------------------------------------------------------------------

void AVIMMEstimator::calculateMixedStates(std::vector<Vector>& mixed_states, std::vector<Matrix>& mixed_covariances)
{
    std::vector<Vector> xs;
    std::vector<Matrix> Ps;
    
    for (int j = 0; j < m_mode_probabilities_matrix.cols(); j++)
    {
        Vector col = m_mode_probabilities_matrix.col(j);
    
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
    mixed_states = xs;
    mixed_covariances = Ps;
}

//--------------------------------------------------------------------------

void AVIMMEstimator::calculateModeProbabilities(Vector& mode_probabilities)
{
    Vector probabilities(m_filters.size(), 1);
    int i = 0;
    auto sum = 0.0;
    for (const auto& filter : m_filters) {
        double probability = m_c[i] * filter->getLikelihood();
        probabilities(i) = probability;
        sum += probability;
        i++;
    }
    probabilities /= sum;
    mode_probabilities = probabilities;
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
    Vector ones_x = Vector::Ones(requested_size, 1);
    for (int i = 0; i < vector_size; i++)
        ones_x(i) = new_x.coeff(i);
    
    ones_x = m_config.expansion_matrix * ones_x;
    return ones_x;
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
    
    Matrix ones_M = AVIMMFilterBase::createUnityMatrix(requested_rows);
    for (int i = 0; i < matrix_rows; i++)
        for (int j = 0; j < matrix_cols; j++)
            ones_M(i,j) = new_M.coeff(i, j);
    new_M = m_config.expansion_matrix * ones_M * m_config.expansion_matrix.transpose();
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
    
    Matrix ones_M = AVIMMFilterBase::createUnityMatrix(requested_rows);
    for (int i = 0; i < matrix_rows; i++)
        for (int j = 0; j < matrix_cols; j++)
            ones_M(i,j) = new_M.coeff(i, j);
    new_M = m_config.expansion_matrix_covariance * ones_M * m_config.expansion_matrix_covariance.transpose();
    return new_M;
}


//--------------------------------------------------------------------------

Vector AVIMMEstimator::shrinkVector(const Vector &x, int dim)
{
    if (dim == REQUESTED_SIZE)
        return x;
    
    return m_config.shrinking_matrix * x;
}

//--------------------------------------------------------------------------

Matrix AVIMMEstimator::shrinkMatrix(const Matrix &M, int dim)
{
    if (dim == REQUESTED_SIZE)
        return M;
    
    return m_config.shrinking_matrix * M * m_config.shrinking_matrix.transpose();
}

//--------------------------------------------------------------------------

void AVIMMEstimator::prepare(bool extrapolate)
{
    // Save data into previous data struct to preserves the previous state and covariance
    if (!extrapolate)
        m_previous_data = m_data;
    
    // Save time of calculation and get time delta since last calculation
    if (!m_test_run)
        m_now = QDateTime::currentDateTimeUtc();
    
    float time_delta = m_last_calculation.msecsTo(m_now) / 1000.0;
    // Calculate all time depended matrices
    for (auto& filter: m_filters)
    {
        const QString filter_key = filter->getFilterKey();
        auto new_data = filter->getData();
        new_data.F = AVIMMConfigParser::singleton().calculateTimeDependentMatrices(m_config.F_map[filter_key], time_delta);
        new_data.P = AVIMMConfigParser::singleton().calculateTimeDependentMatrices(m_config.P_map[filter_key], time_delta);
        new_data.H = AVIMMConfigParser::singleton().calculateTimeDependentMatrices(m_config.H_map[filter_key], time_delta);
        new_data.Q = AVIMMConfigParser::singleton().calculateTimeDependentMatrices(m_config.Q_map[filter_key], time_delta);
        new_data.R = AVIMMConfigParser::singleton().calculateTimeDependentMatrices(m_config.R_map[filter_key], time_delta);
        new_data.B = AVIMMConfigParser::singleton().calculateTimeDependentMatrices(m_config.B_map[filter_key], time_delta);
        
        filter->setData(new_data);
    }
    
    // Save now as las calculation step
    if (!extrapolate)
        m_last_calculation = m_now;
}
