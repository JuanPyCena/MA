//
// Created by Felix on 25.05.2020.
//

#ifndef CPP_IMM_KALMAN_FILTER_H
#define CPP_IMM_KALMAN_FILTER_H

#include "imm_filter_base.h"

class IMMKalmanFilter : public IMMFilterBase
{
public:
    IMMKalmanFilter(const Vector &initialState, const Matrix &transitionsMatrix,
                    const Matrix &covarianceMatrix, const Matrix &measurementMatrix,
                    const Matrix &processNoise, const Matrix &stateUncertainty,
                    const Matrix &controlInputMatrix) : IMMFilterBase(initialState, transitionsMatrix,
                                                                      covarianceMatrix, measurementMatrix,
                                                                      processNoise, stateUncertainty,
                                                                      controlInputMatrix) {}
    
    virtual ~IMMKalmanFilter() = default;
    
    // Implementation of the prediction step of the Kalman Filter
    void predict(const Vector& u=DEFAULT_VECTOR) override;
    // Implementation of the update step of the Kalman Filter
    void update(const Vector& z, const Matrix& R=DEFAULT_MATRIX) override;
    // Returns a string giving Information about which Filter is currently used
    std::string getFilterInfo() override { return std::string("IMM Kalman Filter"); }
};


#endif //CPP_IMM_KALMAN_FILTER_H
