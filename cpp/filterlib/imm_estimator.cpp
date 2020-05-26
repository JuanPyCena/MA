//
// Created by Felix on 25.05.2020.
//

#include "imm_estimator.h"

IMMEstimator::IMMEstimator()
{
    m_imm_config               = Config::instance();
    m_mode_probabilities       = m_imm_config.getModeProbabilities();
    m_markov_transition_matrix = m_imm_config.getMarkovTransitionMatrix();
    m_filter_types             = m_imm_config.getFilterType();

    // Initialize IMM Subfilters
    initializeIMM();
}


IMMEstimator::~IMMEstimator()
{
    cleanup_unique_ptr(m_filters);
}

void IMMEstimator::initializeIMM()
{
}