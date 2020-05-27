///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    TESTS - AVCOMMON Unit Tests
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
  \author    Dietmar Goesseringer, d.goesseringer@avibit.com
    \author  QT5-PORT: Ulrich Feichter, u.feichter@avibit.com
    \brief   Unit Test for AVRunningMean class
*/


// system includes
#include <QTest>
#include <avunittestmain.h>

// AVLib includes
#include "avrunningmean.h"

///////////////////////////////////////////////////////////////////////////////

class AVRunningMeanTest : public QObject
{
    Q_OBJECT

private slots:
    void init()
    {
    }

    void cleanup()
    {
    }

    void testRM()
    {
        // test adding ms milliseconds; with 0 < ms < msecs per day
        AVRunningMean<double> *rm1 = new AVRunningMean<double> (50);
        AVASSERT(rm1 != 0);
        QCOMPARE(0.0, rm1->mean());
        QCOMPARE(0.0, rm1->stdDev());
        QVERIFY(rm1->stdDev() == 0.0);

        rm1->add(40.4);
        QCOMPARE(40.4, rm1->mean());
        QCOMPARE( 0.0, rm1->stdDev());

        rm1->add(41.4);
        QCOMPARE(40.9, rm1->mean());
        QCOMPARE(0.707106781187, rm1->stdDev());

        AVRunningMean<double> rm2(*rm1);
        QCOMPARE(rm1->count(), rm2.count());
        QCOMPARE(rm1->mean(), rm2.mean());
        QCOMPARE(rm1->stdDev(), rm2.stdDev());

        delete rm1;
    }
};

AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(AVRunningMeanTest,"avcalculationlib/unittests/config")
#include "avrunningmeantest.moc"

// End of file
