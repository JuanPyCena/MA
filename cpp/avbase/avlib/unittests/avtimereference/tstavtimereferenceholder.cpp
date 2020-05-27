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
    \brief   Function level test cases for AVTimeReferenceHolder
 */

#include <QtTest>
#include <avunittestmain.h>
#include "avtimereference/av_unittest_timereferenceholder.h"

class TstAVTimeReferenceHolder : public QObject
{
    Q_OBJECT

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
    void test_AVTimeReferenceHolder();
    void test_currentDateTime();
    void test_calculateFromSystemTime();
    void test_setTimeReferenceParameters();
    void test_setTimeReferenceParametersEqualSysTime();
    void test_setSpeed();
    void test_setDateTime();
    void test_setDateTimeSpeed();
    void test_reset();
    void test_setDateTimeShiftedOnly();
    void test_startNewTimeReferenceSession();
    void test_setDifferentReferenceTimeThanSystemTime();

};

///////////////////////////////////////////////////////////////////////////////

//! Declaration of unknown metatypes
// Q_DECLARE_METATYPE(long long)

///////////////////////////////////////////////////////////////////////////////

//! This will be called before the first test function is executed.
//! It is only called once.
void TstAVTimeReferenceHolder::initTestCase()
{

}

///////////////////////////////////////////////////////////////////////////////

//! This will be called after the last test function is executed.
//! It is only called once.
void TstAVTimeReferenceHolder::cleanupTestCase()
{

}

///////////////////////////////////////////////////////////////////////////////

//! This will be called before each test function is executed.
void TstAVTimeReferenceHolder::init()
{

}

///////////////////////////////////////////////////////////////////////////////

//! This will be called after every test function.
void TstAVTimeReferenceHolder::cleanup()
{

}

///////////////////////////////////////////////////////////////////////////////

void TstAVTimeReferenceHolder::test_AVTimeReferenceHolder()
{
    AVLogInfo << "---- test_AVTimeReferenceHolder launched (" << QTest::currentDataTag() << ") ----";

    AVTimeReferenceHolder instance;
    QCOMPARE(instance.m_speed, 1.);
    QCOMPARE(instance.m_offset, 0);
    QVERIFY(instance.m_reference_date_time.isNull());

    AVLogInfo << "---- test_AVTimeReferenceHolder ended (" << QTest::currentDataTag() << ")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVTimeReferenceHolder::test_currentDateTime()
{
    AVLogInfo << "---- test_currentDateTime launched (" << QTest::currentDataTag() << ") ----";

    QDateTime system_time = AVDateTime::currentDateTimeUtc();

    AVUnitTestTimeReferenceHolder instance;
    instance.setSystemTimeOverride(system_time);
    instance.m_offset = 300;

    QCOMPARE(instance.currentDateTime().msecsTo(system_time.addMSecs(300)), 0);
    //see test_calculateFromSystemTime for detailed tests

    AVLogInfo << "---- test_currentDateTime ended (" << QTest::currentDataTag() << ")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVTimeReferenceHolder::test_calculateFromSystemTime()
{
    AVLogInfo << "---- test_calculateFromSystemTime launched (" << QTest::currentDataTag() << ") ----";

    QDateTime system_time = AVDateTime::currentDateTimeUtc();

    AVTimeReferenceHolder instance;

    QCOMPARE(instance.calculateFromSystemTime(system_time), system_time);

    instance.m_offset = 300;

    QCOMPARE(instance.calculateFromSystemTime(system_time), system_time.addMSecs(300));

    instance.m_offset = 0;
    instance.m_reference_date_time = system_time.addMSecs(-1000);

    QCOMPARE(instance.calculateFromSystemTime(system_time), system_time);

    instance.m_speed = 3.0;

    QCOMPARE(instance.calculateFromSystemTime(system_time), system_time.addMSecs(2000));

    instance.m_offset = -700;

    QCOMPARE(instance.calculateFromSystemTime(system_time), system_time.addMSecs(1300));

    AVLogInfo << "---- test_calculateFromSystemTime ended (" << QTest::currentDataTag() << ")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVTimeReferenceHolder::test_setTimeReferenceParameters()
{
    AVLogInfo << "---- test_setTimeReferenceParameters launched (" << QTest::currentDataTag() << ") ----";

    QDateTime system_time = AVDateTime::currentDateTimeUtc();

    AVTimeReferenceHolder instance;
    QSignalSpy spy_jump(&instance, SIGNAL(signalTimeJumped(const QDateTime&, const QDateTime&)));
    QSignalSpy spy_speed(&instance, SIGNAL(signalSpeedChanged(double, double)));
    QSignalSpy spy_shift(&instance, SIGNAL(signalTimeShiftedOnly(const QDateTime&, const QDateTime&)));

    instance.setTimeReferenceParameters(AVDateTime(), 1.0, 0);
    QVERIFY(instance.m_reference_date_time.isNull());
    QCOMPARE(instance.m_reference_date_time, AVDateTime());
    QCOMPARE(instance.m_speed, 1.0);
    QCOMPARE(instance.m_offset, 0);

    QCOMPARE(spy_jump.count(), 0);
    QCOMPARE(spy_speed.count(), 0);
    QCOMPARE(spy_shift.count(), 0);

    instance.setTimeReferenceParameters(system_time, 2.0, 0);
    QVERIFY(!instance.m_reference_date_time.isNull());
    QCOMPARE(instance.m_reference_date_time, AVDateTime(system_time));
    QCOMPARE(instance.m_speed, 2.0);
    QCOMPARE(instance.m_offset, 0);

    QCOMPARE(spy_jump.count(), 1);
    QCOMPARE(spy_speed.count(), 1);
    QCOMPARE(spy_shift.count(), 0);

    instance.setTimeReferenceParameters(system_time, 2.0, 10);
    QVERIFY(!instance.m_reference_date_time.isNull());
    QCOMPARE(instance.m_reference_date_time, AVDateTime(system_time));
    QCOMPARE(instance.m_speed, 2.0);
    QCOMPARE(instance.m_offset, 10);

    QCOMPARE(spy_jump.count(), 1);
    QCOMPARE(spy_speed.count(), 1);
    QCOMPARE(spy_shift.count(), 1);

    instance.setTimeReferenceParameters(system_time.addMSecs(-100), 2.3, 12);
    QVERIFY(!instance.m_reference_date_time.isNull());
    QCOMPARE(instance.m_reference_date_time, AVDateTime(system_time.addMSecs(-100)));
    QCOMPARE(instance.m_speed, 2.3);
    QCOMPARE(instance.m_offset, 12);

    QCOMPARE(spy_jump.count(), 2);
    QCOMPARE(spy_speed.count(), 2);
    QCOMPARE(spy_shift.count(), 1);

    instance.setTimeReferenceParameters(system_time.addMSecs(-100), 2.3, 160);
    QVERIFY(!instance.m_reference_date_time.isNull());
    QCOMPARE(instance.m_reference_date_time, AVDateTime(system_time.addMSecs(-100)));
    QCOMPARE(instance.m_speed, 2.3);
    QCOMPARE(instance.m_offset, 160);

    QCOMPARE(spy_jump.count(), 2);
    QCOMPARE(spy_speed.count(), 2);
    QCOMPARE(spy_shift.count(), 2);

    instance.setTimeReferenceParameters(system_time.addMSecs(-120), 2.3, 160);
    QVERIFY(!instance.m_reference_date_time.isNull());
    QCOMPARE(instance.m_reference_date_time, AVDateTime(system_time.addMSecs(-120)));
    QCOMPARE(instance.m_speed, 2.3);
    QCOMPARE(instance.m_offset, 160);

    QCOMPARE(spy_jump.count(), 3);
    QCOMPARE(spy_speed.count(), 2);
    QCOMPARE(spy_shift.count(), 2);

    instance.setTimeReferenceParameters(AVDateTime(), 1., 0);
    QVERIFY(instance.m_reference_date_time.isNull());
    QCOMPARE(instance.m_reference_date_time, AVDateTime());
    QCOMPARE(instance.m_speed, 1.);
    QCOMPARE(instance.m_offset, 0);

    QCOMPARE(spy_jump.count(), 4);
    QCOMPARE(spy_speed.count(), 3);
    QCOMPARE(spy_shift.count(), 2);

    instance.setTimeReferenceParameters(system_time, 1., -126150324765);
    QVERIFY(!instance.m_reference_date_time.isNull());
    QCOMPARE(instance.m_reference_date_time, AVDateTime(system_time));
    QCOMPARE(instance.m_speed, 1.);
    QCOMPARE(instance.m_offset, static_cast<qint64>(-126150324765));

    QCOMPARE(spy_jump.count(), 5);
    QCOMPARE(spy_speed.count(), 3);
    QCOMPARE(spy_shift.count(), 2);

    instance.setTimeReferenceParameters(AVDateTime(), 1., 0);
    QVERIFY(instance.m_reference_date_time.isNull());
    QCOMPARE(instance.m_reference_date_time, AVDateTime());
    QCOMPARE(instance.m_speed, 1.);
    QCOMPARE(instance.m_offset, 0);

    QCOMPARE(spy_jump.count(), 6);
    QCOMPARE(spy_speed.count(), 3);
    QCOMPARE(spy_shift.count(), 2);

    instance.setTimeReferenceParameters(AVDateTime(), 1., -100000043);
    QVERIFY(instance.m_reference_date_time.isNull());
    QCOMPARE(instance.m_reference_date_time, AVDateTime());
    QCOMPARE(instance.m_speed, 1.);
    QCOMPARE(instance.m_offset, -100000043);

    QCOMPARE(spy_jump.count(), 6);
    QCOMPARE(spy_speed.count(), 3);
    QCOMPARE(spy_shift.count(), 3);

    AVLogInfo << "---- test_setTimeReferenceParameters ended (" << QTest::currentDataTag() << ")    ----";
}


///////////////////////////////////////////////////////////////////////////////

void TstAVTimeReferenceHolder::test_setTimeReferenceParametersEqualSysTime()
{
    AVLogInfo << "---- test_setTimeReferenceParametersEqualSysTime launched (" << QTest::currentDataTag() << ") ----";

    QDateTime system_time = AVDateTime::currentDateTimeUtc();

    AVUnitTestTimeReferenceHolder instance_client;
    instance_client.setSystemTimeOverride(system_time);

    AVUnitTestTimeReferenceHolder instance_server;
    instance_server.setSystemTimeOverride(system_time);

    QSignalSpy spy_jumpServer(&instance_server, SIGNAL(signalTimeJumped(const QDateTime&, const QDateTime&)));
    QSignalSpy spy_speedServer(&instance_server, SIGNAL(signalSpeedChanged(double, double)));
    QSignalSpy spy_shiftServer(&instance_server, SIGNAL(signalTimeShiftedOnly(const QDateTime&, const QDateTime&)));

    QSignalSpy spy_jumpClient(&instance_client, SIGNAL(signalTimeJumped(const QDateTime&, const QDateTime&)));
    QSignalSpy spy_speedClient(&instance_client, SIGNAL(signalSpeedChanged(double, double)));
    QSignalSpy spy_shiftClient(&instance_client, SIGNAL(signalTimeShiftedOnly(const QDateTime&, const QDateTime&)));

    instance_server.setDateTimeSpeed(system_time, 1.0);
    instance_client.setDateTimeSpeed(system_time, 1.0);

    QCOMPARE(spy_jumpClient.count(), 1);
    QCOMPARE(spy_speedClient.count(), 0);
    QCOMPARE(spy_shiftClient.count(), 0);

    QCOMPARE(spy_jumpServer.count(), 1);
    QCOMPARE(spy_speedServer.count(), 0);
    QCOMPARE(spy_shiftServer.count(), 0);

    instance_server.startNewTimeReferenceSession();
    QDateTime referenceDateTime = instance_server.getReferenceDateTime();
    instance_client.setTimeReferenceParameters(referenceDateTime, 1.0, 0);

    QCOMPARE(spy_jumpClient.count(), 2);
    QCOMPARE(spy_speedClient.count(), 0);
    QCOMPARE(spy_shiftClient.count(), 0);

    QCOMPARE(spy_jumpServer.count(), 2);
    QCOMPARE(spy_speedServer.count(), 0);
    QCOMPARE(spy_shiftServer.count(), 0);

    AVLogInfo << "---- test_setTimeReferenceParametersEqualSysTime ended (" << QTest::currentDataTag() << ")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVTimeReferenceHolder::test_setSpeed()
{
    AVLogInfo << "---- test_setSpeed launched (" << QTest::currentDataTag() << ") ----";

    QDateTime system_time = AVDateTime::currentDateTimeUtc();

    AVUnitTestTimeReferenceHolder instance;
    instance.setSystemTimeOverride(system_time);

    QSignalSpy spy_jump(&instance, SIGNAL(signalTimeJumped(const QDateTime&, const QDateTime&)));
    QSignalSpy spy_speed(&instance, SIGNAL(signalSpeedChanged(double, double)));
    QSignalSpy spy_shift(&instance, SIGNAL(signalTimeShiftedOnly(const QDateTime&, const QDateTime&)));

    QDateTime time_before = instance.calculateFromSystemTime(system_time);
    instance.setSpeed(1.);
    QDateTime time_after = instance.calculateFromSystemTime(system_time);
    QCOMPARE(instance.m_reference_date_time, AVDateTime());
    QCOMPARE(instance.m_speed, 1.0);
    QCOMPARE(instance.m_offset, 0);
    QCOMPARE(time_before.msecsTo(time_after), 0);

    QCOMPARE(spy_jump.count(), 0);
    QCOMPARE(spy_speed.count(), 0);
    QCOMPARE(spy_shift.count(), 0);

    instance.m_reference_date_time = system_time;
    instance.m_speed = 1.;
    instance.m_offset = 0.;

    time_before = instance.calculateFromSystemTime(system_time);
    instance.setSpeed(1.2);
    time_after = instance.calculateFromSystemTime(system_time);
    QCOMPARE(instance.m_reference_date_time.msecsTo(system_time), 0);
    QCOMPARE(instance.m_speed, 1.2);
    QCOMPARE(instance.m_offset, 0);
    QCOMPARE(time_before.msecsTo(time_after), 0);

    QCOMPARE(spy_jump.count(), 0);
    QCOMPARE(spy_speed.count(), 1);
    QCOMPARE(spy_shift.count(), 0);

    instance.m_reference_date_time = system_time.addMSecs(-1000);
    instance.m_speed = 2.5;
    instance.m_offset = 330;

    time_before = instance.calculateFromSystemTime(system_time);
    instance.setSpeed(2.0);
    time_after = instance.calculateFromSystemTime(system_time);
    QCOMPARE(instance.m_reference_date_time.msecsTo(system_time.addMSecs(-1000)), 0);
    QCOMPARE(instance.m_speed, 2.0);
    QCOMPARE(instance.m_offset - 830, 0);
    QCOMPARE(time_before.msecsTo(time_after), 0);

    QCOMPARE(spy_jump.count(), 0);
    QCOMPARE(spy_speed.count(), 2);
    QCOMPARE(spy_shift.count(), 0);

    AVLogInfo << "---- test_setSpeed ended (" << QTest::currentDataTag() << ")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVTimeReferenceHolder::test_setDateTime()
{
    AVLogInfo << "---- test_setDateTime launched (" << QTest::currentDataTag() << ") ----";

    QDateTime system_time = AVDateTime::currentDateTimeUtc();

    AVUnitTestTimeReferenceHolder instance;
    instance.setSystemTimeOverride(system_time);

    QSignalSpy spy_jump(&instance, SIGNAL(signalTimeJumped(const QDateTime&, const QDateTime&)));
    QSignalSpy spy_speed(&instance, SIGNAL(signalSpeedChanged(double, double)));
    QSignalSpy spy_shift(&instance, SIGNAL(signalTimeShiftedOnly(const QDateTime&, const QDateTime&)));

    instance.setDateTime(system_time);
    QCOMPARE(instance.m_reference_date_time.msecsTo(system_time), 0);
    QCOMPARE(instance.m_speed, 1.0);
    QCOMPARE(instance.m_offset - 0, 0);

    QCOMPARE(spy_jump.count(), 1);
    QCOMPARE(spy_speed.count(), 0);
    QCOMPARE(spy_shift.count(), 0);

    instance.setDateTime(system_time.addMSecs(300));
    QCOMPARE(instance.m_reference_date_time.msecsTo(system_time), 0 - 1); // 1 ms offset results from AVTimeReferenceHolder::updateReferenceDateTime(...)
    QCOMPARE(instance.m_speed, 1.0);
    QCOMPARE(instance.m_offset - 300, 0); // 1 ms offset results from AVTimeReferenceHolder::updateReferenceDateTime(...)

    QCOMPARE(spy_jump.count(), 2);
    QCOMPARE(spy_speed.count(), 0);
    QCOMPARE(spy_shift.count(), 0);

    instance.m_speed = 2.1;

    instance.setDateTime(system_time.addMSecs(500));
    QCOMPARE(instance.m_reference_date_time.msecsTo(system_time), 0 - 2); // another 1 ms offset from AVTimeReferenceHolder::updateReferenceDateTime(...)
    QCOMPARE(instance.m_speed, 2.1);
    QCOMPARE(instance.m_offset - 500, 2); // another 1 ms offset from AVTimeReferenceHolder::updateReferenceDateTime(...)

    QCOMPARE(spy_jump.count(), 3);
    QCOMPARE(spy_speed.count(), 0);
    QCOMPARE(spy_shift.count(), 0);

    instance.m_reference_date_time = system_time.addMSecs(-1000);
    instance.m_speed = 2.5;
    instance.m_offset = 330;

    instance.setDateTime(system_time.addMSecs(220));
    QCOMPARE(instance.m_reference_date_time.msecsTo(system_time), 0);
    QCOMPARE(instance.m_speed, 2.5);
    QCOMPARE(instance.m_offset - 220, 0);

    QCOMPARE(spy_jump.count(), 4);
    QCOMPARE(spy_speed.count(), 0);
    QCOMPARE(spy_shift.count(), 0);

    AVLogInfo << "---- test_setDateTime ended (" << QTest::currentDataTag() << ")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVTimeReferenceHolder::test_setDateTimeSpeed()
{
    AVLogInfo << "---- test_setDateTimeSpeed launched (" << QTest::currentDataTag() << ") ----";

    QDateTime system_time = AVDateTime::currentDateTimeUtc();

    AVUnitTestTimeReferenceHolder instance;
    instance.setSystemTimeOverride(system_time);

    QSignalSpy spy_jump(&instance, SIGNAL(signalTimeJumped(const QDateTime&, const QDateTime&)));
    QSignalSpy spy_speed(&instance, SIGNAL(signalSpeedChanged(double, double)));
    QSignalSpy spy_shift(&instance, SIGNAL(signalTimeShiftedOnly(const QDateTime&, const QDateTime&)));

    instance.setDateTimeSpeed(system_time, 1.);
    QCOMPARE(instance.m_reference_date_time.msecsTo(system_time), 0);
    QCOMPARE(instance.m_speed, 1.0);
    QCOMPARE(instance.m_offset - 0, 0);

    QCOMPARE(spy_jump.count(), 1);
    QCOMPARE(spy_speed.count(), 0);
    QCOMPARE(spy_shift.count(), 0);

    instance.setDateTimeSpeed(system_time.addMSecs(300), 1.);
    QCOMPARE(instance.m_reference_date_time.msecsTo(system_time), 0 - 1); // 1 ms offset results from AVTimeReferenceHolder::updateReferenceDateTime(...)
    QCOMPARE(instance.m_speed, 1.0);
    QCOMPARE(instance.m_offset - 300, 0); // offset calculation takes 1ms difference in m_reference_date_time into account

    QCOMPARE(spy_jump.count(), 2);
    QCOMPARE(spy_speed.count(), 0);
    QCOMPARE(spy_shift.count(), 0);

    instance.m_speed = 2.1;

    instance.setDateTimeSpeed(system_time.addMSecs(500), 2.5);
    QCOMPARE(instance.m_reference_date_time.msecsTo(system_time), 0 - 2); // another 1 ms offset from AVTimeReferenceHolder::updateReferenceDateTime(...)
    QCOMPARE(instance.m_speed, 2.5);
    QCOMPARE(instance.m_offset - 500, 3); // offset calculation takes 1ms difference in m_reference_date_time into account

    QCOMPARE(spy_jump.count(), 3);
    QCOMPARE(spy_speed.count(), 1);
    QCOMPARE(spy_shift.count(), 0);

    instance.m_reference_date_time = system_time.addMSecs(-1000);
    instance.m_speed = 2.5;
    instance.m_offset = 330;

    instance.setDateTimeSpeed(system_time.addMSecs(220), 4.2);
    QCOMPARE(instance.m_reference_date_time.msecsTo(system_time), 0);
    QCOMPARE(instance.m_speed, 4.2);
    QCOMPARE(instance.m_offset - 220, 0);

    QCOMPARE(spy_jump.count(), 4);
    QCOMPARE(spy_speed.count(), 2);
    QCOMPARE(spy_shift.count(), 0);

    AVLogInfo << "---- test_setDateTimeSpeed ended (" << QTest::currentDataTag() << ")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVTimeReferenceHolder::test_reset()
{
    AVLogInfo << "---- test_reset launched (" << QTest::currentDataTag() << ") ----";

    QDateTime system_time = AVDateTime::currentDateTimeUtc();

    AVTimeReferenceHolder instance;
    QSignalSpy spy_jump(&instance, SIGNAL(signalTimeJumped(const QDateTime&, const QDateTime&)));
    QSignalSpy spy_speed(&instance, SIGNAL(signalSpeedChanged(double, double)));
    QSignalSpy spy_shift(&instance, SIGNAL(signalTimeShiftedOnly(const QDateTime&, const QDateTime&)));

    instance.resetToSystemTime();
    QCOMPARE(instance.m_reference_date_time, AVDateTime());
    QCOMPARE(instance.m_speed, 1.0);
    QCOMPARE(instance.m_offset, 0);

    QCOMPARE(spy_jump.count(), 0);
    QCOMPARE(spy_speed.count(), 0);
    QCOMPARE(spy_shift.count(), 0);

    instance.m_offset = 100;

    instance.resetToSystemTime();
    QCOMPARE(instance.m_reference_date_time, AVDateTime());
    QCOMPARE(instance.m_speed, 1.0);
    QCOMPARE(instance.m_offset, 0);

    QCOMPARE(spy_jump.count(), 0);
    QCOMPARE(spy_speed.count(), 0);
    QCOMPARE(spy_shift.count(), 1);

    instance.m_offset = 100;
    instance.m_speed = 3.;
    instance.m_reference_date_time = system_time.addMSecs(-1000);

    instance.resetToSystemTime();
    QCOMPARE(instance.m_reference_date_time, AVDateTime());
    QCOMPARE(instance.m_speed, 1.0);
    QCOMPARE(instance.m_offset, 0);

    QCOMPARE(spy_jump.count(), 1);
    QCOMPARE(spy_speed.count(), 1);
    QCOMPARE(spy_shift.count(), 1);

    AVLogInfo << "---- test_reset ended (" << QTest::currentDataTag() << ")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVTimeReferenceHolder::test_setDateTimeShiftedOnly()
{
    AVLogInfo << "---- test_setDateTimeShiftedOnly launched (" << QTest::currentDataTag() << ") ----";

    QDateTime system_time = AVDateTime::currentDateTimeUtc();
    AVUnitTestTimeReferenceHolder instance;
    instance.setSystemTimeOverride(system_time);

    QSignalSpy spy_jump(&instance, SIGNAL(signalTimeJumped(const QDateTime&, const QDateTime&)));
    QSignalSpy spy_speed(&instance, SIGNAL(signalSpeedChanged(double, double)));
    QSignalSpy spy_shift(&instance, SIGNAL(signalTimeShiftedOnly(const QDateTime&, const QDateTime&)));

    instance.setDateTime(system_time);
    QCOMPARE(instance.m_reference_date_time.msecsTo(system_time), 0);
    QCOMPARE(instance.m_speed, 1.0);
    QVERIFY(qAbs(instance.m_offset - 0) <3);

    QCOMPARE(spy_jump.count(), 1);
    QCOMPARE(spy_speed.count(), 0);
    QCOMPARE(spy_shift.count(), 0);

    instance.setDateTimeShiftedOnly(system_time.addMSecs(300));
    QCOMPARE(instance.m_reference_date_time.msecsTo(system_time), 0);
    QCOMPARE(instance.m_speed, 1.0);
    QCOMPARE(instance.m_offset - 300, 0);
    QCOMPARE(system_time.addMSecs(300).msecsTo(instance.calculateFromSystemTime(system_time)), 0);

    QCOMPARE(spy_jump.count(), 1);
    QCOMPARE(spy_speed.count(), 0);
    QCOMPARE(spy_shift.count(), 1);

    instance.m_reference_date_time = system_time.addMSecs(-1000);
    instance.m_speed = 2.5;
    instance.m_offset = 330;

    instance.setDateTimeShiftedOnly(system_time.addMSecs(220));
    QVERIFY(!instance.m_reference_date_time.isNull());
    QCOMPARE(instance.m_reference_date_time.msecsTo(system_time.addMSecs(-1000)), 0);
    QCOMPARE(instance.m_speed, 2.5);
    QCOMPARE(instance.m_offset - (1000-2500+220), 0);
    QCOMPARE(system_time.addMSecs(220).msecsTo(instance.calculateFromSystemTime(system_time)) ,0);

    QCOMPARE(spy_jump.count(), 1);
    QCOMPARE(spy_speed.count(), 0);
    QCOMPARE(spy_shift.count(), 2);

    AVLogInfo << "---- test_setDateTimeShiftedOnly ended (" << QTest::currentDataTag() << ")    ----";
}


///////////////////////////////////////////////////////////////////////////////

void TstAVTimeReferenceHolder::test_startNewTimeReferenceSession()
{
    AVLogInfo << "---- test_startNewTimeReferenceSession launched (" << QTest::currentDataTag() << ") ----";

    QDateTime system_time = AVDateTime::currentDateTimeUtc();

    AVUnitTestTimeReferenceHolder instance;
    instance.setSystemTimeOverride(system_time);

    QSignalSpy spy_jump(&instance, SIGNAL(signalTimeJumped(const QDateTime&, const QDateTime&)));
    QSignalSpy spy_speed(&instance, SIGNAL(signalSpeedChanged(double, double)));
    QSignalSpy spy_shift(&instance, SIGNAL(signalTimeShiftedOnly(const QDateTime&, const QDateTime&)));

    QDateTime time_before = instance.calculateFromSystemTime(system_time);
    instance.startNewTimeReferenceSession();
    QDateTime time_after = instance.calculateFromSystemTime(system_time);

    QCOMPARE(instance.m_reference_date_time.msecsTo(system_time), 0);
    QCOMPARE(instance.m_speed, 1.0);
    QCOMPARE(instance.m_offset - 0, 0);
    QCOMPARE(time_before.msecsTo(time_after), 0);

    QCOMPARE(spy_jump.count(), 1);
    QCOMPARE(spy_speed.count(), 0);
    QCOMPARE(spy_shift.count(), 0);

    instance.m_reference_date_time = system_time.addMSecs(-1000);
    instance.m_speed = 2.5;
    instance.m_offset = 330;

    time_before = instance.calculateFromSystemTime(system_time);
    instance.startNewTimeReferenceSession();
    time_after = instance.calculateFromSystemTime(system_time);

    QCOMPARE(instance.m_reference_date_time.msecsTo(system_time), 0);
    QCOMPARE(instance.m_speed, 2.5);
    QCOMPARE(instance.m_offset - 1830, 0);
    QCOMPARE(time_before.msecsTo(time_after), 0);

    QCOMPARE(spy_jump.count(), 2);
    QCOMPARE(spy_speed.count(), 0);
    QCOMPARE(spy_shift.count(), 0);


    AVLogInfo << "---- test_startNewTimeReferenceSession ended (" << QTest::currentDataTag() << ")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVTimeReferenceHolder::test_setDifferentReferenceTimeThanSystemTime()
{
    AVLogInfo << "---- test_setDifferentReferenceTimeThanSystemTime launched (" << QTest::currentDataTag() << ") ----";

    QString now_system_time_str = "2018-01-01 12:00:00";
    QDateTime now_system_time = AVParseDateTimeHuman(now_system_time_str);
    QVERIFY(now_system_time.isValid());

    AVUnitTestTimeReferenceHolder instance;
    instance.setSystemTimeOverride(now_system_time);

    instance.setDateTime(now_system_time);
    QCOMPARE(instance.m_reference_date_time.msecsTo(now_system_time), 0);
    QCOMPARE(instance.m_speed, 1.0);
    QCOMPARE(instance.m_offset - 0, 0);

    QString past_system_time_str = "2018-01-01 11:59:00";
    QDateTime past_system_time = AVParseDateTimeHuman(past_system_time_str);
    QVERIFY(past_system_time.isValid());
    instance.setSystemTimeOverride(past_system_time);

    instance.setDateTime(now_system_time.addMSecs(500));

    // m_reference_date_time is not equal and ahead of past_system_time
    QCOMPARE(instance.m_reference_date_time.msecsTo(past_system_time), -60001ll);
    QCOMPARE(instance.m_speed, 1.0);
    QCOMPARE(instance.m_offset - 0, 60500ll);

    QCOMPARE(instance.currentDateTime().msecsTo(now_system_time.addMSecs(500)), 0);

    AVLogInfo << "---- test_setDifferentReferenceTimeThanSystemTime ended (" << QTest::currentDataTag() << ")    ----";
}

///////////////////////////////////////////////////////////////////////////////

AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(TstAVTimeReferenceHolder,"avlib/unittests/config")
#include "tstavtimereferenceholder.moc"

// End of file
