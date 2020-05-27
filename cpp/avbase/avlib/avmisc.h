///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2010
//
// Module:     AVLIB - AviBit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author    Dr. Thomas Leitner, t.leitner@avibit.com
    \author    QT4-PORT: Christian Muschick, c.muschick@avibit.com
    \brief     Definition of miscellaneous helper classes and functions.
*/

#if !defined(AVMISC_H_INCLUDED)
#define AVMISC_H_INCLUDED

// System includes
#include <cstddef>
#include <math.h>
#include <memory>

// QT includes
#include <QApplication>
#include <QThread>
#include <QTime>
#include <QHostAddress>
#include <QPoint>
#include <QSize>
#include <QTcpSocket>
#include <QDir>
#include <QDataStream>

// AV includes
#include "avlib_export.h"
#include "avsnprintf.h"
#include "avlog.h"
#include "avdatetime.h"

///////////////////////////////////////////////////////////////////////////////
//! the PI constant

#if !defined(AV_PI)
#define AV_PI 3.1415926535897932385E0
#endif

#if !defined(DT_FORMAT_STRING)
#define DT_FORMAT_STRING "yyyyMMddhhmmsszzz"
#endif

#if !defined(D_FORMAT_STRING)
#define D_FORMAT_STRING "yyyyMMdd"
#endif

#if !defined(T_FORMAT_STRING)
#define T_FORMAT_STRING "hhmmsszzz"
#endif

#define AV_MODE_A_FORMAT "%04d"
#define AV_MODE_S_FORMAT "#%06X"
#define AV_MODE_S_MAX    16777215         //<! AV_MODE_S_MAX = 2^24 - 1

//! timeout in ms for intermediate event processing
#define AV_EVENT_PROCESSING_TIMEOUT    10

#if !defined(COMPRESS_MIN_DATA_SIZE)
#define COMPRESS_MIN_DATA_SIZE 200
#endif

#define AVINT32_MAX    INT_MAX            //! MAX INT32
#define AVINT32_MIN    INT_MIN            //! MIN INT32
#define AVINT16_MAX    32767              //! MAX INT16
#define AVINT8_MAX     127                //! MAX INT8

AVLIB_EXPORT extern const QString EmptyQString;
AVLIB_EXPORT extern const QStringList EmptyQStringList;

AVLIB_EXPORT extern const quint32 MODEA_REQ;

//! Answer an empty string if value is a null string
AVLIB_EXPORT const QString& notNull(const QString& value);

// TODO CM there is a circular dependency between avlog and avmisc in Qt5 only - fix that...
class AVLog;

///////////////////////////////////////////////////////////////////////////////
// angle transformations
//! Transform an angle value in degrees into the equivalent value in radiant
inline double deg2rad(double angle)
{
    return (angle * AV_PI) / 180.0;
}

//! Transform an angle value in radiant into the equivalent value in degrees
inline double rad2deg(double angle)
{
    return (angle * 180.0) / AV_PI;
}

///////////////////////////////////////////////////////////////////////////////
// round functions
//! Rounds to the nearest integer.
//! ATTENTION: If the result does not fit into an int the behaviour is undefined!
inline int avroundtoint(double val)
{
    return ( static_cast<int>(((val < 0.0) ? val - 0.5 : val + 0.5)) );
}

///////////////////////////////////////////////////////////////////////////////

//! Rounds the double double val to the nearest integer and performs a range check.
/*! If val does not fit into an integer ok will become false and the returned value is saturated
 *  to INT_MIN or INT_MAX.
 *
 * \param val the double to be rounded
 * \param ok  true if the rounded value fits into an integer, false if not
 * \return    the rounded value if ok is true, or the rounded value, saturated to [INT_MIN, INT_MAX] if ok is false
 */
inline int avroundtoint(double val, bool& ok)
{
    ok = false;
    if (val < 0.0)
    {
        double tmp = val - 0.5;
        if (tmp < INT_MIN) return INT_MIN;
        ok = true;
        return static_cast<int>(tmp);
    }

    double tmp = val + 0.5;
    if (tmp > INT_MAX) return INT_MAX;
    ok = true;
    return static_cast<int>(tmp);
}

///////////////////////////////////////////////////////////////////////////////

//! Rounds to the nearest long int.
//! ATTENTION: If the result does not fit into a long int the behaviour is undefined!
inline long int avroundtolong(double val)
{
    return ( static_cast<long int>(((val < 0.0) ? val - 0.5 : val + 0.5)) );
}

//! Rounds to the nearest qint64
//! ATTENTION: If the result does not fit into a qint64 the behaviour is undefined!
inline qint64 avroundtoqint64(double val)
{
    return ( static_cast<qint64>(((val < 0.0) ? val - 0.5 : val + 0.5)) );
}

//! Rounds to the nearest long long int
inline long long int avroundtolonglong(double val)
{
    return ( static_cast<long long int>(((val < 0.0) ? val - 0.5 : val + 0.5)) );
}

//! Rounds to the nearest integral double. Works also on Windows!
inline double avround(double val)
{
    return ( floor(((val < 0.0) ? val - 0.5 : val + 0.5)) );
}

//! Provides integer power functionality
AVLIB_EXPORT int avpow(int base, uint exponent);

///////////////////////////////////////////////////////////////////////////////
// flags (int's being used as flagmasks, commonly w/ usage of enums)

//! Sets the specified flag(s) in the specified variable and return the result
inline int setflag   (int& variable, int flag) { variable |= flag; return variable; }

//! Clears the specified flag(s) in the specified variable and return the result
inline int clearflag (int& variable, int flag) { variable &= ~flag; return variable; }

//! Returns TRUE if the specified flag(s) is/are set in the specified variable, FALSE otherwise
/*!
 *  If allflags is TRUE (default) all specified flags (if more than 1) must be set in the
 *  specified variable.
 *  If allflags is FALSE at least one from the specified flags (if more than 1) must be set in the
 *  specified variable
 */
inline bool isflagset (int variable, int flags, bool allflags = true)
{
    if (allflags)
        return ((variable & flags) == flags);
    else
        return (variable & flags);
}

//! Returns TRUE if the specified flags are set in the specified variable, FALSE otherwise
inline bool areflagsset (int variable, int flags)
{
    return ((variable & flags) == flags);
}

/////////////////////////////////////////////////////////////////////////////
//! copies a file with the given chunksize (for iteration) and flushes the
//! written file when told to.
AVLIB_EXPORT bool copyFiles(const QString& fromfilename,  const QString& tofilename,
                            uint chunksize, bool doflush = true);

/////////////////////////////////////////////////////////////////////////////
//! searches for files of given type (e.g. *.pdf) in all subdirectories of the
//! given path list and strip base directory and leading directory separators
//! from results.
inline QStringList findFilesRecursively(QStringList paths, QString fileTypes,
                                        QString baseDirectory = QString())
{
    if (fileTypes.isEmpty()) fileTypes = "*";
    QStringList result, more;
    QStringList::Iterator it;
    for (int i=0; i<paths.size(); i++)
    {
        QDir dir(paths[i]);
        more = dir.entryList(QDir::nameFiltersFromString(fileTypes), QDir::Files);
        for (it=more.begin(); it!=more.end(); ++it)
        {
            QString baseDirectoryEntry = paths[i] + "/" + *it;
            if (!baseDirectory.isNull()) baseDirectoryEntry.remove(baseDirectory);
            if (baseDirectoryEntry.at(0) == '/') baseDirectoryEntry.remove(0, 1);
            result.append(baseDirectoryEntry);
        }
        // reg exp in next line excludes . and .. dirs (and .* actually)
        more = dir.entryList(QDir::Dirs).filter(QRegExp("[^.]"));


        for (it=more.begin(); it!=more.end(); ++it)
            *it = paths[i] + "/" + *it;
        more = findFilesRecursively(more, fileTypes, baseDirectory);
        for (it=more.begin(); it!=more.end(); ++it)
        {
            QString subDirectoryEntry = *it;
            if (!baseDirectory.isNull()) subDirectoryEntry.remove(baseDirectory);
            if (subDirectoryEntry.at(0) == '/') subDirectoryEntry.remove(0, 1);
            result.append(subDirectoryEntry);
        }
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////
//! searches for files of given type (e.g. *.pdf) in all subdirectories of the
//! given path.
inline QStringList findFilesRecursively(QString path, QString fileTypes)
{
    QStringList paths;
    paths << path;
    return findFilesRecursively(paths, fileTypes, path);
}

/////////////////////////////////////////////////////////////////////////////
//! if empty, deletes directory and all subdirectories.
inline bool removeDirRecursively(QString path)
{
    QDir dir(path);
    // reg exp in next line excludes . and .. dirs (and .* actually)
    QStringList more = dir.entryList(QDir::Dirs).filter(QRegExp("[^.]"));
    QStringList::Iterator it;
    for (it=more.begin(); it!=more.end(); ++it)
        removeDirRecursively(path + "/" + *it);
    return dir.rmdir(path);
}

///////////////////////////////////////////////////////////////////////////////
// floating point comparison
//! Compares to floating point values for equality using a specified epsilon
#if !defined(AVFLOATEQ)
#define AVFLOATEQ(a, b, eps) ( ((a) < (b)) ? ((b)-(a) < eps) : ((a)-(b) < eps) )
#endif

///////////////////////////////////////////////////////////////////////////////
//! helper class used to create a full path
AVLIB_EXPORT bool makeDirectories(const QString& directories);

///////////////////////////////////////////////////////////////////////////////
//! helper class used solely for the AVSleep inline routine

class AVLIB_EXPORT AVMiscPrivate : public QThread
{
    Q_OBJECT
public:
    ~AVMiscPrivate() override{};

    //! sleep the specified number of milliseconds
    void sleep_ms(int ms) { msleep(ms); };
    //! sleep the specified number of seconds
    void sleep_s(int s)   { sleep(s);   };
    //! dummy QThread run() routine.
    void run() override;
};

///////////////////////////////////////////////////////////////////////////////
//! provides a platform independent millisecond sleep function

inline void AVSleep(int ms)
{
    AVMiscPrivate p;
    int s = ms / 1000;
    ms = ms % 1000;
    if (s > 0)  p.sleep_s(s);
    if (ms > 0) p.sleep_ms(ms);
}

///////////////////////////////////////////////////////////////////////////////

//! provides a platform independent conditional millisecond sleep function.
//! when the given exit_flag is turned to true, the method will return immediately
inline void AVSleepConditional(int ms, const bool& exit_flag)
{
    QTime current_dt;
    current_dt.start();
    while(1)
    {
        if (exit_flag || current_dt.elapsed() > ms) return;
        AVSleep(1);
    }
}

///////////////////////////////////////////////////////////////////////////////

//! *ATTENTION* Usage of this method is discouraged, tests are likely to be flaky if this is used.
//!             Use QSignalSpy to wait for relevant events instead, or use qWaitFor (or qWait if you absolutely have to)
//!
//! Starts the event loop for given amount of milliseconds
//! Returns the actual amount of past time (may vary considerably)
AVLIB_EXPORT qint64 runEventLoop(qint64 msecs);

///////////////////////////////////////////////////////////////////////////////

//! *ATTENTION* Usage of this method is discouraged, tests are likely to be flaky if this is used.
//!             Use QSignalSpy to wait for relevant events instead, or use qWaitFor (or qWait if you absolutely have to)
//!
//! Starts the event loop for given amount of milliseconds
//! Returns true if the actual amount of past time is in interval [min,max]
AVLIB_EXPORT bool runEventLoop(qint64 msecs, qint64 min, qint64 max);

///////////////////////////////////////////////////////////////////////////////
//! parse a human readable date/time string to a QDateTime
/*! This function parses a string in the form YYYY*MM*DD*hh*mm*ss*zzz into a
    QDateTime object. * is a separator out of the following characters: [/-:. ]
    that may be changing within the string (e.g. "2006/24/12 12:00:00.123")
    \param s the string holding the 'YYYY*MM*DD*hh*mm*ss*zzz'-presentation of
           date & time
 */
AVLIB_EXPORT QDateTime AVParseDateTimeHuman(const QString &s);

///////////////////////////////////////////////////////////////////////////////
//! parse a date/time string to a QDateTime
/*! ATTENTION: DEPRECATED use AVFromString
    This function parses a string in the form YYYYMMDD[hhmmss[.xxx]] into a
    QDateTime object. The 'hhmmss.xxx' part is optional. If it's missing
    the time-part of QDateTime will be 00:00:00.000.
    \param s the string holding the 'YYYYMMDDhhmmss'-presentation of
           date & time
*/

AVLIB_EXPORT QDateTime AVParseDateTime(const QString &s);

//! parse a human readable time string to a QTime
/*! This function parses a string in the form hh*mm*ss*zzz into a QTime
    object. * is a separator out of the following characters: [:. ]
    that may be changing within the string (e.g. "12:00:00.123")
    \param s the string holding the 'hh*mm*ss*zzz'-presentation of the time
 */
AVLIB_EXPORT QTime AVParseTimeHuman(const QString &s);

//! parse a time string to a QTime
/*! ATTENTION: DEPRECATED use AVFromString
    This function parses a string in the form hhmmss[.xxx] into a
    QTime object. The '.xxx' part is optional. If it's missing
    the ms-part of QTime will be 000.
    \param s the string holding the 'hhmmss'-representation of time
*/

AVLIB_EXPORT QTime AVParseTime(const QString &s);

//! parse a time string to a QDate
/*! ATTENTION: DEPRECATED use AVFromString
    This function parses a string in the form YYYYMMDD into a
    QDate object.
    \param s the string holding the 'YYYYMMDD'-representation of time
*/

AVLIB_EXPORT QDate AVParseDate(const QString &s);

//! convert QDateTime into string
/*! ATTENTION: DEPRECATED use AVToString
    This function returns the 'YYYYMMDDhhmmss'-presentation of a QDateTime
    object
    \param dt the QDateTime to transform
    \returns the string-presentation of QDateTime

    ATTENTION! This is deprecated, use AVToString
 */
AVLIB_EXPORT QString AVPrintDateTime(const QDateTime &dt);

///////////////////////////////////////////////////////////////////////////////
//! convert QDate into string
/*! ATTENTION: DEPRECATED use AVToString
    This function returns the 'YYYYMMDD'-presentation of a QDate
    object
    \param dt the QDate to transform
    \returns the string-presentation of QDate
 */
AVLIB_EXPORT QString AVPrintDate(const QDate &dt);

///////////////////////////////////////////////////////////////////////////////
//! convert QTime into string
/*! ATTENTION: DEPRECATED use AVToString
    This function returns the 'hhmmss'-presentation of a QTime
    object
    \param dt the QTime to transform
    \returns the string-presentation of QTime
 */
AVLIB_EXPORT QString AVPrintTime(const QTime &dt);

///////////////////////////////////////////////////////////////////////////////
//! convert QDate into a human readable string
/*! This function returns the 'YYYY-MM-DD'-presentation of a QDate
    object
    \param dt the QDate to transform
    \returns the string-presentation of QDate
 */
AVLIB_EXPORT QString AVPrintDateHuman(const QDate &date);

///////////////////////////////////////////////////////////////////////////////
//! convert QDateTime into a human readable string
/*! This function returns the 'YYYY-MM-DD hh:mm:ss'-presentation of a QDateTime
    object
    \param dt the QDateTime to transform
    \returns the string-presentation of QDateTime
 */
AVLIB_EXPORT QString AVPrintDateTimeHuman(const QDateTime &dt);

///////////////////////////////////////////////////////////////////////////////
//! convert QDateTime into a human readable string including ms
/*! This function returns the 'YYYY-MM-DD hh:mm:ss.zzz'-presentation of a
    QDateTime object
    \param dt the QDateTime to transform
    \returns the string-presentation of QDateTime
 */
AVLIB_EXPORT QString AVPrintDateTimeMsHuman(const QDateTime &dt);


///////////////////////////////////////////////////////////////////////////////
//! convert QDateTime into string with milliseconds
/*! ATTENTION: DEPRECATED use AVToString
    This function returns the 'YYYYMMDDhhmmss.xxx'-presentation of a QDateTime
    object where xxx are the milliseconds
    \param dt the QDateTime to transform
    \returns the string-presentation of QDateTime
 */
AVLIB_EXPORT QString AVPrintDateTimeMs(const QDateTime &dt);


///////////////////////////////////////////////////////////////////////////////
//! convert QDateTime into string
/*! This function returns the string presentation of a QDateTime object. The
    format is specified by DT_FORMAT_STRING
    \param dt the QDateTime to transform
    \returns the string-presentation of QDateTime
 */

AVLIB_EXPORT QString AVPrintDtTm(const QDateTime &dt);

///////////////////////////////////////////////////////////////////////////////
//! system beep function, needed because the QT QApplication::beep does not
//! call the system beep but plays the beep through the sound card.
/*! \param freq the frequency of the desired beep in <Hz>
    \param duration duration of the desired beep in <ms>
 */
#include <cstdlib>

#if defined(_OS_WIN32_) || defined(Q_OS_WIN32)

#if _MSC_VER >= 1500
// the next line prevents loading winsock here, otherwise winsock2 can not be included in AVCOMLIB
#define _WINSOCKAPI_
#include <windows.h>
#endif

inline void AVBeep(int freq, int duration)
{
#if _MSC_VER >= 1500
    Beep(freq, duration);
#else
    _beep(freq, duration);
#endif
}
#else
inline void AVBeep(int, int)
{
    QApplication::beep();
}
#endif

///////////////////////////////////////////////////////////////////////////////
//! Wrapper to use random() on platforms where it exits
#if defined(_OS_WIN32_) || defined(Q_OS_WIN32)
#define AVRAND_MAX RAND_MAX
inline long AVRandom() {
    return rand();
};
#else
#define AVRAND_MAX 0x7fffffff         // 2^31 - 1
inline long AVRandom() {
    return random();
};
#endif

///////////////////////////////////////////////////////////////////////////////
//! Returns a random number between the given range (inclusive)
inline long AVRandom(long low, long high)
{
    return (AVRandom() % (high - low + 1) + low);
}


///////////////////////////////////////////////////////////////////////////////
//! Avibit sprintf
AVLIB_EXPORT QString& AVsprintf(QString &s, const char *fmt, ...)
#if defined(Q_CC_GNU) && !defined(__INSURE__) && !defined(__INTEL_COMPILER)
    // this GCC attribute checks for format/argument mismatches -- it only
    // works with GCC
    __attribute__ ((format (printf, 2, 3)))
#endif
;

/////////////////////////////////////////////////////////////////////////////
//! Generates an ID string using a given prefix and a timestamp
//! with millisecond precision and a random number

inline QString AVgenerateID(const QString& prefix)
{
    QDateTime now = AVDateTime::currentDateTimeUtc();

    QString timestamp = AVPrintDateTime(now);
    QString ms;
    AVsprintf(ms, "%03d", now.time().msec());
    QString random;
    AVsprintf(random, "%ld", AVRandom());
    return prefix + timestamp + ms + random;
}

///////////////////////////////////////////////////////////////////////////////
//! Returns a seed using std::random_device (should be cryptographic strength)
AVLIB_EXPORT uint AVSeed();

///////////////////////////////////////////////////////////////////////////////
//! Wrapper to use srandom() on platforms where it exits
#if defined(_OS_WIN32_) || defined(Q_OS_WIN32)
inline void AVSRandom(uint seed) {
    srand(seed);
};
#else
inline void AVSRandom(uint seed) {
    srandom(seed);
};
#endif

///////////////////////////////////////////////////////////////////////////////
//! return a hex dump of raw data
/*! \param addr pointer to the memory location to be dumped
    \param len number of bytes to be dumped
    \return hexadecimal presentation of 'len' bytes at 'addr'

    The following code

    \code
        qDebug ("'%s'", qPrintable(AVHexDump ((unsigned char*) "0123456789ABCFEFXYZ",30)));
    \endcode

    would produce this output:

    \code
    "     0    30 31 32 33 34 35 36 37 38 39 41 42 43 46 45 46 0123456789ABCFEF"
    "    10    58 59 5a 00 00 00 00 00 00 00 00 00 00 00       XYZ           "
    \endcode

    The first hex-value is the offset of the first byte within the dumped memory
    range.
    Then follow 16 hex-values (seperated by spaces), representing the
    memory content.
    The last 16 characters finally are the ASCII-presentation. Byte-values that
    are less than 32 and greater than 126 are replaced by spaces in this part of
    the output.

    \note In this example, everything beyond byte 20 is arbitrary data
          of course.
 */
AVLIB_EXPORT QString AVHexDump(const unsigned char* addr, int len);
AVLIB_EXPORT QString AVHexDump(const QByteArray &array);


////////////////////////////////////////////////////////////////////////////////
//! Outputs a string containing multiple lines to the logging facility.
/*! This function takes a string with multiple lines (seperated by "\\n" or
    "\r\\n" ) and outputs each line to the logging facility.

    The following code snippets produce equivalent output:

    \code
        if (f == 0) {
            AVLogger->Write(LOG_ERROR_ERRNO, "cannot open file."
            AVLogger->Write(LOG_ERROR_ERRNO, "which is not good.");
        }
    \endcode

    \code
    if (f == 0) {
            AVLogMultiLines (LOG_ERROR_ERRNO,
            "cannot open file.\nwhich is not good.");
    }
    \endcode

    \param line parameter that is passed to the logger.
    \param file name of the log file
    \param lev specifies the output level
    \param str the multiline string to be output
    \param log The logger to use, default is AVLogger.
    \sa AVLog::Write
 */
AVLIB_EXPORT void AVLogMultiLines(const int line, const char *file,
                     const int lev, const QString& str, AVLog* log=0);




///////////////////////////////////////////////////////////////////////////////
//! Returns the sign of a number, i.e. -1 if negative, +1 if positive
inline int AVsign(double num)
{
    if (num>=0.0) { return 1; }
    else { return -1; }
}

///////////////////////////////////////////////////////////////////////////////
//! Returns the maximum of a,b
template <typename T>
T AVmax(T a, T b)
{
    return a>b ? a : b;
}

///////////////////////////////////////////////////////////////////////////////
//! Returns the minimum of a,b
template <typename T>
T AVmin(T a, T b)
{
    return a<b ? a : b;
}

///////////////////////////////////////////////////////////////////////////////
//! Returns the value with the greater magnitude
inline double AVmaxMag(double a, double b)
{
    return fabs(a)>fabs(b) ? a : b;
}

///////////////////////////////////////////////////////////////////////////////
//! Returns the value with the smaller magnitude
inline double AVminMag(double a, double b)
{
    return fabs(a)<fabs(b) ? a : b;
}

/////////////////////////////////////////////////////////////////////////////

//! Returns the maximum value of the list, and the according index
template<typename T>
const T& AVmax(const QList<T>& list, int& index_of_max)
{
    index_of_max = 0;
    for(int i = 1; i < list.size(); ++i)
    {
        if(list[i] > list[index_of_max])
            index_of_max = i;
    }
    return list[index_of_max];
}

/////////////////////////////////////////////////////////////////////////////

//! Returns the maximum value of the list
template<typename T>
const T& AVmax(const QList<T>& list)
{
    int dummy = 0;
    return AVmax(list, dummy);
}

/////////////////////////////////////////////////////////////////////////////

//! Returns the minimum value of the list, and the according index
template<typename T>
const T& AVmin(const QList<T>& list, int& index_of_min)
{
    index_of_min = 0;
    for(int i = 1; i < list.size(); ++i)
    {
        if(list[i] < list[index_of_min])
            index_of_min = i;
    }
    return list[index_of_min];
}

/////////////////////////////////////////////////////////////////////////////

//! Returns the minimum value of the list
template<typename T>
const T& AVmin(const QList<T>& list)
{
    int dummy = 0;
    return AVmin(list, dummy);
}

/////////////////////////////////////////////////////////////////////////////

//! Returns the maximum value of the set
template<typename T>
const T& AVmax(const QSet<T>& set)
{
    typename QSet<T>::const_iterator iter_to_max = set.begin();
    for(typename QSet<T>::const_iterator iter = set.begin() +1 ; iter != set.end(); ++iter)
    {
        if(*iter > *iter_to_max)
            iter_to_max = iter;
    }
    return *iter_to_max;
}

/////////////////////////////////////////////////////////////////////////////

//! Returns the minimum value of the set
template<typename T>
const T& AVmin(const QSet<T>& set)
{
    typename QSet<T>::const_iterator iter_to_min = set.begin();
    for(typename QSet<T>::const_iterator iter = set.begin() +1 ; iter != set.end(); ++iter)
    {
        if(*iter < *iter_to_min)
            iter_to_min = iter;
    }
    return *iter_to_min;
}

/////////////////////////////////////////////////////////////////////////////

//! Check the status of a QDataStream
/*! \param s data stream to check
    \param log_prefix Prefix for logging output (DEBUG2)
 */
AVLIB_EXPORT bool checkDSIOStatus(const QDataStream &s, const QString &log_prefix = "");

/////////////////////////////////////////////////////////////////////////////
/**
 * @brief AVResolveHost performs a synchronous name lookup on the DNS.
 *
 * You can also pass along an IP address, in this case this will be a noop.
 *
 * If both IPv4 and IPv6 addresses are configured, IPv4 address will be preferred.
 *
 * Note there apparently is a problem with name resolution in Qt3 (it can hang), see SWE-4542.
 * This method is also used to avoid this problem.
 *
 * Also see AvResolveHostTest.
 *
 * @param name              Hostname for which to look up the IP.
 * @param resultHostAddress The IPv4/IPv6 of the passed 'addr', if the DNS query was successful
 * @return true if the lookup was successful, false if the lookup failed.
 */
AVLIB_EXPORT bool AVResolveHost(const QString &name, QHostAddress &resultHostAddress);

/////////////////////////////////////////////////////////////////////////////
//! Detects if a client has disconnected. Can e.g. be used by clienthandlers.
//! Returns true when the client has disconnected, false otherwise
AVLIB_EXPORT bool hasClientDisconnected(QTcpSocket& sd);

///////////////////////////////////////////////////////////////////////////////
//! Returns true if the given MODE-A code is valid, false otherwise.
/*! Valid codes are 0 to 7777, but all digits must be <= 7.
 */
inline bool isModeAValid(const quint32 mode_a)
{
    uint tmp = mode_a/1000;
    if (tmp > 7) return false;
    int v = mode_a - 1000*tmp;
    tmp = v/100;
    if (tmp > 7) return false;
    v = v - 100*tmp;
    tmp = v/10;
    if (tmp > 7) return false;
    if ( (v - 10*tmp) > 7 ) return false;
    return true;
}

///////////////////////////////////////////////////////////////////////////////
//! Returns true if the given MODE-A code is valid, false otherwise.
/*! Valid codes are 0 to 7777, but all digits must be <= 7.
 */
inline bool isModeAValid(const qint32 mode_a)
{
    return (mode_a >= 0) && isModeAValid(static_cast<quint32>(mode_a));
}

///////////////////////////////////////////////////////////////////////////////
//! Returns true if the given MODE-A code is the special MODE-A 8888,
//!false otherwise.
/*!
 */
inline bool isSpecialModeA(const quint32 mode_a)
{
    if (mode_a == MODEA_REQ) return true;
    return false;
}

///////////////////////////////////////////////////////////////////////////////
//! Returns true if the given MODE-A code is the special MODE-A 8888,
//!false otherwise.
/*!
 */
inline bool isSpecialModeA(const qint32 mode_a)
{
    return (mode_a >= 0) && isSpecialModeA(static_cast<quint32>(mode_a));
}

///////////////////////////////////////////////////////////////////////////////
//! Returns true if the given MODE-S code is valid, false otherwise.
/*! Valid codes are 0 to 2^24-1.
 */
inline bool isModeSValid(const quint32 mode_s)
{
    return mode_s <= AV_MODE_S_MAX;
}

///////////////////////////////////////////////////////////////////////////////
//! Returns true if the given MODE-S code is valid, false otherwise.
/*! Valid codes are 0 to 2^24-1.
 */
inline bool isModeSValid(const qint32 mode_s)
{
    return (mode_s >= 0) && isModeSValid(static_cast<quint32>(mode_s));
}

///////////////////////////////////////////////////////////////////////////////
//! Returns id string for provided sac, sic. See also getSacSicFromString.
AVLIB_EXPORT QString getSacSicString(int sac, int sic);

///////////////////////////////////////////////////////////////////////////////
//! Extracts sac, sic from provided id string. See also getSacSicString.
AVLIB_EXPORT bool getSacSicFromString(const QString& sac_sic_string, int& sac, int& sic);

///////////////////////////////////////////////////////////////////////////////
//! Extracts an unsigned integer value from the given bit sequence.
/*! Extracts an unsigned integer value from the given bit sequence, where
    from_bit_index starts at the LSB (which has index 0) and bit_count
    counts towards the MSB. Thus from_bit_index must be <= 31,
    bit_count must be >= 1 and <= 32 and from_bit_index+bit_count must be
    <= 32. It is assumed that sizeof(quint32) equals 32.
 */
AVLIB_EXPORT quint32 extractBitsFromUInt32(quint32 bit_sequence, uint from_bit_index, uint bit_count);

///////////////////////////////////////////////////////////////////////////////
//! Overloaded and extended operator new - don't use in new code, only kept for convenience
/*! This version allows for an additional LOG_HERE macro parameter (as define in avlog.h) which is now ignored.
*/

AVLIB_EXPORT void *operator new(std::size_t num_bytes, int line, const char *file);

///////////////////////////////////////////////////////////////////////////////
//! Overloaded and extended operator new[] - don't use in new code, only kept for convenience
/*! This version allows for an additional LOG_HERE macro parameter (as define in avlog.h) which is now ignored.
*/

AVLIB_EXPORT void *operator new[](std::size_t num_bytes, int line, const char *file);

///////////////////////////////////////////////////////////////////////////////
//! Overloaded delete operators for the new operators.
/*! This is required to avoid the following warning under Windows:
    warning C4291: 'void *operator new(size_t,int,const char *)' :
    no matching operator delete found; memory will not be freed if initial
    initalization throws an exception
*/
AVLIB_EXPORT void operator delete(void *p, int line, const char *file);

///////////////////////////////////////////////////////////////////////////////
//! Overloaded delete operators for the new operators.
/*! This is required to avoid the following warning under Windows:
    warning C4291: 'void *operator new(size_t,int,const char *)' :
    no matching operator delete found; memory will not be freed if initial
    initalization throws an exception
*/
AVLIB_EXPORT void operator delete[](void *p, int line, const char *file);

///////////////////////////////////////////////////////////////////////////////
//! Returns the system username.
/*! The method checks for the environment variable USER and returns the value
    as QString. If the variable can not be found QString::null is returned.
    TODO RG Move this to AVEnvironment
*/
AVLIB_EXPORT QString AVGetSystemUsername();

///////////////////////////////////////////////////////////////////////////////
//! Escapes and unescapes special strings by the provided escape sequences.
/*!
 *  Replaces all strings from esc_from which are contained in str by the corresponding
 *  string from esc_to. esc_from and esc_to must have the same size.
 *  Use with exchanged "esc_from" and "esc_to" parameters to unescape strings.
 *
 *  It is the responsibility of the user to avoid ambiguous calls, such as
 *  overlapping esc_froms in the input string.
 *
 *  \param str      The input string.
 *  \param esc_from A list of strings which should be replaced.
 *  \param esc_to   A list of strings which should replace the ones from esc_from.
 *  \return         The string obtained by modifying the input string.
 */
AVLIB_EXPORT QString AVEscape(const QString& str, const QStringList& esc_from,
                 const QStringList& esc_to);


///////////////////////////////////////////////////////////////////////////////
/**
 * Qt5: wraps QHostInfo::localHostName for Qt3 backwards compatibility.
 *      DEPRECATED, use native Qt functionality instead.
 *
 * Qt3: implemented via gethostname, see related documentation.
 */
AVLIB_EXPORT QString AVHostName();

///////////////////////////////////////////////////////////////////////////////

//! Splits a string into strings whenever a delimiter occurs.
/*! A delimiter character that is contained within quotes defined by quote
    will not be considered as a delimiter but as part of the string instead.
    \param delim Delimiter used to separate strings
    \param str The string to be split
    \param quote A character used to quote a certain part of the string.
           Delimiters that occur inside quotes will not be considered as such.
           If a quote occurs *inside* a string (e.g. "ab\"c;d\"ef") the result
           is undefined. It is also not possible to escape quotes to be
           considered as part of the string.
    \param leaveQuotes If leaveQuotes is true, the quotes won't be removed
           when splitting the string.
    \param allowEmptyEntries If allowEmptyEntries is true, a null string is
           inserted in the list wherever the separator matches twice without
           intervening text.
    \return List of separated strings.
    \sa QStringList::split()
 */
inline QStringList AVStringSplit(const QChar& delim, const QString& str,
                                 bool allowEmptyEntries = false,
                                 const QChar& quote = '\"',
                                 bool leaveQuotes = false)
{
    if (quote.isNull())
        return str.split(delim, allowEmptyEntries ?
                                QString::KeepEmptyParts : QString::SkipEmptyParts);

    QStringList ret;

    const uint l = str.length();
    uint index = 0;
    const QChar* uc = str.unicode() + index;
    const QChar* end = str.unicode() + l;
    bool do_split = true;
    int last_match = 0, sub_len = 0;

    while (uc < end)
    {
        if (*uc == quote)
            do_split = !do_split;
        else if (*uc == delim && do_split)
        {
            index = static_cast<int>(uc - str.unicode());
            sub_len = index - last_match;
            if (sub_len > 0 || allowEmptyEntries)
            {
                if (!leaveQuotes                 &&
                    sub_len > 0                  &&
                    str.at(last_match) == quote  &&
                    str.at(index - 1)  == quote)
                {
                    ret << str.mid(last_match + 1, sub_len - 2);
                }
                else
                {
                    ret << ((sub_len > 0) ?
                            str.mid(last_match, sub_len) :
                            QString::null);
                }
            }
            last_match = index + 1;
        }
        uc++;
    }

    index = static_cast<int>(uc - str.unicode());
    sub_len = index - last_match;
    if (sub_len > 0 || allowEmptyEntries)
    {
        ret << ((sub_len > 0) ?
                str.mid(last_match, sub_len) :
                QString::null);
    }

    return ret;
}


/////////////////////////////////////////////////////////////////////////////
//! serialize a UINT8
inline void serializeUINT8(QDataStream& ds, quint8 data)
{
    ds << data;
}

/////////////////////////////////////////////////////////////////////////////
//! serialize a quint16, first LOW, then HIGH byte
inline void serializeUINT16LH(QDataStream& ds, quint16 data)
{
    ds << static_cast<uchar>(data & 0x00ff);
    ds << static_cast<uchar>((data >> 8) & 0x00ff);
}

/////////////////////////////////////////////////////////////////////////////
//! serialize a INT16, first LOW, then HIGH byte
inline void serializeINT16LH(QDataStream& ds, qint16 data)
{
    ds << static_cast<uchar>(data & 0x00ff);
    ds << static_cast<uchar>((data >> 8) & 0x00ff);
}

/////////////////////////////////////////////////////////////////////////////
//! serialize a UINT32, first LOW, 2nd, 3rd and then HIGH byte
inline void serializeUINT32LH(QDataStream& ds, quint32 data)
{
    ds << static_cast<uchar>(data & 0x000000ff);
    ds << static_cast<uchar>((data >> 8) & 0x000000ff);
    ds << static_cast<uchar>((data >> 16) & 0x000000ff);
    ds << static_cast<uchar>((data >> 24) & 0x000000ff);
}

/////////////////////////////////////////////////////////////////////////////
//! serialize a INT32, first LOW, 2nd, 3rd and then HIGH byte
inline void serializeINT32LH(QDataStream& ds, qint32 data)
{
    ds << static_cast<uchar>(data & 0x000000ff);
    ds << static_cast<uchar>((data >> 8) & 0x000000ff);
    ds << static_cast<uchar>((data >> 16) & 0x000000ff);
    ds << static_cast<uchar>((data >> 24) & 0x000000ff);
}

/////////////////////////////////////////////////////////////////////////////
//! serialize a quint16, first HIGH, then LOW byte (big endian)
inline void serializeUINT16HL(QDataStream& ds, quint16 data)
{
    ds << static_cast<uchar>((data >> 8) & 0x00ff);
    ds << static_cast<uchar>(data & 0x00ff);
}

/////////////////////////////////////////////////////////////////////////////
//! serialize a UINT32, highest byte first (big endian)
inline void serializeUINT32HL(QDataStream& ds, quint32 data)
{
    ds << static_cast<uchar>((data >> 24) & 0xff);
    ds << static_cast<uchar>((data >> 16) & 0xff);
    ds << static_cast<uchar>((data >> 8) & 0xff);
    ds << static_cast<uchar>(data & 0xff);
}

/////////////////////////////////////////////////////////////////////////////
//! serialize a UINT64, highest byte first (big endian)
inline void serializeUINT64HL(QDataStream& ds, quint64 data)
{
    ds << static_cast<uchar>((data >> 56) & 0xff);
    ds << static_cast<uchar>((data >> 48) & 0xff);
    ds << static_cast<uchar>((data >> 40) & 0xff);
    ds << static_cast<uchar>((data >> 32) & 0xff);
    ds << static_cast<uchar>((data >> 24) & 0xff);
    ds << static_cast<uchar>((data >> 16) & 0xff);
    ds << static_cast<uchar>((data >> 8) & 0xff);
    ds << static_cast<uchar>(data & 0xff);
}

/////////////////////////////////////////////////////////////////////////////
//! de-serialize a UINT8
inline void deserializeUINT8(QDataStream& ds, quint8 &data)
{
    ds >> data;
}


/////////////////////////////////////////////////////////////////////////////
//! de-serialize a quint16, first LOW, then HIGH byte
inline void deserializeUINT16LH(QDataStream& ds, quint16 &data)
{
    uchar low, high;
    ds >> low >> high;
    data = (high << 8) | low;
}

/////////////////////////////////////////////////////////////////////////////
//! de-serialize a INT16, first LOW, then HIGH byte
inline void deserializeINT16LH(QDataStream& ds, qint16 &data)
{
    uchar low, high;
    ds >> low >> high;
    data = static_cast<qint16>((high << 8) | low);
}

/////////////////////////////////////////////////////////////////////////////
//! de-serialize a UINT32, first LOW, 2nd, 3rd and then HIGH byte
inline void deserializeUINT32LH(QDataStream& ds, quint32 &data)
{
    uchar low, second, third, high;
    ds >> low >> second >> third >> high;
    data = static_cast<quint32>((high << 24) | (third << 16) | (second << 8) | low);
}

/////////////////////////////////////////////////////////////////////////////
//! de-serialize a INT32, first LOW, 2nd, 3rd and then HIGH byte
inline void deserializeINT32LH(QDataStream& ds, qint32 &data)
{
    uchar low, second, third, high;
    ds >> low >> second >> third >> high;
    data = static_cast<qint32>((high << 24) | (third << 16) | (second << 8) | low);
}

/////////////////////////////////////////////////////////////////////////////
//! de-serialize a quint16, first HIGH, then LOW byte (big endian)
inline void deserializeUINT16HL(QDataStream& ds, quint16 &data)
{
    uchar low, high;
    ds >> high >> low;
    data = (high << 8) | low;
}

/////////////////////////////////////////////////////////////////////////////
//! de-serialize a UINT32, beginning with the HIGH bytes (big endian)
inline void deserializeUINT32HL(QDataStream& ds, quint32 &data)
{
    uchar low, second, third, high;
    ds >> high >> third >> second >> low;
    data = static_cast<quint32>((high << 24) | (third << 16) | (second << 8) | low);
}

/////////////////////////////////////////////////////////////////////////////
//! de-serialize a UINT64, beginning with the HIGH bytes (big endian)
inline void deserializeUINT64HL(QDataStream& ds, quint64 &data)
{
    uchar low, second, third, fourth, fifth, sixth, seventh, high;
    ds >> high >> seventh >> sixth >> fifth >> fourth >> third >> second >> low;
    quint64 high_half = (high << 24) | (seventh << 16) | (sixth << 8) | fifth;
    quint64 low_half = (fourth << 24)| (third << 16) | (second << 8) | low;
    data =  (high_half << 32) | (low_half & 0xffffffffLL);;
}

/////////////////////////////////////////////////////////////////////////////

/**
 *  Breaks a string on whitespaces (or other symbols) so its parts won't exceed a given length.
 *  The parts of the string are returned in a QStringList so they can be joined by an arbitrary
 *  string (e.g. "<br>" or simply "\n").
 *  Note that the break characters are consumed when used for breaking!
 *
 *  If soft is false (the default), the parts in the returned list will never exceed max_length,
 *  even if this means that a string is broken in the middle of a word.
 *  If soft is true, words in str which exceed max_length will not be split, thus leading to
 *  parts exceeding max_length.
 *
 *  This method returns an empty QStringList if passed a null string, and a QStringList containing
 *  an empty string if passed an empty string. If a max_length of 0 is passed, it returns a
 *  QStringList containing a single entry corresponding to str.
 *
 *  \param str          The string to break.
 *  \param max_length   The maximum line length after which the string should be broken.
 *  \param soft         If false, the strings will never exceed max_length, even if this means that
 *                      a string is broken in the middle of a word. If true, words exceeding
 *                      max_length will not be broken.
 *  \param break_regexp The regular expression which determines where a string can be broken.
 *  \return             A list of the string parts.
 */
AVLIB_EXPORT QStringList AVBreakString(
        QString str, uint max_length, bool soft = false, QRegExp break_regexp = QRegExp("\\s"));

///////////////////////////////////////////////////////////////////////////////
//! \brief Qt hash function for shared_ptrs

template <typename T>
uint qHash(const std::shared_ptr<T>& ptr, uint seed = 0)
{
    return qHash(ptr.get(), seed);
}

///////////////////////////////////////////////////////////////////////////////
//! \brief Qt hash function for enums

template <typename Enum>
typename std::enable_if<std::is_enum<Enum>::value, uint>::type
qHash(Enum value)
{
    return qHash(static_cast<typename std::underlying_type<Enum>::type>(value));
}

#endif

// End of file
