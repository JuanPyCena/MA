//-----------------------------------------------------------------------------
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2018
//
// Module:    AVLIB - AviBit Library
//
//-----------------------------------------------------------------------------

/*! \file
    \brief   Function level test cases for AVVersionReader
 */

#include <QtTest>
#include <avunittestmain.h>
#include "avversionreader.h"

class TstAVVersionReader : public QObject
{
    Q_OBJECT

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
    void test_readVersion_data();
    void test_readVersion();

    void test_readVersionFromFile();

private:
    QFileInfo version_file_info;

};

//-----------------------------------------------------------------------------

//! Declaration of unknown metatypes

//-----------------------------------------------------------------------------

//! This will be called before the first test function is executed.
//! It is only called once.
void TstAVVersionReader::initTestCase()
{
    version_file_info = QFileInfo(QDir(AVEnvironment::getApplicationHome()), AVVersionReader::VERSION_FILENAME);
}

//-----------------------------------------------------------------------------

//! This will be called after the last test function is executed.
//! It is only called once.
void TstAVVersionReader::cleanupTestCase()
{

}

//-----------------------------------------------------------------------------

//! This will be called before each test function is executed.
void TstAVVersionReader::init()
{
    QFile::remove(version_file_info.absoluteFilePath());
}

//-----------------------------------------------------------------------------

//! This will be called after every test function.
void TstAVVersionReader::cleanup()
{

}

///////////////////////////////////////////////////////////////////////////////

void TstAVVersionReader::test_readVersion_data()
{
    QTest::addColumn<QString>("fallback");
    QTest::addColumn<QString>("ret_val");

    QTest::newRow("row1") << QString() << AVPackageInfo::getVersion();
    QTest::newRow("row2") << "fallback" << "fallback";
}

void TstAVVersionReader::test_readVersion()
{
    AVLOG_ENTER_TESTCASE;

    QFETCH(QString, fallback);
    QFETCH(QString, ret_val);

    QCOMPARE(AVVersionReader::readVersion(fallback), ret_val);
}

void TstAVVersionReader::test_readVersionFromFile()
{
    AVLOG_ENTER_TESTCASE;

    // use scope so file is closed immediately
    {
        QFile version_file(version_file_info.absoluteFilePath());
        QVERIFY(version_file.open(QIODevice::WriteOnly));
        QVERIFY(version_file.write("version_info\nignored") > 0);
    }

    QCOMPARE(AVVersionReader::readVersion(), QString("version_info"));
}

//-----------------------------------------------------------------------------

AV_QTEST_MAIN_AVCONFIG2(TstAVVersionReader)
#include "tstavversionreader.moc"

// End of file
