//
// Created by Felix on 26.05.2020.
//

#include "imm_config.h"

IMMConfig::IMMConfig() {
    m_filter_types             = std::list<FilterType>();
    m_mode_probabilities       = DEFAULT_VECTOR;
    m_markov_transition_matrix = DEFAULT_MATRIX;
}
