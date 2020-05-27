//
// Created by Felix on 26.05.2020.
//

#ifndef CPP_IMM_CONFIG_H
#define CPP_IMM_CONFIG_H
#include "utils/makros.h"
#include "utils/typedefs.h"
#include <map>
#include <list>

struct SubFilterConfig
{
    Matrix m_transition_matrix;
    Matrix m_covariance_matrix;
    Matrix m_measurement_control_matrix;
    Matrix m_input_control_matrix;
    Matrix m_process_noise_matrix;
    Matrix m_measurement_uncertainty_matrix;
};

struct KFConfig : SubFilterConfig
{
    const std::string m_filter_cfg_info = "KFConfig";
};

struct EKFConfig : SubFilterConfig
{
    const std::string m_filter_cfg_info = "EKFConfig";
    Matrix m_jacobi_matrix;
};

typedef std::map<FilterType, SubFilterConfig> FilterDict;

class IMMConfig {
public:
    IMMConfig();
    virtual ~IMMConfig() = default;
    Matrix createUnityMatrix(int size);
    
    DEFINE_GET(FilterType, std::list<FilterType>, m_filter_types)
    DEFINE_GET(ModeProbabilities, Vector, m_mode_probabilities)
    DEFINE_GET(MarkovTransitionMatrix, Matrix, m_markov_transition_matrix)
    DEFINE_GET(ExpansionMatrix, Matrix, m_expansion_matrix)
    DEFINE_GET(ExpansionMatrixCovariance, Matrix, m_expansion_matrix_covariance)
    DEFINE_GET(ExpansionMatrixInnovation, Matrix, m_expansion_matrix_innovation)
    DEFINE_GET(ShrinkingMatrix, Matrix, m_shrinking_matrix)
    DEFINE_GET(FilterConfigs, FilterDict, m_filters)
    DEF_SINGLETON(IMMConfig)

private:
    std::list<FilterType> m_filter_types;
    Vector m_mode_probabilities;
    Matrix m_markov_transition_matrix;
    Matrix m_expansion_matrix;
    Matrix m_expansion_matrix_covariance;
    Matrix m_expansion_matrix_innovation;
    Matrix m_shrinking_matrix;
    
    FilterDict m_filters;
    
    void readKalmanFilter();
    void readExtendedKalmanFilter();
};
#endif //CPP_IMM_CONFIG_H
