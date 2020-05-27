///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT5
// Copyright:  AviBit data processing GmbH, 2001-2010
//
// Module:    AVLIBDEPRECATED - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Dietmar Gösseringer, d.goesseringer@avibit.com
    \author  QT4-PORT: Karlheinz Wohlmuth, k.wohlmuth@avibit.com
    \brief   Deprecated AVDateTime extension to QDateTime
*/

// local includes
#include "avdatetimedeprecated.h"

///////////////////////////////////////////////////////////////////////////////
//                          AVDateTime
///////////////////////////////////////////////////////////////////////////////

AVDateTime AVDateTimeDeprecated::getLocalDateTimeVienna() const
{
    AVDateTime dtLocal(this->date(), this->time());

    // Austrian day savings time (DST) is currently based on this rules:
    // 1. Begin DST: Last Sunday in March at 01:00 AM UTC
    // 2. End DST:   Last Sunday in Oktober at 01:00 AM UTC

    QDate date = dtLocal.date();
    QTime time = dtLocal.time();
    int month = date.month();
    int year = date.year();
    int dayOfWeek = date.dayOfWeek();

    // Determine if current date lies past the begin of the DST.
    // true, if month is past March
    // true, if we are in the last week of March and it is Sunday, past 01:00 AM UTC
    // true, if we are in the last week of March and have already passed the last Sunday
    // false, in all other cases
    bool pastBeginDST = false;
    if (month > 3) pastBeginDST = true;
    else if (month == 3) {
        int daysToApr = date.daysTo(QDate(year, 4, 1));
        if (daysToApr <= 7) {
            if (dayOfWeek == 7) {
                if (time >= QTime(1, 0)) pastBeginDST = true;
            } else if (daysToApr + dayOfWeek <= 7) pastBeginDST = true;
        }
    }

    // Determine if current date lies past the end of the DST.
    // true, if month is past Oktober
    // true, if we are in the last week of October and it is Sunday, past 01:00 AM UTC
    // true, if we are in the last week of October and have already passed the last Sunday
    // false, in all other cases
    bool pastEndDST = false;
    if (month > 10) pastEndDST = true;
    else if (month == 10) {
        int daysToNov = date.daysTo(QDate(year, 11, 1));
        if (daysToNov <= 7) {
            if (dayOfWeek == 7) {
                if (time >= QTime(1, 0)) pastEndDST = true;
            } else if (daysToNov + dayOfWeek <= 7) pastEndDST = true;
        }
    }

    if (pastBeginDST && !pastEndDST) dtLocal = dtLocal.addSecs(3600);
    uint tzOffsetInSecs = 3600;
    dtLocal = dtLocal.addSecs(tzOffsetInSecs);
    return dtLocal;
}

// end of file
