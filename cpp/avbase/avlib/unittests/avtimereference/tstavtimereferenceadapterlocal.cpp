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
    \brief   Function level test cases for AVTimeReferenceAdapterLocal
 */

#include <QtTest>
#include <avunittestmain.h>
#include "avtimereference/avtimereferenceadapterlocal.h"
#include "avtimereference/avtimereferenceholder.h"

#include "avtimereference/av_unittest_timereferenceholder.h"

class TstAVTimeReferenceAdapterLocal : public QObject
{
    Q_OBJECT

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

public:
    //! This is only for rounding errors, not to shadow timing problems
    static const int TOLERANCE = 2;

private slots:
    void test_AVTimeReferenceAdapterLocal();
    void test_setSpeed();
    void test_setCurrentDateTime();
    void test_setCurrentDateTimeSpeed();
    void test_reset();

};

///////////////////////////////////////////////////////////////////////////////

//! Declaration of unknown metatypes

///////////////////////////////////////////////////////////////////////////////

//! This will be called before the first test function is executed.
//! It is only called once.
void TstAVTimeReferenceAdapterLocal::initTestCase()
{

}

///////////////////////////////////////////////////////////////////////////////

//! This will be called after the last test function is executed.
//! It is only called once.
void TstAVTimeReferenceAdapterLocal::cleanupTestCase()
{

}

///////////////////////////////////////////////////////////////////////////////

//! This will be called before each test function is executed.
void TstAVTimeReferenceAdapterLocal::init()
{

}

///////////////////////////////////////////////////////////////////////////////

//! This will be called after every test function.
void TstAVTimeReferenceAdapterLocal::cleanup()
{
    AVTimeReferenceAdapterLocal instance;
    instance.resetProcess();
}

///////////////////////////////////////////////////////////////////////////////

void TstAVTimeReferenceAdapterLocal::test_AVTimeReferenceAdapterLocal()
{
    AVLogInfo << "---- test_AVTimeReferenceAdapterLocal launched (" << QTest::currentDataTag() << ") ----";

    AVTimeReferenceAdapterLocal instance;

    AVLogInfo << "---- test_AVTimeReferenceAdapterLocal ended (" << QTest::currentDataTag() << ")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVTimeReferenceAdapterLocal::test_setSpeed()
{
    AVLogInfo << "---- test_setSpeed launched (" << QTest::currentDataTag() << ") ----";

    double speed = 1.23;

    AVTimeReferenceAdapterLocal instance;
    instance.setCurrentDateTime(AVDateTime::currentDateTimeUtc());
    instance.setSpeed(1.23);

    QCOMPARE(instance.getLocalTimeReferenceHolder().getSpeed(), speed);

    AVLogInfo << "---- test_setSpeed ended (" << QTest::currentDataTag() << ")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVTimeReferenceAdapterLocal::test_setCurrentDateTime()
{
    AVLogInfo << "---- test_setCurrentDateTime launched (" << QTest::currentDataTag() << ") ----";

    QDateTime system_time = AVDateTime::currentDateTimeUtc();
    AVUnitTestTimeReferenceHolder::setAVTimeReferenceSystemTimeOverride(system_time);

    QDateTime sim_time = system_time.addDays(2);

    AVTimeReferenceAdapterLocal instance;

    instance.setCurrentDateTime(sim_time);

    QVERIFY(qAbs(instance.getLocalTimeReferenceHolder().calculateFromSystemTime(system_time).msecsTo(sim_time)) < TOLERANCE);

    AVLogInfo << "---- test_setCurrentDateTime ended (" << QTest::currentDataTag() << ")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVTimeReferenceAdapterLocal::test_setCurrentDateTimeSpeed()
{
    AVLogInfo << "---- test_setCurrentDateTimeSpeed launched (" << QTest::currentDataTag() << ") ----";

    QDateTime system_time = AVDateTime::currentDateTimeUtc();
    AVUnitTestTimeReferenceHolder::setAVTimeReferenceSystemTimeOverride(system_time);

    QDateTime sim_time = system_time.addDays(2);
    double speed = 1.23;

    AVTimeReferenceAdapterLocal instance;

    instance.setCurrentDateTimeSpeed(sim_time, speed);

    QVERIFY(qAbs(instance.getLocalTimeReferenceHolder().calculateFromSystemTime(system_time).msecsTo(sim_time)) < TOLERANCE);
    QCOMPARE(instance.getLocalTimeReferenceHolder().getSpeed(), speed);

    AVLogInfo << "---- test_setCurrentDateTimeSpeed ended (" << QTest::currentDataTag() << ")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVTimeReferenceAdapterLocal::test_reset()
{
    AVLogInfo << "---- test_reset launched (" << QTest::currentDataTag() << ") ----";

    AVTimeReferenceAdapterLocal instance;

    instance.setCurrentDateTimeSpeed(AVDateTime::currentDateTimeUtc().addDays(2), 1.23);

    instance.reset();

    QDateTime system_time = AVDateTime::currentDateTimeUtc();

    QVERIFY(qAbs(instance.getLocalTimeReferenceHolder().calculateFromSystemTime(system_time).
                 msecsTo(system_time)) < TOLERANCE);
    QCOMPARE(instance.getLocalTimeReferenceHolder().getSpeed(), 1.);

    AVLogInfo << "---- test_reset ended (" << QTest::currentDataTag() << ")    ----";
}

///////////////////////////////////////////////////////////////////////////////

AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(TstAVTimeReferenceAdapterLocal,"avlib/unittests/config")
#include "tstavtimereferenceadapterlocal.moc"


// End of file
