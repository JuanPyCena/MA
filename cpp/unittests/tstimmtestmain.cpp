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
    \brief   Function level test cases for AVFieldImagePatchMsg
 */

#include <QObject>
#include <QTest>
#include <avunittest.h>
#include <QApplication>

#include "testhelper/avimmtester.h"

class TstIMMTestMain : public QObject
{
Q_OBJECT

public:
    TstIMMTestMain() {}

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

///////////////////////////////////////////////////////////////////////////////

void TstIMMTestMain::test_IMM_test_main()
{
    std::cout << "Starting Test" << std::endl;
    AVIMMTester avimm_tester("/home/users/felix/workspace/trunk/svn/avcommon/src5/avimmlib/unittests/test_data/test.csv");
    avimm_tester.run_sim();
    QVector<Vector> results = avimm_tester.getResultingStates();
    
    for (const auto& result : results)
        std::cout << std::endl << result;
    
    std::cout << "Done" << std::endl;


}

AV_QTEST_MAIN(TstIMMTestMain)
#include "tstimmtestmain.moc"
