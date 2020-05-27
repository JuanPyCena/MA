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
    \author  Dietmar Gösseringer, d.goesseringer@avibit.com
    \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   AVDateTime extension to QDateTime

    AVDateTime extends QDateTime by methods only accessible via QDate or QTime.
*/


//qt includes
#include <QTextStream>

// System includes
#include <limits.h>
#include <time.h>

// local includes
#include "avlog.h"
#include "avmisc.h"
#include "avdatetime.h"
#include "avtimereference.h"


///////////////////////////////////////////////////////////////////////////////
//                              AVTime
///////////////////////////////////////////////////////////////////////////////

uint AVTime::msecsSinceMidnight () const
{
    return (msec() + 1000 * second() + 1000 * 60 * minute() + 1000 * 60 * 60 * hour());
}

///////////////////////////////////////////////////////////////////////////////

bool AVTime::setMS ( int ms )
{
    setHMS(ms/(1000*3600), (ms/(1000*60))%60, (ms/1000)%60, ms%1000);
    return (ms < int(MSECS_PER_DAY));
}

///////////////////////////////////////////////////////////////////////////////

bool AVTime::setS ( double s )
{
    double dbl_ms = s*1000.0;
    int ms = int(dbl_ms+0.5);
    ms = ms%MSECS_PER_DAY;

    setHMS(ms/(1000*3600), (ms/(1000*60))%60, (ms/1000)%60, ms%1000);
    return (ms < int(MSECS_PER_DAY));
}

///////////////////////////////////////////////////////////////////////////////
//                          AVDateTime
///////////////////////////////////////////////////////////////////////////////

void AVDateTime::setDateTime ( const AVDateTime& dt )
{
    this->setTime( dt.time() );
    this->setDate( dt.date() );
}

/////////////////////////////////////////////////////////////////////////////

bool AVDateTime::isEqualExceptMSecs(const AVDateTime& dt) const
{
    if (time().second() != dt.time().second() ||
        time().minute() != dt.time().minute() ||
        time().hour() != dt.time().hour() ||
        date() != dt.date()) return false;
    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool AVDateTime::isEqual(const AVDateTime& dt) const
{
    if (time().msec() != dt.time().msec() ||
        time().second() != dt.time().second() ||
        time().minute() != dt.time().minute() ||
        time().hour() != dt.time().hour() ||
        date() != dt.date()) return false;
    return true;
}

/////////////////////////////////////////////////////////////////////////////

void AVDateTime::startWithAvtimereference()
{
    setDateTime(AVDateTime(AVTimeReference::currentDateTime()));
}

/////////////////////////////////////////////////////////////////////////////

int AVDateTime::elapsedWithAvtimereference() const
{
    return msecsTo(AVDateTime(AVTimeReference::currentDateTime()));
}

/////////////////////////////////////////////////////////////////////////////

int  AVDateTime::restartWithAvtimereference ()
{
    AVDateTime now( AVTimeReference::currentDateTime() );
    int dt = msecsTo( now );
    setDateTime( now );
    return dt;
}

/////////////////////////////////////////////////////////////////////////////

QDateTime AVDateTime::fromString(const QString & string, Qt::DateFormat format, UtcOffsetBehaviour utc_offset_behaviour)
{
    QDateTime dt = QDateTime::fromString(string, format);

    if (dt.timeSpec() == Qt::OffsetFromUTC)
    {
        if (utc_offset_behaviour == UtcOffsetBehaviour::ConvertToUtc)
        {
            dt = dt.toTimeSpec(Qt::UTC);
        }
    } else
    {
        dt.setTimeSpec(Qt::UTC); // does not change the "hour" value
    }
    return dt;
}

/////////////////////////////////////////////////////////////////////////////

QDateTime AVDateTime::fromString(const QString& string, const QString& format)
{
    QDateTime dt = QDateTime::fromString(string, format);
    dt.setTimeSpec(Qt::UTC); //does not change the "hour" value
    return dt;
}

/////////////////////////////////////////////////////////////////////////////

QDateTime AVDateTime::getDateTimeAfter(const QDateTime& ref_dt, const QTime& time)
{
    if (ref_dt.time() > time)
        return AVDateTime(ref_dt.date().addDays(1), time);
    return AVDateTime(ref_dt.date(), time);
}

/////////////////////////////////////////////////////////////////////////////

QDateTime AVDateTime::currentDateTimeUtc()
{
    return QDateTime::currentDateTimeUtc();
}

/////////////////////////////////////////////////////////////////////////////

QDateTime AVDateTime::currentDateTimeLocal()
{
    return QDateTime::currentDateTime();
}

/////////////////////////////////////////////////////////////////////////////

QDateTime AVDateTime::fromTime_t(uint seconds)
{
    // clazy allows us calling this here...
    return QDateTime::fromTime_t(seconds).toTimeSpec(Qt::UTC);
}

/////////////////////////////////////////////////////////////////////////////

QDateTime AVDateTime::fromMSecsSinceEpoch(qint64 msecs, Qt::TimeSpec spec, int offset_seconds)
{
    // clazy allows us calling this here...
    return QDateTime::fromMSecsSinceEpoch(msecs, spec, offset_seconds);
}

/////////////////////////////////////////////////////////////////////////////

QDateTime AVDateTime::fromMSecsSinceEpoch(qint64 msecs, const QTimeZone &time_zone)
{
    // clazy allows us calling this here...
    return QDateTime::fromMSecsSinceEpoch(msecs, time_zone);
}

/////////////////////////////////////////////////////////////////////////////

QTextStream& operator<<(QTextStream &ts, const AVDateTime &dt)
{
    QString str;
    AVsprintf(str, "%s",
        dt.isValid() ? qPrintable(dt.toString(QString("yyyy/MM/dd hh:mm:ss.zzz"))) :
                       qPrintable(dt.time().toString(QString("0000/00/00 hh:mm:ss.zzz"))));
    ts << str;
    return ts;
};

/////////////////////////////////////////////////////////////////////////////

QTextStream& operator>>(QTextStream &ts, AVDateTime &dt)
{
    QString str_date, str_time, str_datetime;

    ts >> str_date;
    if (str_date.length() <= 10 ) {
        ts >> str_time;
        str_datetime = str_date + " " + str_time;
    } else {
        str_datetime = str_date;
    }

    dt = AVParseDateTimeHuman(str_datetime);

    return ts;
};


/////////////////////////////////////////////////////////////////////////////

QDataStream &operator>>(QDataStream &data_stream, AVDateTime &dt)
{
    operator>>(data_stream, static_cast<QDateTime&>(dt));
    dt.setTimeSpec(Qt::UTC);
    return data_stream;
}

/////////////////////////////////////////////////////////////////////////////

uint qHash(const AVDateTime& date_time, uint seed)
{
    return qHash(static_cast<const QDateTime&>(date_time), seed);
}

///////////////////////////////////////////////////////////////////////////////

template <>
QString AVToString(const AVDateTime& arg, bool enable_escape)
{
    return AVToString(static_cast<const QDateTime&>(arg), enable_escape);
}

///////////////////////////////////////////////////////////////////////////////

template <>
bool AVFromString(const QString& str, AVDateTime& arg)
{
    return AVFromString(str, static_cast<QDateTime&>(arg));
}

/////////////////////////////////////////////////////////////////////////////

// end of file
