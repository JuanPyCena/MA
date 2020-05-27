///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2013
//
// Module:    TESTS - AVCOMMON Unit Tests
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Matthias Fuchs, m.fuchs@avibit.com
    \author  QT5-PORT: Matthias Fuchs, m.fuchs@avibit.com
    \brief   Function level test cases for AVProf*
 */

// activate profiling here independently of the compiler flags
#ifndef AVPROFILING
#define AVPROFILING
#endif

#include <QtTest>
#include <avunittestmain.h>

#include "avmisc.h"
#include "avlog.h"
#include "avprofiling.h"
#include "avprofilingreporters.h"

///////////////////////////////////////////////////////////////////////////////

class TestDataReporter : public AVProfDataReporter
{
    AVDISABLECOPY(TestDataReporter);
public:

    TestDataReporter() { }

    ~TestDataReporter() override {}

    void onActivityStart(const AVProfActivity *, const AVProfDataManager::TaskDict &) override {}

    void onActivityStop(const AVProfActivity *, const AVProfDataManager::TaskDict &) override {}

    void taskReport(const AVProfTask *) override {}

    void fullReport(const AVProfDataManager::TaskDict &tasks) override { m_tasks = tasks; }

    AVProfDataManager::TaskDict m_tasks;
};

///////////////////////////////////////////////////////////////////////////////

class ScopedProfilingManager
{
    AVDISABLECOPY(ScopedProfilingManager);
public:
    ScopedProfilingManager()
    {
        m_reporter = new (LOG_HERE) TestDataReporter;
        AVProfDataManager::setInstance(new AVProfDataManager(m_reporter));
    }

    ~ScopedProfilingManager()
    {
        if (AVProfDataManager::hasInstance())
            AVProfDataManager::destroyInstance();
    }

    TestDataReporter *reporter()
    {
        return m_reporter;
    }

private:
    TestDataReporter *m_reporter;
};

///////////////////////////////////////////////////////////////////////////////

class TstAVProfiling : public QObject
{
    Q_OBJECT

private slots:
    void test_noTasksAreRunWhenOff();
    void test_onlyActivatedTasksRunOnDemand();
    void test_activtedTasksCanBeDeactivatedAgain();
    void test_existingTasksCanBeActivated();
};

///////////////////////////////////////////////////////////////////////////////

void TstAVProfiling::test_noTasksAreRunWhenOff()
{
    ScopedProfilingManager scoped;

    AVProfDataManager::getInstance()->setProfileLevel(AVProfileLevel::Off);

    {
        AVPROFRECORD_SCOPE("t1", "a1");
    }

    AVProfDataManager::getInstance()->fullReport();

    QCOMPARE( scoped.reporter()->m_tasks.count(), 0);
}

/////////////////////////////////////////////////////////////////////////////

void TstAVProfiling::test_onlyActivatedTasksRunOnDemand()
{
    ScopedProfilingManager scoped;

    AVProfDataManager::getInstance()->setProfileLevel(AVProfileLevel::TasksOnDemand);
    AVProfDataManager::getInstance()->setTaskActivated("t1", true);

    {
        {
            AVPROFRECORD_SCOPE("t1", "a1");
        }
        {
            AVPROFRECORD_SCOPE("t1", "a1");
        }
        AVPROFRECORD_SCOPE("t1", "a2");
        AVPROFRECORD_SCOPE("t2", "a1");
        AVPROFRECORD_SCOPE("t2", "a2");
    }

    AVProfDataManager::getInstance()->fullReport();

    TestDataReporter *reporter = scoped.reporter();

    QCOMPARE(reporter->m_tasks.count(), 2);

    AVProfTask *t1 = reporter->m_tasks["t1"];
    QVERIFY(t1 != 0);
    QCOMPARE(t1->isActivated(), true);
    QCOMPARE(t1->activities().count(), 2);
    QCOMPARE(t1->activity("a1")->numRuns(), 2);
    QCOMPARE(t1->activity("a2")->numRuns(), 1);

    AVProfTask *t2 = reporter->m_tasks["t2"];
    QVERIFY(t2 != 0);
    QCOMPARE(t2->isActivated(), false);
    QCOMPARE(t2->activities().count(), 0);
}

///////////////////////////////////////////////////////////////////////////////

void TstAVProfiling::test_activtedTasksCanBeDeactivatedAgain()
{
    ScopedProfilingManager scoped;
    TestDataReporter *reporter = scoped.reporter();

    // activate t1
    AVProfDataManager::getInstance()->setProfileLevel(AVProfileLevel::TasksOnDemand);
    AVProfDataManager::getInstance()->setTaskActivated("t1", true);

    {
        AVPROFRECORD_SCOPE("t1", "a1");
    }
    AVProfDataManager::getInstance()->fullReport();

    AVProfTask *t1 = reporter->m_tasks["t1"];
    QVERIFY(t1 != 0);
    QCOMPARE(t1->isActivated(), true);
    QCOMPARE(t1->activity("a1")->numRuns(), 1);

    // deactivate t1
    AVProfDataManager::getInstance()->setTaskActivated("t1", false);

    {
        AVPROFRECORD_SCOPE("t1", "a1");
    }
    AVProfDataManager::getInstance()->fullReport();

    QCOMPARE(t1->isActivated(), false);
    QCOMPARE(t1->activity("a1")->numRuns(), 1);
}

///////////////////////////////////////////////////////////////////////////////

void TstAVProfiling::test_existingTasksCanBeActivated()
{
    ScopedProfilingManager scoped;
    TestDataReporter *reporter = scoped.reporter();

    // no task activated

    AVProfDataManager::getInstance()->setProfileLevel(AVProfileLevel::TasksOnDemand);

    {
        AVPROFRECORD_SCOPE("t1", "a1");
    }
    AVProfDataManager::getInstance()->fullReport();

    AVProfTask *t1 = reporter->m_tasks["t1"];
    QVERIFY(t1 != 0);
    QCOMPARE(t1->isActivated(), false);

    // activating t1

    AVProfDataManager::getInstance()->setTaskActivated("t1", true);

    {
        AVPROFRECORD_SCOPE("t1", "a1");
    }

    AVProfDataManager::getInstance()->fullReport();

    QCOMPARE(t1->isActivated(), true);
    QCOMPARE(t1->activity("a1")->numRuns(), 1);
}

///////////////////////////////////////////////////////////////////////////////

AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(TstAVProfiling,"avlib/unittests/config")
#include "tstavprofiling.moc"

// End of file
