//
// Created by Felix on 25.05.2020.
//

#include "imm_extended_kalman_filter.h"

void IMMExtendedKalmanFilter::predict(const Vector &u)
{
    // Save previous data
    m_previous_data = m_data;
    const Matrix& F = m_data.F;
    const Matrix& B = m_data.B;
    const Matrix& Q = m_data.Q;
    const Matrix& P = m_data.P;
    const Vector x  = m_data.x;
    Matrix& P_prior = m_data.P_prior;
    Vector& x_prior = m_data.x_prior;
    
    // x = Fx + Bu
    if (u!=DEFAULT_VECTOR)
        x_prior = F*x + B*u;
    else
        x_prior = F*x;
    
    // P = FPF' + Q
    P_prior = F*P*F.transpose() + Q;
    
    // Save calculations in prior to preserve the previous state
    m_data.x_prior = x_prior;
    m_data.P_prior = P_prior;
}

//--------------------------------------------------------------------------

void IMMExtendedKalmanFilter::update(const Vector &z, const Matrix &R)
{
    const Matrix& I = createUnityMatrix(z.size());
    const Matrix& P = m_data.P_prior;
    const Vector& x = m_data.x_prior;
    const Vector& H = HJacobian(x);
    Matrix& P_post  = m_data.P_post;
    Vector& x_post  = m_data.x_post;
    
    if (R == DEFAULT_MATRIX) {
        const Matrix& R = m_data.R;
    }
    
    // y = z - Hx
    const Vector& y = z - Hx(x);
    // S = HPH' + R
    const Matrix& S = H*P*H.transpose() + R;
    // K = PH'inv(S)
    const Matrix& K = (P*H.transpose())*S.inverse();
    
    // x = x + Ky
    x_post = x + K*y;
    // P = (I-KH)P(I-KH)' + KRK'
    P_post = (I - K*H) * P * (I - K*H).transpose() + K*R*K.transpose();
    
    
    // Save results, error and innovation
    m_data.x      = x_post;
    m_data.x_post = x_post;
    m_data.P      = P_post;
    m_data.P_post = P_post;
    m_data.error  = expandVector(y);
    m_data.S      = expandMatrix(S);
}