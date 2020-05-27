///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2013
//
// Module:    TESTS - AVCOMMON Unit Tests
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \author  QT5-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   Function level test cases for AviBit macros
 */

#include <QtTest>
#include <avunittestmain.h>
#include "avexecstandbychecker.h"

class TstAVExecStandbyChecker : public QObject
{
    Q_OBJECT

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:


    void testExecIfNoTwin();

};

///////////////////////////////////////////////////////////////////////////////

//! Declaration of unknown metatypes

///////////////////////////////////////////////////////////////////////////////

//! This will be called before the first test function is executed.
//! It is only called once.
void TstAVExecStandbyChecker::initTestCase()
{

}

///////////////////////////////////////////////////////////////////////////////

//! This will be called after the last test function is executed.
//! It is only called once.
void TstAVExecStandbyChecker::cleanupTestCase()
{

}

///////////////////////////////////////////////////////////////////////////////

//! This will be called before each test function is executed.
void TstAVExecStandbyChecker::init()
{

}

///////////////////////////////////////////////////////////////////////////////

//! This will be called after every test function.
void TstAVExecStandbyChecker::cleanup()
{

}

///////////////////////////////////////////////////////////////////////////////

void TstAVExecStandbyChecker::testExecIfNoTwin()
{
    AVLogInfo << "---- testExecIfNoTwin launched ----";


    AVExecStandbyChecker standby_checker;

    QSignalSpy spy(&standby_checker, SIGNAL(signalSwitchedToExec()));

    runEventLoop(100);

    QVERIFY(standby_checker.isExec());
    QCOMPARE(spy.size(), 1);


    AVLogInfo << "---- testExecIfNoTwin ended ----";
}

///////////////////////////////////////////////////////////////////////////////

AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(TstAVExecStandbyChecker,"avlib/unittests/config")
#include "tstavexecstandbychecker.moc"


// End of file
