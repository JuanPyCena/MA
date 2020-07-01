//
// Created by Felix on 25.05.2020.
//

#include "avimmextendedkalmanfilter.h"

void AVIMMExtendedKalmanFilter::predict(const Vector &u)
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
    if (&u!=&DEFAULT_VECTOR)
        x_prior = F*x + B*u;
    else
        x_prior = F*x;
    
    // P = FPF' + Q
    P_prior = F*P*F.transpose() + Q;
    
    x_prior = zeroSmallElements(x_prior);
    P_prior = zeroSmallElements(P_prior);
    
    // Save calculations in prior to preserve the previous state
    m_data.x_prior = x_prior;
    m_data.P_prior = P_prior;
}

//--------------------------------------------------------------------------

void AVIMMExtendedKalmanFilter::update(const Vector &z, const Matrix &R)
{
    const Matrix& I = createUnityMatrix(z.size());
    const Matrix& P = m_data.P_prior;
    const Vector& x = m_data.x_prior;
    const Matrix& H = HJacobian(x);
    Matrix& P_post  = m_data.P_post;
    Vector& x_post  = m_data.x_post;
    
    // y = z - Hx
    Vector y = z - Hx(x);
    y = zeroSmallElements(y);
    // S = HPH' + R
    Matrix S = H*P*H.transpose() + R;
    S = zeroSmallElements(S);
    // K = PH'inv(S)
    Matrix K = (P*H.transpose())*S.inverse();
    K = zeroSmallElements(K);
    
    // x = x + Ky
    x_post = x + K*y;
    x_post = zeroSmallElements(x_post);
    // P = (I-KH)P(I-KH)' + KRK'
    P_post = (I - K*H) * P * (I - K*H).transpose() + K*R*K.transpose();
    P_post = zeroSmallElements(P_post);
    
    
    // Save results, error and innovation
    m_data.x      = x_post;
    m_data.x_post = x_post;
    m_data.P      = P_post;
    m_data.P_post = P_post;
    m_data.R      = R;
    m_data.error  = expandVector(y);
    m_data.S      = expandMatrix(S);
}
