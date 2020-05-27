///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
// QT-Version: QT4
//
// Module:    AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Wolfgang Eder, w.eder@avibit.com
    \author  QT4-PORT: Andreas Eibler, a.eibler@avibit.com
    \brief

    Provides system profiling facilities.
*/

// AVLib includes
#include "avdatetime.h"
#include "avenvironment.h"
#include "avlog.h"
#include "avmisc.h"
#include "avsysprof.h"
#include "avsysprofconfig.h"
#include "avthread.h"

///////////////////////////////////////////////////////////////////////////////

AVSysProf::AVSysProf()
{
    m_process_name = AVEnvironment::getProcessName();
    if (m_process_name.startsWith("./"))
        m_process_name = m_process_name.mid(2);

    m_config = new AVSysProfConfig();
    bool enable_system_profiling = m_config->m_enable_system_profiling;
    if (m_config->m_exception_process_names.contains(m_process_name))
        enable_system_profiling = !enable_system_profiling;

    if (!enable_system_profiling)
    {
        AVLogInfo << "AVSysProf " << m_process_name << " disabled";
        m_logger = 0;
        return;
    }

    AVLogInfo << "AVSysProf " << m_process_name << " enabled";
    m_host_name = AVHostName();
    // the suffix "prof1" is intended to include the file format version
    m_logger = new AVLog(m_process_name + "prof1");
    AVLogWriter& logWriter = m_logger->logWriter()->getShare();
    logWriter.setAlwaysFlush(false);
    logWriter.setLogThreadContextChanges(false);
    m_separator = "\t";

#if defined(Q_OS_UNIX)
    struct timespec ts;
    AVASSERT(clock_getres(CLOCK_MONOTONIC, &ts) == 0);
    AVASSERT(ts.tv_sec == 0);
    AVLogInfoTo(*m_logger) << "AVSysProf clock resolution is " << ts.tv_nsec << " nanoseconds";
#else
    AVLogInfoTo(*m_logger) << "AVSysProf clock resolution is 1 millisecond";
#endif // Q_OS_UNIX

    AVLogInfoTo(*m_logger)
        << m_separator
        << "currentDateTime" << m_separator
        << "nanoseconds" << m_separator
        << "host" << m_separator
        << "process" << m_separator
        << "instance" << m_separator
        << "thread" << m_separator
        << "type" << m_separator
        << "activity/key" << m_separator
        << "id/value" << m_separator
        << "elapsed";

    if (AVConsole::isSingletonInitialized())
        AVConsole::singleton().registerSlot(
            "avSysProf",
            this, SLOT(slotConsoleExecuteCommand(AVConsoleConnection&,const QStringList&)),
            "Start or stop profiling task.\n"
            " usage: avSysProf START <activity>\n"
            " avSysProf STOP <activity>",
            SLOT(slotCompleteCommand(QStringList&,const QStringList&)));
}

///////////////////////////////////////////////////////////////////////////////

AVSysProf::~AVSysProf()
{
    delete m_logger;
    delete m_config;
}

///////////////////////////////////////////////////////////////////////////////

AVSysProfNanoseconds AVSysProf::startActivity(const QString& activity, const QString& id, int process_instance)
{
    if (m_logger == 0) return -1;
    static const QString type = "START";
    AVSysProfNanoseconds ns = nanoseconds();
    AVLogInfoTo(*m_logger)
        << m_separator
        << currentDateTime() << m_separator
        << ns << m_separator
        << m_host_name << m_separator
        << m_process_name << m_separator
        << process_instance << m_separator
        << AVThread::threadName() << m_separator
        << type << m_separator
        << activity << m_separator
        << id;
    return ns;
}

///////////////////////////////////////////////////////////////////////////////

AVSysProfNanoseconds AVSysProf::stopActivity(
    const QString& activity, const QString& id, int process_instance, AVSysProfNanoseconds start)
{
    if (m_logger == 0) return -1;
    static const QString type = "STOP";
    if (start == -1)
    {
        AVLogInfoTo(*m_logger)
            << m_separator
            << currentDateTime() << m_separator
            << nanoseconds() << m_separator
            << m_host_name << m_separator
            << m_process_name << m_separator
            << process_instance << m_separator
            << AVThread::threadName() << m_separator
            << type << m_separator
            << activity << m_separator
            << id;
        return -1;
    }
    else
    {
        AVSysProfNanoseconds ns = nanoseconds();
        AVSysProfNanoseconds elapsed = ns - start;
        AVLogInfoTo(*m_logger)
            << m_separator
            << currentDateTime() << m_separator
            << ns << m_separator
            << m_host_name << m_separator
            << m_process_name << m_separator
            << process_instance << m_separator
            << AVThread::threadName() << m_separator
            << type << m_separator
            << activity << m_separator
            << id << m_separator
            << elapsed / 1000;
        return elapsed;
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVSysProf::recordString(const QString& key, const QString& valueToString, int process_instance)
{
    if (m_logger == 0) return;
    static const QString type = "VALUE";
    AVLogInfoTo(*m_logger)
        << m_separator
        << currentDateTime() << m_separator
        << nanoseconds() << m_separator
        << m_host_name << m_separator
        << m_process_name << m_separator
        << process_instance << m_separator
        << AVThread::threadName() << m_separator
        << type << m_separator
        << key << m_separator
        << valueToString;
}

///////////////////////////////////////////////////////////////////////////////

QString AVSysProf::formatDateTime(const QDateTime& dt)
{
    const QDate d = dt.date();
    const QTime t = dt.time();
    QString result;
    static const char *format = "%.4d-%.2d-%.2dT%.2d:%.2d:%.2d.%.3d";
    result.sprintf(format, d.year(), d.month(), d.day(), t.hour(), t.minute(), t.second(), t.msec());
    return result;
}

///////////////////////////////////////////////////////////////////////////////

QString AVSysProf::currentDateTime() const
{
    return formatDateTime(AVDateTime::currentDateTimeUtc());
}

///////////////////////////////////////////////////////////////////////////////

AVSysProfNanoseconds AVSysProf::nanoseconds() const
{
    AVSysProfNanoseconds result;
#if defined(Q_OS_UNIX)
    struct timespec ts;
    AVASSERT(clock_gettime(CLOCK_MONOTONIC, &ts) == 0);
    AVASSERT(ts.tv_sec >= 0);
    AVASSERT(ts.tv_nsec >= 0);
    AVASSERT(ts.tv_nsec < 1000000000);
    // this expression overflows
    // result = ts.tv_sec * 1000000000 + ts.tv_nsec;
    result = static_cast<AVSysProfNanoseconds>(ts.tv_sec) * 1000000000 + ts.tv_nsec;
#else
    QDateTime now = AVDateTime::currentDateTimeUtc();
    // this expression overflows
    // result = (now.toTime_t() * 1000 + now.time().msec()) * 1000000;
    result = (static_cast<AVSysProfNanoseconds>(now.toTime_t()) * 1000 + now.time().msec()) * 1000000;
#endif // Q_OS_UNIX
    AVASSERT(result >= 0);
    static AVSysProfNanoseconds prev_result = -1;
    if (result < prev_result)
    {
        AVLogErrorTo(*m_logger) << "AVSysProf::nanoseconds jump from " << prev_result << " to " << result;
    }
    prev_result = result;
    return result;
}

///////////////////////////////////////////////////////////////////////////////

void AVSysProf::slotConsoleExecuteCommand(AVConsoleConnection& connection, const QStringList& args)
{
    if (args.count() != 2)
    {
        connection.printError("Invalid argument: Number of arguments must be 2.");
        return;
    }
    QString status = args[0];
    QString activity = args[1];

    if (status == "START")
        this->startActivity(activity);
    else if (status == "STOP")
        this->stopActivity(activity);
    else
        connection.printError("Argument 1 must be either START or STOP.");
}

///////////////////////////////////////////////////////////////////////////////

void AVSysProf::slotCompleteCommand(QStringList& completions, const QStringList& args)
{
    if (args.count() == 1)
        completions << "START" << "STOP";

    completions.sort();
}

///////////////////////////////////////////////////////////////////////////////

AVSysProfScope::AVSysProfScope(const QString& activity, int process_instance, AVSysProfNanoseconds *elapsed) :
    m_activity(activity), m_id(EmptyQString), m_process_instance(process_instance), m_elapsed(elapsed)
{
    m_start = AVSysProf::start(activity, EmptyQString, process_instance);
}

///////////////////////////////////////////////////////////////////////////////

AVSysProfScope::AVSysProfScope(
    const QString& activity, const QString& id, int process_instance, AVSysProfNanoseconds *elapsed) :
    m_activity(activity), m_id(id), m_process_instance(process_instance), m_elapsed(elapsed)
{
    m_start = AVSysProf::start(activity, id, process_instance);
}

///////////////////////////////////////////////////////////////////////////////

AVSysProfScope::~AVSysProfScope()
{
    AVSysProfNanoseconds elapsed = AVSysProf::stop(m_activity, m_id, m_process_instance, m_start);
    if (m_elapsed != 0)
        *m_elapsed = elapsed;
}

// End of file
