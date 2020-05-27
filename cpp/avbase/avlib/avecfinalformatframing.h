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

#ifndef __AVECFINALFORMATFRAMING_H__
#define __AVECFINALFORMATFRAMING_H__

// Qt includes
#include <QByteArray>

#include "avlib_export.h"

// forward declarations
class QDateTime;
class QDataStream;

///////////////////////////////////////////////////////////////////////////////

//! Class to add EC final format framing to a data packet
class AVLIB_EXPORT AVECFinalFormatFraming
{
public:
    //! returns the header data of EC Final Format
    static QByteArray getHeader(const QDateTime &ref_dt, const QByteArray &data);

    //! writes the header data of EC Final Format to the data stream
    static void addHeader(QDataStream &ds, const QDateTime &ref_dt, const QByteArray &data);

    //! returns the footer data of EC Final Format
    static QByteArray getFooter();

    //! writes the footer data of EC Final Format to the data stream
    static void addFooter(QDataStream &ds);

    //! returns data with EC Final Format framing
    static QByteArray getFramedData(const QDateTime &ref_dt, const QByteArray &data);

    //! writes data with EC Final Format framing to the data stream
    static void addFramedData(QDataStream &ds, const QDateTime &ref_dt, const QByteArray &data);

    //! the size of the framing data (header + footer) in bytes
    static uint FRAMING_SIZE;

protected:

    //! Standard Constructor
    AVECFinalFormatFraming();

    //! Destructor
    virtual ~AVECFinalFormatFraming();

private:
    //! Hidden copy-constructor
    AVECFinalFormatFraming(const AVECFinalFormatFraming&);
    //! Hidden assignment operator
    const AVECFinalFormatFraming& operator= (const AVECFinalFormatFraming&);
};

#endif /* __AVECFINALFORMATFRAMING_H__ */

// End of file

