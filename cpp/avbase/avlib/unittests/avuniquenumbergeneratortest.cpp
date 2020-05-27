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
  \author  DI Alexander Wemmer, a.wemmer@avibit.com
  \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
  \brief   Unit test for AVUniqueNumberGenerator class
*/


#include "stdio.h"
#include <QTest>
#include <avunittestmain.h>


#include "avlog.h"
#include "avuniquenumbergenerator.h"
#include "avsingleton.h"

class GeneratorSingleton : public AVUniqueNumberGenerator, public AVSingleton<GeneratorSingleton>
{
};

///////////////////////////////////////////////////////////////////////////////

class AVUniqueNumberGeneratorTest : public QObject
{
    Q_OBJECT


public:
    AVUniqueNumberGeneratorTest()
    {
        AVLogger->Write(LOG_INFO, "AVUniqueNumberGeneratorTest");
    }

private slots:


    void init()
    {
        AVLogger->setMinLevel(AVLog::LOG__FATAL);
    }

    //----------------------------------------

    void cleanup()
    {

    }

    //----------------------------------------

    void testNumber()
    {
        uint min = 1;
        uint max = 10;
        bool ok = false;

        GeneratorSingleton::getInstance()->setMinMax(min, max);

        // acquire all free

        for (uint index=min+1; index <= max; ++index)
        {
            QCOMPARE(GeneratorSingleton::getInstance()->acquireNumber(ok), index);
            QVERIFY(ok);
        }

        QCOMPARE(GeneratorSingleton::getInstance()->acquireNumber(ok), min);
        QVERIFY(ok);

        // overrun
        QCOMPARE(GeneratorSingleton::getInstance()->acquireNumber(ok), (uint)0);
        QVERIFY(!ok);
        QCOMPARE(GeneratorSingleton::getInstance()->acquireNumber(ok), (uint)0);
        QVERIFY(!ok);
        QCOMPARE(GeneratorSingleton::getInstance()->acquireNumber(ok), (uint)0);
        QVERIFY(!ok);
        QCOMPARE(GeneratorSingleton::getInstance()->acquireNumber(ok), (uint)0);
        QVERIFY(!ok);

        // revoking one number
        QVERIFY(GeneratorSingleton::getInstance()->freeNumber(2));
        QCOMPARE(GeneratorSingleton::getInstance()->acquireNumber(ok), (uint)2);
        QVERIFY(ok);
        QCOMPARE(GeneratorSingleton::getInstance()->acquireNumber(ok), (uint)0);
        QVERIFY(!ok);
        QCOMPARE(GeneratorSingleton::getInstance()->acquireNumber(ok), (uint)0);
        QVERIFY(!ok);

        // revoking two numbers
        QVERIFY(GeneratorSingleton::getInstance()->freeNumber(5));
        QVERIFY(!GeneratorSingleton::getInstance()->freeNumber(5));
        QVERIFY(!GeneratorSingleton::getInstance()->freeNumber(5));
        QVERIFY(GeneratorSingleton::getInstance()->freeNumber(3));
        QVERIFY(!GeneratorSingleton::getInstance()->freeNumber(3));
        QVERIFY(!GeneratorSingleton::getInstance()->freeNumber(3));
        QCOMPARE(GeneratorSingleton::getInstance()->acquireNumber(ok), (uint)3);
        QVERIFY(ok);
        QCOMPARE(GeneratorSingleton::getInstance()->acquireNumber(ok), (uint)5);
        QVERIFY(ok);
        QCOMPARE(GeneratorSingleton::getInstance()->acquireNumber(ok), (uint)0);
        QVERIFY(!ok);
        QCOMPARE(GeneratorSingleton::getInstance()->acquireNumber(ok), (uint)0);
        QVERIFY(!ok);
    }

    //----------------------------------------

    void testString()
    {
        bool ok = false;

        GeneratorSingleton::getInstance()->setMinMax(1, 5);

        // acquire all free
        QCOMPARE(GeneratorSingleton::getInstance()->acquireNumberAsFixedWidthString(3, ok),
                 QString("002"));
        QVERIFY(ok);

        QCOMPARE(GeneratorSingleton::getInstance()->acquireNumberAsFixedWidthString(3, ok),
                 QString("003"));
        QVERIFY(ok);

        QCOMPARE(GeneratorSingleton::getInstance()->acquireNumberAsFixedWidthString(3, ok),
                 QString("004"));
        QVERIFY(ok);

        QCOMPARE(GeneratorSingleton::getInstance()->acquireNumberAsFixedWidthString(3, ok),
                 QString("005"));
        QVERIFY(ok);

        QCOMPARE(GeneratorSingleton::getInstance()->acquireNumberAsFixedWidthString(3, ok),
                 QString("001"));
        QVERIFY(ok);

        // overrun
        QCOMPARE(GeneratorSingleton::getInstance()->acquireNumberAsFixedWidthString(3, ok),
                 QString("000"));
        QVERIFY(!ok);

        QCOMPARE(GeneratorSingleton::getInstance()->acquireNumberAsFixedWidthString(3, ok),
                 QString("000"));
        QVERIFY(!ok);

        QCOMPARE(GeneratorSingleton::getInstance()->acquireNumberAsFixedWidthString(3, ok),
                 QString("000"));
        QVERIFY(!ok);

        // revoking one number
        QVERIFY(GeneratorSingleton::getInstance()->freeNumber(2));
        QCOMPARE(GeneratorSingleton::getInstance()->acquireNumberAsFixedWidthString(3, ok),
                 QString("002"));
        QVERIFY(ok);

        QCOMPARE(GeneratorSingleton::getInstance()->acquireNumberAsFixedWidthString(3, ok),
                 QString("000"));
        QVERIFY(!ok);

        QCOMPARE(GeneratorSingleton::getInstance()->acquireNumberAsFixedWidthString(3, ok),
                 QString("000"));

        // revoking two numbers
        QVERIFY(GeneratorSingleton::getInstance()->freeNumber(5));
        QVERIFY(!GeneratorSingleton::getInstance()->freeNumber(5));
        QVERIFY(!GeneratorSingleton::getInstance()->freeNumber(5));
        QVERIFY(GeneratorSingleton::getInstance()->freeNumber(3));
        QVERIFY(!GeneratorSingleton::getInstance()->freeNumber(3));
        QVERIFY(!GeneratorSingleton::getInstance()->freeNumber(3));

        QCOMPARE(GeneratorSingleton::getInstance()->acquireNumberAsFixedWidthString(3, ok),
                 QString("003"));

        QCOMPARE(GeneratorSingleton::getInstance()->acquireNumberAsFixedWidthString(3, ok),
                 QString("005"));

        QCOMPARE(GeneratorSingleton::getInstance()->acquireNumberAsFixedWidthString(3, ok),
                 QString("000"));

        QVERIFY(!ok);

        QCOMPARE(GeneratorSingleton::getInstance()->acquireNumberAsFixedWidthString(3, ok),
                 QString("000"));
    }

    void testGetNFreeNumbers()
    {
        GeneratorSingleton::getInstance()->setMinMax(0, 100);
        QCOMPARE(GeneratorSingleton::getInstance()->getNFreeNumbers(),101U);
    }
};

AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(AVUniqueNumberGeneratorTest,"avlib/unittests/config")
#include "avuniquenumbergeneratortest.moc"


// End of file
