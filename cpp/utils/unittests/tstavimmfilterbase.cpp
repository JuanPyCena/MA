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
    \brief   Function level test cases for AVIMMFilterBase
 */

#include <QObject>
#include <QTest>
#include <avunittest.h>
#include <QApplication>

#include "testhelper/avimmtester.h"

class TstAVIMMFilterBase: public QObject
{
Q_OBJECT

public:
    TstAVIMMFilterBase() {}

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
    void test_AVIMMFilterBase_constructor();
    void test_AVIMMFilterBase_expandInnovation();
    void test_AVIMMFilterBase_expandErrorVector();
    void test_AVIMMFilterBase_expandVector();
    void test_AVIMMFilterBase_expandMatrix();
    void test_AVIMMFilterBase_predict();
    void test_AVIMMFilterBase_update();
    void test_AVIMMFilterBase_getFilterInfo();
    void test_AVIMMFilterBase_getFilterKey();
    void test_AVIMMFilterBase_getLogLikelihood();
    void test_AVIMMFilterBase_getLikelihood();
    void test_AVIMMFilterBase_createUnityMatrix();
    void test_AVIMMFilterBase_zeroSmallElements();
};

class FilterMock : public AVIMMFilterBase
{
public:
    FilterMock(const Vector &initialState, const Matrix &transitionsMatrix,
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
    
    
    virtual ~FilterMock() {};
    
    // Implementation of the prediction step of the Kalman Filter
    void predict(const Vector& u=DEFAULT_VECTOR) override { Q_UNUSED(u); Vector state(3,1); state << 3,3,3; m_data.x = state;}
    // Implementation of the update step of the Kalman Filter
    void update(const Vector& z, const Matrix& R=DEFAULT_MATRIX) { Q_UNUSED(z); Q_UNUSED(R); Vector state(3,1); state << 4,4,4; m_data.x = state;}
    // Returns a string giving Information about which Filter is currently used
    QString getFilterInfo() override { return QString("Filter Mock"); }
};
//--------------------------------------------------------------------------

void TstAVIMMFilterBase::test_AVIMMFilterBase_constructor()
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
    
    FilterMock tester(ini_state, transitions_matrix, covariance_matrix,
                      measurement_matrix, process_noise, state_uncertainty,
                      control_input_matrix, filter_key);
    
    auto data          = tester.getData();
    auto previous_data = tester.getPreviousData();
    
    QVERIFY(AVIMMTester::getMatricesEqual(ini_state, data.x).first);
    QVERIFY(AVIMMTester::getMatricesEqual(ini_state, data.x).second == 0.0);
    QVERIFY(AVIMMTester::getMatricesEqual(transitions_matrix, data.F).first);
    QVERIFY(AVIMMTester::getMatricesEqual(transitions_matrix, data.F).second == 0.0);
    QVERIFY(AVIMMTester::getMatricesEqual(covariance_matrix, data.P).first);
    QVERIFY(AVIMMTester::getMatricesEqual(covariance_matrix, data.P).second == 0.0);
    QVERIFY(AVIMMTester::getMatricesEqual(measurement_matrix, data.H).first);
    QVERIFY(AVIMMTester::getMatricesEqual(measurement_matrix, data.H).second == 0.0);
    QVERIFY(AVIMMTester::getMatricesEqual(process_noise, data.Q).first);
    QVERIFY(AVIMMTester::getMatricesEqual(process_noise, data.Q).second == 0.0);
    QVERIFY(AVIMMTester::getMatricesEqual(state_uncertainty, data.R).first);
    QVERIFY(AVIMMTester::getMatricesEqual(state_uncertainty, data.R).second == 0.0);
    QVERIFY(AVIMMTester::getMatricesEqual(control_input_matrix, data.B).first);
    QVERIFY(AVIMMTester::getMatricesEqual(control_input_matrix, data.B).second == 0.0);
    QVERIFY(AVIMMTester::getMatricesEqual(ini_state, previous_data.x).first);
    QVERIFY(AVIMMTester::getMatricesEqual(ini_state, previous_data.x).second == 0.0);
    QVERIFY(AVIMMTester::getMatricesEqual(transitions_matrix, previous_data.F).first);
    QVERIFY(AVIMMTester::getMatricesEqual(transitions_matrix, previous_data.F).second == 0.0);
    QVERIFY(AVIMMTester::getMatricesEqual(covariance_matrix, previous_data.P).first);
    QVERIFY(AVIMMTester::getMatricesEqual(covariance_matrix, previous_data.P).second == 0.0);
    QVERIFY(AVIMMTester::getMatricesEqual(measurement_matrix, previous_data.H).first);
    QVERIFY(AVIMMTester::getMatricesEqual(measurement_matrix, previous_data.H).second == 0.0);
    QVERIFY(AVIMMTester::getMatricesEqual(process_noise, previous_data.Q).first);
    QVERIFY(AVIMMTester::getMatricesEqual(process_noise, previous_data.Q).second == 0.0);
    QVERIFY(AVIMMTester::getMatricesEqual(state_uncertainty, previous_data.R).first);
    QVERIFY(AVIMMTester::getMatricesEqual(state_uncertainty, previous_data.R).second == 0.0);
    QVERIFY(AVIMMTester::getMatricesEqual(control_input_matrix, previous_data.B).first);
    QVERIFY(AVIMMTester::getMatricesEqual(control_input_matrix, previous_data.B).second == 0.0);
    QVERIFY(filter_key == tester.m_filter_key);
}

//--------------------------------------------------------------------------

void TstAVIMMFilterBase::test_AVIMMFilterBase_expandInnovation()
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
    
    FilterMock tester(ini_state, transitions_matrix, covariance_matrix,
                      measurement_matrix, process_noise, state_uncertainty,
                      control_input_matrix, filter_key);
    
    auto& config = AVIMMConfigContainer::singleton();
    Matrix innovation_expansion(6, 6);
    innovation_expansion << 2,0,0,0,0,0,
                            0,2,0,0,0,0,
                            0,0,2,0,0,0,
                            0,0,0,2,0,0,
                            0,0,0,0,2,0,
                            0,0,0,0,0,2;
    config.expansion_matrix_innovation = innovation_expansion;
    
    Matrix to_be_expanded (2,2);
    to_be_expanded << 2,0,0,1;
    
    Matrix ref (6,6);
    ref << 8,0,0,0,0,0,
           0,4,0,0,0,0,
           0,0,4,0,0,0,
           0,0,0,4,0,0,
           0,0,0,0,4,0,
           0,0,0,0,0,4;
    
    Matrix expanded = tester.expandInnovation(to_be_expanded);
    QVERIFY(AVIMMTester::getMatricesEqual(expanded, ref).first);
    QVERIFY(AVIMMTester::getMatricesEqual(expanded, ref).second == 0.0);
}

//--------------------------------------------------------------------------

void TstAVIMMFilterBase::test_AVIMMFilterBase_expandErrorVector()
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
    
    FilterMock tester(ini_state, transitions_matrix, covariance_matrix,
                      measurement_matrix, process_noise, state_uncertainty,
                      control_input_matrix, filter_key);
    
    auto& config = AVIMMConfigContainer::singleton();
    Matrix error_expansion(6, 6);
    error_expansion << 2,0,0,0,0,0,
                       0,2,0,0,0,0,
                       0,0,2,0,0,0,
                       0,0,0,2,0,0,
                       0,0,0,0,2,0,
                       0,0,0,0,0,2;
    config.expansion_matrix = error_expansion;
    
    Vector to_be_expanded (2,1);
    to_be_expanded << 2,3;
    
    Vector ref (6,1);
    ref << 4,6,2,2,2,2;
    
    Vector expanded = tester.expandErrorVector(to_be_expanded);
    
    QVERIFY(AVIMMTester::getMatricesEqual(expanded, ref).first);
    QVERIFY(AVIMMTester::getMatricesEqual(expanded, ref).second == 0.0);
}

//--------------------------------------------------------------------------

void TstAVIMMFilterBase::test_AVIMMFilterBase_expandVector()
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
    
    FilterMock tester(ini_state, transitions_matrix, covariance_matrix,
                      measurement_matrix, process_noise, state_uncertainty,
                      control_input_matrix, filter_key);
    
    auto& config = AVIMMConfigContainer::singleton();
    Matrix error_expansion(6, 6);
    error_expansion << 2,0,0,0,0,0,
                       0,2,0,0,0,0,
                       0,0,2,0,0,0,
                       0,0,0,2,0,0,
                       0,0,0,0,2,0,
                       0,0,0,0,0,2;
    config.expansion_matrix = error_expansion;
    
    Vector to_be_expanded (2,1);
    to_be_expanded << 2,3;
    
    Vector ref (6,1);
    ref << 4,6,2,2,2,2;
    
    Vector expanded = tester.expandVector(to_be_expanded);
    
    QVERIFY(AVIMMTester::getMatricesEqual(expanded, ref).first);
    QVERIFY(AVIMMTester::getMatricesEqual(expanded, ref).second == 0.0);
}

//--------------------------------------------------------------------------

void TstAVIMMFilterBase::test_AVIMMFilterBase_expandMatrix()
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
    
    FilterMock tester(ini_state, transitions_matrix, covariance_matrix,
                      measurement_matrix, process_noise, state_uncertainty,
                      control_input_matrix, filter_key);
    
    auto& config = AVIMMConfigContainer::singleton();
    Matrix innovation_expansion(6, 6);
    innovation_expansion << 2,0,0,0,0,0,
                            0,2,0,0,0,0,
                            0,0,2,0,0,0,
                            0,0,0,2,0,0,
                            0,0,0,0,2,0,
                            0,0,0,0,0,2;
    config.expansion_matrix_innovation = innovation_expansion;
    
    Matrix to_be_expanded (2,2);
    to_be_expanded << 2,0,0,1;
    
    Matrix ref (6,6);
    ref << 8,0,0,0,0,0,
           0,4,0,0,0,0,
           0,0,4,0,0,0,
           0,0,0,4,0,0,
           0,0,0,0,4,0,
           0,0,0,0,0,4;
    
    Matrix expanded = tester.expandMatrix(to_be_expanded);
    QVERIFY(AVIMMTester::getMatricesEqual(expanded, ref).first);
    QVERIFY(AVIMMTester::getMatricesEqual(expanded, ref).second == 0.0);
}

//--------------------------------------------------------------------------

void TstAVIMMFilterBase::test_AVIMMFilterBase_predict()
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
    
    FilterMock tester(ini_state, transitions_matrix, covariance_matrix,
                      measurement_matrix, process_noise, state_uncertainty,
                      control_input_matrix, filter_key);
    
    Vector ref(3,1);
    ref << 3,3,3;
    
    tester.predict();
    
    QVERIFY(AVIMMTester::getMatricesEqual(tester.getData().x, ref).first);
}

//--------------------------------------------------------------------------

void TstAVIMMFilterBase::test_AVIMMFilterBase_update()
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
    
    FilterMock tester(ini_state, transitions_matrix, covariance_matrix,
                      measurement_matrix, process_noise, state_uncertainty,
                      control_input_matrix, filter_key);
    
    Vector ref(3,1);
    ref << 4,4,4;
    Vector z(3,1);
    z << 4,4,4;
    
    
    tester.update(z);
    
    QVERIFY(AVIMMTester::getMatricesEqual(tester.getData().x, ref).first);
}

//--------------------------------------------------------------------------

void TstAVIMMFilterBase::test_AVIMMFilterBase_getFilterInfo()
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
    
    FilterMock tester(ini_state, transitions_matrix, covariance_matrix,
                      measurement_matrix, process_noise, state_uncertainty,
                      control_input_matrix, filter_key);
    
    QVERIFY(QString("Filter Mock") == tester.getFilterInfo());
}

//--------------------------------------------------------------------------

void TstAVIMMFilterBase::test_AVIMMFilterBase_getFilterKey()
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
    
    FilterMock tester(ini_state, transitions_matrix, covariance_matrix,
                      measurement_matrix, process_noise, state_uncertainty,
                      control_input_matrix, filter_key);
    
    QVERIFY(QString("Test") == tester.getFilterKey());
}

//--------------------------------------------------------------------------

void TstAVIMMFilterBase::test_AVIMMFilterBase_getLogLikelihood()
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
    
    FilterMock tester(ini_state, transitions_matrix, covariance_matrix,
                      measurement_matrix, process_noise, state_uncertainty,
                      control_input_matrix, filter_key);
    
    Matrix S(3,3);
    S << 1,0,0,0,1,0,0,0,1;
    Vector error(3,1);
    error << 0,0,0;
    
    tester.m_data.S = S;
    tester.m_data.error = error;
    
    QVERIFY((tester.getLogLikelihood() - (-2.75682)) < 1*exp(-5));
}

//--------------------------------------------------------------------------

void TstAVIMMFilterBase::test_AVIMMFilterBase_getLikelihood()
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
    
    FilterMock tester(ini_state, transitions_matrix, covariance_matrix,
                      measurement_matrix, process_noise, state_uncertainty,
                      control_input_matrix, filter_key);
    
    Matrix S(3,3);
    S << 1,0,0,0,1,0,0,0,1;
    Vector error(3,1);
    error << 0,0,0;
    
    tester.m_data.S = S;
    tester.m_data.error = error;
    
    QVERIFY((tester.getLogLikelihood() - (0.0634936) )< 1*exp(-5));
}

//--------------------------------------------------------------------------

void TstAVIMMFilterBase::test_AVIMMFilterBase_createUnityMatrix()
{
    Matrix ref(6,6);
    ref << 1,0,0,0,0,0,
           0,1,0,0,0,0,
           0,0,1,0,0,0,
           0,0,0,1,0,0,
           0,0,0,0,1,0,
           0,0,0,0,0,1;
    
    auto unity = AVIMMFilterBase::createUnityMatrix(6);
    QVERIFY(AVIMMTester::getMatricesEqual(unity, ref).first);
    QVERIFY(AVIMMTester::getMatricesEqual(unity, ref).second == 0.0);
}

//--------------------------------------------------------------------------

void TstAVIMMFilterBase::test_AVIMMFilterBase_zeroSmallElements()
{
    Matrix ref(6,6);
    ref <<  0,0,0,0,0,0,
            0,1*exp(-1),0,0,0,0,
            0,0,1,0,0,0,
            0,0,0,1,0,0,
            0,0,0,0,1,0,
            0,0,0,0,0,1;
    
    Matrix to_be_zeroed(6,6);
    to_be_zeroed <<  1*exp(-16),0,0,0,0,0,
                     0,1*exp(-1),0,0,0,0,
                     0,0,1,0,0,0,
                     0,0,0,1,0,0,
                     0,0,0,0,1,0,
                     0,0,0,0,0,1;
    
    auto zeroed = AVIMMFilterBase::zeroSmallElements(to_be_zeroed);
    QVERIFY(AVIMMTester::getMatricesEqual(zeroed, ref).first);
    
    Vector ref_vec(6,1);
    ref_vec <<  0,1*exp(-1),0,0,0,0;
    
    Vector to_be_zeroed_vec(6,6);
    to_be_zeroed_vec << 1*exp(-16),1*exp(-1),0,0,0,0;
    
    auto zeroed_vec = AVIMMFilterBase::zeroSmallElements(to_be_zeroed_vec);
    QVERIFY(AVIMMTester::getMatricesEqual(zeroed_vec, ref_vec).first);
}

AV_QTEST_MAIN(TstAVIMMFilterBase)
#include "tstavimmfilterbase.moc"
