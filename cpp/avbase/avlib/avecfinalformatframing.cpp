///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  DI Alexander Randeu, a.randeu@avibit.com
    \author  QT4-PORT: Gerhard Scheikl, g.scheikl@avibit.com
    \brief   Class to add EC final format framing to a data packet
*/



// Qt includes
#include <qdatastream.h>
#include <qdatetime.h>

// AVLib includes
#include "avlog.h"

// local includes
#include "avecfinalformatframing.h"

///////////////////////////////////////////////////////////////////////////////

uint AVECFinalFormatFraming::FRAMING_SIZE = 12;

///////////////////////////////////////////////////////////////////////////////

QByteArray AVECFinalFormatFraming::getHeader(const QDateTime &ref_dt, const QByteArray &data)
{
    QByteArray final_format_header;
    QDataStream ds(&final_format_header, QIODevice::WriteOnly);
    addHeader(ds, ref_dt, data);
    return final_format_header;
}

///////////////////////////////////////////////////////////////////////////////

void AVECFinalFormatFraming::addHeader(QDataStream &ds, const QDateTime &ref_dt,
                                       const QByteArray &data)
{
    uint length = FRAMING_SIZE + data.size();
    unsigned long long milliseconds_div_10 = ref_dt.time().hour() * 360000 +
                                             ref_dt.time().minute() * 6000 +
                                             ref_dt.time().second() * 100 +
                                             ref_dt.time().msec() / 10;

    ds << static_cast<quint8>((length >> 8) & 0x00ff)
       << static_cast<quint8>(length & 0x00ff)
       << static_cast<quint8>(0) // error code
       << static_cast<quint8>(0) // board/line nr
       << static_cast<quint8>(ref_dt.date().day())
       << static_cast<quint8>((milliseconds_div_10 >> 16) & 0x000000ff)
       << static_cast<quint8>((milliseconds_div_10 >> 8) & 0x000000ff)
       << static_cast<quint8>(milliseconds_div_10 & 0x000000ff);
}

///////////////////////////////////////////////////////////////////////////////

QByteArray AVECFinalFormatFraming::getFooter()
{
    QByteArray final_format_footer;
    QDataStream ds(&final_format_footer, QIODevice::WriteOnly);
    addFooter(ds);
    return final_format_footer;
}

///////////////////////////////////////////////////////////////////////////////

void AVECFinalFormatFraming::addFooter(QDataStream &ds)
{
    ds << static_cast<quint8>(0xa5)
       << static_cast<quint8>(0xa5)
       << static_cast<quint8>(0xa5)
       << static_cast<quint8>(0xa5);
}

///////////////////////////////////////////////////////////////////////////////

QByteArray AVECFinalFormatFraming::getFramedData(const QDateTime &ref_dt, const QByteArray &data)
{
    QByteArray final_format;
    QDataStream ds(&final_format, QIODevice::WriteOnly);
    addHeader(ds, ref_dt, data);
    ds.writeRawData(data.constData(), data.size());
    addFooter(ds);
    return final_format;
}

///////////////////////////////////////////////////////////////////////////////

void AVECFinalFormatFraming::addFramedData(QDataStream &ds, const QDateTime &ref_dt,
                                           const QByteArray &data)
{
    addHeader(ds, ref_dt, data);
    ds.writeRawData(data.constData(), data.size());
    addFooter(ds);
}

// End of file
