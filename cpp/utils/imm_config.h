//
// Created by Felix on 26.05.2020.
//

#ifndef CPP_IMM_CONFIG_H
#define CPP_IMM_CONFIG_H
#include "utils/makros.h"
#include "utils/typedefs.h"
#include <map>

// Include libconfig to read config
#include <libconfig/lib/libconfig.h++>

class SubFilterConfig
{
public:
    Matrix m_transition_matrix;
    Matrix m_covariance_matrix;
    Matrix m_measurement_control_matrix;
    Matrix m_input_control_matrix;
    Matrix m_process_noise_matrix;
    Matrix m_measurement_uncertainty_matrix;
};

class KFConfig : public SubFilterConfig
{
public:
    const std::string m_filter_cfg_info = "KFConfig";
};

class EKFConfig : public SubFilterConfig
{
public:
    const std::string m_filter_cfg_info = "EKFConfig";
    Matrix m_jacobi_matrix;
};

typedef std::map<FilterType, SubFilterConfig> FilterDict;

class IMMConfig {
public:
    IMMConfig();
    ~IMMConfig() {};
    Matrix createUnityMatrix(int size);
    
    DEFINE_ACCESSORS_REF(ConfigFile, std::string, m_config_file)
    DEFINE_GET(FilterType, std::list<FilterType>, m_filter_types)
    DEFINE_GET(FilterDefinitions, std::list<std::string>, m_filter_definitions)
    DEFINE_GET(StateDefinition, std::list<std::string>, m_state_definition)
    DEFINE_GET(ModeProbabilities, Vector, m_mode_probabilities)
    DEFINE_GET(MarkovTransitionMatrix, Matrix, m_markov_transition_matrix)
    DEFINE_GET(ExpansionMatrix, Matrix, m_expansion_matrix)
    DEFINE_GET(ExpansionMatrixCovariance, Matrix, m_expansion_matrix_covariance)
    DEFINE_GET(ExpansionMatrixInnovation, Matrix, m_expansion_matrix_innovation)
    DEFINE_GET(ShrinkingMatrix, Matrix, m_shrinking_matrix)
    DEFINE_GET(FilterConfigs, FilterDict, m_filters)
    DEF_SINGLETON(IMMConfig)

private:
    std::string m_config_file;
    libconfig::Config m_cfg;
    std::map<std::string, FilterType> m_enum_map;
    std::list<FilterType> m_filter_types;
    std::list<std::string> m_filter_definitions;
    std::list<std::string> m_state_definition;
    Vector m_mode_probabilities;
    Matrix m_markov_transition_matrix;
    Matrix m_expansion_matrix;
    Matrix m_expansion_matrix_covariance;
    Matrix m_expansion_matrix_innovation;
    Matrix m_shrinking_matrix;
    
    FilterDict m_filters;
    
    void readIMM();
    void readSubFilterConfigs();
    void readKalmanFilter();
    void readExtendedKalmanFilter();
};
#endif //CPP_IMM_CONFIG_H
