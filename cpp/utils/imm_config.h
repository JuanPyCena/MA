//
// Created by Felix on 26.05.2020.
//

#ifndef CPP_IMM_CONFIG_H
#define CPP_IMM_CONFIG_H
#include "utils/makros.h"
#include "utils/typedefs.h"
#include <list>

class IMMConfig {
public:
    IMMConfig();
    virtual ~IMMConfig() = default;

    DEFINE_ACCESSORS_REF(FilterType, std::list<FilterType>, m_filter_types)
    DEFINE_ACCESSORS_REF(ModeProbabilities, Vector, m_mode_probabilities)
    DEFINE_ACCESSORS_REF(MarkovTransitionMatrix, Matrix, m_markov_transition_matrix)
    DEF_SINGLETON(IMMConfig)

private:
    std::list<FilterType> m_filter_types;
    Vector m_mode_probabilities;
    Matrix m_markov_transition_matrix;
};


#endif //CPP_IMM_CONFIG_H
