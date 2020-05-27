///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    TESTS - AVCOMMON Unit Tests
//
///////////////////////////////////////////////////////////////////////////////

/*!
  \file
  \author  DI Christian Muschick c.muschick@avibit.com
  \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
  \brief   Unit test for the AVThread class
*/



#include "avconfig.h" // TODO QTP-10
#include <QTest>
#include <avunittestmain.h>
#include "avlog.h"
#include "avmacros.h"
#include "avdaemon.h"
#include "avthread.h"


/////////////////////////////////////////////////////////////////////////////

class TestThread : public AVThread
{

 public:
    TestThread() : AVThread("TestThread") {}
    bool checkForStop() const { return AVThread::checkForStop(); }

 protected:
     void runThread() override
     {
         QVERIFY((AVThread::isMainThread() == false));
         QVERIFY(AVThread::threadName() == "TestThread");
         sendNotification(0);

         while (!sleepMs(1))
             ;
    }

    void stopThread() override
    {
        // stopThread is called in context of thread which called stop()
        QVERIFY((AVThread::isMainThread() == true));
    }
};

class ExecThread: public AVThread {

protected:
    void runThread() override
    {
        QObject context_object;
        QTimer::singleShot(0, &context_object, [&] {thread_has_run = true;});
        QTimer::singleShot(100, &context_object, [&] {exit();});
        exec();
    }

public:
    bool thread_has_run = false;
};

/////////////////////////////////////////////////////////////////////////////

class AVThreadUnitTest : public QObject
{
    Q_OBJECT




private slots:

    //----------------------------------------

    virtual void init()
    {
        AVThread::setMainThread();
        m_notification = -1;
    }

    //----------------------------------------

    void testStartStop()
    {
        TestThread thread;
        thread.start();
        bool stopped = thread.stop();
        thread.wait();

        QVERIFY(stopped);
        QVERIFY(thread.checkForStop());

        thread.start();
        QVERIFY(thread.running());
        thread.stop();
        QVERIFY(!thread.running());
        thread.wait();

        QVERIFY((AVThread::isMainThread() == true));
    }

    //----------------------------------------

    void testNotify()
    {
        QVERIFY(qApp);
        TestThread thread;
        thread.connectNotification(this, SLOT(threadNotification(int)));
        thread.start();
        thread.stop();
        thread.wait();
        qApp->processEvents();

        QVERIFY(m_notification == 0);
        QVERIFY((AVThread::isMainThread() == true));
    }

    void testExec()
    {
        ExecThread thread;
        thread.start();
        thread.wait();
        QVERIFY(thread.thread_has_run);
    }

protected slots:
    void threadNotification(int i)
    {
        m_notification = i;
    }

protected:
    int m_notification;
};


AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(AVThreadUnitTest,"avlib/unittests/config")
#include "avthreadtest.moc"

// End of file
