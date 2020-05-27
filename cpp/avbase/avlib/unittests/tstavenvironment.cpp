//-----------------------------------------------------------------------------
//
// Package:   AVCOMMON
// Copyright: AviBit data processing GmbH, 2001-2015
//
// Module:    UNIT TESTS
//
//-----------------------------------------------------------------------------

/*! \file
    \brief   Function level test cases for AVEnvironment
 */

#include <QTest>

#include <avenvironment.h>
#include <avunittestmain.h>

class TstAVEnvironment : public QObject
{
    Q_OBJECT



private slots:
    void init()
    {
    }

    void cleanup()
    {
    }


    void test_setEnv()
    {

        AVEnvironment::setEnv("MY_TEST_ENV_VAR", "ONE");

        QString value = AVEnvironment::getEnv("MY_TEST_ENV_VAR",true, AVEnvironment::NoPrefix);

        QCOMPARE(value, QString("ONE"));

        AVEnvironment::setEnv("MY_TEST_ENV_VAR", "TWO", false);
        QCOMPARE(qgetenv("MY_TEST_ENV_VAR"), QByteArray("ONE"));

        AVEnvironment::setEnv("MY_TEST_ENV_VAR", "THREE", true);
        QCOMPARE(qgetenv("MY_TEST_ENV_VAR"), QByteArray("THREE"));

    }

    void test_extendPath()
    {
#if defined (_OS_WIN32_) || defined(Q_OS_WIN32)
        QChar separator = ';';
#else
        QChar separator = ':';
#endif

        QVERIFY(AVEnvironment::extendPath("/path1", "TESTPATH"));
        QCOMPARE(AVEnvironment::getEnv("TESTPATH", false, AVEnvironment::NoPrefix), QString("/path1"));
        QVERIFY(AVEnvironment::extendPath("/path2", "TESTPATH"));
        QCOMPARE(AVEnvironment::getEnv("TESTPATH", false, AVEnvironment::NoPrefix), QString("/path1") + separator + "/path2");
        QVERIFY(!AVEnvironment::extendPath("/path2", "TESTPATH"));
    }

    void test_getPath()
    {
#if defined (_OS_WIN32_) || defined(Q_OS_WIN32)
        QString separator = ";";
#else
        QString separator = ":";
#endif

        AVEnvironment::setEnv("TESTPATH", "/path1" + separator + "/path2", true);
        QVERIFY(AVEnvironment::getPath("TESTPATH") == QStringList() << "/path1" << "/path2");
        QVERIFY(AVEnvironment::getPath("NONEXISTANT_PATH") == QStringList());
    }

};

AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(TstAVEnvironment,"avlib/unittests/config")
#include "tstavenvironment.moc"


// End of file
