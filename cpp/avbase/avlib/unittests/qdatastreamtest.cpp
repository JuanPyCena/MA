///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON
// Copyright: AviBit data processing GmbH, 2001-2011
//
// Module:    UnitTests
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief
*/


// QT includes
#include <QtGlobal>
#include <QTest>
#include <avunittestmain.h>
#include <QDataStream>
#include <QByteArray>
#include <QDebug>
#include <QBuffer>

#include <avdatastream.h>

// local includes



///////////////////////////////////////////////////////////////////////////////

class QDataStreamTest : public QObject
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

    void testQDataStream()
    {
        AVLogInfo << "----testQDataStream launched----";

        QByteArray arr(20, '\0');

        QDataStream ws(&arr, QIODevice::WriteOnly);
        QDataStream rs(&arr, QIODevice::ReadOnly);

        ws<<(qint32)5;

        qint32 out1 = -1;
        rs>>out1;

        QCOMPARE(out1, (qint32)5);

        ws<<(qint32)6;

        qint32 out2 = -1;
        rs>>out2;

        if(out2 != 6)
        {
            AVLogError <<
                "Using reading and writing QDataStreams after another does not work in QT4";
        }

        AVLogInfo << "----testQDataStream ended----";
    }


};

AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(QDataStreamTest,"avlib/unittests/config")
#include "qdatastreamtest.moc"


// End of file
