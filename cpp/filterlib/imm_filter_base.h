//
// Created by Felix on 25.05.2020.
//

#ifndef CPP_IMM_FILTER_BASE_H
#define CPP_IMM_FILTER_BASE_H

#include "utils/makros.h"

#include <string>
#include <math.h>
#include <Eigen/Dense>

class Mvn {
// Based on the implementation in: http://blog.sarantop.com/notes/mvn
public:
    Mvn(const Eigen::VectorXd &mu, const Eigen::MatrixXd &s);
    
    ~Mvn();
    
    Eigen::VectorXd m_mean;
    Eigen::MatrixXd m_sigma;
    
    double pdf(const Eigen::VectorXd &x) const {
        double n = x.rows();
        double sqrt2pi = std::sqrt(2 * M_PI);
        double quadform = (x - m_mean).transpose() * m_sigma.inverse() * (x - m_mean);
        double norm = std::pow(sqrt2pi, -n) *
                      std::pow(m_sigma.determinant(), -0.5);
        
        return norm * exp(-0.5 * quadform);
    }
    
    double logpdf(const Eigen::VectorXd &x) const { return log(pdf(x)); }
};

class IMMFilterBase
{
public:
    IMMFilterBase(Eigen::VectorXd initial_state, Eigen::MatrixXd transitions_matrix, Eigen::MatrixXd covariance_matrix, Eigen::MatrixXd measurement_matrix,
                  Eigen::MatrixXd process_noise, Eigen::MatrixXd state_uncertainty, Eigen::MatrixXd control_input_matrix)
     {
        m_data.x = initial_state;
        m_data.F = transitions_matrix;
        m_data.P = covariance_matrix;
        m_data.H = measurement_matrix;
        m_data.Q = process_noise;
        m_data.R = state_uncertainty;
        m_data.B = control_input_matrix;
        m_data.S = Eigen::MatrixXd();
        m_data.error = Eigen::VectorXd();
        // Initialize previous data with current data, avoid having issue in starting phase
        m_previous_data = m_data;
     };
    
    ~IMMFilterBase() {};

protected:
    
    struct FilterData
    {
        Eigen::VectorXd x;
        Eigen::MatrixXd F;
        Eigen::MatrixXd P;
        Eigen::MatrixXd Q;
        Eigen::MatrixXd R;
        Eigen::MatrixXd H;
        Eigen::MatrixXd B;
        Eigen::MatrixXd S;
        Eigen::MatrixXd error;
    } m_data, m_previous_data;

public:
    // Accessors
    DEFINE_ACCESSORS_REF(Data, FilterData, m_data)
    DEFINE_ACCESSORS_REF(PreviousData, FilterData, m_previous_data)
    
    // Pure  virtual functions for filter prediction and update
    virtual void predict() = 0;
    virtual void update(Eigen::VectorXd z, Eigen::MatrixXd R) = 0;
    
    // Pure virtual function which gives a general information about the Filter, useful for logging
    virtual std::string getFilterInfo() = 0;

    // Likelihood functions which should be the same for each filter
    double getLogLikelihood() {
        const Eigen::VectorXd mean = Eigen::VectorXd();
        const Eigen::MatrixXd sigma = m_data.S;
        const Eigen::VectorXd y = m_data.error;
        Mvn mvn(mean, sigma);
        return mvn.logpdf(y);
    }
    
    double getLikelihood() {
       double likelihood = exp(getLogLikelihood());
       if (likelihood == 0)
       {
           // Use e-18 if likelihood is 0 to avoid numerical errors, No filter must have excactly 0 probability
           // since the state is no longer recoverable than
           likelihood = 1*exp(-18);
       }
       return likelihood;
    }
    
    // Override "->" operator to directly acces the current filter data, quality of life accessor
    FilterData &operator->() { return m_data; }
};

#endif //CPP_IMM_FILTER_BASE_H
