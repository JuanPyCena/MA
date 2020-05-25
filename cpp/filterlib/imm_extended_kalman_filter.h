//
// Created by Felix on 25.05.2020.
//

#ifndef CPP_IMM_EXTENDED_KALMAN_FILTER_H
#define CPP_IMM_EXTENDED_KALMAN_FILTER_H

#include "imm_filter_base.h"

class IMMExtendedKalmanFilter : public IMMFilterBase
{
public:
    IMMExtendedKalmanFilter(const Vector &initialState, const Matrix &transitionsMatrix,
                            const Matrix &covarianceMatrix, const Matrix &measurementMatrix,
                            const Matrix &processNoise, const Matrix &stateUncertainty,
                            const Matrix &controlInputMatrix, const Vector (*H_fcnPtr)(const Vector&),
                            const Vector (*HJacobian_fcnPtr)(const Vector&))
                            : m_H_fcnPtr(H_fcnPtr), m_HJacobian_fcnPtr(HJacobian_fcnPtr),
                            IMMFilterBase(initialState,
                                          transitionsMatrix,
                                          covarianceMatrix,
                                          measurementMatrix,
                                          processNoise,
                                          stateUncertainty,
                                          controlInputMatrix) {}
                                            
    virtual ~IMMExtendedKalmanFilter() = default;
    
    // Implementation of the prediction step of the Extended Kalman Filter
    void predict(const Vector& u=DEFAULT_VECTOR) override;
    // Implementation of the update step of the Extended Kalman Filter
    void update(const Vector& z, const Matrix& R=DEFAULT_MATRIX) override;
    // Returns a string giving Information about which Filter is currently used
    std::string getFilterInfo() override { return std::string("IMM Extended Kalman Filter"); }

private:
    // Function pointer for moving state into measurement space
    const Vector (*m_H_fcnPtr)(const Vector&);
    // Function pointer for linearization of the state vector via Jacobian matrix
    const Vector (*m_HJacobian_fcnPtr)(const Vector&);
};


#endif //CPP_IMM_EXTENDED_KALMAN_FILTER_H
