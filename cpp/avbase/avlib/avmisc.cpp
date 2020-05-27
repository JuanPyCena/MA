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
    \brief     Implementation of miscellaneous helper classes and functions.
*/


#include <iostream>
#include <limits>
#include <random>
using namespace std;

// qt includes
#include <QRegExp>
#include <QDir>
#include <QFile>
#include <QMutex>
#include <QElapsedTimer>
#include <QTimer>
#include <QHostInfo>

// avlib includes
#include "avmisc.h"
#include "avthread.h"
#include "avfromtostring.h"
#include "avdeprecate.h"
#include "avenvironment.h"
// ATTENTION: the following headers are only included here to force the generation of the ctor and dtor
//            of the abstract, dll-exported class (e.g. AVClientHandlerCallbackIface) using MSVC 2013 compiler.
//            Otherwise unresolved symbols get reported when linking libraries using this interface
//            (e.g. AVArchiveReplayCallback in avarchcommmonlib).
#include "avclienthandlercallbackiface.h"
#include "avrwydesignatorprovider.h"
#include "avstringmap.h"

#if defined(Q_OS_UNIX)
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif
#if defined(Q_OS_WIN32)
#include <winsock2.h>
#include <windows.h>
#include <Lmcons.h>
#undef max
#undef min
#endif

const QString EmptyQString = "";
const QStringList EmptyQStringList;

const quint32 MODEA_REQ = 8888;

///////////////////////////////////////////////////////////////////////////////

const QString& notNull(const QString& value)
{
    if (value.isNull()) return EmptyQString;
    return value;
}

///////////////////////////////////////////////////////////////////////////////

int avpow(int base, uint exponent)
{
    if (exponent == 0) return 1;
    if (exponent == 1) return base;

    int tmp = avpow(base, exponent/2);
    if (exponent%2 == 0)
        return tmp * tmp;
    return base * tmp * tmp;
}

///////////////////////////////////////////////////////////////////////////////
// NOTE: not used actually. This class only exists for the sleep call.

void AVMiscPrivate::run()
{
}

///////////////////////////////////////////////////////////////////////////////

qint64 runEventLoop(qint64 msecs)
{
    QElapsedTimer timer;
    timer.start();
    QTimer::singleShot(msecs,Qt::PreciseTimer, qApp,SLOT(quit()));
    qApp->exec();
    return timer.elapsed();
}

///////////////////////////////////////////////////////////////////////////////

bool runEventLoop(qint64 msecs, qint64 min, qint64 max)
{
    qint64 span = runEventLoop(msecs);

    return (min <= span && span <= max);
}

///////////////////////////////////////////////////////////////////////////////

QDateTime AVParseDateTimeHuman(const QString &s)
{
    QRegExp regexp("[-:/. ]");
    QStringList stl_dt = s.split(regexp);

    AVASSERTNOLOGGER(stl_dt.size()==6 || stl_dt.size()==7);

    bool ok=false;
    int yyyy = stl_dt[0].toInt(&ok);
    AVASSERTNOLOGGER(ok);
    int MM   = stl_dt[1].toInt(&ok);
    AVASSERTNOLOGGER(ok);
    int dd   = stl_dt[2].toInt(&ok);
    AVASSERTNOLOGGER(ok);
    int hh   = stl_dt[3].toInt(&ok);
    AVASSERTNOLOGGER(ok);
    int mm   = stl_dt[4].toInt(&ok);
    AVASSERTNOLOGGER(ok);
    int ss   = stl_dt[5].toInt(&ok);
    AVASSERTNOLOGGER(ok);
    int zzz=0;
    if(stl_dt.size()>6)
    {
        zzz  = stl_dt[6].toInt(&ok);
        AVASSERTNOLOGGER(ok);
    }

    return AVDateTime(QDate(yyyy, MM, dd),QTime(hh, mm, ss, zzz));
}

///////////////////////////////////////////////////////////////////////////////

QDateTime AVParseDateTime(const QString &s)
{
    AVDEPRECATE(AVParseDateTime);
    AVDateTime dt;
    if(AVFromString(s, dt))
        return dt;
    QDate date;
    if(AVFromString(s, date))
        return AVDateTime(date, QTime(0,0,0));

    return AVDateTime();
}

///////////////////////////////////////////////////////////////////////////////

QTime AVParseTimeHuman(const QString &s)
{
    QRegExp regexp("[:. ]");
    QStringList stl_time = s.split(regexp);

    QStringList::Iterator it = stl_time.begin();
    int hh   = (*(it++)).toInt();
    int mm   = (*(it++)).toInt();
    int ss   = (*(it++)).toInt();
    int zzz  = (*(it++)).toInt();

    return QTime(hh, mm, ss, zzz);

}

///////////////////////////////////////////////////////////////////////////////

QTime AVParseTime(const QString &s)
{
    AVDEPRECATE(AVParseTime);
    QTime time;
    if(AVFromString(s, time))
        return time;
    return QTime();
}

///////////////////////////////////////////////////////////////////////////////

QDate AVParseDate(const QString &s)
{
    AVDEPRECATE(AVParseDate);
    QDate date;
    if(AVFromString(s, date))
        return date;
    return QDate();
}

///////////////////////////////////////////////////////////////////////////////

QString AVPrintDateTime(const QDateTime &dt)
{
    AVDEPRECATE(AVPrintDateTime);
    if (dt.isValid()) {
        return QString(dt.toString("yyyyMMddhhmmss"));
    } else {
        return QString("0");
    }
}

///////////////////////////////////////////////////////////////////////////////

QString AVPrintDate(const QDate &dt)
{
    AVDEPRECATE(AVPrintDate);
    if (dt.isValid()) {
        return QString(dt.toString("yyyyMMdd"));
    } else {
        return QString("0");
    }
}

///////////////////////////////////////////////////////////////////////////////

QString AVPrintTime(const QTime &dt)
{
    AVDEPRECATE(AVPrintTime);
    if (dt.isValid()) {
        return QString(dt.toString("hhmmss"));
    } else {
        return QString("0");
    }
}

///////////////////////////////////////////////////////////////////////////////

QString AVPrintDateHuman(const QDate &date)
{
    if (date.isValid()) {
        return QString(date.toString("yyyy-MM-dd"));
    } else {
        return QString("0");
    }
}

///////////////////////////////////////////////////////////////////////////////

QString AVPrintDateTimeHuman(const QDateTime &dt)
{
    if (dt.isValid()) {
        return QString(dt.toString("yyyy-MM-dd hh:mm:ss"));
    } else {
        return QString("InvalidDT");
    }
}

///////////////////////////////////////////////////////////////////////////////

QString AVPrintDateTimeMsHuman(const QDateTime &dt)
{
    if (dt.isValid()) {
        return QString(dt.toString("yyyy-MM-dd hh:mm:ss.zzz"));
    } else {
        return QString("InvalidDT");
    }
}

///////////////////////////////////////////////////////////////////////////////

QString AVPrintDateTimeMs(const QDateTime &dt)
{
    if (dt.isValid()) {
        return QString(dt.toString("yyyyMMddhhmmss.zzz"));
    } else {
        return QString("0");
    }
}

///////////////////////////////////////////////////////////////////////////////

QString AVPrintDtTm(const QDateTime &dt)
{
    return dt.toString (DT_FORMAT_STRING);
}

/////////////////////////////////////////////////////////////////////////////

bool makeDirectories(const QString& directories)
{
    QString converted_directories = directories;
    converted_directories.replace("\\", "/");

    bool is_windows_and_preceeding_drive_in_path = false;
    QDir* dir = NULL;
    QDir directories_dircheck(converted_directories);

    if (!directories_dircheck.isRelative())
    {
        if (converted_directories.startsWith("/"))
        {
            // UNIX
            AVLogger->Write(LOG_DEBUG2, "makeDirectories: unix");
            dir = new QDir("/");
            AVASSERT(dir != 0);
        }
        else if (converted_directories[0].isLetter() &&
                 converted_directories[1] == ':')
        {
            // WINDOWZE
            AVLogger->Write(LOG_DEBUG2, "makeDirectories: win: "+
                            converted_directories.left(3));

            dir = new QDir(converted_directories.left(3));
            AVASSERT(dir != 0);
            is_windows_and_preceeding_drive_in_path = true;
        }
        else
        {
            // SOMETHING STRANGE ?!
            AVLogger->Write(LOG_DEBUG2, "makeDirectories: "
                            "%s does not start with \\, / or drive letter",
                            qPrintable(converted_directories));
        }
    }
    else
    {
        dir = new QDir(QDir::currentPath());
        AVASSERT(dir != 0);
    }

    AVASSERT(dir);

    AVLogger->Write(LOG_DEBUG2, "makeDirectories: (" +
                    converted_directories + ")");

    QStringList dirlist = converted_directories.split("/", QString::SkipEmptyParts);

    QStringList::iterator it;
    for (it = dirlist.begin(); it != dirlist.end(); ++it)
    {
        AVLogger->Write(LOG_DEBUG2, "makeDirectories: (" + *it + ")");

        if (is_windows_and_preceeding_drive_in_path)
        {
            is_windows_and_preceeding_drive_in_path = false;
            continue;
        }

        if (!dir->exists(*it))
            if (!dir->mkdir(*it))
            {
                AVLogger->Write(LOG_ERROR, "makeDirectories: "
                                "Could not mkdir %s", qPrintable(*it));
                delete dir;
                return false;
            }

        dir->cd(*it);
    }

    delete dir;
    return true;
}

///////////////////////////////////////////////////////////////////////////////

QString &AVsprintf(QString &s, const char *fmt, ...)
{
    va_list ap;
    char buf[2048];
    va_start(ap, fmt);
    vsnprintf(buf, 2048, fmt, ap);
    buf[2047] = '\0';
    va_end(ap);
    s = buf;
    return s;
}

///////////////////////////////////////////////////////////////////////////////

uint AVSeed()
{
    //note just using msecs here is not sufficient, test-framework starts processes in same ms!
    std::random_device random_device;
    uint seed = random_device() % std::numeric_limits<uint>::max();
    return seed;
}

///////////////////////////////////////////////////////////////////////////////

QString AVHexDump(const unsigned char* addr, int len)
{
    int HEXDUMPWIDTH = 16;
    int i,n;
    unsigned char c;

    QString reply, tmp;

    n=0;

    while(n<len)
    {
        AVsprintf(tmp, "%8x    ", n);
        reply += tmp;
        for (i=0; i<HEXDUMPWIDTH; i++)
        {
            if (n+i < len)
            {
                c = *(addr+n+i);
                AVsprintf(tmp, "%02x ", c);
                reply += tmp;
            }
            else
                reply += "   ";
        }
        for (i=0; i<HEXDUMPWIDTH; i++)
        {
            if (n+i < len)
            {
                c = *(addr+n+i);
                if( (c >=32) && (c<=126) )
                    reply += c;
                else
                    reply += " ";
            }
            else
                reply += " ";
        }
        n += HEXDUMPWIDTH;
        reply += "\n";
    }
    return reply;
}

///////////////////////////////////////////////////////////////////////////////

QString AVHexDump(const QByteArray &array)
{
    return AVHexDump(reinterpret_cast<const unsigned char*>(array.data()), array.size());
}

///////////////////////////////////////////////////////////////////////////////

void AVLogMultiLines(const int line, const char *file,
                     const int lev, const QString& str, AVLog *log)
{
    QRegExp reg("[\r\n]");
    QStringList list = str.split(reg, QString::SkipEmptyParts);

    AVLog * target_logger = log ? log : AVLogger;
    for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it )
    {
        target_logger->Write(line, file, lev, "%s", qPrintable(*it));
    }
}

///////////////////////////////////////////////////////////////////////////////

QString AVHostName()
{
    return QHostInfo::localHostName();
}

///////////////////////////////////////////////////////////////////////////////

bool AVResolveHost(const QString &name, QHostAddress &resultHostAddress)
{
    AVLogDebug2 << "AVResolveHost: trying to resolve: " << name;

    if (resultHostAddress.setAddress(name))
    {
        AVLogDebug2 << "AVResolveHost: we already got an address: " << resultHostAddress.toString();
        return true;
    }

    resultHostAddress = QHostAddress();

    QHostInfo hostinfo = QHostInfo::fromName(name);

    if (hostinfo.error() != QHostInfo::NoError)
    {
        AVLogError << "AVResolveHost: could not resolve ("<< name <<"): " << hostinfo.errorString();
        return false;
    }

    bool returnVal = false;

    Q_FOREACH(const QHostAddress& foundAddress, hostinfo.addresses())
    {
        // if we got an ipv4 address, we assign it to the result and exit right away.
        // if we got an ipv6 address, we assign it to the result, but we keep looking because we give ipv4 a higher priority.

        if (foundAddress.protocol() == QAbstractSocket::IPv4Protocol)
        {
            AVLogDebug2 << "AVResolveHost: found IPv4 result: " << foundAddress.toString();
            resultHostAddress = foundAddress;
            return true;
        }
        else if (foundAddress.protocol() == QAbstractSocket::IPv6Protocol)
        {
            if (resultHostAddress.isNull())
            {
                AVLogDebug2 << "AVResolveHost: found IPv6 result: " << foundAddress.toString();
                resultHostAddress = foundAddress;
                returnVal = true;
            }
            else
            {
                AVLogDebug2 << "AVResolveHost: found IPv6 result: " << foundAddress.toString() << ", ignoring";
            }
        }
    }

    if (!returnVal) { AVLogError << "AVResolveHost: could not resolve ("<< name <<")"; }
    return returnVal;
}

/////////////////////////////////////////////////////////////////////////////

bool hasClientDisconnected(QTcpSocket& sd)
{
    // check for socket errors and bail out if yes

    //QT4PORT code taken from QT4 porting documentation
    qint64 available = 0;
    if (sd.waitForReadyRead(0))
        available = sd.bytesAvailable();
    bool timeout = (sd.error() == QAbstractSocket::SocketTimeoutError);

    if(available < 0 || (available == 0 && !timeout))
        return true;
    else
        return false;
}

/////////////////////////////////////////////////////////////////////////////

bool copyFiles(const QString& fromfilename,
               const QString& tofilename,
               uint chunksize, bool doflush)
{
    QDir fromfilename_dircheck(fromfilename);
    if (fromfilename_dircheck.isRelative())
    {
        AVLogger->Write(LOG_ERROR, "copyFiles: "
                        "fromfilename is not absolute");
        return false;
    }

    QDir tofilename_dircheck(tofilename);
    if (tofilename_dircheck.isRelative())
    {
        AVLogger->Write(LOG_ERROR, "copyFiles: "
                        "tofilename is not absolute");
        return false;
    }

    QFile file_from(fromfilename);
    QFile file_to(tofilename);

    if (!file_from.open(QIODevice::ReadOnly))
    {
        AVLogger->Write(LOG_ERROR, "copyFiles: "
                        "could not open from-file %s", qPrintable(fromfilename));
        return false;
    }

    if (!file_to.open(QIODevice::WriteOnly))
    {
        AVLogger->Write(LOG_ERROR, "copyFiles: "
                        "could not open to-file %s", qPrintable(tofilename));
        return false;
    }

    AVLogger->Write(LOG_DEBUG2, "copyFiles: "
                    "Copying %s to %s", qPrintable(fromfilename),
                    qPrintable(tofilename));

    long bytes_read = 0;

    do
    {
        QByteArray ba;
        ba.resize(chunksize);

        bytes_read = file_from.read(ba.data(), chunksize);

        AVLogger->Write(LOG_DEBUG2, "read %ld bytes", bytes_read);

        if (bytes_read < 0)
        {
            AVLogger->Write(LOG_ERROR,
                            "copyFiles: could not write to to-file %s",
                            qPrintable(tofilename));

            file_from.close();
            file_to.close();

            if (!QDir().remove(tofilename))
            {
                AVLogger->Write(LOG_ERROR,
                                "copyFiles: Could not remove tmpfile %s",
                                qPrintable(tofilename));
            }

            return false;
        }

        file_to.write(ba.data(), bytes_read);
    }
    while (bytes_read > 0 && !file_from.atEnd());

    if (doflush) file_to.flush();
    file_from.close();
    file_to.close();

    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool checkDSIOStatus(const QDataStream &s, const QString &log_prefix)
{
    const QIODevice *dev = s.device();
    if (dev == 0) {
        if (!AVLogger->isLevelSuppressed(AVLog::LOG__DEBUG2))
            AVLogger->Write(LOG_DEBUG2, "%sdevice is null", qPrintable(log_prefix));
        return false;
    }
    QDataStream::Status status = s.status();
    if (status == QDataStream::Ok) return true;

    if (!AVLogger->isLevelSuppressed(AVLog::LOG__DEBUG2))
        AVLogger->Write(LOG_DEBUG2, "%s%s",
                        qPrintable(log_prefix),
                        status == QDataStream::ReadPastEnd ? "ReadPastEnd" :
                        status == QDataStream::ReadCorruptData ? "ReadCorruptData" :
                        "invalid status");
    return false;
}

///////////////////////////////////////////////////////////////////////////////

QStringList AVBreakString(QString str, uint max_length, bool soft, QRegExp break_regexp)
{
    if (str.isNull()) return QStringList();

    if (str.isEmpty()) return QStringList("");
    if (max_length == 0) return QStringList() << str;

    QStringList ret;
    QStringList input_lines = str.split("\n");

    for (QStringList::const_iterator it = input_lines.begin(); it != input_lines.end(); ++it)
    {
        QString cur_line = *it;

        if (cur_line.isEmpty())
        {
            ret << "";
            continue;
        }

        while (static_cast<uint>(cur_line.length()) > max_length)
        {
            // whether the break occured in the middle of a word (and we don't have to skip the
            // whitespace)
            bool hard_break = false;
            int break_index = break_regexp.lastIndexIn(cur_line, max_length);

            if (break_index == -1)
            {
                if (soft)
                {
                    break_index = break_regexp.indexIn(cur_line, max_length);
                    if (break_index == -1) break_index = cur_line.length();
                } else
                {
                    break_index = max_length;
                    hard_break = true;
                }
            }

            ret << cur_line.left(break_index);
            cur_line = cur_line.mid(break_index + (hard_break ? 0 : 1));
        }
        // Append the remaining string
        if (!cur_line.isEmpty()) ret << cur_line;
    }

    return ret;
}

///////////////////////////////////////////////////////////////////////////////

QString getSacSicString(int sac, int sic)
{
    return QString("%1/%2").arg(sac).arg(sic);
}

///////////////////////////////////////////////////////////////////////////////

bool getSacSicFromString(const QString& sac_sic_string, int& sac, int& sic)
{
    bool convok = false;
    sac = sac_sic_string.section("/", 0, 0).toInt(&convok);
    if (!convok) return false;
    sic = sac_sic_string.section("/", 1, 1).toInt(&convok);
    if (!convok) return false;
    return true;
}

///////////////////////////////////////////////////////////////////////////////

quint32 extractBitsFromUInt32(quint32 bit_sequence, uint from_bit_index, uint bit_count)
{
    AVASSERT(sizeof(quint32) == 4);
    AVASSERT(from_bit_index < 32);
    AVASSERT(bit_count > 0);
    AVASSERT(bit_count <= 32);
    AVASSERT(from_bit_index + bit_count <= 32);

    quint32 mask = 0;
    uint count = from_bit_index;
    for (; count < from_bit_index+bit_count; ++count) mask |= (1 << count);
    bit_sequence &= mask;

    bit_sequence = bit_sequence >> from_bit_index;

    return bit_sequence;
}

///////////////////////////////////////////////////////////////////////////////

//! Overloaded and extended operator new[] to avoid throwing an exception
//! and return 0 in case of a memory allocation failure. This version
//! also allows for an additional LOG_HERE macro parameter (as define in avlog.h)
//! to have the calling line and filename for the logger.
void *operator new(std::size_t num_bytes, int line, const char *file)
{
    void *p = ::operator new(num_bytes);
    if (p == 0) {
        LOGGER_ROOT.Write(line, file, AVLog::LOG__FATAL, "memory allocation error");
    }
    return p;
}

void *operator new[](std::size_t num_bytes, int line, const char *file)
{
    void *p = ::operator new[](num_bytes);
    if (p == 0) {
        LOGGER_ROOT.Write(line, file, AVLog::LOG__FATAL, "memory allocation error");
    }
    return p;
}

///////////////////////////////////////////////////////////////////////////////
//! Overloaded delete operators for the new operators.
/*! This is required to avoid the following warning under Windows:
    warning C4291: 'void *operator new(size_t,int,const char *)' :
    no matching operator delete found; memory will not be freed if initial
    initalization throws an exception.
    It is incorrect to write "delete p" here. The job here is to free the memory,
    not call the destructor (which is impossible because the type is a void*).
*/

void operator delete(void *p, int line, const char *file)
{
    Q_UNUSED(line);
    Q_UNUSED(file);

    ::operator delete(p);
}

void operator delete[](void *p, int line, const char *file)
{
    Q_UNUSED(line);
    Q_UNUSED(file);

    ::operator delete[](p);
}

///////////////////////////////////////////////////////////////////////////////

QString AVGetSystemUsername()
{
    // verbose "false" to avoid deadlock, see SWE-5055
    QString user_name = AVEnvironment::getEnv("USER", false, AVEnvironment::NoPrefix);

    if(user_name.isEmpty()) // try "username", which is used in Windows without cygwin
        user_name = AVEnvironment::getEnv("username", false, AVEnvironment::NoPrefix);

    return user_name;

    // TODO CM use alternative implementation from below in the future?
    // Does using ENV variables make a difference? Is there a reason why we use them?

#if defined(Q_OS_UNIX)
    char buf [L_cuserid+1];
    size_t bufsize = L_cuserid+1;
    int ret = getlogin_r(buf, bufsize);
    if (ret != 0) return QString::null;
    else return buf;
#endif

#if defined(Q_OS_WIN32)
    char username[UNLEN+1];
    DWORD username_len = UNLEN+1;
    if (GetUserName(username, &username_len) == 0) return QString::null;
    else return username;
#endif
}

///////////////////////////////////////////////////////////////////////////////

QString AVEscape(const QString& str, const QStringList& esc_from,
                 const QStringList& esc_to)
{
    AVASSERT(esc_from.count() == esc_to.count());
    QString ret;
    if (str.isNull())  return ret;
    if (str.isEmpty()) return EmptyQString;

    ret.reserve(str.length());
    int cur_pos = 0;
    while (cur_pos < str.length())
    {
        bool replaced = false;
        for (int esc_index=0; esc_index<esc_from.count(); ++esc_index)
        {
            const QString& from = esc_from[esc_index];
            QStringRef str_ref = str.midRef(cur_pos, from.size());
            if (str_ref == from)
            {
                replaced = true;
                ret += esc_to[esc_index];
                cur_pos += from.length();
                break;
            }
        }
        if (!replaced) {
            ret += str[cur_pos];
            ++cur_pos;
        }
    }
    return ret;
}

///////////////////////////////////////////////////////////////////////////////

// End of file
