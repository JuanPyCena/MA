//
// Created by Felix on 26.05.2020.
//

#include "imm_config.h"

IMMConfig::IMMConfig()
{
    m_config_file                      = std::string("D:\\programming\\clion\\MasterarbeitCPP\\MA\\cpp\\config\\imm_config2_models.cfg");
    m_enum_map["KalmanFilter"]         = KalmanFilter;
    m_enum_map["ExtendedKalmanFilter"] = ExtendedKalmanFilter;
    m_filter_types                     = std::list<FilterType>();
    m_filters                          = std::map<FilterType, SubFilterConfig>();
    m_mode_probabilities               = DEFAULT_VECTOR;
    m_markov_transition_matrix         = DEFAULT_MATRIX;
    
    m_cfg.readString(m_config_file);
    
    readIMM();
}

void IMMConfig::readIMM()
{
    const libconfig::Setting& imm_section = m_cfg.getRoot()["IMM"];

    // Get general singular values
    float a, b, sigma;
    imm_section.lookupValue("a", a);
    imm_section.lookupValue("b", b);
    imm_section.lookupValue("sigma_a_sq", sigma);

    const libconfig::Setting &state_vector_settings = imm_section.lookup("state_vector");
    for (auto& vector_elem : state_vector_settings)
    {
        m_state_definition.push_back(vector_elem);
    }

}

void IMMConfig::readSubFilterConfigs()
{
    std::list<std::string>::const_iterator it = m_filter_definitions.cbegin();
    for (const auto& filter_type : m_filter_types)
    {
        std::string filter_definition = *it;
        switch(filter_type) {
            case KalmanFilter:
                readKalmanFilter();
                break;
            case ExtendedKalmanFilter:
                readExtendedKalmanFilter();
                break;
            default:
                assert(("Invalid Filtertype!", false));
        }
        it = std::next(it);
    }
}

void IMMConfig::readKalmanFilter()
{
    Matrix P = createUnityMatrix(6);
    Matrix H = createUnityMatrix(6);
    Matrix F = createUnityMatrix(6);
    Matrix B = createUnityMatrix(6);
    Matrix R = createUnityMatrix(6);
    Matrix Q = createUnityMatrix(6);
    
    KFConfig kalman_config = KFConfig();
    kalman_config.m_covariance_matrix              = P;
    kalman_config.m_measurement_control_matrix     = H;
    kalman_config.m_transition_matrix              = F;
    kalman_config.m_input_control_matrix           = B;
    kalman_config.m_measurement_uncertainty_matrix = R;
    kalman_config.m_process_noise_matrix           = Q;
    
    m_filters[KalmanFilter] = kalman_config;
}

void IMMConfig::readExtendedKalmanFilter()
{
    Matrix P = createUnityMatrix(6);
    Matrix H = createUnityMatrix(6);
    Matrix F = createUnityMatrix(6);
    Matrix B = createUnityMatrix(6);
    Matrix R = createUnityMatrix(6);
    Matrix Q = createUnityMatrix(6);
    Matrix J = createUnityMatrix(6);
    
    EKFConfig extended_kalman_config = EKFConfig();
    extended_kalman_config.m_covariance_matrix              = P;
    extended_kalman_config.m_measurement_control_matrix     = H;
    extended_kalman_config.m_transition_matrix              = F;
    extended_kalman_config.m_input_control_matrix           = B;
    extended_kalman_config.m_measurement_uncertainty_matrix = R;
    extended_kalman_config.m_process_noise_matrix           = Q;
    extended_kalman_config.m_jacobi_matrix                  = J;
    
    m_filters[ExtendedKalmanFilter] = extended_kalman_config;
}

// This has to be done manually since we don't know the size of the matrices at compile time
Matrix IMMConfig::createUnityMatrix(int size)
{
    Matrix m(size, size);
    for (int i = 0; i < size; i++)
        for (int j = 0; j < size; j++)
        {
            if (i == j)
                m << 1.0;
            else
                m << 0.0;
        }
    return m;
}