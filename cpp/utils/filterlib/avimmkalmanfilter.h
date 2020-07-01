//
// Created by Felix on 25.05.2020.
//

#ifndef AVIMM_KALMAN_FILTER_H
#define AVIMM_KALMAN_FILTER_H

#include "avimmfilterbase.h"

class AVIMMKalmanFilter : public AVIMMFilterBase
{
public:
    AVIMMKalmanFilter(const Vector &initialState, const Matrix &transitionsMatrix,
                             const Matrix &covarianceMatrix, const Matrix &measurementMatrix,
                             const Matrix &processNoise, const Matrix &stateUncertainty,
                             const Matrix &controlInputMatrix, const QString& filter_key)
                             : AVIMMFilterBase(initialState,
                                             transitionsMatrix,
                                             covarianceMatrix,
                                             measurementMatrix,
                                             processNoise,
                                             stateUncertainty,
                                             controlInputMatrix,
                                             filter_key) {}
    
    virtual ~AVIMMKalmanFilter() {};
    
    // Implementation of the prediction step of the Kalman Filter
    void predict(const Vector& u=DEFAULT_VECTOR) override;
    // Implementation of the update step of the Kalman Filter
    void update(const Vector& z, const Matrix& R=DEFAULT_MATRIX) override;
    // Returns a string giving Information about which Filter is currently used
    QString getFilterInfo() override { return QString("IMM Kalman Filter"); }
    
    friend class TstAVIMMKalmanFilter;
};


#endif //AVIMM_KALMAN_FILTER_H
