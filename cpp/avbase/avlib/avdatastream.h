///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2009
//
// Module:     AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
 \author  Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
 \brief   avdatastream class header.
 */

#if !defined AVDATASTREAM_H_INCLUDED
#define AVDATASTREAM_H_INCLUDED

// QT includes
#include <QDataStream>
#include <QByteArray>
#include <QIODevice>

// local includes
#include "avlib_export.h"

// forward declarations
class AVDateTime;

/////////////////////////////////////////////////////////////////////////////
//! Extends QDataStream to set the correct QDataStream-Version (QDataStream::Qt_3_3)
/*!
 *  Also, QDateTime deserializations always result in UTC format
 */

class AVLIB_EXPORT AVDataStream : public QDataStream
{
public:
    AVDataStream ();
    explicit AVDataStream (QIODevice *dev);

    //! Constructor to open a datastream directly on a ByteArray
    /*!
     *  The QDataStream uses an internal QBuffer in this case to
     *  create an QIODevice for the byte array.
     *  Here, the QBuffer is opened with the QIODevice::Unbuffered flag
     *  to avoid problems with an additional internal Buffer of the QIODevice
     *  which may not be actualized properly.
     */
    AVDataStream (QByteArray * ba, QIODevice::OpenMode mode);

    //! Overloaded constructor for reading of const byte arrays
    explicit AVDataStream (const QByteArray & ba );

    static const int QDATASTREAM_VERSION = QDataStream::Qt_3_3;


    virtual ~AVDataStream();
};

//! override deserialization operator to use UTC
AVLIB_EXPORT AVDataStream &operator>>(AVDataStream &, QDateTime &);

//! override deserialization operator to use UTC
AVLIB_EXPORT AVDataStream &operator>>(AVDataStream &, AVDateTime &);

#endif

// End of file
