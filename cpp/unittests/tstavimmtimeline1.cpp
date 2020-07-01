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
    \brief  Tests a uniform linear motion with equidistant time intervalls
 */

#include <QObject>
#include <QTest>
#include <avunittest.h>
#include <QApplication>
#include <numeric>

#include "testhelper/avimmtester.h"

class TstIMMTimeLine1 : public QObject
{
Q_OBJECT

public:
    TstIMMTimeLine1() {}

public slots:
    void initTestCase() {};
    void cleanupTestCase() {};
    void init()
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
    };
    void cleanup() {
        AVIMMConfigParser::deleteSingleton();
        AVIMMConfigContainer::deleteSingleton();
        AVConfig2Global::deleteSingleton();
    };

private slots:
    void test_IMM_test_main();
};

//--------------------------------------------------------------------------

void TstIMMTimeLine1::test_IMM_test_main()
{
    std::cout << "Starting Test" << std::endl;
    AVIMMTester avimm_tester("/home/users/felix/workspace/trunk/svn/avcommon/src5/avimmlib/unittests/test_data/test_data_linear_uniform_motion.csv");

    // Run simulation
    avimm_tester.run_sim();
    
    // Evaluate errors
    Vector zero(6,1);
    zero << 0, 0, 0, 0, 0, 0;
    auto errors_accumulated = std::accumulate(avimm_tester.getErrorToMeasurementData().begin(), avimm_tester.getErrorToMeasurementData().end(), zero);
    double error_value      = errors_accumulated.sum();
    
    QVERIFY(error_value < 1*exp(-9));
    std::cout << std::endl << "Error value: " << error_value << std::endl;
    
    std::cout << std::endl << "Done" << std::endl;


}

AV_QTEST_MAIN(TstIMMTimeLine1)
#include "tstavimmtimeline1.moc"
