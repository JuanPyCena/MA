//////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT5
// Copyright:  AviBit data processing GmbH, 2001-2016
//
// Module:    AVLIBDEPRECATED - Avibit Library
//
//////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Dietmar Gösseringer, d.goesseringer@avibit.com
    \author  QT4-PORT: Karlheinz Wohlmuth, k.wohlmuth@avibit.com
    \brief   Deprecated AVDateTime extension to QDateTime
*/

///////////////////////////////////////////////////////////////////////////////

#if !defined(AVDATETIMEDEPRECATED_H_INCLUDED)
#define AVDATETIMEDEPRECATED_H_INCLUDED

///////////////////////////////////////////////////////////////////////////////

// Qt includes
#include <qdatetime.h>
#include <QTextStream>

// AVLib includes
#include "avdatetime.h"
#include "avlibdeprecated_export.h"

///////////////////////////////////////////////////////////////////////////////

class AVLIBDEPRECATED_EXPORT AVDateTimeDeprecated : public AVDateTime
{
public:
    //! Default constructor.
    /*!
     *  Constructs a null datetime (i.e. null date and null time).
     *  A null datetime is invalid, since the date is invalid.
     */
    AVDateTimeDeprecated () : AVDateTime () {}

    //! Constructs a datetime with date \p date and time \p time.
    AVDateTimeDeprecated ( const QDate& new_date, const QTime& new_time ) : AVDateTime ( new_date, new_time ) {}

    //! Constructs a datetime with year \p Y, month \p M, day \p D, hour \p h ...
    AVDateTimeDeprecated ( int Y, int M, int D, int h, int m, int s, int ms = 0 ) :
        AVDateTime ( QDate(Y, M, D), QTime(h, m, s, ms) ) {}

    //! Automatic type conversion constructor.
    explicit AVDateTimeDeprecated ( const QDateTime& rhs ) : AVDateTime ( rhs ) {}

    //! Returns the local date/time by considering day savings time (DST).
    /*!
     *  The returned AVDateTime object contains this date/time considering the Austrian
     *  time zone offset and day savings time.
     *
     *  If date/time is not in UTC, the resulting date/time is not predictable.
     */
    AVDateTime getLocalDateTimeVienna() const;
};

#endif

// End of file
