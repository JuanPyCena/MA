//
// Created by Felix on 25.05.2020.
//

#ifndef CPP_IMM_FILTER_BASE_H
#define CPP_IMM_FILTER_BASE_H

#include "utils/makros.h"
#include "utils/typedefs.h"
#include "utils/imm_config.h"
#define M_PI 3.14159265358979323846  /* pi needs to be defined manually since VS compiler somehow gets rid of the M_PI constant of cmath*/


typedef IMMConfig Config;

class Mvn {
// Based on the implementation in: http://blog.sarantop.com/notes/mvn
public:
    Mvn(const Vector &mu, const Matrix &s): m_mean(mu), m_sigma(s) {}
    ~Mvn() = default;
    
    const Vector& m_mean;
    const Matrix& m_sigma;
    
    double pdf(const Vector &x) const {
        double n = x.rows();
        double sqrt2pi = std::sqrt(2 * M_PI);
        double quadform = (x - m_mean).transpose() * m_sigma.inverse() * (x - m_mean);
        double norm = std::pow(sqrt2pi, -n) *
                      std::pow(m_sigma.determinant(), -0.5);
        
        return norm * exp(-0.5 * quadform);
    }
    
    double logpdf(const Vector &x) const { return log(pdf(x)); }
};

//--------------------------------------------------------------------------
class IMMEstimator;
class IMMFilterBase
{
public:
    explicit IMMFilterBase(const Vector &initial_state, const Matrix &transitions_matrix, const Matrix &covariance_matrix,
                           const Matrix &measurement_matrix, const Matrix &process_noise, const Matrix &state_uncertainty,
                           const Matrix &control_input_matrix)
     {
        m_data.x                = initial_state;
        m_data.x_prior          = DEFAULT_VECTOR;
        m_data.x_post           = DEFAULT_VECTOR;
        m_data.F                = transitions_matrix;
        m_data.P                = covariance_matrix;
        m_data.P_prior          = DEFAULT_MATRIX;
        m_data.P_post           = DEFAULT_MATRIX;
        m_data.H                = measurement_matrix;
        m_data.Q                = process_noise;
        m_data.R                = state_uncertainty;
        m_data.B                = control_input_matrix;
        m_data.S                = DEFAULT_MATRIX;
        m_data.error            = DEFAULT_VECTOR;
        
        // Initialize previous data with current data, avoid having issue in starting phase
        m_previous_data = m_data;
     };
    
    ~IMMFilterBase() = default;

protected:
    
    struct FilterData
    {
        Vector x; // State
        Vector x_prior; // State after prediction
        Vector x_post; // State after update
        Matrix F; // Transition matrix
        Matrix P; // Covariance matrix
        Matrix P_prior; // Covariance matrix after prediction
        Matrix P_post; // Covariance matrix after update
        Matrix Q; // Process noise matrix
        Matrix R; // Measurement uncertainty matrix
        Matrix H; // Measurement control matrix
        Matrix B; // Input control matrix
        Matrix S; // Innovation matrix, used for calculation of likelihood
        Vector error; // Error of the prediction, , used for calculation of likelihood
    } m_data, m_previous_data;  // data containter for current calculation and previous calculation
    
    //--------------------------------------------------------------------------
    
    // This has to be done manually since we don't know the size of the matrices at compile time
    static Matrix createUnityMatrix(int size)
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
    
    //--------------------------------------------------------------------------
    
    Matrix expandInnovation(const Matrix &M)
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
        new_M = Config::instance().getExpansionMatrixInnovation() * ones_M * Config::instance().getExpansionMatrixInnovation().transpose();
        return new_M;
    }
    
    //--------------------------------------------------------------------------
    
    Vector expandErrorVector(const Vector &x)
    {
        Vector new_x = x;
        auto requested_size = REQUESTED_SIZE;
        auto vector_size = x.size();
        if (vector_size == requested_size)
            return new_x;
        
        // Fill with one until the requested size has been reached.
        for (int i = vector_size; i < requested_size; i++)
            new_x << 1.0;
        
        new_x = Config::instance().getExpansionMatrix() * new_x;
        return new_x;
    }

public:
    // Accessors
    DEFINE_ACCESSORS_VAL(Data, FilterData, m_data)
    DEFINE_ACCESSORS_VAL(PreviousData, FilterData, m_previous_data)
    Vector expandVector(const Vector& x) { return expandErrorVector(x); }
    Matrix expandMatrix(const Matrix& M) { return expandInnovation(M); }
    
    // Pure  virtual functions for filter prediction and update
    virtual void predict(const Vector& u=DEFAULT_VECTOR) = 0;
    virtual void update(const Vector& z, const Matrix& R=DEFAULT_MATRIX) = 0;
    
    // Pure virtual function which gives a general information about the Filter, useful for logging
    // Returns a string giving Information about which Filter is currently used
    virtual std::string getFilterInfo() = 0;
    
    //--------------------------------------------------------------------------
    
    // Likelihood functions which should be the same for each filter
    double getLogLikelihood() const {
        const Vector mean  = DEFAULT_VECTOR;
        const Matrix sigma = m_data.S;
        const Vector y     = m_data.error;
        Mvn mvn(mean, sigma);
        return mvn.logpdf(y);
    }
    
    //--------------------------------------------------------------------------
    
    double getLikelihood() const {
       double likelihood = exp(getLogLikelihood());
       if (likelihood <= 1*exp(-18))
       {
           // Use e-18 if likelihood is too small to avoid numerical errors, No filter must have excactly 0 probability
           // since this state is no longer recoverable
           likelihood = 1*exp(-18);
       }
       return likelihood;
    }
};

#endif //CPP_IMM_FILTER_BASE_H
