///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2011
//
// Module:     AVLIB - AviBit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
 \author  Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
 \brief   avcstring class implementation.
 */


// QT includes
#include <QDebug>

// AVCOMMON includes
#include <avlog.h>

// local includes

///////////////////////////////////////////////////////////////////////////////

#include "avcstring.h"


QDataStream &operator<< ( QDataStream & out, const AVCString & ba )
{
    if(out.version() <= QDataStream::Qt_3_3)
    {

        if(ba.isNull())
            out<<static_cast<qint32>(0);
        else
        {
            out<<static_cast<qint32>(ba.size() + 1);
            out.writeRawData(ba.data(), ba.size() + 1);
        }
    }
    else
        out << static_cast<const QByteArray&>(ba);

    return out;
}

QDataStream &operator>> ( QDataStream & in, AVCString & ba )
{
    if(in.version() <= QDataStream::Qt_3_3)
    {
        qint32 size=0;
        in >> size;

        if(size==0)
            ba=AVCString();
        else
        {
            ba.resize(size);
            AVASSERT(in.readRawData(ba.data(),size) == size);
            AVASSERT(ba.endsWith('\0'));
            ba.chop(1);
        }
    }
    else
        in>>static_cast<QByteArray&>(ba);

    return in;
}

// End of file
