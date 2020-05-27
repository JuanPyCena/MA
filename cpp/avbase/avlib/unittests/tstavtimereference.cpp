///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    UNITTESTS
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   Function level test cases for AVTimeReference
 */

#include <QtTest>
#include <avunittestmain.h>
#include "avtimereference.h"
#include "avtimereference/avtimereferenceadapterlocal.h"

#include "avtimereference/av_unittest_timereferenceholder.h"

class TstAVTimeReference : public QObject
{
    Q_OBJECT

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
    void test_AVTimeReference();
    void test_className();
    void test_initializeSingleton();
    void test_currentDateTime();
    void test_getSpeed();
    void test_getDateTimeOffsetMs();

    void test_setSpeed_multiple();

};

///////////////////////////////////////////////////////////////////////////////

//! Declaration of unknown metatypes
// Q_DECLARE_METATYPE(char *)
// Q_DECLARE_METATYPE(AVTimeReference)
// Q_DECLARE_METATYPE(long long)

///////////////////////////////////////////////////////////////////////////////

//! This will be called before the first test function is executed.
//! It is only called once.
void TstAVTimeReference::initTestCase()
{
    QVERIFY(!AVTimeReferenceAdapter::isSingletonInitialized());
}

///////////////////////////////////////////////////////////////////////////////

//! This will be called after the last test function is executed.
//! It is only called once.
void TstAVTimeReference::cleanupTestCase()
{

}

///////////////////////////////////////////////////////////////////////////////

//! This will be called before each test function is executed.
void TstAVTimeReference::init()
{

}

///////////////////////////////////////////////////////////////////////////////

//! This will be called after every test function.
void TstAVTimeReference::cleanup()
{    
    AVTimeReference::singleton().m_time_reference_holder->resetToSystemTime();
    AVUnitTestTimeReferenceHolder::resetAVTimeReferenceSystemTimeOverride();
}

///////////////////////////////////////////////////////////////////////////////

void TstAVTimeReference::test_AVTimeReference()
{
    AVLogInfo << "---- test_AVTimeReference launched (" << QTest::currentDataTag() << ") ----";

    AVTimeReference instance;

    AVLogInfo << "---- test_AVTimeReference ended (" << QTest::currentDataTag() << ")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVTimeReference::test_className()
{
    AVLogInfo << "---- test_className launched (" << QTest::currentDataTag() << ") ----";

    AVTimeReference instance;

    QCOMPARE(instance.className(), "AVTimeReference");

    AVLogInfo << "---- test_className ended (" << QTest::currentDataTag() << ")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVTimeReference::test_initializeSingleton()
{
    AVLogInfo << "---- test_initializeSingleton launched (" << QTest::currentDataTag() << ") ----";

    //cannot test

    AVLogInfo << "---- test_initializeSingleton ended (" << QTest::currentDataTag() << ")    ----";
}

///////////////////////////////////////////////////////////////////////////////


void TstAVTimeReference::test_currentDateTime()
{
    AVLogInfo << "---- test_currentDateTime launched (" << QTest::currentDataTag() << ") ----";

    QDateTime system_time = AVDateTime::currentDateTimeUtc();
    AVUnitTestTimeReferenceHolder::setAVTimeReferenceSystemTimeOverride(system_time);

    AVTimeReference::singleton().m_time_reference_holder->setTimeReferenceParameters(system_time.addMSecs(-1000), 2., 56);

    QCOMPARE(AVTimeReference::currentDateTime().msecsTo(system_time.addMSecs(1056)), 0);
    //see TstAVTimeReferenceHolder for detailed tests

    AVLogInfo << "---- test_currentDateTime ended (" << QTest::currentDataTag() << ")    ----";
}

///////////////////////////////////////////////////////////////////////////////


void TstAVTimeReference::test_getSpeed()
{
    AVLogInfo << "---- test_getSpeed launched (" << QTest::currentDataTag() << ") ----";

    QDateTime system_time = AVDateTime::currentDateTimeUtc();
    QCOMPARE(AVTimeReference::getSpeed(), 1.);

    AVTimeReference::singleton().m_time_reference_holder->setTimeReferenceParameters(system_time.addMSecs(-1000), 2., 56);

    QCOMPARE(AVTimeReference::getSpeed(), 2.);

    AVLogInfo << "---- test_getSpeed ended (" << QTest::currentDataTag() << ")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVTimeReference::test_getDateTimeOffsetMs()
{
    AVLogInfo << "---- test_getDateTimeOffsetMs launched (" << QTest::currentDataTag() << ") ----";

    QDateTime system_time = AVDateTime::currentDateTimeUtc();
    AVUnitTestTimeReferenceHolder::setAVTimeReferenceSystemTimeOverride(system_time);
    QCOMPARE(AVTimeReference::getDateTimeOffsetMs(), 0);

    AVTimeReference::singleton().m_time_reference_holder->setTimeReferenceParameters(system_time.addMSecs(-1000), 2., 560);

    QCOMPARE(AVTimeReference::getDateTimeOffsetMs()- 1560, 0);

    AVLogInfo << "---- test_getDateTimeOffsetMs ended (" << QTest::currentDataTag() << ")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVTimeReference::test_setSpeed_multiple()
{
    QDateTime system_time = AVDateTime::currentDateTimeUtc();
    AVUnitTestTimeReferenceHolder::setAVTimeReferenceSystemTimeOverride(system_time);

    // system time and time reference shall be the same = START
    QCOMPARE(AVTimeReference::currentDateTime().msecsTo(system_time), 0);

    AVTimeReferenceAdapterLocal adapter;
    adapter.setCurrentDateTime(system_time);
    adapter.setSpeed(2.);

    system_time = system_time.addMSecs(500);
    AVUnitTestTimeReferenceHolder::setAVTimeReferenceSystemTimeOverride(system_time);

    // system time = START + 500
    // time reference = START + 2*500
    // delta = time reference - system time = 500 (note: the delta does not include logging etc.)
    QDateTime current_time = AVTimeReference::currentDateTime();

    AVLogInfo << "TstAVTimeReference::test_setSpeed_multiple: system: "<< system_time<<", current "<<current_time;

    QCOMPARE(current_time.msecsTo(system_time) + 500, 0);

    adapter.setSpeed(0.5);

    // system time = (START + 500) + epsilon
    // time reference = (START + 2*500) + 0.5*epsilon
    // delta = time reference - system time = (500) - 0.5*epsilon

    QCOMPARE(current_time.msecsTo(system_time) + 500, 0);

    system_time = system_time.addMSecs(500);
    AVUnitTestTimeReferenceHolder::setAVTimeReferenceSystemTimeOverride(system_time);

    // system time = (START + 500 + epsilon) + 500
    // time reference = (START + 2*500 + 0.5*epsilon) + 0.5*500
    // delta = time reference - system time = (500 - 0.5*epsilon) - 0.5*500 = 250 - 0.5*epsilon
    current_time = AVTimeReference::currentDateTime();

    QCOMPARE(current_time.msecsTo(system_time) + 250, 0);

    adapter.setSpeed(0.);

    QCOMPARE(current_time.msecsTo(system_time) + 250, 0);

    system_time = system_time.addMSecs(500);
    AVUnitTestTimeReferenceHolder::setAVTimeReferenceSystemTimeOverride(system_time);

    // system time = (START + 500 + epsilon + 500) + 500
    // time reference = (START + 2*500 + 0.5*epsilon + 0.5*500) + 0*500
    // delta = time reference - system time = (500 - 0.5*epsilon - 0.5*500) - 500 = -250 - 0.5*epsilon
    current_time = AVTimeReference::currentDateTime();

    QCOMPARE(current_time.msecsTo(system_time) - 250, 0);

}

///////////////////////////////////////////////////////////////////////////////

AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(TstAVTimeReference,"avlib/unittests/config")
#include "tstavtimereference.moc"

// End of file
