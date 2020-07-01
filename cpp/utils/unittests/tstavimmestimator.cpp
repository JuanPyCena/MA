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
    \brief   Function level test cases for AVIMMEstimator
 */

#include <QObject>
#include <QTest>
#include <avunittest.h>
#include <QApplication>
#include <typeinfo>

#include "testhelper/avimmtester.h"

class TstAVIMMEstimator: public QObject
{
Q_OBJECT

public:
    TstAVIMMEstimator() {}

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
    void test_IMMEstimator_constructor();
    void test_IMMEstimator_initializeSubfilters();
    void test_IMMEstimator_calculateModeProbabilityMatrix();
    void test_IMMEstimator_calculateIMMState();
    void test_IMMEstimator_calculateMixedStates();
    void test_IMMEstimator_calculateModeProbabilities();
    void test_IMMEstimator_expandVector();
    void test_IMMEstimator_expandMatrix();
    void test_IMMEstimator_expandCovariance();
    void test_IMMEstimator_shrinkVector();
    void test_IMMEstimator_shrinkMatrix();
    void test_IMMEstimator_predictAndUpdate();
    void test_IMMEstimator_extrapolate();
};

//--------------------------------------------------------------------------

void TstAVIMMEstimator::test_IMMEstimator_constructor()
{
    Vector initial_state(6,1);
    initial_state << 1,2,3,4,5,6;
    
    Matrix ref(6,6);
    ref << 1,0,0,0,0,0,
           0,1,0,0,0,0,
           0,0,1,0,0,0,
           0,0,0,1,0,0,
           0,0,0,0,1,0,
           0,0,0,0,0,1;
    
    AVIMMEstimator tester(initial_state);
    QVERIFY(tester.getData().x == initial_state);
    QVERIFY(tester.getData().P == ref);
    QVERIFY(tester.getPreviousData().x == initial_state);
    QVERIFY(tester.getPreviousData().P == ref);
    QVERIFY(tester.m_last_calculation == QDateTime::currentDateTimeUtc());
}

//--------------------------------------------------------------------------

void TstAVIMMEstimator::test_IMMEstimator_initializeSubfilters()
{
    Vector initial_state(6,1);
    initial_state << 1,2,3,4,5,6;
    Vector test_state(6,1);
    test_state << 7,8,9,10,11,12,13;

    AVIMMEstimator tester(initial_state);
    
    // Clear filters list before reinitializing them
    tester.m_filters.clear();
    
    Matrix F(6,6);
    Matrix P(6,6);
    Matrix H(6,6);
    Matrix Q(6,6);
    Matrix R(6,6);
    Matrix B(6,6);
    Matrix J(6,6);
    F << 1,0,0,0,0,0,
         0,1,0,0,0,0,
         0,0,1,0,0,0,
         0,0,0,1,0,0,
         0,0,0,0,1,0,
         0,0,0,0,0,1;
    P << 1,0,0,0,0,0,
         0,1,0,0,0,0,
         0,0,1,0,0,0,
         0,0,0,1,0,0,
         0,0,0,0,1,0,
         0,0,0,0,0,1;
    H << 1,0,0,0,0,0,
         0,1,0,0,0,0,
         0,0,1,0,0,0,
         0,0,0,1,0,0,
         0,0,0,0,1,0,
         0,0,0,0,0,1;
    Q << 1,0,0,0,0,0,
         0,1,0,0,0,0,
         0,0,1,0,0,0,
         0,0,0,1,0,0,
         0,0,0,0,1,0,
         0,0,0,0,0,1;
    R << 1,0,0,0,0,0,
         0,1,0,0,0,0,
         0,0,1,0,0,0,
         0,0,0,1,0,0,
         0,0,0,0,1,0,
         0,0,0,0,0,1;
    B << 1,0,0,0,0,0,
         0,1,0,0,0,0,
         0,0,1,0,0,0,
         0,0,0,1,0,0,
         0,0,0,0,1,0,
         0,0,0,0,0,1;
    J << 1,0,0,0,0,0,
         0,1,0,0,0,0,
         0,0,1,0,0,0,
         0,0,0,1,0,0,
         0,0,0,0,1,0,
         0,0,0,0,0,1;
    
    
    std::vector<AVIMMFilterBase*> ref_filters;
    
    auto* filter1 = new AVIMMKalmanFilter(initial_state, F, P, H, Q, R, B, "TEST1");
    ref_filters.push_back(filter1);

    auto* filter2 = new AVIMMKalmanFilter(initial_state, F, P, H, Q, R, B, "TEST2");
    ref_filters.push_back(filter2);
    
    // Reinitialize filters and verify if lists hold objects of same type
    tester.initializeSubfilters(test_state);
    
    int i = 0;
    for (auto& sub_filter : tester.m_filters)
    {
        QVERIFY(typeid(sub_filter) == typeid(ref_filters[i]));
        i++;
    }
    
    cleanup_ptr_container(ref_filters);
}

//--------------------------------------------------------------------------

void TstAVIMMEstimator::test_IMMEstimator_calculateModeProbabilityMatrix()
{
    Vector initial_state(6,1);
    initial_state << 1,2,3,4,5,6;
    
    Matrix ref(2,2);
    ref << 0.954545, 0.205882,
           0.0454545, 0.794118;
    
    Vector c_ref(2,1);
    c_ref << 0.66, 0.34;
    
    Matrix markov_matrix(2,2);
    markov_matrix << 0.9, 0.1,
                     0.1, 0.9;
    
    Vector modes(2,1);
    modes << 0.7, 0.3;
    
    AVIMMEstimator tester(initial_state);
    
    tester.m_markov_transition_matrix = markov_matrix;
    tester.m_mode_probabilities = modes;
    
    tester.calculateModeProbabilityMatrix(tester.m_mode_probabilities_matrix);
    
    QVERIFY(c_ref == tester.m_c);
    QVERIFY(AVIMMTester::getMatricesEqual(ref, tester.m_mode_probabilities_matrix).first);
}

//--------------------------------------------------------------------------

void TstAVIMMEstimator::test_IMMEstimator_calculateIMMState()
{
    Vector initial_state(6,1);
    initial_state << 1,2,3,4,5,6;
    
    Matrix P_ref(6,6);
    P_ref << 2,     0,   2.1,  1.4,  3.5,   2.8,
             0,   2.5,   1.2,  2.4,  2.4,   3.6,
             2.1, 1.2, 112.8,  6.6, 12.9,    12,
             1.4, 2.4,   6.6,  8.9, 11.8,  12.8,
             3.5, 2.4,  12.9, 11.8, 23.6,  21.2,
             2.8, 3.6,    12, 12.8, 21.2, 127.3;
    
    Vector x_ref(6,1);
    x_ref << 0.3, 1.4, 0.3, 1.4, 0.3, 1.4;
    
    Vector x_filter1(6,1);
    x_filter1 << 0,2,0,2,0,2;
    
    Vector x_filter2(6,1);
    x_filter2 << 1,0,1,0,1,0;
    
    Matrix P_filter1(6,6);
    P_filter1 << 1,0,0,0,0,0,
                 0,1,0,0,0,0,
                 0,0,1,0,0,0,
                 0,0,0,1,0,0,
                 0,0,0,0,1,0,
                 0,0,0,0,0,1;
    
    Matrix P_filter2(6,6);
    P_filter2 << 2,0,0,0,0,0,
                 0,2,0,0,0,0,
                 0,0,2,0,0,0,
                 0,0,0,2,0,0,
                 0,0,0,0,2,0,
                 0,0,0,0,0,2;
    
    QList<Vector> xs = {x_filter1, x_filter2};
    QList<Vector> Ps = {P_filter1, P_filter2};
    
    Vector modes(2,1);
    modes << 0.7, 0.3;
    
    AVIMMEstimator tester(initial_state);
    
    tester.m_mode_probabilities = modes;
    
    int i = 0;
    for (auto& filter: tester.m_filters)
    {
        filter->m_data.x = xs[i];
        filter->m_data.P = Ps[i];
        i++;
    }
    
    tester.calculateIMMState(tester.m_data.x, tester.m_data.P);
    
    QVERIFY(AVIMMTester::getMatricesEqual(x_ref, tester.getData().x).first);
    QVERIFY(AVIMMTester::getMatricesEqual(P_ref, tester.getData().P).first);
}

//--------------------------------------------------------------------------

void TstAVIMMEstimator::test_IMMEstimator_calculateMixedStates()
{
    Vector initial_state(6,1);
    initial_state << 1,2,3,4,5,6;
    
    Matrix P_ref1(6,6);
    P_ref1 << 2,     0,  1.5,    1,  2.5,     2,
              0,   3.5,    2,    4,    4,     6,
              1.5,   2,  128,    7, 11.5,    12,
              1,     4,    7, 11.5,   13,    16,
              2.5,   4, 11.5,   13,   22,    22,
              2,     6,   12,   16,   22, 147.5;
    
    Matrix P_ref2(6,6);
    P_ref2 << 2,     0,  1.5,    1,  2.5,     2,
              0,   3.5,    2,    4,    4,     6,
              1.5,   2,  128,    7, 11.5,    12,
              1,     4,    7, 11.5,   13,    16,
              2.5,   4, 11.5,   13,   22,    22,
              2,     6,   12,   16,   22, 147.5;
    
    Vector x_ref1(6,1);
    x_ref1 << 0.5,1,0.5,1,0.5,1;
    
    Vector x_ref2(6,1);
    x_ref2 << 0.5,1,0.5,1,0.5,1;
    
    Vector x_filter1(6,1);
    x_filter1 << 0,2,0,2,0,2;
    
    Vector x_filter2(6,1);
    x_filter2 << 1,0,1,0,1,0;
    
    Matrix P_filter1(6,6);
    P_filter1 << 1,0,0,0,0,0,
                 0,1,0,0,0,0,
                 0,0,1,0,0,0,
                 0,0,0,1,0,0,
                 0,0,0,0,1,0,
                 0,0,0,0,0,1;
    
    Matrix P_filter2(6,6);
    P_filter2 << 2,0,0,0,0,0,
                 0,2,0,0,0,0,
                 0,0,2,0,0,0,
                 0,0,0,2,0,0,
                 0,0,0,0,2,0,
                 0,0,0,0,0,2;
    
    QList<Vector> x_filters = {x_filter1, x_filter2};
    QList<Vector> P_filters = {P_filter1, P_filter2};
    std::vector<Vector> xs_ref = {x_ref1, x_ref2};
    std::vector<Matrix> Ps_ref = {P_ref1, P_ref2};
    
    Vector modes(2,1);
    modes << 0.5, 0.5;
    
    Matrix mode_probs(2,2);
    mode_probs << 0.5, 0.5,
                  0.5, 0.5;
    
    AVIMMEstimator tester(initial_state);
    
    tester.m_mode_probabilities = modes;
    tester.m_mode_probabilities_matrix = mode_probs;
    
    int i = 0;
    for (auto& filter: tester.m_filters)
    {
        filter->m_data.x = x_filters[i];
        filter->m_data.P = P_filters[i];
        i++;
    }
    
    tester.calculateMixedStates(tester.m_mixed_states, tester.m_mixed_covariances);
    
    QVERIFY(AVIMMTester::getMatricesEqual(xs_ref[0], tester.m_mixed_states[0]).first);
    QVERIFY(AVIMMTester::getMatricesEqual(xs_ref[1], tester.m_mixed_states[1]).first);
    QVERIFY(AVIMMTester::getMatricesEqual(Ps_ref[0], tester.m_mixed_covariances[0]).first);
    QVERIFY(AVIMMTester::getMatricesEqual(Ps_ref[1], tester.m_mixed_covariances[1]).first);
}

//--------------------------------------------------------------------------

void TstAVIMMEstimator::test_IMMEstimator_calculateModeProbabilities()
{
    Vector initial_state(6,1);
    initial_state << 1,2,3,4,5,6;
    
    Vector mode_probabilities_ref(2,1);
    mode_probabilities_ref << 0.5, 0.5;
    
    Vector mode_probabilities(6,1);
    mode_probabilities << 0, 0;
    
    Matrix S_filter(6,6);
    S_filter << 1,0,0,0,0,0,
                0,1,0,0,0,0,
                0,0,1,0,0,0,
                0,0,0,1,0,0,
                0,0,0,0,1,0,
                0,0,0,0,0,1;
    
    Vector error_filter(6,1);
    error_filter << 1,0,0,0,0,0;

    
    QList<Vector> S_filters = {S_filter, S_filter};
    QList<Vector> error_filters = {error_filter, error_filter};
    
    Vector modes(2,1);
    modes << 0.5, 0.5;
    
    Matrix mode_probs(2,2);
    mode_probs << 0.5, 0.5,
                  0.5, 0.5;
    
    AVIMMEstimator tester(initial_state);
    
    int i = 0;
    for (auto& filter: tester.m_filters)
    {
        filter->m_data.S = S_filters[i];
        filter->m_data.S.resize(6,6);
        filter->m_data.error = error_filters[i];
        i++;
    }
    
    tester.calculateModeProbabilities(mode_probabilities);
    
    QVERIFY(AVIMMTester::getMatricesEqual(mode_probabilities, mode_probabilities_ref).first);
}

//--------------------------------------------------------------------------

void TstAVIMMEstimator::test_IMMEstimator_expandVector()
{
    Vector initial_state(6,1);
    initial_state << 1,2,3,4,5,6;
    AVIMMEstimator tester(initial_state);
    
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

void TstAVIMMEstimator::test_IMMEstimator_expandMatrix()
{
    Vector initial_state(6,1);
    initial_state << 1,2,3,4,5,6;
    AVIMMEstimator tester(initial_state);
    
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
    to_be_expanded << 2,0,
                      0,1;
    
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

void TstAVIMMEstimator::test_IMMEstimator_expandCovariance()
{
    Vector initial_state(6,1);
    initial_state << 1,2,3,4,5,6;
    AVIMMEstimator tester(initial_state);
    
    auto& config = AVIMMConfigContainer::singleton();
    Matrix covariance_expansion(6, 6);
    covariance_expansion << 2,0,0,0,0,0,
            0,2,0,0,0,0,
            0,0,2,0,0,0,
            0,0,0,2,0,0,
            0,0,0,0,2,0,
            0,0,0,0,0,2;
    
    config.expansion_matrix_covariance = covariance_expansion;
    
    Matrix to_be_expanded (2,2);
    to_be_expanded << 2,0,
                      0,1;
    
    Matrix ref (6,6);
    ref << 8,0,0,0,0,0,
            0,4,0,0,0,0,
            0,0,4,0,0,0,
            0,0,0,4,0,0,
            0,0,0,0,4,0,
            0,0,0,0,0,4;
    
    Matrix expanded = tester.expandCovariance(to_be_expanded);
    
    QVERIFY(AVIMMTester::getMatricesEqual(expanded, ref).first);
    QVERIFY(AVIMMTester::getMatricesEqual(expanded, ref).second == 0.0);
}

//--------------------------------------------------------------------------

void TstAVIMMEstimator::test_IMMEstimator_shrinkVector()
{
    Vector initial_state(6,1);
    initial_state << 1,2,3,4,5,6;
    AVIMMEstimator tester(initial_state);
    
    auto& config = AVIMMConfigContainer::singleton();
    Matrix shrinking_matrix(4, 6);
    shrinking_matrix << 2,0,0,0,0,0,
                        0,2,0,0,0,0,
                        0,0,0,2,0,0,
                        0,0,0,0,2,0;
    
    config.shrinking_matrix = shrinking_matrix;
    
    Vector to_be_shrunk (6,1);
    to_be_shrunk << 2,3,5,6,7,8;
    
    Vector ref (4,1);
    ref << 4,6,12,14;
    
    Vector shrunk = tester.shrinkVector(to_be_shrunk, 4);
    
    QVERIFY(AVIMMTester::getMatricesEqual(shrunk, ref).first);
    QVERIFY(AVIMMTester::getMatricesEqual(shrunk, ref).second == 0.0);
}

//--------------------------------------------------------------------------

void TstAVIMMEstimator::test_IMMEstimator_shrinkMatrix()
{
    Vector initial_state(6,1);
    initial_state << 1,2,3,4,5,6;
    AVIMMEstimator tester(initial_state);
    
    auto& config = AVIMMConfigContainer::singleton();
    Matrix shrinking_matrix(4, 6);
    shrinking_matrix << 1,0,0,0,0,0,
                        0,1,0,0,0,0,
                        0,0,0,1,0,0,
                        0,0,0,0,1,0;
    
    config.shrinking_matrix = shrinking_matrix;
    
    Matrix to_be_shrunk (6,6);
    to_be_shrunk << 2,0,0,0,0,0,
                    0,2,0,0,0,0,
                    0,0,2,0,0,0,
                    0,0,0,2,0,0,
                    0,0,0,0,2,0,
                    0,0,0,0,0,2;
    
    Matrix ref (4,4);
    ref << 2,0,0,0,
           0,2,0,0,
           0,0,2,0,
           0,0,0,2;
    
    Matrix shrunk = tester.shrinkMatrix(to_be_shrunk, 4);
    
    QVERIFY(AVIMMTester::getMatricesEqual(shrunk, ref).first);
    QVERIFY(AVIMMTester::getMatricesEqual(shrunk, ref).second == 0.0);
}

//--------------------------------------------------------------------------

void TstAVIMMEstimator::test_IMMEstimator_predictAndUpdate()
{
    Matrix P(6,6);
    P << 2,0,0,0,0,0,
            0,2,0,0,0,0,
            0,0,2,0,0,0,
            0,0,0,2,0,0,
            0,0,0,0,2,0,
            0,0,0,0,0,2;
    
    QList<Vector> P_filters = {P, P};
    
    Vector initial_state(6,1);
    initial_state << 1,2,3,1,2,3;
    AVIMMEstimator tester(initial_state);
    AVIMMEstimator tester_R(initial_state);
    AVIMMEstimator tester_input(initial_state);
    
    tester.m_test_run = true;
    tester.m_now = QDateTime::currentDateTimeUtc().addSecs(1);
    tester_R.m_test_run = true;
    tester_R.m_now = QDateTime::currentDateTimeUtc().addSecs(1);
    tester_input.m_test_run = true;
    tester_input.m_now = QDateTime::currentDateTimeUtc().addSecs(1);
    
    Vector modes(2,1);
    modes << 0.5, 0.5;
    
    Matrix mode_probs(2,2);
    mode_probs << 0.5, 0.5,
            0.5, 0.5;
    
    tester.m_mode_probabilities = modes;
    tester_input.m_mode_probabilities = modes;
    tester_R.m_mode_probabilities = modes;
    tester.m_mode_probabilities_matrix = mode_probs;
    tester_input.m_mode_probabilities_matrix = mode_probs;
    tester_R.m_mode_probabilities_matrix = mode_probs;
    
    
    int i = 0;
    for (auto& filter: tester.m_filters)
    {
        filter->m_data.P = P_filters[i];
        filter->m_data.P.resize(6,6);
        i++;
    }
    
    i = 0;
    for (auto& filter: tester_R.m_filters)
    {
        filter->m_data.P = P_filters[i];
        filter->m_data.P.resize(6,6);
        i++;
    }
    
    i = 0;
    for (auto& filter: tester_input.m_filters)
    {
        filter->m_data.P = P_filters[i];
        filter->m_data.P.resize(6,6);
        i++;
    }
    
    Vector measurement(6,1);
    measurement << 1,1,1,1,1,1;
    
    Vector ref_state(6,1);
    ref_state << 1.63508, 3.02994, 2.26171, 1.63508, 3.02994, 2.26171;
    Matrix ref_cov(6,6);
    ref_cov << 9.10236,  6.25644,  2.33752,  8.27685,  5.68903,  2.12553,
    6.25644,  6.65581,  3.30734,  5.68903,  3.91031,  1.46096,
    2.33752,  3.30734,   3.3014,  2.12553,  1.46096, 0.545843,
    8.27685,  5.68903,  2.12553,  9.10236,  6.25644,  2.33752,
    5.68903,  3.91031,  1.46096,  6.25644,  6.65581,  3.30734,
    2.12553,  1.46096, 0.545843,  2.33752,  3.30734,   3.3014;
    
    tester.predictAndUpdate(measurement);
    
    QVERIFY(((tester.getData().x - ref_state).norm() < 0.1));
    QVERIFY(((tester.getData().P - ref_cov).norm() < 0.1));
    
    
    Matrix R(6,6);
    R << 10,0,0,0,0,0,
         0,10,0,0,0,0,
         0,0,10,0,0,0,
         0,0,0,10,0,0,
         0,0,0,0,10,0,
         0,0,0,0,0,10;
    
    Vector ref_state_R(6,1);
    ref_state_R << 1.63486, 3.03099, 2.26351, 1.63486, 3.03099, 2.26351;
    Matrix ref_cov_R(6,6);
    ref_cov_R << 9.07451,  6.2349, 2.32859, 8.24924, 5.66787, 2.11681,
                  6.2349, 6.63847, 3.29961, 5.66787, 3.89427, 1.45442,
                 2.32859, 3.29961, 3.29852, 2.11681, 1.45442, 0.54319,
                 8.24924, 5.66787, 2.11681, 9.07451,  6.2349, 2.32859,
                 5.66787, 3.89427, 1.45442,  6.2349, 6.63847, 3.29961,
                 2.11681, 1.45442, 0.54319, 2.32859, 3.29961, 3.29852,
    
    tester_R.predictAndUpdate(measurement, R);
    
    QVERIFY(((tester_R.getData().x - ref_state_R).norm() < 0.1));
    QVERIFY(((tester_R.getData().P - ref_cov_R).norm() < 0.1));
    
    Vector ref_state_input(6,1);
    ref_state_input << 1.63464, 3.03204, 2.2653, 1.63464, 3.03204, 2.2653;
    Matrix ref_cov_input(6,6);
    ref_cov_input << 9.04674, 6.21342,  2.31968,  8.2217, 5.64677,  2.10813,
                     6.21342, 6.62117,   3.2919, 5.64677, 3.87828,  1.44789,
                     2.31968,  3.2919,  3.29565, 2.10813, 1.44789, 0.540546,
                      8.2217, 5.64677,  2.10813, 9.04674, 6.21342,  2.31968,
                     5.64677, 3.87828,  1.44789, 6.21342, 6.62117,   3.2919,
                     2.10813, 1.44789, 0.540546, 2.31968,  3.2919,  3.29565;
    
    Vector input(6,1);
    input << 1,0,0,0,0,0;

    tester_input.predictAndUpdate(measurement, R, input);
    
    QVERIFY(((tester_input.getData().x - ref_state_input).norm() < 0.1));
    QVERIFY(((tester_input.getData().P - ref_cov_input).norm() < 0.1));
}

//--------------------------------------------------------------------------

void TstAVIMMEstimator::test_IMMEstimator_extrapolate()
{
    Matrix P(6,6);
    P << 2,0,0,0,0,0,
            0,2,0,0,0,0,
            0,0,2,0,0,0,
            0,0,0,2,0,0,
            0,0,0,0,2,0,
            0,0,0,0,0,2;
    
    QList<Vector> P_filters = {P, P};

    Vector initial_state(6,1);
    initial_state << 1,2,3,1,2,3;
    AVIMMEstimator tester(initial_state);
    AVIMMEstimator tester_input(initial_state);
    
    Vector modes(2,1);
    modes << 0.5, 0.5;
    
    Matrix mode_probs(2,2);
    mode_probs << 0.5, 0.5,
                  0.5, 0.5;
    
    tester.m_mode_probabilities = modes;
    tester_input.m_mode_probabilities = modes;
    tester.m_mode_probabilities_matrix = mode_probs;
    tester_input.m_mode_probabilities_matrix = mode_probs;
    
    
    int i = 0;
    for (auto& filter: tester.m_filters)
    {
        filter->m_data.P = P_filters[i];
        filter->m_data.P.resize(6,6);
        i++;
    }
    
    i = 0;
    for (auto& filter: tester_input.m_filters)
    {
        filter->m_data.P = P_filters[i];
        filter->m_data.P.resize(6,6);
        i++;
    }
    
    Vector ref_state(6,1);
    ref_state << 4.505, 5.003, 3, 4.505, 5.003, 3;
    Matrix ref_cov(6,6);
    ref_cov << 16.8413,  13.656, 1.16917,  12.285, 10.5255,       0,
               13.656,  13.5235,   2.503, 10.5255, 9.01801,       0,
               1.16917,   2.503,   3.001,       0,       0,       0,
               12.285,  10.5255,       0, 16.8413,  13.656, 1.16917,
               10.5255, 9.01801,       0,  13.656, 13.5235,   2.503,
               0,             0,       0, 1.16917,   2.503,   3.001;
               
               
    tester.m_test_run = true;
    tester.m_now = QDateTime::currentDateTimeUtc().addSecs(1);
    auto ret = tester.extrapolate();
    
    QVERIFY(((ret.first - ref_state).norm() < 0.1));
    QVERIFY(((ret.second - ref_cov).norm() < 0.1));
    
    Vector input(6,1);
    input << 1,0,0,0,0,0;
    
    Vector ref_state_input(6,1);
    ref_state_input << 4.505, 5.003, 3, 4.505, 5.003, 3;
    Matrix ref_cov_input(6,6);
    ref_cov_input << 16.8413,  13.656, 1.16917,  12.285, 10.5255,       0,
                     13.656,  13.5235,   2.503, 10.5255, 9.01801,       0,
                     1.16917,   2.503,   3.001,       0,       0,       0,
                     12.285,  10.5255,       0, 16.8413,  13.656, 1.16917,
                     10.5255, 9.01801,       0,  13.656, 13.5235,   2.503,
                     0,             0,       0, 1.16917,   2.503,   3.001;
    
    tester_input.m_test_run = true;
    tester_input.m_now = QDateTime::currentDateTimeUtc().addSecs(1);
    auto ret_input = tester_input.extrapolate(input);
    
    QVERIFY(((ret_input.first - ref_state_input).norm() < 0.1));
    QVERIFY(((ret_input.second - ref_cov_input).norm() < 0.1));
}

AV_QTEST_MAIN(TstAVIMMEstimator)
#include "tstavimmestimator.moc"
