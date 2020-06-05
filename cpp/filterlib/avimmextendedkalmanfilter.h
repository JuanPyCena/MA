//
// Created by Felix on 25.05.2020.
//

#ifndef CPP_IMM_EXTENDED_KALMAN_FILTER_H
#define CPP_IMM_EXTENDED_KALMAN_FILTER_H

#include "avimmfilterbase.h"
class AVIMMExtendedKalmanFilter : public AVIMMFilterBase
{
public:
    AVIMMExtendedKalmanFilter(const Vector &initial_state, const Matrix &transitions_matrix,
                                     const Matrix &covariance_matrix, const Matrix &measurement_matrix,
                                     const Matrix &process_noise, const Matrix &state_uncertainty,
                                     const Matrix &control_input_matrix, const Matrix &jacobi_matrix,
                                     const QString& filter_key)
                                     : AVIMMFilterBase(initial_state,
                                                     transitions_matrix,
                                                     covariance_matrix,
                                                     measurement_matrix,
                                                     process_noise,
                                                     state_uncertainty,
                                                     control_input_matrix,
                                                     filter_key), m_jacobi_matrix(jacobi_matrix) {}
    virtual ~AVIMMExtendedKalmanFilter() {};
    
    // Implementation of the prediction step of the Extended Kalman Filter
    void predict(const Vector& u=DEFAULT_VECTOR) override;
    // Implementation of the update step of the Extended Kalman Filter
    void update(const Vector& z, const Matrix& R=DEFAULT_MATRIX) override;
    // Returns a string giving Information about which Filter is currently used
    QString getFilterInfo() override { return QString("IMM Extended Kalman Filter"); }
    
private:
    const Matrix& m_jacobi_matrix;
    
    // Use a function pointer since we need to move the state into the measurement space from external
    Vector Hx(const Vector& x) { return x; }// Todo: For now assume we are in the same space}
    // Use a function pointer since we need to linearize the state via the jacobi matrix
    Vector HJacobian(const Vector& x) { return m_jacobi_matrix * x; } // Todo: For now multiply with jacobi matrix
};


#endif //CPP_IMM_EXTENDED_KALMAN_FILTER_H
