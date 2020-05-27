///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2013
//
// Module:    TESTS - AVCOMMON Unit Tests
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   Function level test cases for AVTimer
 */

#include <QtTest>
#include <avunittestmain.h>
#include "avtimer.h"
#include "avtimereference.h"
#include "avtimereference/avtimereferenceadapter.h"
#include "avtimereference/avtimereferenceadapterlocal.h"

class TstAVTimer : public QObject
{
    Q_OBJECT

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
    void test_AVTimer();
    void test_isActive();
    void test_setInterval();
    void test_setSingleShot();
    void test_isSingleShot();
    void test_start();
    void test_stop();
    void test_slotInternalTimeout();
    void test_slotTimeJumped();
    void test_getInterval();
    void test_calculateScaledInterval();
    void test_timed_timeJumpForwardOutSpeed0();
    void test_remainingTime();

private:

    const qint64 m_tolerance        = 20;
};

///////////////////////////////////////////////////////////////////////////////

//! Declaration of unknown metatypes
// Q_DECLARE_METATYPE(QObject)
// Q_DECLARE_METATYPE(AVTimer)

///////////////////////////////////////////////////////////////////////////////

//! This will be called before the first test function is executed.
//! It is only called once.
void TstAVTimer::initTestCase()
{}

///////////////////////////////////////////////////////////////////////////////

//! This will be called after the last test function is executed.
//! It is only called once.
void TstAVTimer::cleanupTestCase()
{}

///////////////////////////////////////////////////////////////////////////////

//! This will be called before each test function is executed.
void TstAVTimer::init()
{
    QVERIFY(!AVTimeReferenceAdapter::isSingletonInitialized());
    AVTimeReferenceAdapter::setSingleton(new AVTimeReferenceAdapterLocal());
    AVTimeReferenceAdapter::singleton().startNewTimeReferenceSession();
    AVTimeReferenceAdapter::singleton().setSpeed(1);
}

///////////////////////////////////////////////////////////////////////////////

//! This will be called after every test function.
void TstAVTimer::cleanup()
{
    AVTimeReferenceAdapter::singleton().reset();
    AVTimeReferenceAdapter::deleteSingleton();
}

///////////////////////////////////////////////////////////////////////////////

void TstAVTimer::test_AVTimer()
{
    AVLogInfo << "---- test_AVTimer launched (" << QTest::currentDataTag() << ") ----";

    AVTimer instance;
    QVERIFY(!instance.m_timer.isActive());
    QVERIFY(!instance.m_timer.isSingleShot());
    QCOMPARE(instance.m_timer.timerType(), Qt::PreciseTimer);
    QCOMPARE(instance.m_speed, AVTimeReference::getSpeed());

    AVLogInfo << "---- test_AVTimer ended (" << QTest::currentDataTag() << ")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVTimer::test_isActive()
{
    AVLogInfo << "---- test_isActive launched (" << QTest::currentDataTag() << ") ----";

    AVTimer instance;
    QVERIFY(!instance.isActive());
    instance.start(100);
    QVERIFY(instance.isActive());

    AVLogInfo << "---- test_isActive ended (" << QTest::currentDataTag() << ")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVTimer::test_setInterval()
{
    QSKIP("race condition, SWE-4273");

    AVLogInfo << "---- test_setInterval launched (" << QTest::currentDataTag() << ") ----";

    AVTimer instance;
    QSignalSpy spy(&instance, SIGNAL(timeout()));
    instance.setInterval(100);
    QCOMPARE(instance.m_interval, 100);

    instance.start(100);

    runEventLoop(100 - m_tolerance);
    QCOMPARE(spy.size(), 0);

    runEventLoop(2 * m_tolerance);

    QCOMPARE(spy.size(), 1);

    spy.clear();
    QVERIFY(instance.isActive());
    instance.setInterval(200);

    runEventLoop(200 - m_tolerance/2);

    QCOMPARE(spy.size(), 0);

    runEventLoop(2 * m_tolerance);

    QCOMPARE(spy.size(), 1);

    AVLogInfo << "---- test_setInterval ended (" << QTest::currentDataTag() << ")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVTimer::test_setSingleShot()
{
    AVLogInfo << "---- test_setSingleShot launched (" << QTest::currentDataTag() << ") ----";

    AVTimer instance;
    instance.setSingleShot(true);
    QVERIFY(instance.m_timer.isSingleShot());

    AVLogInfo << "---- test_setSingleShot ended (" << QTest::currentDataTag() << ")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVTimer::test_isSingleShot()
{
    AVLogInfo << "---- test_isSingleShot launched (" << QTest::currentDataTag() << ") ----";

    AVTimer instance;
    QVERIFY(!instance.isSingleShot());
    instance.setSingleShot(true);
    QVERIFY(instance.isSingleShot());

    AVLogInfo << "---- test_isSingleShot ended (" << QTest::currentDataTag() << ")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVTimer::test_start()
{
    AVLogInfo << "---- test_start launched (" << QTest::currentDataTag() << ") ----";

    AVTimer instance;
    instance.start(100);

    int seconds_to_end = AVDateTime(AVTimeReference::currentDateTime()).msecsTo(instance.m_end_time);
    QVERIFY(qAbs(seconds_to_end - 100) < m_tolerance);

    AVLogInfo << "---- test_start ended (" << QTest::currentDataTag() << ")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVTimer::test_stop()
{
    AVLogInfo << "---- test_stop launched (" << QTest::currentDataTag() << ") ----";

    AVTimer instance;
    QSignalSpy spy(&instance, SIGNAL(timeout()));

    instance.start(100);
    QVERIFY(instance.isActive());

    instance.stop();

    QVERIFY(!instance.isActive());

    QCOMPARE(spy.size(), 0);

    AVLogInfo << "---- test_stop ended (" << QTest::currentDataTag() << ")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVTimer::test_slotInternalTimeout()
{
    AVLogInfo << "---- test_slotInternalTimeout launched (" << QTest::currentDataTag() << ") ----";

    AVTimer instance;
    QSignalSpy spy(&instance, SIGNAL(timeout()));

    instance.slotInternalTimeout();

    QCOMPARE(spy.size(), 1);

    AVLogInfo << "---- test_slotInternalTimeout ended (" << QTest::currentDataTag() << ")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVTimer::test_slotTimeJumped()
{
    AVLogInfo << "---- test_slotTimeJumped launched (" << QTest::currentDataTag() << ") ----";
    const qint64 interval = 100;

    AVTimer instance;
    QSignalSpy spy(&instance, SIGNAL(timeout()));
    instance.setInterval(interval);
    QCOMPARE(instance.m_interval, int(interval));

    AVLogInfo << "TstAVTimer::test_slotTimeJumped: JUMP FORWARD OUTSIDE";

    instance.start(100, true);

    QDateTime now(AVTimeReference::currentDateTime());
    instance.slotTimeJumped(now, now.addMSecs(interval));

    QCOMPARE(spy.size(), 1);
    int time = now.msecsTo(instance.m_end_time);
    QVERIFY(qAbs(time - interval) < m_tolerance);

    spy.clear();

    AVLogInfo << "TstAVTimer::test_slotTimeJumped: JUMP BACKWARD";

    instance.start(interval, true);

    now = AVTimeReference::currentDateTime();
    instance.slotTimeJumped(now, now.addMSecs(-interval));

    QCOMPARE(spy.size(), 0);
    time = now.msecsTo(instance.m_end_time);
    QVERIFY(qAbs(time - interval) < m_tolerance);

    AVLogInfo << "---- test_slotTimeJumped ended (" << QTest::currentDataTag() << ")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVTimer::test_getInterval()
{
    AVLogInfo << "---- test_getInterval launched (" << QTest::currentDataTag() << ") ----";

    // simple getter

    AVLogInfo << "---- test_getInterval ended (" << QTest::currentDataTag() << ")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVTimer::test_calculateScaledInterval()
{
    AVLogInfo << "---- test_calculateScaledInterval launched (" << QTest::currentDataTag() << ") ----";

    QCOMPARE(AVTimer::calculateScaledInterval(100,1.), 100);
    QCOMPARE(AVTimer::calculateScaledInterval(100,2.5), 40);
    QCOMPARE(AVTimer::calculateScaledInterval(100,0.1), 1000);
    QCOMPARE(AVTimer::calculateScaledInterval(-100,2.5), 0);
    QCOMPARE(AVTimer::calculateScaledInterval(100,0.), INT_MAX);

    AVLogInfo << "---- test_calculateScaledInterval ended (" << QTest::currentDataTag() << ")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVTimer::test_timed_timeJumpForwardOutSpeed0()
{
    AVLogInfo << "---- test_timed_timeJumpForwardOutSpeed0 launched ----";

    AVTimeReferenceAdapter::singleton().setSpeed(0);

    QDateTime now = AVTimeReference::currentDateTime();

    AVTimer timer;

    QSignalSpy spy(&timer, SIGNAL(timeout()));
    timer.start(100);

    QTest::qWait(20);

    QCOMPARE(spy.size(), 0);

    AVTimeReferenceAdapter::singleton().setCurrentDateTime(now.addMSecs(200));

    QCOMPARE(spy.size(), 1);

    AVLogInfo << "---- test_timed_timeJumpForwardOutSpeed0 ended ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVTimer::test_remainingTime()
{
    AVLogInfo << "---- test_remainingTime launched ----";

    AVTimeReferenceAdapter::singleton().setSpeed(2);

    AVTimer timer;
    timer.setInterval(200);
    timer.start();

    AVLogInfo << "End time: " << timer.m_end_time << " Current time: " << AVTimeReference::currentDateTime();
    QVERIFY(qAbs(timer.remainingTime() - 200) < m_tolerance);
    QVERIFY(qAbs(timer.realRemainingTime() - 100) < m_tolerance);

    AVLogInfo << "---- test_remainingTime ended ----";

}

///////////////////////////////////////////////////////////////////////////////

AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(TstAVTimer,"avlib/unittests/config")
#include "tstavtimer.moc"

// End of file
