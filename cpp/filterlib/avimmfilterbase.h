//
// Created by Felix on 25.05.2020.
//

#ifndef AVIMM_FILTER_BASE_H
#define AVIMM_FILTER_BASE_H

#include "utils/avimmmakros.h"
#include "utils/avimmtypedefs.h"
#include "utils/avimmconfig.h"
#define M_PI 3.14159265358979323846  /* pi needs to be defined manually since VS compiler somehow gets rid of the M_PI constant of cmath*/
#define REQUESTED_SIZE  6
#define MIN_THRESHOLD 1*exp(-6)

typedef AVIMMStaticConfigContainer Config;

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
class AVIMMFilterBase
{
friend class TstAVIMMFilterBase;
friend class TstAVIMMEstimator;
public:
    AVIMMFilterBase(const Vector &initial_state, const Matrix &transitions_matrix, const Matrix &covariance_matrix,
                           const Matrix &measurement_matrix, const Matrix &process_noise, const Matrix &state_uncertainty,
                           const Matrix &control_input_matrix, const QString& filter_key)
     {
        m_data.x       = initial_state;
        m_data.x_prior = DEFAULT_VECTOR;
        m_data.x_post  = DEFAULT_VECTOR;
        m_data.F       = transitions_matrix;
        m_data.P       = covariance_matrix;
        m_data.P_prior = DEFAULT_MATRIX;
        m_data.P_post  = DEFAULT_MATRIX;
        m_data.H       = measurement_matrix;
        m_data.Q       = process_noise;
        m_data.R       = state_uncertainty;
        m_data.B       = control_input_matrix;
        m_data.S       = DEFAULT_MATRIX;
        m_data.error   = DEFAULT_VECTOR;
        m_filter_key   = filter_key;
        
        // Initialize previous data with current data, avoid having issue in starting phase
        m_previous_data = m_data;
     };
    
    virtual ~AVIMMFilterBase() = default;

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
    
    QString m_filter_key;
    
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
        
        Matrix ones_M = createUnityMatrix(requested_rows);
        for (int i = 0; i < matrix_rows; i++)
            for (int j = 0; j < matrix_cols; j++)
                ones_M(i,j) = new_M.coeff(i, j);
        new_M = Config::singleton().expansion_matrix_innovation * ones_M * Config::singleton().expansion_matrix_innovation.transpose();
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
        Vector ones_x = Vector::Ones(requested_size, 1);
        for (int i = 0; i < vector_size; i++)
            ones_x(i) = new_x.coeff(i);
    
        ones_x = Config::singleton().expansion_matrix * ones_x;
        return ones_x;
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
    virtual QString getFilterInfo() = 0;
    // Function which gives the filter key used to read from the config
    const QString& getFilterKey() { return m_filter_key; }
    
    //--------------------------------------------------------------------------
    
    // Likelihood functions which should be the same for each filter
    double getLogLikelihood() const {
        const Vector mean  = Vector::Zero(m_data.error.size(), 1);
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
    
    //--------------------------------------------------------------------------
    
    // This has to be done manually since we don't know the size of the matrices at compile time
    static Matrix createUnityMatrix(int size)
    {
        Matrix m(size, size);
        for (int i = 0; i < size; i++)
            for (int j = 0; j < size; j++)
            {
                if (i == j)
                    m(i,j) = 1.0;
                else
                    m(i,j) = 0.0;
            }
        return m;
    }
    
    //--------------------------------------------------------------------------
    
    static Matrix zeroSmallElements(const Matrix &M)
    {
        // Find all elements below a certain threshold and set them to zero to gain numerical stability
        Matrix new_M(M.rows(), M.cols());
        for (int i = 0; i < M.size(); i++) {
            if (M(i) <= MIN_THRESHOLD) {
                new_M(i) = 0.0;
            } else
            {
                new_M(i) = M(i);
            }
        }
        return new_M;
    }
    
    //--------------------------------------------------------------------------
    
    static Vector zeroSmallElements(const Vector &M)
    {
        // Find all elements below a certain threshold and set them to zero to gain numerical stability
        Vector new_M(M.rows(), M.cols());
        for (int i = 0; i < M.size(); i++) {
            if (M(i) <= MIN_THRESHOLD) {
                new_M(i) = 0.0;
            }
            {
                new_M(i) = M(i);
            }
        }
        return new_M;
    }
};

#endif //AVIMM_FILTER_BASE_H
