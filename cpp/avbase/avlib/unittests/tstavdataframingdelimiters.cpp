///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    AVLIB - AviBit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \brief   Function level test cases for AVDataFramingDelimiters
 */


// cppunit macros
#include <QtTest>
#include <avunittestmain.h>
#include <avdataframingdelimiters.h>

#include "avtimereference/av_unittest_timereferenceholder.h"

class TstAVDataFramingDelimiters : public QObject
{
    Q_OBJECT
    private slots:
        void test_slotGotData();
};

void TstAVDataFramingDelimiters::test_slotGotData()
{
    AVLOG_ENTER(TstAVDataFramingDelimiters::test_slotGotData);
    AVDataFramingDelimiters delim(QByteArray(), QByteArray("\n", 1));
    QSignalSpy spy(&delim, SIGNAL(signalGotMessage(const QByteArray&)));

    delim.slotGotData(QByteArray("", 0));
    QCOMPARE(spy.count(), 0);
    delim.slotGotData(QByteArray("Text", 4));
    QCOMPARE(spy.count(), 0);
    delim.slotGotData(QByteArray("Text2\n", 6));
    QCOMPARE(spy.count(), 1);
}

AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(TstAVDataFramingDelimiters,"avlib/unittests/config")
#include "tstavdataframingdelimiters.moc"
