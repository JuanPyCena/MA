//-----------------------------------------------------------------------------
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2019
//
// Module:    AVLIB - AviBit Library
//
//-----------------------------------------------------------------------------

/*! \file
    \brief   Function level test cases for AVProcessWatchdog
 */

#include <QtTest>
#include <avexithandler.h>
#include <avunittestmain.h>
#include <avlog/avlogmonitor.h>
#include "avprocesswatchdog.h"

class TstAVProcessWatchdog : public QObject
{
    Q_OBJECT

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
    void test_mainThread();

    void test_workerThread();

};
namespace {

void log_sleep (uint msec){
    AVLogInfo << "About to sleep for " << msec << " msec";
    AVSleep(msec);
    AVLogInfo << "Slept for " << msec << " msec";
    QTest::qWait(100); //process all events in the queue
};

///////////////////////////////////////////////////////////////////////////////
class ExitHandler: public AVExitHandler {
    void handleExit() override {
        called = true;
    }
public:
    std::atomic<bool> called { false };
} *exit_handler = new ExitHandler;

}

//-----------------------------------------------------------------------------

//! Declaration of unknown metatypes

//-----------------------------------------------------------------------------


//! This will be called before the first test function is executed.
//! It is only called once.
void TstAVProcessWatchdog::initTestCase()
{
    LOGGER_ROOT.setMinLevel(AVLog::LOG__DEBUG2);
    LOGGER_ROOT.setExitHandler(exit_handler);
}

//-----------------------------------------------------------------------------

//! This will be called after the last test function is executed.
//! It is only called once.
void TstAVProcessWatchdog::cleanupTestCase()
{

}

//-----------------------------------------------------------------------------

//! This will be called before each test function is executed.
void TstAVProcessWatchdog::init()
{

}

//-----------------------------------------------------------------------------

//! This will be called after every test function.
void TstAVProcessWatchdog::cleanup()
{
    exit_handler->called=false;

}

void TstAVProcessWatchdog::test_mainThread()
{
    AVLogInfo << "---- test_mainThread launched (" << QTest::currentDataTag() << ") ----";

    AVProcessWatchdog::singleton().setAbortOnDeadlockEnabled(true);

    {
        AVLogMonitor monitor {&LOGGER_ROOT};
        monitor.setMinLevel(AVLog::LOG__INFO);
        AVLOG_MONITOR_START(monitor);

        log_sleep(1000);
        QVERIFY(!exit_handler->called);
        AVLOG_MONITOR_QSTOP(monitor);
    }

    {

        AVLogMonitor monitor {&LOGGER_ROOT,};
        AVLOG_MONITOR_START(monitor);
        log_sleep(3500);
        QVERIFY(!exit_handler->called);
        AVProcessWatchdog::singleton().reportAlive();
        QVERIFY(!monitor.stopAndGetReport().isEmpty()); // verify that a WARNING has been logged

    }
    log_sleep(7000);
    QTRY_VERIFY_WITH_TIMEOUT(exit_handler->called, 7000);

    AVLogInfo << "---- test_mainThread ended (" << QTest::currentDataTag() << ")    ----";
}

void TstAVProcessWatchdog::test_workerThread()
{
    AVProcessWatchdog::singleton().setAbortOnDeadlockEnabled(true);

    class _ : public QThread
    {
        void run() override
        {
            AVProcessWatchdog::singleton().registerThread();
            log_sleep(sleep_msec);
            AVProcessWatchdog::singleton().reportAlive();


        }
    public:
        int sleep_msec = 0;
    } thread;

    QCOMPARE(static_cast<int>(AVProcessWatchdog::singleton().m_monitored_threads.size()), 1);

    thread.setObjectName("TestWorker");

    thread.sleep_msec = 3500;
    thread.start();

    {
        AVLogMonitor monitor {&LOGGER_ROOT,};
        monitor.setMinLevel(AVLog::LOG__ERROR);
        AVLOG_MONITOR_START(monitor);
        QSignalSpy spy(&thread, &QThread::finished);
        spy.wait();
        QVERIFY(!exit_handler->called);

        QVERIFY(!monitor.stopAndGetReport().isEmpty()); // verify that a WARNING has been logged

    }

    QVERIFY(thread.wait());
    QCOMPARE(static_cast<int>(AVProcessWatchdog::singleton().m_monitored_threads.size()), 1);

    thread.sleep_msec = 7000;
    thread.start();

    QTRY_VERIFY_WITH_TIMEOUT(exit_handler->called, 7000);

    thread.quit();
    thread.wait();
    QCOMPARE(static_cast<int>(AVProcessWatchdog::singleton().m_monitored_threads.size()), 1);

}

//-----------------------------------------------------------------------------

AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(TstAVProcessWatchdog, "avlib/unittests/config/processwatchdog")
#include "tstavprocesswatchdog.moc"

// End of file
