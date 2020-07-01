//
// Created by felix on 6/4/20.
//

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
    \brief   Function level test cases for AVIMMConfig
 */

#include <QObject>
#include <QTest>
#include <avunittest.h>
#include <QApplication>

#include "testhelper/avimmtester.h"

class TstAVIMMConfigReader : public QObject
{
Q_OBJECT

public:
    TstAVIMMConfigReader() {}

public slots:
    void initTestCase() {
        std::vector<char*> args;
        QByteArray         dummy_arg("dummy");  // program name, is ignored by config
        args.push_back(dummy_arg.data());
    
        AVEnvironment::setProcessName("imm_tester");
        AVConfig2Global::initializeSingleton(args.size(), args.data(), false, AVEnvironment::APP_ASTOS, "imm_tester");
        AVConfig2Global::singleton().initialize();
    
        AVIMMConfigContainer::setSingleton(new AVIMMConfigContainer());
        AVIMMConfigParser::setSingleton(new AVIMMConfigParser());
    };
    void cleanupTestCase()
    {
        AVIMMConfigParser::deleteSingleton();
        AVIMMConfigContainer::deleteSingleton();
        AVConfig2Global::deleteSingleton();
    };
    void init() {};
    void cleanup() {};

private slots:

    void test_AVIMMConfigRead();
    void test_AVIMMClassName();
    void test_AVIMMCreateUnityMatrix();
    void test_AVIMMgetFilterTypesFromConfig();
    void test_AVIMMconvertAVMatrixFloatToEigenMatrix();
    void test_AVIMMconvertQListFloatToEigenVector();
};

//--------------------------------------------------------------------------

void TstAVIMMConfigReader::test_AVIMMClassName()
{
    auto& config = AVIMMConfigContainer::singleton();
    QVERIFY(config.className() == QString("AVIMMConfigContainer"));
}

//--------------------------------------------------------------------------

void TstAVIMMConfigReader::test_AVIMMCreateUnityMatrix()
{
    Matrix ref(6,6);
    ref << 1,0,0,0,0,0,
           0,1,0,0,0,0,
           0,0,1,0,0,0,
           0,0,0,1,0,0,
           0,0,0,0,1,0,
           0,0,0,0,0,1;
    
    auto unity = AVIMMConfigContainer::createUnityMatrix(6);
    QVERIFY(AVIMMTester::getMatricesEqual(unity, ref).first);
    QVERIFY(AVIMMTester::getMatricesEqual(unity, ref).second == 0.0);
}

//--------------------------------------------------------------------------

void TstAVIMMConfigReader::test_AVIMMgetFilterTypesFromConfig()
{
    auto& new_conf = AVIMMConfigContainer::singleton();
    QList<FilterType> ref = {KalmanFilter, ExtendedKalmanFilter, ExtendedKalmanFilter};
    QStringList test_filter_def = {"KalmanFilter", "ExtendedKalmanFilter", "ExtendedKalmanFilter"};
    new_conf.filter_definitions = test_filter_def;
    
    // Delete default filter type and set correct filter type according to test_filter_def
    new_conf.filter_types.clear();
    new_conf.getFilterTypesFromConfig();
    QVERIFY(new_conf.filter_types == ref);
}

//--------------------------------------------------------------------------

void TstAVIMMConfigReader::test_AVIMMconvertAVMatrixFloatToEigenMatrix()
{
    Matrix ref(2, 2);
    ref << 1.0, 2.0,
           3.0, 4.0;
    
    AVMatrix<float> M(2,2, 0.0);
    M.set(0,0, 1.0);
    M.set(0,1, 2.0);
    M.set(1,0, 3.0);
    M.set(1,1, 4.0);
    
    auto converted = AVIMMConfigContainer::convertAVMatrixFloatToEigenMatrix(M);
    
    QVERIFY(AVIMMTester::getMatricesEqual(converted, ref).first);
    QVERIFY(AVIMMTester::getMatricesEqual(converted, ref).second == 0.0);
}

//--------------------------------------------------------------------------

void TstAVIMMConfigReader::test_AVIMMconvertQListFloatToEigenVector()
{
    Matrix ref(2, 1);
    ref << 1.0, 2.0;
    
    QList<float> M = {1.0, 2.0};
    
    auto converted = AVIMMConfigContainer::convertQListFloatToEigenVector(M);
    
    QVERIFY(AVIMMTester::getMatricesEqual(converted, ref).first);
    QVERIFY(AVIMMTester::getMatricesEqual(converted, ref).second == 0.0);
}

//--------------------------------------------------------------------------

void TstAVIMMConfigReader::test_AVIMMConfigRead()
{
    auto& config = AVIMMConfigContainer::singleton();
    auto& parser = AVIMMConfigParser::singleton();
    
    //References
    QList<FilterType> filter_types_ref = {KalmanFilter, KalmanFilter};
    QStringList sub_filter_def_ref = {"kf", "kf1"};
    QStringList state_vector_ref = {"pos_x", "vel_x", "acc_x", "pos_y", "vel_y", "acc_y"};
    Matrix2d markov_matrix_ref;
    Vector2d mode_probabilities_ref;
    Matrix6d expansion_matrix_ref;
    Matrix6d expansion_matrix_covariance_ref;
    Matrix6d expansion_matrix_innovation_ref;
    Matrix46d shrinking_matrix_ref;
    Matrix6d F_kf_ref;
    Matrix6d H_kf_ref;
    Matrix6d Q_kf_ref;
    Matrix6d R_kf_ref;
    Matrix62d B_kf_ref;
    Matrix6d F_kf1_ref;
    Matrix6d H_kf1_ref;
    Matrix6d Q_kf1_ref;
    Matrix6d R_kf1_ref;
    Matrix62d B_kf1_ref;
    
    // Fill with values
    markov_matrix_ref << 0.95, 0.05, 0.05, 0.95;
    mode_probabilities_ref << 0.5, 0.5;
    expansion_matrix_ref << 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0;
    expansion_matrix_covariance_ref <<  1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 9, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 9;
    expansion_matrix_innovation_ref <<  1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1.05, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1.05;
    shrinking_matrix_ref <<  1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0;
    F_kf_ref << 1, 1, 0.5, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0.5, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1;
    H_kf_ref << 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0;
    Q_kf_ref << 0.05, 0.125, 0.166667, 0, 0, 0, 0.125, 0.5, 0.5, 0, 0, 0, 0.166667, 0.5, 1, 0, 0, 0, 0, 0, 0, 0.05, 0.125, 0.166667, 0, 0, 0, 0.125, 0.5, 0.5, 0, 0, 0, 0.166667, 0.5, 1;
    R_kf_ref << 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1;
    B_kf_ref << 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0;
    F_kf1_ref << 1, 1, 0.5, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0.5, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1;
    H_kf1_ref << 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0;
    Q_kf1_ref << 0.05, 0.125, 0.166667, 0, 0, 0, 0.125, 0.5, 0.5, 0, 0, 0, 0.166667, 0.5, 1, 0, 0, 0, 0, 0, 0, 0.05, 0.125, 0.166667, 0, 0, 0, 0.125, 0.5, 0.5, 0, 0, 0, 0.166667, 0.5, 1;
    R_kf1_ref << 1000, 0, 0, 0, 0, 0, 0, 1000, 0, 0, 0, 0, 0, 0, 1000, 0, 0, 0, 0, 0, 0, 1000, 0, 0, 0, 0, 0, 0, 1000, 0, 0, 0, 0, 0, 0, 1000;
    B_kf1_ref << 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0;
    markov_matrix_ref.resize(2,2);
    mode_probabilities_ref.resize(2,12);
    expansion_matrix_ref.resize(6,6);
    expansion_matrix_covariance_ref.resize(6,6);
    expansion_matrix_innovation_ref.resize(6,6);
    shrinking_matrix_ref.resize(4,6);
    F_kf_ref.resize(6,6);
    H_kf_ref.resize(6,6);
    Q_kf_ref.resize(6,6);
    R_kf_ref.resize(6,6);
    B_kf_ref.resize(6,2);
    F_kf1_ref.resize(6,6);
    H_kf1_ref.resize(6,6);
    Q_kf1_ref.resize(6,6);
    R_kf1_ref.resize(6,6);
    B_kf1_ref.resize(6,2);
    

    // Read all general configs
    QList<FilterType> filter_types     = config.filter_types;
    QStringList sub_filter_def         = config.sub_filter_config_definitions;
    QStringList state_vector           = config.state_definition;
    Matrix markov_matrix               = config.markov_transition_matrix;
    Vector mode_probabilities          = config.mode_probabilities;
    Matrix expansion_matrix            = config.expansion_matrix;
    Matrix expansion_matrix_covariance = config.expansion_matrix_covariance;
    Matrix expansion_matrix_innovation = config.expansion_matrix_innovation;
    Matrix shrinking_matrix            = config.shrinking_matrix;
    
    // Get matrices of all subfilters
    Matrix F_kf = parser.calculateTimeDependentMatrices(config.filters["kf"]->transition_matrix, 1);
    Matrix H_kf = parser.calculateTimeDependentMatrices(config.filters["kf"]->measurement_control_matrix, 1);
    Matrix Q_kf = parser.calculateTimeDependentMatrices(config.filters["kf"]->process_noise_matrix, 1);
    Matrix R_kf = parser.calculateTimeDependentMatrices(config.filters["kf"]->measurement_uncertainty_matrix, 1);
    Matrix B_kf = parser.calculateTimeDependentMatrices(config.filters["kf"]->input_control_matrix, 1);
    
    Matrix F_kf1 = parser.calculateTimeDependentMatrices(config.filters["kf1"]->transition_matrix, 1);
    Matrix H_kf1 = parser.calculateTimeDependentMatrices(config.filters["kf1"]->measurement_control_matrix, 1);
    Matrix Q_kf1 = parser.calculateTimeDependentMatrices(config.filters["kf1"]->process_noise_matrix, 1);
    Matrix R_kf1 = parser.calculateTimeDependentMatrices(config.filters["kf1"]->measurement_uncertainty_matrix, 1);
    Matrix B_kf1 = parser.calculateTimeDependentMatrices(config.filters["kf1"]->input_control_matrix, 1);
    
    // Verify that all matrices are equal to the reference matrices

    QVERIFY(filter_types == filter_types_ref);
    QVERIFY(sub_filter_def == sub_filter_def_ref);
    QVERIFY(state_vector == state_vector_ref);
    QVERIFY(AVIMMTester::getMatricesEqual(markov_matrix, markov_matrix_ref).first);
    QVERIFY(AVIMMTester::getMatricesEqual(mode_probabilities, mode_probabilities_ref).first);
    QVERIFY(AVIMMTester::getMatricesEqual(expansion_matrix, expansion_matrix_ref).first);
    QVERIFY(AVIMMTester::getMatricesEqual(expansion_matrix_covariance, expansion_matrix_covariance_ref).first);
    QVERIFY(AVIMMTester::getMatricesEqual(expansion_matrix_innovation, expansion_matrix_innovation_ref).first);
    QVERIFY(AVIMMTester::getMatricesEqual(shrinking_matrix, shrinking_matrix_ref).first);
    QVERIFY(AVIMMTester::getMatricesEqual(F_kf, F_kf_ref).first);
    QVERIFY(AVIMMTester::getMatricesEqual(H_kf, H_kf_ref).first);
    QVERIFY(AVIMMTester::getMatricesEqual(Q_kf, Q_kf_ref).first);
    QVERIFY(AVIMMTester::getMatricesEqual(R_kf, R_kf_ref).first);
    QVERIFY(AVIMMTester::getMatricesEqual(B_kf, B_kf_ref).first);
    QVERIFY(AVIMMTester::getMatricesEqual(F_kf1, F_kf1_ref).first);
    QVERIFY(AVIMMTester::getMatricesEqual(H_kf1, H_kf1_ref).first);
    QVERIFY(AVIMMTester::getMatricesEqual(Q_kf1, Q_kf1_ref).first);
    QVERIFY(AVIMMTester::getMatricesEqual(R_kf1, R_kf1_ref).first);
    QVERIFY(AVIMMTester::getMatricesEqual(B_kf1, B_kf1_ref).first);
}

AV_QTEST_MAIN(TstAVIMMConfigReader)
#include "tstavimmconfigreader.moc"
