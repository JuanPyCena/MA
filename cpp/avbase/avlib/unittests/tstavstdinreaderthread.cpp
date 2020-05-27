//-----------------------------------------------------------------------------
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2017
//
// Module:    AVLIB - AviBit Library
//
//-----------------------------------------------------------------------------

/*! \file
    \brief   AVStdinReaderThreadTest implementation
 */

#include <QtTest>
#include <avunittestmain.h>
#include <avstdinreaderthread.h>

// local includes
class TstAVStdinReaderThread : public QObject
{
    Q_OBJECT
    //! friend declaration for function level test case
    //friend class TstAVStdinReaderThreadTest;
private slots:
    void test_stop();


};

void TstAVStdinReaderThread::test_stop() {
    AVLogInfo << "---- test_stop launched ----";
    AVStdinReaderThread thread;
    thread.start();
    QThread::sleep(1);
    QVERIFY(thread.running());

    thread.stop(4000);

    QVERIFY(!thread.running());
    AVLogInfo << "---- test_stop ended ----";
}

AV_QTEST_MAIN_AVCONFIG2_HOME(TstAVStdinReaderThread,"avlib/unittests")
#include "tstavstdinreaderthread.moc"

// End of file
