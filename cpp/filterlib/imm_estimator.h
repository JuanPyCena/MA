//
// Created by Felix on 25.05.2020.
//

#ifndef CPP_IMM_ESTIMATOR_H
#define CPP_IMM_ESTIMATOR_H

#include "imm_filter_base.h"
#include "utils/imm_config.h"
#include <vector>

typedef IMMConfig Config;

class IMMEstimator
{
    struct FilterData
    {
        Vector x; // State
        Vector x_prior; // State after prediction
        Vector x_post; // State after update
        Matrix P; // Covariance matrix
        Matrix P_prior; // Covariance matrix after prediction
        Matrix P_post; // Covariance matrix after update
    } m_data, m_previous_data;  // data containter for current calculation and previous calculation
    
    // Vector which holds the probabilities of each filter
    Vector m_mode_probabilities;
    // Matrix which defines the probabilities of changeing the filters. This must not change during execution,
    // the diagonal must be dominant
    Matrix m_markov_transition_matrix;
    // List which holds the required filter types for the IMM
    std::list<FilterType> m_filter_types;
    
    
    // Container to hold the subfilters for the IMM
    std::list<std::unique_ptr<IMMFilterBase>> m_filters;
    
    // Constant useful for probability calculation
    Vector m_c;
    Matrix m_mode_probabilities_matrix;
    std::vector<Vector> m_mixed_states;
    std::vector<Matrix> m_mixed_covariances;

    // imm_config for the IMM, also holds the configuration for the subfilters
    Config m_imm_config;

    // Used in constructor to initialize the subfilters according to the given m_filter_type
    void initializeIMM();
    // Compute the mixing probability for each filter.
    void calculateModeProbabilityMatrix();
    // Computes the IMM's mixed state estimate from each filter using the mode probability to weight the estimates.
    void calculateIMMState();
    // Calculate the mixed states and covariances of the filters
    void calculateMixedStates();
    // Calculate the Probabilities of each Mode/Subfilter
    void calculateModeProbabilities();
    
    // Functions used to expand and shrink subfilter matrices and vectors
    Vector expandVector(const Vector& x);
    Matrix expandMatrix(const Matrix& M);
    Matrix expandCovariance(const Matrix& M);
    Matrix expandInnovation(const Matrix& M);
    Vector shrinkVector(const Vector& x, int dim);
    Matrix shrinkMatrix(const Matrix& M, int dim);
    
public:
    IMMEstimator();
    virtual ~IMMEstimator();
    // This function makes a prediction of each filter using their respective predict function and updates their states
    // and covariances aswell
    void predictAndUpdate(const Vector& z, const Matrix& R=DEFAULT_MATRIX, const Vector& u=DEFAULT_VECTOR);
};

#endif //CPP_IMM_ESTIMATOR_H
