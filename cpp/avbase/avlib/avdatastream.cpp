///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2011
//
// Module:     AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
 \author  Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
 \brief   avdatastream class implementation.
 */


// QT includes
#include <QDateTime>

// local includes
#include "avdatetime.h"

///////////////////////////////////////////////////////////////////////////////

#include "avdatastream.h"



AVDataStream::AVDataStream() :
    QDataStream()
{
    setVersion(QDATASTREAM_VERSION);
}

AVDataStream::AVDataStream(QIODevice *dev) :
    QDataStream(dev)
{
    setVersion(QDATASTREAM_VERSION);
}

AVDataStream::AVDataStream(QByteArray * ba, QIODevice::OpenMode mode) :
    QDataStream(ba, mode | QIODevice::Unbuffered)
{
    setVersion(QDATASTREAM_VERSION);
}

AVDataStream::AVDataStream (const QByteArray & ba ) :
        QDataStream(const_cast<QByteArray*>(&ba), QIODevice::ReadOnly | QIODevice::Unbuffered)
{
    setVersion(QDATASTREAM_VERSION);
}

AVDataStream::~AVDataStream()
{
}

AVDataStream &operator>>(AVDataStream& data_stream, QDateTime &dt)
{
    if (data_stream.version() < QDataStream::Qt_4_0)
    {
        //Prevent reading from envvars which may lead to crash. (SWE-5866)
        //This is a workaround to fix spurious crashes in Squish runs.
        QDate d;
        QTime tm;
        data_stream >> d >> tm;
        dt = QDateTime(d, tm, Qt::UTC); //clazy:exclude=avdatetime
    }
    else
    {
        operator>>(static_cast<QDataStream&>(data_stream), dt);
        dt.setTimeSpec(Qt::UTC);
    }
   return data_stream;
}

AVDataStream &operator>>(AVDataStream& data_stream, AVDateTime &dt)
{
   operator>>(data_stream, static_cast<QDateTime&>(dt));
   return data_stream;
}


// End of file
