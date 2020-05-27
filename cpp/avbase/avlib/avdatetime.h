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

    AVDateTime extends QDateTime by methods only accessible via QDate or QTime:
    - addMSecs
    - msecsTo
*/

///////////////////////////////////////////////////////////////////////////////

#if !defined(AVDATETIME_H_INCLUDED)
#define AVDATETIME_H_INCLUDED

///////////////////////////////////////////////////////////////////////////////

// Qt includes
#include <QDateTime>
#include <QTextStream>

// AVLib includes
#include "avlib_export.h"
#include "avfromtostring.h"

///////////////////////////////////////////////////////////////////////////////

// constants (taken from qdatetime.cpp)
static const uint SECS_PER_DAY  = 86400;
static const uint MSECS_PER_DAY = 86400000;
static const uint SECS_PER_HOUR = 3600;
static const uint MSECS_PER_HOUR= 3600000;
static const uint SECS_PER_MIN  = 60;
static const uint MSECS_PER_MIN = 60000;

///////////////////////////////////////////////////////////////////////////////

class AVLIB_EXPORT AVTime : public QTime
{
public:
    //! Default constructor.
    /*!
     *  Constructs the time 0 hours, minutes, seconds and milliseconds
     *  (i.e. 00:00:00.000 = midnight).
     *  This is a valid time.
     */
    AVTime () : QTime () {};

    //! Constructs a time with hour \p h, minute \p m, seconds \p s and milliseconds \p ms.
    /*!
     *  \p h must be in the range 0..23, \p m and \p s must be in the range 0..59,
     *  and \p ms must be in the range 0..999.
     */
    AVTime ( int h, int m, int s=0, int ms=0 ) : QTime (h, m, s, ms) {};

    //! Constructs a time with milliseconds \p ms.
    /*!
     *  \p ms must be in the range 0..1000*60*60*24.
     */
    explicit AVTime (int ms) : QTime (ms/(1000*3600), (ms/(1000*60))%60, (ms/1000)%60, ms%1000) {};

    //! Automatic type conversion constructor.
    AVTime ( const QTime& rhs ) { QTime::operator=(rhs); };

    //! assign operator for QTime to AVTime
    void operator=(const QTime& dt) { *this = AVTime(dt); }

    //! Sets the time to milliseconds \p ms from midnight.
    /*!
     *  \p ms must be in the range 0..1000*60*60*24.
     *  Returns true if the set time is valid; otherwise returns false.
     */
    bool setMS ( int ms );

    //! Sets the time to seconds \p s from midnight.
    bool setS ( double s );

    //! Sum of two AVTimes, i.e. sum of both msecsSinceMidnight(). (Wraps when passing midnight).
    AVTime operator+( const AVTime& t ) { return AVTime(t.addMSecs(msecsSinceMidnight())); }

    //! Quotient of two AVTimes, i.e. division of both msecsSinceMidnight().
    double operator/( const AVTime& t ) {
        return ( double(msecsSinceMidnight()) / double(t.msecsSinceMidnight()) ); }

    //! Remainder of the division of two AVTimes, i.e. modulus of both msecsSinceMidnight().
    int operator%( const AVTime& t ) {
        return ( msecsSinceMidnight() % t.msecsSinceMidnight() ); }

    //! Returns this AVTime in milliseconds, i.e. the milliseconds since 00:00:00.000
    uint msecsSinceMidnight () const;

};

/**
 * @brief The primary purpose of the AVDateTime class is to have UTC as the default time zone in our products in Qt5.
 *
 * Additionally this class provides some convenience methods which are not defined by Qt on the datetime level.
 *
 * Usage of this class instead of QDateTime is enforced via a custom clazy check in Qt5. See IB-22.
 * For Qt3 this is not relevant as the QDateTime implementation is not time-zone aware.
 */
class AVLIB_EXPORT AVDateTime : public QDateTime
{
public:
    //! Default constructor.
    /*!
     *  Constructs a null datetime (i.e. null date and null time).
     *  A null datetime is invalid, since the date is invalid.
     */
    AVDateTime () : QDateTime () { setTimeSpec(Qt::UTC); }

    //! Constructs a datetime with date \p date and time \p time.
    explicit AVDateTime ( const QDate& new_date, const QTime& new_time, Qt::TimeSpec spec = Qt::UTC) :
        QDateTime (new_date, new_time, spec) {}

    //! Constructs a datetime with date \p date in UTC
    explicit AVDateTime (const QDate& new_date) :
        QDateTime (new_date) { setTimeSpec(Qt::UTC); }

    //! Constructs a datetime with year \p Y, month \p M, day \p D, hour \p h ...
    explicit AVDateTime ( int Y, int M, int D, int h, int m, int s, int ms = 0 ) :
        AVDateTime (QDate(Y, M, D), QTime(h, m, s, ms) ) {}

    //! Automatic type conversion constructor.
    AVDateTime ( const QDateTime& rhs ) : QDateTime ( rhs ) {}

    //! See Qt docs
    explicit AVDateTime(const QDate &date, const QTime &time, Qt::TimeSpec spec, int offset_seconds) :
        QDateTime(date, time, spec, offset_seconds) {}

    //! assign operator for QDateTime to AVDateTime
    AVDateTime& operator=(const QDateTime& dt) { *this = AVDateTime(dt); return *this; }

    //! Sets this datetime to \p dt.
    void setDateTime ( const AVDateTime& dt );

    //! Compares the year, hour, minute and seconds between this and the given date/time.
    /*!
     *  \returns true if they match, false otherwise.
     */
    bool isEqualExceptMSecs ( const AVDateTime& dt ) const;

    //! Compares the year, hour, minute, second and milisecond between this and the given date/time.
    /*!
     *  \returns true if they match, false otherwise.
     */
    bool isEqual( const AVDateTime& dt ) const;

    //! Compares the year, hour, minute and seconds between this and the given date/time.
    /*!
     *  \returns true if this date/time is earlier than the given one.
     */
    bool isEarlierThanExceptMSecs ( const AVDateTime& dt ) const;

    //! Compares the year, hour, minute and seconds between this and the given date/time.
    /*!
     *  \returns true if this date/time is later than the given one.
     */
    bool isLaterThanExceptMSecs ( const AVDateTime& dt ) const;

    //! Sets this datetime to the current AVTimeReference datetime.
    /*!
     *  *ATTENTION* Only use this method if you care about the simulated time! This is affected
     *              by speed changes and time jumps. Also see AVTimeReference.
     *
     * Similar to QTime::start(), but does not wrap after 24 hours and uses the AVTimereference as time source.
     *
     *  \sa elapsedWithAvtimereference(), elapsedWithAvtimereference()
     */
    void startWithAvtimereference();

    //! Returns the milliseconds of simulated time that have elapsed since the time stored in the AVDateTime object
    //! (e.g. recorded by call of startWithAvtimereference() or restartWithAvtimereference()).
    /*!
     *  *ATTENTION* Only use this method if you care about the simulated time! This is affected
     *              by speed changes and time jumps. Also see AVTimeReference.
     *
     * The result is undefined if the simulated and/or system time were changed since the reference time was stored.
     *
     * Similar to QTime::elapsed(), but does not wrap after 24 hours and uses the AVTimereference as time source.
     *
     * The result is saturated to INT_MAX/INT_MIN in case of overflow/underflow (if the time difference > 23 days).
     *
     *  \sa startWithAvtimereference(), restartWithAvtimereference()
     */
    int elapsedWithAvtimereference() const;

    //! Performs the method calls elapsedWithAvtimereference() and startWithAvtimereference() at once.
    /*!
     *  *ATTENTION* Only use this method if you care about the simulated time! This is affected
     *              by speed changes and time jumps. Also see AVTimeReference.
     *
     *  \sa startWithAvtimereference(), elapsedWithAvtimereference()
     */
    int restartWithAvtimereference();

    enum class UtcOffsetBehaviour { ConvertToUtc, KeepUtcOffset };
    /**
     * @brief fromString behaves as the QDateTime method, but returns a QDateTime object
     *                   with timespec UTC by default.
     *
     * The returned a QDateTime will have timespec "Qt::OffsetFromUTC" if all of the following is true
     * (otherwise it will be UTC):
     * - utc_offset_behaviour is KeepUtcOffset
     * - The input string contains an offset (e.g. "2002-02-06T15:32:27.123+01:00")
     *
     * If the input string contains an offset, and utc_offset_behaviour is ConvertToUtc, the offset will be used to correct the
     * datetime to UTC (hour will be modified).
     *
     * If the input string does not contain an offset, it is interpreted as UTC (not local time).
     *
     * @param string               The datetime string to convert.
     * @param format               The format of the string as described in the Qt docs.
     * @param utc_offset_behaviour As described above.
     * @return                     A UTC QDateTime if utc_offset_behaviour is ConvertToUtc, a UTC or OffsetFromUTC QDateTime
     *                             if utc_offset_behaviour is KeepUtcOffset.
     */
    static QDateTime fromString(const QString & string, Qt::DateFormat format = Qt::TextDate,
                                UtcOffsetBehaviour utc_offset_behaviour = UtcOffsetBehaviour::ConvertToUtc);

    //! same as QDateTime::fromString, but always assumes UTC
    static QDateTime fromString(const QString& string, const QString& format);

    //! Adds the fitting Date to a given Time by taking the given DateTime as reference
    static QDateTime getDateTimeAfter(const QDateTime& ref_dt, const QTime& time);

    //! Return the current system date time in UTC
    static QDateTime currentDateTimeUtc();
    //! Returns the current system date time int the time zone of the current system setting
    static QDateTime currentDateTimeLocal();
    //! Enforce explicit usage of UTC or local flavour by deleting the QDateTime method. This is mostly to prevent
    //! clazy complaining about a Qt method returning a QDateTime (see IB-22 etc).
    static QDateTime currentDateTime() = delete;

    //! DEPRECATED, don't use in new code (deprecated since Qt 5.8)
    //! Returns the time given seconds from 1970-01-01T00:00:00 UTC as a time given in UTC
    static QDateTime fromTime_t(uint seconds);

    //! Wrapper for Qt functionality, uses UTC per default. Required to fix clazy AVDateTime check, also see class docs.
    static QDateTime fromMSecsSinceEpoch(qint64 msecs, Qt::TimeSpec spec = Qt::UTC, int offset_seconds = 0);
    //! Wrapper for Qt functionality. Required to fix clazy AVDateTime check, also see class docs.
    static QDateTime fromMSecsSinceEpoch(qint64 msecs, const QTimeZone &timeZone);

};

Q_DECLARE_METATYPE(AVDateTime);

//! "Textalize" to the text stream
/*!
 *  The AVDateTime object is written to the QTextStream.
 *
 *  Format: yyyy/MM/dd hh:mm:ss.zzz
 */
AVLIB_EXPORT QTextStream& operator<<(QTextStream &ts, const AVDateTime &dt);

//! "De-Textalize" from the text stream
/*!
 *  The AVDateTime is read from the QTextStream.
 *
 *  Expected format: yyyy/MM/dd hh:mm:ss.zzz (2 consecutive strings separated by 1 whitespace)
 */
AVLIB_EXPORT QTextStream& operator>>(QTextStream &ts, AVDateTime &dt);

//! override deserialization operator to use UTC
AVLIB_EXPORT QDataStream &operator>>(QDataStream &, AVDateTime &);

//! add qHash method to avoid ambiguities
AVLIB_EXPORT uint qHash(const AVDateTime& date_time, uint seed =0);

//! add AVToString to avoid ambiguities
template <> QString AVLIB_EXPORT AVToString(const AVDateTime& arg, bool enable_escape);

//! add AVToString to avoid ambiguities
template <> bool AVLIB_EXPORT    AVFromString(const QString& str, AVDateTime& arg);

#endif

// End of file
