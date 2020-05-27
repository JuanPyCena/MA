/////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2012
//
// Module:     UnitTests
//
/////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   Unit tests to check timings
*/


// QT includes
#include <QObject>
#include <QTimer>
#include <QTest>

// AVCOMMON includes
#include "avlog.h"
#include "avtimereference.h"
#include <avunittestmain.h>

// system includes
#if defined(Q_OS_UNIX)
#include <unistd.h>
#endif


/////////////////////////////////////////////////////////////////////////////

class AVTimingTests : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        QDateTime t = AVTimeReference::currentDateTime(); //assure AVTimeReference initialization
        Q_UNUSED(t);
    }

    //FAILS SOMETIMES ON VIRTUAL BUILD MACHINES
    //this test should check whether QTimer signals work accurately
    void testQTimerSpan()
    {
        QSKIP("SWE-4588");

        AVLogInfo << "---- testQTimerSpan launched ----";

        QList<int> spans;

        for (uint i = 0; i < 100; ++i)
        {
            QDateTime start = AVDateTime::currentDateTimeUtc();

            QTimer::singleShot(100, qApp, SLOT(quit()));
            qApp->exec();

            QDateTime end = AVDateTime::currentDateTimeUtc();

            int span = start.msecsTo(end);

            spans << span;
        }

        AVLogInfo << "time spans: " << spans;
        Q_FOREACH(int span, spans)
            QVERIFY(abs(span-100)<20);

        AVLogInfo << "---- testQTimerSpan ended ----";
    }

    //FAILS SOMETIMES ON VIRTUAL BUILD MACHINES
    //this test should check whether sleep() works accurately
    void testSleepSpan()
    {
        AVLogInfo << "---- testSleepSpan launched ----";

        QList<int> spans;

        for (uint i = 0; i < 100; ++i)
        {
            QDateTime start = AVDateTime::currentDateTimeUtc();

#if defined(Q_OS_WIN32)
            Sleep(100);       // milliseconds sleep
#else
            usleep(100000);   // microseconds sleep
#endif

            QDateTime end = AVDateTime::currentDateTimeUtc();

            int span = start.msecsTo(end);

            spans << span;
        }

        AVLogInfo << "time spans: " << spans;
        Q_FOREACH(int span, spans)
            QVERIFY(abs(span-100)<20);

        AVLogInfo << "---- testSleepSpan ended ----";
    }

    //FAILS SOMETIMES ON VIRTUAL BUILD MACHINES
    //this test should check whether QTimer signals work accurately using AVTimeReference
    void testQTimerSpanAV()
    {
        AVLogInfo << "---- testQTimerSpanAV launched ----";

        QList<int> spans;

        for (uint i = 0; i < 100; ++i)
        {
            QDateTime start = AVTimeReference::currentDateTime();

            QTimer::singleShot(100, qApp, SLOT(quit()));
            qApp->exec();

            QDateTime end = AVTimeReference::currentDateTime();

            int span = start.msecsTo(end);

            spans << span;
        }

        AVLogInfo << "time spans: " << spans;
        Q_FOREACH(int span, spans)
            QVERIFY(abs(span-100)<20);

        AVLogInfo << "---- testQTimerSpanAV ended ----";
    }

    //FAILS SOMETIMES ON VIRTUAL BUILD MACHINES
    //this test should check whether sleep() works accurately using AVTimeReference
    void testSleepSpanAV()
    {
        AVLogInfo << "---- testSleepSpanAV launched ----";

        QList<int> spans;

        for (uint i = 0; i < 100; ++i)
        {
            QDateTime start = AVTimeReference::currentDateTime();

#if defined(Q_OS_WIN32)
            Sleep(100);       // milliseconds sleep
#else
            usleep(100000);   // microseconds sleep
#endif

            QDateTime end = AVTimeReference::currentDateTime();

            int span = start.msecsTo(end);

            spans << span;
        }

        AVLogInfo << "time spans: " << spans;
        Q_FOREACH(int span, spans)
            QVERIFY(abs(span-100)<20);

        AVLogInfo << "---- testSleepSpanAV ended ----";
    }

    //FAILS SOMETIMES ON VIRTUAL BUILD MACHINES
    void testEventloopSpan()
    {
        QSKIP("SWE-4588");

        AVLogInfo << "---- testEventloopSpan launched ----";

        QList<int> spans;

        for (uint i = 0; i < 100; ++i)
        {
            QDateTime start = AVDateTime::currentDateTimeUtc();

            QElapsedTimer timer;
            timer.start();
            while(timer.elapsed()<100)
            {
                QTimer::singleShot(100-timer.elapsed() >10 ? 10 : 100-timer.elapsed(),
                        qApp, SLOT(quit()));
                qApp->exec();
            }


            QDateTime end = AVDateTime::currentDateTimeUtc();

            int span = start.msecsTo(end);

            spans << span;
        }

        AVLogInfo << "time spans: " << spans;
        Q_FOREACH(int span, spans)
            QVERIFY(abs(span-100)<20);

        AVLogInfo << "---- testEventloopSpan ended ----";
    }

    //FAILS SOMETIMES ON VIRTUAL BUILD MACHINES
    void testEventloopSpanAV()
    {
        QSKIP("SWE-4588");

        AVLogInfo << "---- testEventloopSpanAV launched ----";

        QList<int> spans;

        for (uint i = 0; i < 100; ++i)
        {
            QDateTime start = AVTimeReference::currentDateTime();

            QElapsedTimer timer;
            timer.start();
            while(timer.elapsed()<100)
            {
                QTimer::singleShot(100-timer.elapsed() >10 ? 10 : 100-timer.elapsed(),
                        qApp, SLOT(quit()));
                qApp->exec();
            }


            QDateTime end = AVTimeReference::currentDateTime();

            int span = start.msecsTo(end);

            spans << span;
        }

        AVLogInfo << "time spans: " << spans;
        Q_FOREACH(int span, spans)
            QVERIFY(abs(span-100)<20);

        AVLogInfo << "---- testEventloopSpanAV ended ----";
    }

    //FAILS SOMETIMES ON VIRTUAL BUILD MACHINES
    void testCompareElapsedAndCurrentDateTime()
    {
        AVLogInfo << "---- testCompareElapsedAndCurrentDateTime launched ----";

        QList<QPair<int, int> > spans;

        for (uint i = 0; i < 100; ++i)
        {
            QDateTime start = AVDateTime::currentDateTimeUtc();

            QElapsedTimer timer;
            timer.start();

            QTimer::singleShot(100, qApp, SLOT(quit()));
            qApp->exec();

            int elapsed = timer.elapsed();

            QDateTime end = AVDateTime::currentDateTimeUtc();

            int span = start.msecsTo(end);

            spans << qMakePair(span, elapsed);
        }

        AVLogInfo << "time spans: " << spans;
        for (int i = 0; i < spans.size(); ++i)
        {
            int diff = abs(spans[i].first - spans[i].second);
            if(diff >20)
            {
                AVLogInfo << "testCompareElapsedAndCurrentDateTime: span: "<<spans[i].first
                        <<", elapsed: "<<spans[i].second;
            }
            QVERIFY(diff<=20);
        }

        AVLogInfo << "---- testCompareElapsedAndCurrentDateTime ended ----";
    }

    //FAILS SOMETIMES ON VIRTUAL BUILD MACHINES
    void testCompareElapsedAndCurrentDateTimeAV()
    {
        AVLogInfo << "---- testCompareElapsedAndCurrentDateTimeAV launched ----";

        QList<QPair<int, int> > spans;

        for (uint i = 0; i < 100; ++i)
        {
            QDateTime start = AVTimeReference::currentDateTime();

            QElapsedTimer timer;
            timer.start();

            QTimer::singleShot(100, qApp, SLOT(quit()));
            qApp->exec();

            int elapsed = timer.elapsed();

            QDateTime end = AVTimeReference::currentDateTime();

            int span = start.msecsTo(end);

            spans << qMakePair(span, elapsed);
        }

        AVLogInfo << "time spans: " << spans;
        for (int i = 0; i < spans.size(); ++i)
        {
            int diff = abs(spans[i].first - spans[i].second);
            if(diff >20)
            {
                AVLogInfo << "testCompareElapsedAndCurrentDateTime: span: "<<spans[i].first
                        <<", elapsed: "<<spans[i].second;
            }
            QVERIFY(diff<=20);
        }

        AVLogInfo << "---- testCompareElapsedAndCurrentDateTimeAV ended ----";
    }

/////////////////////////////////////////////////////////////////////////////

};

AV_QTEST_MAIN_AVCONFIG2_HOME(AVTimingTests,"avlib/unittests")
#include "avtimingtests.moc"


// End of file
