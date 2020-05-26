//
// Created by Felix on 25.05.2020.
//

#ifndef CPP_IMM_ESTIMATOR_H
#define CPP_IMM_ESTIMATOR_H

#include "imm_filter_base.h"
#include "utils/imm_config.h"

typedef IMMConfig Config;

class IMMEstimator{
public:
    IMMEstimator();
    virtual ~IMMEstimator();

private:
    // Vector which holds the probabilities of each filter
    Vector m_mode_probabilities;
    // Matrix which defines the probabilities of changeing the filters. This must not change during execution,
    // the diagonal must be dominant
    Matrix m_markov_transition_matrix;
    // List which holds the required filter types for the IMM
    std::list<FilterType> m_filter_types;
    // Container to hold the subfilters for the IMM
    std::list<std::unique_ptr<IMMFilterBase>> m_filters;

    // imm_config for the IMM, also holds the configuration for the subfilters
    Config m_imm_config;

    // Used in constructor to initialize the subfilters according to the given m_filter_type
    void initializeIMM();
};


#endif //CPP_IMM_ESTIMATOR_H
