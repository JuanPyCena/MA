///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2013
//
// Module:    TESTS - AVCOMMON Unit Tests
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Christian Muschick, c.muschick@avibit.com
    \author  QT5-PORT: Christian Muschick, c.muschick@avibit.com
    \brief
 */

#include <QtTest>
#include <QTemporaryDir>

#include <avunittestmain.h>
#include "avexecstandbychecker.h"

class TstAVLogWriter : public QObject
{
    Q_OBJECT

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:

    void testCreateFilename();
    void testCloseInterval();

};

///////////////////////////////////////////////////////////////////////////////

//! This will be called before the first test function is executed.
//! It is only called once.
void TstAVLogWriter::initTestCase()
{

}

///////////////////////////////////////////////////////////////////////////////

//! This will be called after the last test function is executed.
//! It is only called once.
void TstAVLogWriter::cleanupTestCase()
{

}

///////////////////////////////////////////////////////////////////////////////

//! This will be called before each test function is executed.
void TstAVLogWriter::init()
{

}

///////////////////////////////////////////////////////////////////////////////

//! This will be called after every test function.
void TstAVLogWriter::cleanup()
{

}

///////////////////////////////////////////////////////////////////////////////

void TstAVLogWriter::testCreateFilename()
{
    AVLOG_ENTER(TstAVLogWriter::testCreateFilename);

    // No close interval - no time in log file name
    {
        AVLogWriter writer("pre", "", "/tmp", 0, true, false);
        AVDateTime dt(QDate(2015, 2, 20), QTime(10, 5, 5));
        QVERIFY(writer.recomputeFilename(dt));
        QCOMPARE(writer.m_fileName, QString("/tmp/pre-20150220.log"));
        QCOMPARE(writer.m_fileNameDateTime, AVDateTime(QDate(2015, 02, 20), QTime(10, 5)));

        QVERIFY(!writer.recomputeFilename(dt));
    }

    // Close interval of one hour
    {
        AVLogWriter writer("pre", "", "/tmp", 60*60, true, false);
        AVDateTime dt(QDate(2015, 2, 20), QTime(10, 5));

        QVERIFY(writer.recomputeFilename(dt));
        QCOMPARE(writer.m_fileName, QString("/tmp/pre-20150220-1000.log"));
        QVERIFY(!writer.recomputeFilename(dt));

        dt = dt.addSecs(55*60-1);
        QVERIFY(!writer.recomputeFilename(dt));
        QCOMPARE(writer.m_fileName, QString("/tmp/pre-20150220-1000.log"));
        dt = dt.addSecs(1);
        QVERIFY(writer.recomputeFilename(dt));
        QCOMPARE(writer.m_fileName, QString("/tmp/pre-20150220-1100.log"));
    }

    // close interval of 17 minutes (pretty academic...)
    {
        AVLogWriter writer("pre", "", "/tmp", 17*60, true, false);
        AVDateTime dt(QDate(2015, 2, 20), QTime(10, 5));

        QVERIFY(writer.recomputeFilename(dt));
        QCOMPARE(writer.m_fileName, QString("/tmp/pre-20150220-0955.log"));
        QVERIFY(!writer.recomputeFilename(dt));

        dt = dt.addSecs(7*60-1);
        QVERIFY(!writer.recomputeFilename(dt));
        QCOMPARE(writer.m_fileName, QString("/tmp/pre-20150220-0955.log"));
        dt = dt.addSecs(1);
        QVERIFY(writer.recomputeFilename(dt));
        QCOMPARE(writer.m_fileName, QString("/tmp/pre-20150220-1012.log"));
    }

    // full override file name is unconditionally used
    {
        AVLogWriter writer("pre", "/xyz/override.dat", "/tmp", 60*60, true, false);
        AVDateTime dt(QDate(2015, 2, 20), QTime(10, 5));
        QVERIFY(!writer.recomputeFilename(dt));
        QCOMPARE(writer.m_fileName, QString("/xyz/override.dat"));
    }
}

///////////////////////////////////////////////////////////////////////////////

void TstAVLogWriter::testCloseInterval()
{
    AVLOG_ENTER(TstAVLogWriter::testCloseInterval);

    QTemporaryDir tmpdir;

    AVLogWriter writer("close_interval", "", tmpdir.path(), 1, true, false);
    QSignalSpy recreate_logfile_spy(&writer, SIGNAL(signalNewLogFile(const QString&)));

    QVERIFY(recreate_logfile_spy.wait(1100));

    QDir qtmpdir(tmpdir.path());
    // For technical reasons, creating the log writer does not immediately open the log file (which is
    // irrelevant in practice since every process logs something at startup)
    // Check for the first log file created by the log close interval here.    
    QCOMPARE(qtmpdir.entryList(QStringList() << "close_interval*").count(), 1);
}

///////////////////////////////////////////////////////////////////////////////

AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(TstAVLogWriter,"avlib/unittests/config")
#include "tstavlogwriter.moc"

// End of file
