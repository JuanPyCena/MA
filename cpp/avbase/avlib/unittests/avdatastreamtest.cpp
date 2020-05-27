///////////////////////////////////////////////////////////////////////////////
//
// Package:    TODO
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2011
//
// Module:     UnitTests
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief
*/


// QT includes

// AVCOMMON includes
#include <avdatastream.h>
#include <QTest>
#include <avunittestmain.h>
#include <qbuffer.h>

// local includes



///////////////////////////////////////////////////////////////////////////////

class AVDataStreamTest : public QObject
{
    Q_OBJECT



private:


private slots:

    void init()
    {
    }

    void cleanup()
    {
    }

    void testAVDataStream()
    {
        AVLogInfo << "----testAVDataStream launched----";

        AVDataStream stream1;

        QVERIFY(stream1.version() == QDataStream::Qt_3_3);

        QByteArray ba;
        AVDataStream stream2(&ba, QIODevice::WriteOnly);

        QVERIFY(stream2.version() == QDataStream::Qt_3_3);

        AVDataStream stream3(ba);

        QVERIFY(stream3.version() == QDataStream::Qt_3_3);

        QBuffer buffer;
        AVDataStream stream4(&buffer);

        QVERIFY(stream4.version() == QDataStream::Qt_3_3);

        AVLogInfo << "----testAVDataStream ended----";
    }

    void testAlternateReadWrite()
    {
        AVLogInfo << "----testAlternateReadWrite launched----";

        QByteArray arr(20, '\0');

        AVDataStream ws(&arr, QIODevice::WriteOnly);
        AVDataStream rs(&arr, QIODevice::ReadOnly);

        ws<<static_cast<qint32>(5);

        qint32 out1 = -1;
        rs>>out1;

        QCOMPARE(out1, static_cast<qint32>(5));

        ws<<(qint32)6;

        qint32 out2 = -1;
        rs>>out2;

        QCOMPARE(out2, static_cast<qint32>(6));

        AVLogInfo << "----testAlternateReadWrite ended----";
    }

    void testQDateTimeReadWrite()
    {
        QDateTime dt = AVDateTime::fromString("2019-12-01T10:00:00Z", Qt::ISODate);
        QVERIFY(dt.isValid());

        QByteArray arr(20, '\0');

        AVDataStream ws(&arr, QIODevice::WriteOnly);
        AVDataStream rs(&arr, QIODevice::ReadOnly);

        ws << dt;
        QDateTime dt2 = AVDateTime{};

        rs >> dt2;

        QCOMPARE(dt, dt2);

    }

};

AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(AVDataStreamTest,"avlib/unittests/config")
#include "avdatastreamtest.moc"


// End of file
