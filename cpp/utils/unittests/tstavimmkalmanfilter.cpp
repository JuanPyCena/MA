//
// Created by felix on 6/2/20.
//

///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON
// QT-Version: QT5
// Copyright:  AviBit data processing GmbH, 2001-2018
//
// Module:     UnitTests
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \brief   Function level test cases for AVIMMKalmanFilter
 */

#include <QObject>
#include <QTest>
#include <avunittest.h>
#include <QApplication>

#include "testhelper/avimmtester.h"

class TstAVIMMKalmanFilter: public QObject
{
Q_OBJECT

public:
    TstAVIMMKalmanFilter() {}

public slots:
    void initTestCase()
    {
        // Set singletons
        std::vector<char*> args;
        QByteArray         dummy_arg("dummy");  // program name, is ignored by config
        args.push_back(dummy_arg.data());
        
        AVEnvironment::setProcessName("imm_tester");
        AVConfig2Global::initializeSingleton(args.size(), args.data(), false, AVEnvironment::APP_ASTOS, "imm_tester");
        AVConfig2Global::singleton().initialize();
        
        AVIMMConfigContainer::setSingleton(new AVIMMConfigContainer());
        AVIMMConfigParser::setSingleton(new AVIMMConfigParser());
    }
    void cleanupTestCase()
    {
        AVIMMConfigParser::deleteSingleton();
        AVIMMConfigContainer::deleteSingleton();
        AVConfig2Global::deleteSingleton();
    };
    void init() {}
    void cleanup() {}

private slots:
    void test_AVIMMKalmanFilter_getFilterInfo();
    void test_AVIMMKalmanFilter_predict();
    void test_AVIMMKalmanFilter_update();
};

//--------------------------------------------------------------------------

void TstAVIMMKalmanFilter::test_AVIMMKalmanFilter_predict()
{
    Vector ini_state(3,1);
    Matrix transitions_matrix(3,3);
    Matrix covariance_matrix(3,3);
    Matrix measurement_matrix(3,3);
    Matrix process_noise(3,3);
    Matrix state_uncertainty(3,3);
    Matrix control_input_matrix(3,3);
    QString filter_key = "Test";
    
    ini_state << 1,2,3;
    transitions_matrix << 2,0,0,0,3,0,0,0,1;
    covariance_matrix << 2,0,0,0,2,0,0,0,2;
    measurement_matrix << 3,0,0,0,3,0,0,0,3;
    process_noise << 4,0,0,0,4,0,0,0,4;
    state_uncertainty << 5,0,0,0,5,0,0,0,5;
    control_input_matrix << 6,0,0,0,6,0,0,0,6;
    
    AVIMMKalmanFilter tester(ini_state, transitions_matrix, covariance_matrix,
                      measurement_matrix, process_noise, state_uncertainty,
                      control_input_matrix, filter_key);
    
    Vector ref_state(3,1);
    ref_state << 2,6,3;
    Matrix ref_cov(3,3);
    ref_cov << 12,0,0,0,22,0,0,0,6;
    
    tester.predict();

    
    QVERIFY(AVIMMTester::getMatricesEqual(tester.getData().x_prior, ref_state).first);
    QVERIFY(AVIMMTester::getMatricesEqual(tester.getData().P_prior, ref_cov).first);
    
    tester.m_data.x = ini_state;
    tester.m_data.x_prior = ini_state;
    tester.m_data.P = covariance_matrix;
    tester.m_data.P_prior = covariance_matrix;
    
    Vector ref_state_input(3,1);
    ref_state_input << 8,6,3;
    
    Vector input(3,1);
    input << 1,0,0;
    
    tester.predict(input);
    
    Matrix ref_cov_input(3,3);
    ref_cov_input << 12,0,0,0,22,0,0,0,6;
    
    QVERIFY(AVIMMTester::getMatricesEqual(tester.getData().x, ini_state).first);
    QVERIFY(AVIMMTester::getMatricesEqual(tester.getData().x_prior, ref_state_input).first);
    QVERIFY(AVIMMTester::getMatricesEqual(tester.getData().P, covariance_matrix).first);
    QVERIFY(AVIMMTester::getMatricesEqual(tester.getData().P_prior, ref_cov_input).first);
}

//--------------------------------------------------------------------------

void TstAVIMMKalmanFilter::test_AVIMMKalmanFilter_update()
{
    Vector ini_state(3,1);
    Matrix transitions_matrix(3,3);
    Matrix covariance_matrix(3,3);
    Matrix measurement_matrix(3,3);
    Matrix process_noise(3,3);
    Matrix state_uncertainty(3,3);
    Matrix control_input_matrix(3,3);
    Matrix R(3,3);
    QString filter_key = "Test";
    
    ini_state << 1,2,3;
    transitions_matrix << 1,0,0,0,1,0,0,0,1;
    covariance_matrix << 2,0,0,0,2,0,0,0,2;
    measurement_matrix << 3,0,0,0,3,0,0,0,3;
    process_noise << 4,0,0,0,4,0,0,0,4;
    state_uncertainty << 5,0,0,0,5,0,0,0,5;
    control_input_matrix << 6,0,0,0,6,0,0,0,6;
    R << 1,0,0,0,1,0,0,0,1;
    
    AVIMMKalmanFilter tester(ini_state, transitions_matrix, covariance_matrix,
                             measurement_matrix, process_noise, state_uncertainty,
                             control_input_matrix, filter_key);
    
    Vector ref(3,1);
    ref << 1.31579,1.36842,1.42105;
    Matrix ref_cov(3,3);
    ref_cov << 0.105263,0,0,0,0.105263,0,0,0,0.105263;
    
    Vector measurement(3,1);
    measurement << 4,4,4;
    
    tester.m_data.x_prior = ini_state;
    tester.m_data.P_prior = covariance_matrix;
    
    tester.update(measurement, R);
    
    QVERIFY(AVIMMTester::getMatricesEqual(tester.getData().x, ref).first);
    QVERIFY(AVIMMTester::getMatricesEqual(tester.getData().x_post, ref).first);
}

//--------------------------------------------------------------------------

void TstAVIMMKalmanFilter::test_AVIMMKalmanFilter_getFilterInfo()
{
    Vector ini_state(3,1);
    Matrix transitions_matrix(3,3);
    Matrix covariance_matrix(3,3);
    Matrix measurement_matrix(3,3);
    Matrix process_noise(3,3);
    Matrix state_uncertainty(3,3);
    Matrix control_input_matrix(3,3);
    QString filter_key = "Test";
    
    ini_state << 1,2,3;
    transitions_matrix << 1,0,0,0,1,0,0,0,1;
    covariance_matrix << 2,0,0,0,2,0,0,0,2;
    measurement_matrix << 3,0,0,0,3,0,0,0,3;
    process_noise << 4,0,0,0,4,0,0,0,4;
    state_uncertainty << 5,0,0,0,5,0,0,0,5;
    control_input_matrix << 6,0,0,0,6,0,0,0,6;
    
    AVIMMKalmanFilter tester(ini_state, transitions_matrix, covariance_matrix,
                             measurement_matrix, process_noise, state_uncertainty,
                             control_input_matrix, filter_key);
    
    QVERIFY(tester.getFilterInfo() == QString("IMM Kalman Filter"));
}

AV_QTEST_MAIN(TstAVIMMKalmanFilter)
#include "tstavimmkalmanfilter.moc"
