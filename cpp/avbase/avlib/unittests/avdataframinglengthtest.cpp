///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    TESTS - AVCOMMON Unit Tests
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Ulrich Feichter u.feichter@avibit.com
    \brief   Tests for the AVDataFramingLength class
*/



// system includes

// qt includes

// AVLib Includes
#include "avdataframinglength.h"

// QT includes

// AVCOMMON includes
#include <avdatastream.h>
#include <QTest>
#include <avunittestmain.h>
#include <qbuffer.h>

// local includes

#include "avmisc.h"
////////////////////////////////////////////////////////////////////////////////

class AVDataFramingLengthTest : public QObject
{
    Q_OBJECT

private slots:
    void init()
    {
        LOGGER_ROOT.setMinLevel(AVLog::LOG__DEBUG2);
    }

    void cleanup()
    {

    }

    void testFunctionality()
    {
        qDebug("start: testFunctionality");
        AVDataFramingLength framing(4, 2, AVDataFramingLength::SHORT, 100);
        AVDIRECTCONNECT(&framing, SIGNAL(signalGotMessage(const QByteArray&,const QByteArray&)),
                  this, SLOT(slotGotMessage(const QByteArray&,const QByteArray&)));

        QByteArray data1(54, '\0');
        data1.fill(1);
        data1[0] = 9;
        data1[1] = 9;
        data1[2] = 0;
        data1[3] = 50;
        data1[10] = 0xab; // magic byte for verification

        m_last_checked = -1;
        m_check_ok = false;
        m_expected_headers.resize(1);
        m_expected_datas.resize(1);
        m_expected_headers[0].setRawData(data1.data(), 4);
        m_expected_datas[0].setRawData(data1.data() + 4, data1.count() - 4);
        framing.slotGotData(data1);
        m_expected_headers[0].clear();
        m_expected_datas[0].clear();

        QCOMPARE(m_check_ok, true);
        QCOMPARE(m_last_checked, 0);
        QCOMPARE(framing.numBytesInBuffer(), 0);

        // test longer data -> must be possible as maximum buffer size is 100*2
        QByteArray data2(104, '\0');
        data2.fill(2);
        data2[0] = 5;
        data2[1] = 5;
        data2[2] = 0;
        data2[3] = 100;
        data2[20] = 0xbb; // magic byte for verification

        m_last_checked = -1;
        m_check_ok = false;
        m_expected_headers.resize(1);
        m_expected_datas.resize(1);
        m_expected_headers[0].setRawData(data2.data(), 4);
        m_expected_datas[0].setRawData(data2.data() + 4, data2.count() - 4);
        framing.slotGotData(data2);
        m_expected_headers[0].clear();
        m_expected_datas[0].clear();

        QCOMPARE(m_check_ok, true);
        QCOMPARE(m_last_checked, 0);
        QVERIFY(framing.m_buffer.count() > 100); // resize must have been happened
        QCOMPARE(framing.numBytesInBuffer(), 0);

        // test 2 messages in one
        QByteArray data3(80, '\0');
        data3.fill(2);
        data3[0] = 1;
        data3[1] = 2;
        data3[2] = 0;
        data3[3] = 26; // first message is 30 bytes long
        data3[21] = 0xff; // magic byte for verification

        data3[30] = 3;
        data3[31] = 4;
        data3[32] = 0;
        data3[33] = 46; // second is 50 bytes long
        data3[60] = 0xaa; // magic byte for verification

        m_last_checked = -1;
        m_check_ok = false;
        m_expected_headers.resize(2);
        m_expected_datas.resize(2);
        m_expected_headers[0].setRawData(data3.data(), 4);
        m_expected_datas[0].setRawData(data3.data() + 4, 30 - 4);
        m_expected_headers[1].setRawData(data3.data() + 30, 4);
        m_expected_datas[1].setRawData(data3.data() + 30 + 4, 50 - 4);
        framing.slotGotData(data3);

        QCOMPARE(m_check_ok, true);
        QCOMPARE(m_last_checked, 1);
        QCOMPARE(framing.numBytesInBuffer(), 0);

        // test the 2 messages in 3 parts
        m_last_checked = -1;
        m_check_ok = false;

        // first part is only part of first message
        QByteArray part;
        part.setRawData(data3.data(), 15);
        framing.slotGotData(part);
        QCOMPARE(m_check_ok, false);
        QCOMPARE(m_last_checked, -1);
        part.clear();
        // second part is rest of first message + part of second
        part.setRawData(data3.data()+15, 30);
        framing.slotGotData(part);
        QCOMPARE(m_check_ok, true);
        QCOMPARE(m_last_checked, 0);
        part.clear();

        // rest of second
        part.setRawData(data3.data()+15+30, 80-15-30);
        framing.slotGotData(part);
        QCOMPARE(m_check_ok, true);
        QCOMPARE(m_last_checked, 1);
        part.clear();

        m_expected_headers[0].clear();
        m_expected_datas[0].clear();
        m_expected_headers[1].clear();
        m_expected_datas[1].clear();

        qDebug("end: testFunctionality");
    }

    void tst_slotGotData()
    {
        qDebug("start: tst_slotGotData");
        AVDataFramingLength framing(4, 0, AVDataFramingLength::INT, 80);
        AVDIRECTCONNECT(&framing, SIGNAL(signalGotMessage(const QByteArray&,const QByteArray&)),
                  this, SLOT(slotGotMessage(const QByteArray&,const QByteArray&)));

        // test index adjustment
        QByteArray data3(100, '\0');
        data3.fill(2);
        data3[0] = 0;
        data3[1] = 0;
        data3[2] = 0;
        data3[3] = 56; // first message is 60 bytes long
        data3[21] = 0xa1; // magic byte for verification

        data3[60] = 0;
        data3[61] = 0;
        data3[62] = 0;
        data3[63] = 16; // second is 20 bytes long
        data3[70] = 0x12; // magic byte for verification

        m_last_checked = -1;
        m_check_ok = false;
        m_expected_headers.resize(2);
        m_expected_datas.resize(2);
        m_expected_headers[0].setRawData(data3.data(), 4);
        m_expected_datas[0].setRawData(data3.data() + 4, 60 - 4);
        m_expected_headers[1].setRawData(data3.data() + 60, 4);
        m_expected_datas[1].setRawData(data3.data() + 60 + 4, 20 - 4);

        // test the 2 messages in 3 parts
        m_last_checked = -1;
        m_check_ok = false;

        // first part is only part of first message
        QByteArray part;
        part.setRawData(data3.data(), 15);
        framing.slotGotData(part);
        QCOMPARE(m_check_ok, false);
        QCOMPARE(m_last_checked, -1);
        QCOMPARE(15, framing.numBytesInBuffer());
        QCOMPARE(0, framing.m_read_index);
        QCOMPARE(15, framing.m_write_index);
        part.clear();

        // second part is rest of first message + part of second
        part.setRawData(data3.data()+15, 60);
        framing.slotGotData(part);
        QCOMPARE(m_check_ok, true);
        QCOMPARE(m_last_checked, 0);
        QCOMPARE(15, framing.numBytesInBuffer());
        QCOMPARE(60, framing.m_read_index);
        QCOMPARE(75, framing.m_write_index);
        part.clear();

        // rest of second.
        // as we have extracted a message with 60 bytes. the write index is bigger than half of the buffer.
        // so a index adjustment on a non empty buffer must happen
        part.setRawData(data3.data()+15+60, 80-15-60);
        framing.slotGotData(part);
        QCOMPARE(m_check_ok, true);
        QCOMPARE(m_last_checked, 1);
        part.clear();
        QCOMPARE(0, framing.numBytesInBuffer());
        QCOMPARE(20, framing.m_read_index);
        QCOMPARE(20, framing.m_write_index);

        // ------------ test buffer enlargement -----------------

        // add an additional message
        data3[80] = 0;
        data3[81] = 0;
        data3[82] = 0;
        data3[83] = 16; // second is 20 bytes long
        data3[90] = 0x13; // magic byte for verification

        m_expected_headers.resize(3);
        m_expected_datas.resize(3);
        m_expected_headers[2].setRawData(data3.data() + 80, 4);
        m_expected_datas[2].setRawData(data3.data() + 80 + 4, 20 - 4);

        m_last_checked = -1;
        m_check_ok = false;

        // add all the data in once -> buffer enlargement by 20 to 100
        framing.slotGotData(data3);

        QCOMPARE(m_check_ok, true);
        QCOMPARE(m_last_checked, 2);
        QCOMPARE(0, framing.numBytesInBuffer());
        QCOMPARE(100, framing.m_buffer.count());

        m_expected_headers[0].clear();
        m_expected_datas[0].clear();
        m_expected_headers[1].clear();
        m_expected_datas[1].clear();
        m_expected_headers[2].clear();
        m_expected_datas[2].clear();

        // ------------ test buffer overflow -----------------

        // add an additional message
        data3.resize(180);
        data3[100] = 0;
        data3[100] = 0;
        data3[100] = 0;
        data3[100] = 76; // 80 bytes long -> 180 bytes but buffer is only 80*2

        m_last_checked = -1;
        m_check_ok = false;

        // add all the data in once -> buffer overflow which will result in a reset. no message will be extracted
        framing.slotGotData(data3);

        QCOMPARE(m_check_ok, false);
        QCOMPARE(m_last_checked, -1);
        QCOMPARE(0, framing.numBytesInBuffer());
        QCOMPARE(0, framing.m_read_index); // after reset
        QCOMPARE(0, framing.m_write_index);
        QCOMPARE(false, framing.m_reset_request);

        qDebug("end: tst_slotGotData");
    }

protected slots:
    void slotGotMessage(const QByteArray& header, const QByteArray& data)
    {
        AVASSERT(m_expected_headers.count() == m_expected_datas.count());
        m_last_checked++;
        if(m_last_checked >= static_cast<int>(m_expected_headers.count()))
        {
            qDebug("attempting to check index %d, but have only %d elements to check", m_last_checked,
                   m_expected_headers.count());
            m_check_ok = false;
            return;
        }

        if(header == m_expected_headers[m_last_checked] && data == m_expected_datas[m_last_checked])
        {
            m_check_ok = true;
        }
        else
        {
            m_check_ok = false;
            qDebug("got wrong data at check %d", m_last_checked);
            if(header != m_expected_headers[m_last_checked])
            {
                qDebug("expected header:\n %s", qPrintable(AVHexDump(m_expected_headers[m_last_checked])));
                qDebug("received header:\n %s", qPrintable(AVHexDump(header)));
            }
            if(data != m_expected_datas[m_last_checked])
            {
                qDebug("expected data:\n %s", qPrintable(AVHexDump(m_expected_datas[m_last_checked])));
                qDebug("received data:\n %s", qPrintable(AVHexDump(data)));
            }
        }
    }

private:
    QVector<QByteArray> m_expected_headers;
    QVector<QByteArray> m_expected_datas;
    bool m_check_ok;
    int m_last_checked;

};

//CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( AVDataFramingLengthTest, "AVLIB" );
AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(AVDataFramingLengthTest,"avlib/unittests/config")
#include "../mocs/avdataframinglengthtest.moc"

// End of file
