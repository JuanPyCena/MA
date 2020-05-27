///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2014
//
// Module:    AVLIB - AviBit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Christian Muschick, c.muschick@avibit.com
    \brief   Function level test cases for AVConfig2ExporterCstyle
 */

#include <QtTest>
#include <avunittestmain.h>
#include "avconfig2cstyle.h"
#include "avfromtostring.h"

class TstAVConfig2ExporterCstyle : public QObject
{
    Q_OBJECT

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
    //! AC-654
    void test_establishPreconditions();
};

///////////////////////////////////////////////////////////////////////////////

//! This will be called before the first test function is executed.
//! It is only called once.
void TstAVConfig2ExporterCstyle::initTestCase()
{

}

///////////////////////////////////////////////////////////////////////////////

//! This will be called after the last test function is executed.
//! It is only called once.
void TstAVConfig2ExporterCstyle::cleanupTestCase()
{

}

///////////////////////////////////////////////////////////////////////////////

//! This will be called before each test function is executed.
void TstAVConfig2ExporterCstyle::init()
{

}

///////////////////////////////////////////////////////////////////////////////

//! This will be called after every test function.
void TstAVConfig2ExporterCstyle::cleanup()
{

}

///////////////////////////////////////////////////////////////////////////////

void TstAVConfig2ExporterCstyle::test_establishPreconditions()
{
    AVConfig2ExporterCstyle exporter;
    AVConfig2StorageLocation location;

    {
        AVConfig2StorageLocation::FactConditionContainer condition1;
        QVERIFY(AVFromString("[[SYS;[AIRMAX]];[HOSTTYPE;[DP]]]", condition1));

        location.m_fact_conditions = condition1;
        exporter.establishPreconditions(location);

        AVConfig2StorageLocation::FactCondition condition11, condition12;
        QVERIFY(AVFromString("[SYS;     [AIRMAX]]", condition11));
        QVERIFY(AVFromString("[HOSTTYPE;[DP]    ]", condition12));

        QCOMPARE(exporter.m_fact_condition_stack.count(), 2);
        QCOMPARE(AVToString(exporter.m_fact_condition_stack[0]), AVToString(condition11));
        QCOMPARE(AVToString(exporter.m_fact_condition_stack[1]), AVToString(condition12));
    }

    {
        AVConfig2StorageLocation::FactConditionContainer condition2;
        QVERIFY(AVFromString("[[SYS;[DIFLIS]]]", condition2));

        location.m_fact_conditions = condition2;
        exporter.establishPreconditions(location);

        QCOMPARE(exporter.m_fact_condition_stack.count(), 1);
        QCOMPARE(AVToString(exporter.m_fact_condition_stack), AVToString(condition2));
    }
}

///////////////////////////////////////////////////////////////////////////////

AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(TstAVConfig2ExporterCstyle,"avlib/unittests/config")
#include "tstavconfig2exportercstyle.moc"

// End of file
