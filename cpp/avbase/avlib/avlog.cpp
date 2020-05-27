///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:     AVLIB - AviBit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Dr. Thomas Leitner, t.leitner@avibit.com
    \author    QT4-PORT: Matthias Fuchs, m.fuchs@avibit.com
    \brief   Message logging class implementation.

    This class provides basic message logging facilities.
*/


// QT includes
#include <qglobal.h>
#include <QApplication>
#include <QRegExp>
#include <QVarLengthArray>
#include <QtGlobal>

// system includes
#if defined(Q_OS_UNIX)
#include <cstdlib>
#include <cstdarg>
#include <cstring>
#include <cerrno>
#include <csignal>
#endif
#include <iostream>
using namespace std;

// QT includes
#include <QMessageBox>
#include <QDir>
#include <QStringList>
#include <QTimer>
#include <QRegularExpression>

#if defined(Q_OS_WIN32)
#include <windows.h>
#endif

// local includes
#include "avdeprecate.h"
#include "avlog.h"
#include "avshare.h"
#include "avmisc.h"
#include "avsnprintf.h"
#include "avexithandler.h"
#include "avdaemon.h"
#include "avenvironment.h"
#include "avpackageinfo.h"
#include "avdatetime.h"


///////////////////////////////////////////////////////////////////////////////

bool AVLogWriter::m_prependCrOnStdout       = false;

///////////////////////////////////////////////////////////////////////////////

AVExitHandler *AVLog::m_exitHandler   = 0;
bool           AVLog::m_fatalExit     = true;
bool           AVLog::m_fatalSegfault = true;
AVLog         *AVLog::m_root_logger   = 0;
AVLog::ExecutionPhase AVLog::m_execution_phase = AVLog::EP_RUNNING;
QList<QRegularExpression> AVLog::m_fatal_error_whitelist;
QMutex         AVLog::m_fatal_error_whitelist_mutex;

///////////////////////////////////////////////////////////////////////////////

AVLogFactory* volatile AVLogFactory::m_instance = 0;
QMutex AVLogFactory::m_singleton_mutex;

///////////////////////////////////////////////////////////////////////////////

//This is a copy of qCleanupFuncinfo in qtbase rev 8ad59677
QByteArray avCleanupFuncinfo(QByteArray info)
{
    // Strip the function info down to the base function name
    // note that this throws away the template definitions,
    // the parameter types (overloads) and any const/volatile qualifiers.

    if (info.isEmpty())
        return info;

    int pos;

    // Skip trailing [with XXX] for templates (gcc), but make
    // sure to not affect Objective-C message names.
    pos = info.size() - 1;
    if (info.endsWith(']') && !(info.startsWith('+') || info.startsWith('-'))) {
        while (--pos) {
            if (info.at(pos) == '[')
                info.truncate(pos);
        }
    }

    // operator names with '(', ')', '<', '>' in it
    static const char operator_call[] = "operator()";
    static const char operator_lessThan[] = "operator<";
    static const char operator_greaterThan[] = "operator>";
    static const char operator_lessThanEqual[] = "operator<=";
    static const char operator_greaterThanEqual[] = "operator>=";

    // canonize operator names
    info.replace("operator ", "operator");

    // remove argument list
    forever {
        int parencount = 0;
        pos = info.lastIndexOf(')');
        if (pos == -1) {
            // Don't know how to parse this function name
            return info;
        }

        // find the beginning of the argument list
        --pos;
        ++parencount;
        while (pos && parencount) {
            if (info.at(pos) == ')')
                ++parencount;
            else if (info.at(pos) == '(')
                --parencount;
            --pos;
        }
        if (parencount != 0)
            return info;

        info.truncate(++pos);

        if (info.at(pos - 1) == ')') {
            if (info.indexOf(operator_call) == pos - (int)strlen(operator_call))
                break;

            // this function returns a pointer to a function
            // and we matched the arguments of the return type's parameter list
            // try again
            info.remove(0, info.indexOf('('));
            info.chop(1);
            continue;
        } else {
            break;
        }
    }

    // find the beginning of the function name
    int parencount = 0;
    int templatecount = 0;
    --pos;

    // make sure special characters in operator names are kept
    if (pos > -1) {
        switch (info.at(pos)) {
        case ')':
            if (info.indexOf(operator_call) == pos - (int)strlen(operator_call) + 1)
                pos -= 2;
            break;
        case '<':
            if (info.indexOf(operator_lessThan) == pos - (int)strlen(operator_lessThan) + 1)
                --pos;
            break;
        case '>':
            if (info.indexOf(operator_greaterThan) == pos - (int)strlen(operator_greaterThan) + 1)
                --pos;
            break;
        case '=': {
            int operatorLength = (int)strlen(operator_lessThanEqual);
            if (info.indexOf(operator_lessThanEqual) == pos - operatorLength + 1)
                pos -= 2;
            else if (info.indexOf(operator_greaterThanEqual) == pos - operatorLength + 1)
                pos -= 2;
            break;
        }
        default:
            break;
        }
    }

    while (pos > -1) {
        if (parencount < 0 || templatecount < 0)
            return info;

        char c = info.at(pos);
        if (c == ')')
            ++parencount;
        else if (c == '(')
            --parencount;
        else if (c == '>')
            ++templatecount;
        else if (c == '<')
            --templatecount;
        else if (c == ' ' && templatecount == 0 && parencount == 0)
            break;

        --pos;
    }
    info = info.mid(pos + 1);

    // remove trailing '*', '&' that are part of the return argument
    while ((info.at(0) == '*')
           || (info.at(0) == '&'))
        info = info.mid(1);

    // we have the full function name now.
    // clean up the templates
    while ((pos = info.lastIndexOf('>')) != -1) {
        if (!info.contains('<'))
            break;

        // find the matching close
        int end = pos;
        templatecount = 1;
        --pos;
        while (pos && templatecount) {
            char c = info.at(pos);
            if (c == '>')
                ++templatecount;
            else if (c == '<')
                --templatecount;
            --pos;
        }
        ++pos;
        info.remove(pos, end - pos + 1);
    }

    return info;
}

///////////////////////////////////////////////////////////////////////////////

/**
 * @brief qtMessageHandler is a custom message handler to ensure that messages emitted by Qt also end up in our log files.
 */
static void qtMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &m)
{
    if (type == QtWarningMsg)
    {
        constexpr const char* warnings_to_criticals[] {
            "QObject:", //is mostly about timers and connect.
            "QThread:"
        };
        if (std::any_of(std::cbegin(warnings_to_criticals), std::cend(warnings_to_criticals),
                        [&m] (auto& message_part) { return m.contains(message_part); }))
        {
            type = QtCriticalMsg;
        }
    }

    QString msg = qFormatLogMessage(type, context, m);

    // no logging instantiated yet
    if (AVLogger == 0) {
        cout << msg.toUtf8().constData() << "\n" << flush;

        if (type == QtFatalMsg) abort();

        return;
    }

    // output log msg
    switch (type) {
        case QtDebugMsg:
            // make sure msg is written to cout when debugging is off
            if (AVLogger->minLevel() >= AVLog::LOG__INFO)
            {
                AVLogger->Write(context.line, context.file, AVLog::LOG__DEBUG | FORCE_STDOUT_FLAG, msg);
            } else
            {
                if(!AVLogger->isLevelSuppressed(AVLog::LOG__DEBUG))
                    AVLogStream(context.line, context.file, AVLog::LOG__DEBUG, AVLogger).getStream() << msg;
            }
            break;
        case QtWarningMsg:
            if(!AVLogger->isLevelSuppressed(AVLog::LOG__WARNING))
                AVLogStream(context.line, context.file, AVLog::LOG__WARNING, AVLogger).getStream() << msg;
            break;
        case QtCriticalMsg:
            if(!AVLogger->isLevelSuppressed(AVLog::LOG__ERROR))
                AVLogStream(context.line, context.file, AVLog::LOG__ERROR, AVLogger).getStream() << msg;
            break;
        case QtFatalMsg:
            AVLogStream(context.line, context.file, AVLog::LOG__FATAL, AVLogger).getStream() << msg;
            abort();
        case QtInfoMsg:
        default:
            if(!AVLogger->isLevelSuppressed(AVLog::LOG__INFO))
                AVLogStream(context.line, context.file, AVLog::LOG__INFO, AVLogger).getStream() << msg;
            break;
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//TODO_QT3
//TODO_QT3 class AVLog::GUILogEvent : public QEvent
//TODO_QT3 {
//TODO_QT3 public:
//TODO_QT3     GUILogEvent(int level, const QString& message)
//TODO_QT3         : QEvent(QEvent::User + 1), m_level(level), m_message(message)
//TODO_QT3     {}
//TODO_QT3
//TODO_QT3     int level() const { return m_level; }
//TODO_QT3
//TODO_QT3     QString message() const { return m_message; }
//TODO_QT3
//TODO_QT3 private:
//TODO_QT3     int m_level;
//TODO_QT3     QString m_message;
//TODO_QT3 };


AVLogListener::AVLogListener()
  : m_minLevel(AVLog::LOG__DEBUG2)
{
}

///////////////////////////////////////////////////////////////////////////////

AVLogListener::~AVLogListener()
{
}

///////////////////////////////////////////////////////////////////////////////

void AVLogListener::addLogMsg(int level, const QString &message)
{
    if (logLevel(level))
        doAddLogMsg(level, message);
}

///////////////////////////////////////////////////////////////////////////////

int AVLogListener::minLevel() const
{
    return m_minLevel;
}

///////////////////////////////////////////////////////////////////////////////

void AVLogListener::setMinLevel(int minLevel)
{
    AVASSERT (minLevel >= AVLog::LOG__DEBUG2 && minLevel < AVLog::LOG__OFF);
    m_minLevel = minLevel;
}

///////////////////////////////////////////////////////////////////////////////

bool AVLogListener::logLevel(int level) const
{
    int lev = level & ~LOG_ERRNO_FLAG;
    if (lev < m_minLevel || lev >= AVLog::LOG__OFF)
        return false;

    return true;
}

///////////////////////////////////////////////////////////////////////////////

AVLogMainThreadListenerHelper::AVLogMainThreadListenerHelper(AVLogMainThreadListener *listener) :
        m_listener(listener)
{
}

///////////////////////////////////////////////////////////////////////////////

AVLogMainThreadListenerHelper::~AVLogMainThreadListenerHelper()
{
}

///////////////////////////////////////////////////////////////////////////////

void AVLogMainThreadListenerHelper::scheduleProcessing()
{
    QTimerEvent *timer_event = new (LOG_HERE) QTimerEvent(0);
    qApp->postEvent(this, timer_event);
}

///////////////////////////////////////////////////////////////////////////////

void AVLogMainThreadListenerHelper::timerEvent(QTimerEvent*)
{
    m_listener->processLogMessages();
}

///////////////////////////////////////////////////////////////////////////////

AVLogMainThreadListener::AVLogMainThreadListener() :
    // recursive because calls to processLogMessages can trigger qt warnings, which results in calls to addLogMsg
    // from the same thread context.
    m_logListenerMutex(QMutex::Recursive),
    m_helper(this)
{
}

///////////////////////////////////////////////////////////////////////////////

AVLogMainThreadListener::~AVLogMainThreadListener()
{
}

///////////////////////////////////////////////////////////////////////////////

void AVLogMainThreadListener::addLogMsg(int level, const QString &message)
{
    if (!logLevel(level)) return;

    // this intentionally does not directly forward the log message if we are in the main thread
    // because this can (in conjunction with qt message handling) result in nested calls.

    QMutexLocker locker(&m_logListenerMutex);

    LogMessage msg;
    msg.logLevel = level;
    msg.message  = message;

    m_logEntries.push_back(msg);
    m_helper.scheduleProcessing();
}

///////////////////////////////////////////////////////////////////////////////

void AVLogMainThreadListener::processLogMessages()
{
    AVASSERT(AVThread::isMainThread());
    QMutexLocker locker(&m_logListenerMutex);

    // Calling doAddLogMsg can result in Qt warnings, leading to calls to addLogMsg ->
    // don't iterate over log entries, but pop them off the stack.
    // Don't do it until the stack is empty, but only for the number which is present upon method entry.
    // Otherwise, an endless recursive loop might occur if every call to doAddLogMsg results in a Qt warning.
    int count = m_logEntries.count();
    for (int i=0; i<count; ++i)
    {
        LogMessage cur_msg = m_logEntries.front();
        m_logEntries.pop_front();
        doAddLogMsg(cur_msg.logLevel, cur_msg.message);
    }
}

///////////////////////////////////////////////////////////////////////////////

AVLog::AVLog(const QString& filePrefix, const QString&  logDir,
             const QString& loggerId)
  : QObject(0),
    m_logWriter(0),
    m_logToFile(true),
    m_headerWritten(false),
    m_minLevel(LOG__INFO),
    m_echoStdout(false),
    m_echoStdoutLn(false),
    m_indent(0),
    m_mutex(QMutex::Recursive),
    m_logErrorLocation(true),
    m_logStdoutTimeStamps(true),
    m_logMSecTimeStamps(false),
    m_logView(0)
{
    setObjectName(loggerId);

    // clear gui output flags
    for (int i = 0; i < LOG__OFF; i++) {
        m_guiOutFlags[i] = m_guiOutLnFlags[i] = false;
    }

    internalInit(filePrefix, logDir, loggerId, false, true, LOG__INFO);
}

///////////////////////////////////////////////////////////////////////////////

AVLog::AVLog(const QString& filePrefix, const bool stdo, const int level,
             const bool fileLog, const QString& loggerId)
  : QObject(0),
    m_logWriter(0),
    m_logToFile(fileLog),
    m_headerWritten(false),
    m_minLevel(level),
    m_echoStdout(stdo),
    m_echoStdoutLn(false),
    m_indent(0),
    m_mutex(QMutex::Recursive),
    m_logErrorLocation(true),
    m_logStdoutTimeStamps(true),
    m_logMSecTimeStamps(false),
    m_logView(0)
{
    setObjectName(loggerId);

    // clear gui output flags
    for (int i = 0; i < LOG__OFF; i++) {
        m_guiOutFlags[i] = m_guiOutLnFlags[i] = false;
    }

    internalInit(filePrefix, QString(), loggerId, stdo, fileLog, level);
}

///////////////////////////////////////////////////////////////////////////////

void AVLog::setRootLogger(AVLog* logger)
{
    m_root_logger = logger;
}

///////////////////////////////////////////////////////////////////////////////

AVLog *AVLog::getRootLogger()
{
    return m_root_logger;
}

void AVLog::addFatalErrorWhitelistRegex(const QRegularExpression &regex)
{
    Q_ASSERT(regex.isValid());

    QMutexLocker locker(&m_fatal_error_whitelist_mutex);
    m_fatal_error_whitelist.append(regex);
}

const QList<QRegularExpression> AVLog::getFatalErrorWhitelist()
{
    QMutexLocker locker(&m_fatal_error_whitelist_mutex);
    return m_fatal_error_whitelist;
}

///////////////////////////////////////////////////////////////////////////////

void AVLog::startTimers()
{
    // the log registry potentially messes with our log writer at startup (see
    // AVLogFactory::registerLoggerInternal), so the access to the writer must be mutexed here.
    QMutexLocker locker(&m_mutex);

    AVASSERT(qApp != nullptr);
    AVASSERT(AVThread::isMainThread());

    m_logWriter->getShare().startReopenLogfileTimer();
    if (m_housekeeping_timer == nullptr)
    {
        m_housekeeping_timer = new QTimer(qApp);
        AVASSERT(connect(m_housekeeping_timer, SIGNAL(timeout()), this, SLOT(slotHousekeeping())));
        m_housekeeping_timer->start(HOUSEKEEP_TIMER_INTERVAL_MS);
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVLog::internalInit(const QString& pf, const QString& ld, const QString& id,
                         bool stdo, bool filelog, int level)
{
    if (qApp) moveToThread(qApp->thread());

    // install QT message handler
    qInstallMessageHandler(qtMessageHandler);

    // qInfo etc are typically not used by AviBit code -> the qtMessageHandler prefix makes it clear
    // that the origin of the message probably is Qt code.
    qSetMessagePattern("qtMessageHandler: %{function}(): %{message}");

    // ignore SIGPIPE signals (when clients bail out)
#if defined(Q_OS_UNIX)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
    ::signal(SIGPIPE, SIG_IGN);
#pragma GCC diagnostic pop
#endif

    QString fileNamePrefix, logDir, fileNameOverride;
    int closeInterval, logLevel;

    if(id != id.toUpper())
    {
        AVLogFatal<< "AVLog::internalInit: Logger ID is not uppercase: "<<id;
    }

    // set flags
    m_logWriter           = 0;
    m_id                  = id;
    m_logToFile           = filelog;
//     m_parentLogger        = 0;
    m_echoStdout          = stdo;

    // local vars
    fileNamePrefix        = pf;
    closeInterval         = 0;
    logLevel              = level;

    if (! m_id.isEmpty())
    {
        QRegExp re("([A-Z]|[0-9]|_)*");
        if (! re.exactMatch(m_id))
        {
            AVLogger->Write(LOG_WARNING, "Logger ID '" + m_id + "' does not conform "
                            "to environment variable conventions.");
        }
    }

    // if the logdir is specified as NULL, check if an environment
    // variable APP_HOME exists and set the logging directory accordingly
    if (ld.isNull()) {
        QString al = AVEnvironment::getApplicationLog();
        if (!al.isEmpty()) {
            QDir dir(al);
            if (!dir.exists()) dir.mkdir(al);
            logDir = dir.absolutePath();
        } else {
            QDir dir;
            logDir = dir.absolutePath();
        }
    } else {

        // absolute paths are used directly while relative paths are
        // appended to the applicationHome directory
        if (!QDir::isRelativePath(ld)) {
            logDir = QString(ld);
        } else {
            QString home = AVEnvironment::getApplicationHome();
            if (!home.isEmpty()) {
                logDir = home + "/" + QString(ld);
            } else {
                QDir dir;
                logDir = dir.absolutePath();
            }
        }
        QDir dir(logDir);
        if (!dir.exists()) dir.mkdir(logDir);
        logDir = dir.absolutePath();
    }


    // parse all environ vars an overwrite variables if environment var exists
    parseEnvironVars(logLevel, m_logToFile, fileNamePrefix, closeInterval,
                     fileNameOverride, m_echoStdout, m_echoStdoutLn,
                     m_logStdoutTimeStamps, m_logMSecTimeStamps);

    // set minimum level to info to avoid debugging msgs in std mode
    setMinLevel(logLevel);
    m_logView = 0;

    if (fileNamePrefix.isEmpty())
    {
        if (! m_id.isEmpty() && m_id != LOGGER_ROOT_ID)
        {
            fileNamePrefix = LOGGER_ROOT.logWriter()->getShare().
                             m_fileNamePrefix;
        }
    }

    // create log writer which can be shared
    AVLogWriter* writer = new AVLogWriter(fileNamePrefix, fileNameOverride,
                                          logDir, closeInterval*60, m_logToFile, m_echoStdout);
    AVASSERTNOLOGGER(writer != 0);
    m_logWriter = new AVShare<AVLogWriter, AVShareEmptyLoggerPolicy>(writer, "Log Writer");
    AVASSERTNOLOGGER(m_logWriter != 0);

    // forward signal from log writer
    QObject::connect(&(m_logWriter->getShare()),
                     SIGNAL(signalNewLogFile(const QString&)),
                     this,
                     SIGNAL(signalNewLogFile(const QString&)));

    connect(this, SIGNAL(signalNewLogFile(const QString&)), SLOT(slotRepeatedSuppressionsOverview()));

    if (qApp != nullptr)
    {
        // There is special handling for the root logger, which is instantiated before QApplication and before the
        // main thread is registered.
        // See avdaemon's call to AVLog::startTimers()
        if (AVThread::isMainThread())
        {
            startTimers();
        } else
        {
            qApp->postEvent(this, new QEvent(static_cast<QEvent::Type>(CustomEventType::START_TIMERS)));
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVLog::parseEnvironVars(int& level, bool& toFile, QString& filePrefix,
                             int& closeInterval, QString& fileNameOverride,
                             bool& stdo, bool& stdoutLines,
                             bool& stdoutTimestamps, bool &logMSecTimeStamps) const
{
    QString value;
    int foundAtLevel = -1;

    if (findEnvironVar(LOGGER_VARNAME_LEVEL, value, foundAtLevel))
    {
        level = value.toInt();
    }

    if (findEnvironVar(LOGGER_VARNAME_TO_FILE, value, foundAtLevel))
    {
        toFile = (value.toInt() != 0);
    }

    if (findEnvironVar(LOGGER_VARNAME_FILE_PREFIX, value, foundAtLevel))
    {
        filePrefix = value.trimmed();
    }

    if (findEnvironVar(LOGGER_VARNAME_CLOSE_INTERVAL, value, foundAtLevel))
    {
        closeInterval = value.toInt();
    }

    if (findEnvironVar(LOGGER_VARNAME_FILENAME, value, foundAtLevel))
    {
        fileNameOverride = value.trimmed();
    }

    if (findEnvironVar(LOGGER_VARNAME_STDOUT, value, foundAtLevel))
    {
        stdo = (value.toInt() != 0);
    }

    if (findEnvironVar(LOGGER_VARNAME_STDOUT_LINES, value, foundAtLevel))
    {
        stdoutLines = (value.toInt() != 0);
    }

    if (findEnvironVar(LOGGER_VARNAME_STDOUT_TIMESTAMPS, value, foundAtLevel))
    {
        stdoutTimestamps = (value.toInt() != 0);
    }

    if (findEnvironVar(LOGGER_VARNAME_MSEC_TIMESTAMPS, value, foundAtLevel))
    {
        logMSecTimeStamps = (value.toInt() != 0);
    }
}

///////////////////////////////////////////////////////////////////////////////

bool AVLog::findEnvironVar(const QString& varName,
                           QString& varValue, int& foundAtLevel) const
{
    static const QString SEP = "_";
    QString curVarName, curVarValue, loggerId;
    // ROOT logger ID shall be handled as empty ID
    if (m_id != LOGGER_ROOT_ID) loggerId = m_id;
    QStringList loggerIdSplit = loggerId.split(SEP);

    foundAtLevel = loggerIdSplit.count() + 1;

    while (loggerIdSplit.count() > 0)
    {
        curVarName  = loggerIdSplit.join(SEP) + SEP + varName;
        curVarValue = AVEnvironment::getEnv(curVarName, false,
                                       AVEnvironment::ProcessNamePrefix);
        if (! curVarValue.isEmpty())
        {
            varValue = curVarValue;
            return true;
        }
        curVarValue = AVEnvironment::getEnv(curVarName, false,
                                       AVEnvironment::AppNamePrefix);
        if (! curVarValue.isEmpty())
        {
            varValue = curVarValue;
            return true;
        }
        --foundAtLevel;
        loggerIdSplit.pop_back();
    }

    // ROOT logger
    curVarValue = AVEnvironment::getEnv(varName, false,
                                   AVEnvironment::ProcessNamePrefix);
    if (! curVarValue.isEmpty())
    {
        varValue = curVarValue;
        return true;
    }
    --foundAtLevel;

    // APP level
    curVarValue = AVEnvironment::getEnv(varName, false,
                                   AVEnvironment::AppNamePrefix);
    if (! curVarValue.isEmpty())
    {
        varValue = curVarValue;
        return true;
    }
    --foundAtLevel;

    return false;
}

///////////////////////////////////////////////////////////////////////////////

void AVLog::doGuiLog(int level, const QString& message)
{
    QString title;
    if (!m_appname.isEmpty()) title = m_appname + " - ";

    switch (level)
    {
        case LOG__INFO:
            title += "Info";
            break;
        case LOG__WARNING:
             title += "Warning";
             break;
        case LOG__ERROR:
            title += "Error";
            break;
        case LOG__FATAL:
            title += "Fatal";
            break;
        default:
            title += "UNKNOWN";
            break;
    }

    if (m_logView == 0)
    {
        switch (level)
        {
            case LOG__INFO:
                QMessageBox::information(0, title, message, "Ok", 0, 0, 0, 1);
                break;
            case LOG__ERROR: // fall through
            case LOG__FATAL:
                QMessageBox::critical(0, title, message, "Ok", 0, 0, 0, 1);
                break;
            default:
                QMessageBox::warning(0, title, message, "Ok", 0, 0, 0, 1);
                break;
        }
    } else m_logView->addLogMsg(level, message);
}

///////////////////////////////////////////////////////////////////////////////

void AVLog::logNewRepeatedSuppressedMessage(const AVLog::SuppressedRepeatedMessageInfo &info, uint duration)
{
    QString message;
    if(info.m_log_periodically)
    {
        message = QString("(suppressed for the next %1 seconds) ").arg(duration);
    } else
    {
        message = QString("(repeated occurrences suppressed) ");
    }
    message += info.m_msg;
    Write(info.m_line, info.m_file, info.m_level, message);
}

///////////////////////////////////////////////////////////////////////////////

void AVLog::slotHousekeeping()
{
    QMutexLocker locker(&m_mutex);
    auto removeElapsed = [this](QMap<QString, SuppressedRepeatedMessageInfo>& suppressions)
    {
        for(const SuppressedRepeatedMessageInfo& suppression_info : suppressions.values()) //clazy:exclude=container-anti-pattern
        {
            bool elapsed = suppression_info.m_expiration_time < AVDateTime::currentDateTimeUtc();
            if(elapsed)
                deleteRepeatedSuppression(suppression_info);
        }
    };

    removeElapsed(m_messages_suppressed_by_position);
    removeElapsed(m_messages_suppressed_by_text);
}

///////////////////////////////////////////////////////////////////////////////

void AVLog::slotRepeatedSuppressionsOverview()
{
    QMutexLocker locker(&m_mutex);

    auto informAboutSuppressions = [this](QMap<QString, SuppressedRepeatedMessageInfo>& suppressions)
    {
        for(SuppressedRepeatedMessageInfo& suppression_info : suppressions.values())
        {
            if (suppression_info.m_log_periodically) continue;
            logNewRepeatedSuppressedMessage(suppression_info, 0);
        }
    };

    informAboutSuppressions(m_messages_suppressed_by_position);
    informAboutSuppressions(m_messages_suppressed_by_text);
}
///////////////////////////////////////////////////////////////////////////////

void AVLog::slotRepeatLogPreamble()
{
    if (qApp)
        logPreamble(qApp->arguments());
}

///////////////////////////////////////////////////////////////////////////////

void AVLog::deleteRepeatedSuppression(const AVLog::SuppressedRepeatedMessageInfo& info)
{
    QMutexLocker locker(&m_mutex);

    // If this is logged periodically but only with a bigger interval, nothing is logged when the
    // condition no longer applies.
    if (!info.m_log_periodically)
    {
        QString message = QString("(no longer applies) ") + info.m_msg;
        Write(info.m_line, info.m_file, info.m_level, message);
    }

    if(info.m_ignore_content)
    {
        AVASSERT(m_messages_suppressed_by_position.contains(info.key()));
        m_messages_suppressed_by_position.remove(info.key());
    }
    else
    {
        AVASSERT(m_messages_suppressed_by_text.contains(info.key()));
        m_messages_suppressed_by_text.remove(info.key());
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVLog::setLogView(AVLogViewIface* view)
{
    // Use AVLog::registerListener instead.
    // TODO CM implement listener which replaces GUI logging functionality; remove GUI stuff from
    // log
    AVDEPRECATE(AVLog::setLogView);
    m_logView = view;
}

///////////////////////////////////////////////////////////////////////////////

void AVLog::registerListener(AVLogListener *listener, const QString &writerPrefix)
{
    QMutexLocker locker(&m_mutex);
    m_listener[listener] = writerPrefix;
}

///////////////////////////////////////////////////////////////////////////////

void AVLog::unregisterListener(AVLogListener *listener)
{
    QMutexLocker locker(&m_mutex);
    m_listener.remove(listener);
}

///////////////////////////////////////////////////////////////////////////////
// Method: AVLog, DTOR
// Desc:   Destructor, closes the logfile.

AVLog::~AVLog()
{
    delete m_logWriter;   // shared pointer

    static QMutex deleteMutex;
    deleteMutex.lock();

    delete m_exitHandler;
    m_exitHandler = 0;
    deleteMutex.unlock();
}

///////////////////////////////////////////////////////////////////////////////

AVLogWriter::AVLogWriter(const QString& fileNamePrefix,
                         const QString& fileNameOverride,
                         const QString& logDir, int closeIntervalSecs,
                         bool logToFile, bool echoStdout)
: m_fileNamePrefix(fileNamePrefix),
  m_fileNameOverride(fileNameOverride),
  m_logDir(logDir),
  m_closeIntervalSecs(closeIntervalSecs),
  m_logToFile(logToFile),
  m_echoStdout(echoStdout),
  m_alwaysFlush(true),
  m_logThreadContextChanges(true)
{
    if (qApp) moveToThread(qApp->thread());

    if (m_logToFile)
    {
        if (!m_fileNameOverride.isEmpty())
        {
            m_fileName = m_fileNameOverride;
        } else if (m_fileNamePrefix.isEmpty())
        {
            // fallback1
            if (m_fileNamePrefix.isEmpty())
                m_fileNamePrefix = AVGetProcessName();

            // fallback2
            if (m_fileNamePrefix.isEmpty())
                m_fileNamePrefix = AVEnvironment::getProcessName();

            // fallback3
            if (m_fileNamePrefix.isEmpty())
                m_fileNamePrefix = AVEnvironment::getApplicationName().toLower();

            // avoid filenames consisting only of a date
            AVASSERTNOLOGGER(! m_fileNamePrefix.isEmpty());
        }

        // we cannot simply call slotOpenNewLogfile here, because this would restart m_openNewLogfileTimer,
        // which is not possible if the QApplication has not yet been instantiated.

        // make e.g. truncateFile work by calculating this once right at startup time.
        recomputeFilename(AVDateTime::currentDateTimeUtc());

        if (qApp != 0)
        {
            // There is special handling for the root logger, which is instantiated before QApplication and before the
            // main thread is registered.
            // See avdaemon's call to AVLog::startTimers()
            if (AVThread::isMainThread())
            {
                startReopenLogfileTimer();
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

AVLogWriter::~AVLogWriter()
{
    closeLogFile(AVDateTime::currentDateTimeUtc());
}

///////////////////////////////////////////////////////////////////////////////

QString AVLogWriter::getLogBaseName() const
{
    if (! m_logToFile)
        return QString::null;
    if (! m_fileNameOverride.isEmpty())
        return m_fileNameOverride;
    return (m_logDir + "/" + m_fileNamePrefix);
}

///////////////////////////////////////////////////////////////////////////////

void AVLogWriter::setAlwaysFlush(bool flush)
{
    m_alwaysFlush = flush;
}

///////////////////////////////////////////////////////////////////////////////

void AVLogWriter::setLogThreadContextChanges(bool b)
{
    m_logThreadContextChanges = b;
}

///////////////////////////////////////////////////////////////////////////////

bool AVLogWriter::getLogThreadContextChanges() const
{
    return m_logThreadContextChanges;
}

///////////////////////////////////////////////////////////////////////////////

bool AVLogWriter::truncateLogfile()
{
    if (m_fileName.isEmpty()) return true;
    AVASSERT(!m_file.isOpen());

    QFile f(m_fileName);
    f.remove();
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void AVLogWriter::startReopenLogfileTimer()
{
    if (m_openNewLogfileTimer != nullptr) return;
    if (!m_logToFile || !m_fileNameOverride.isEmpty()) return;

    AVASSERT(qApp != nullptr);
    AVASSERT(AVThread::isMainThread());

    AVASSERT(QThread::currentThread() == qApp->thread());
    delete m_openNewLogfileTimer;
    m_openNewLogfileTimer = new QTimer(qApp);
    m_openNewLogfileTimer->setTimerType(Qt::PreciseTimer);
    m_openNewLogfileTimer->setSingleShot(true);
    AVASSERT(connect(m_openNewLogfileTimer, SIGNAL(timeout()),
                     SLOT(slotReopenLogfile())));

    uint msecsToNextLogfile;
    AVDateTime baseTime;
    computeIntervalTime(AVDateTime::currentDateTimeUtc(), m_closeIntervalSecs, baseTime, &msecsToNextLogfile);
    m_openNewLogfileTimer->start(msecsToNextLogfile);
}

///////////////////////////////////////////////////////////////////////////////

void AVLogWriter::setPrependCrOnStdout(bool a)
{
    m_prependCrOnStdout = a;
}

///////////////////////////////////////////////////////////////////////////////

bool AVLogWriter::getPrependCrOnStdout()
{
    return m_prependCrOnStdout;
}

///////////////////////////////////////////////////////////////////////////////

bool AVLogWriter::Write(const QDateTime& dt,
                        const QString& stdoutHeader,
                        const QString& fileHeader,
                        QString stdoutMsg,
                        QString fileMsg)
{
    // Precaution against deadlock: a QT warning produced in any of the code below would cause
    // this method to be recursively called.
    qInstallMessageHandler(0);

    stdoutMsg = stdoutHeader + stdoutMsg;
    fileMsg   = fileHeader   + fileMsg;

    QString threadName = m_logThreadContextChanges ? QThread::currentThread()->objectName() : QString::null;

    if(threadName.isNull())
        threadName = ((qApp == NULL) || qApp->thread() == QThread::currentThread()) ? AVThread::MAIN_THREAD_NAME : AVToString(QThread::currentThreadId());

    // Lock the mutex after acquiring the thread name, otherwise deadlock can occur - see NC-440.
    // It is unclear whether this is also true for Qt5.
    QMutexLocker locker(&m_mutex);

    if (m_logThreadContextChanges)
    {
        bool fileContextChanged   = m_lastThreadContextFile   != threadName;
        bool stdoutContextChanged = m_lastThreadContextStdout != threadName;
        if ( fileContextChanged || stdoutContextChanged)
        {
            QString threadMsg = QString("----- Thread context: %1 -----\n").arg(threadName);

            if (stdoutContextChanged && !stdoutMsg.isEmpty())
            {
                stdoutMsg = stdoutHeader + threadMsg + stdoutMsg;
                m_lastThreadContextStdout = threadName;
            }

            if (fileContextChanged && !fileMsg.isEmpty())
            {
                fileMsg   = fileHeader   + threadMsg + fileMsg;
                m_lastThreadContextFile = threadName;
            }
        }
    }

    bool logFileOpened = false;
    if (!internalWrite(dt, stdoutMsg, fileMsg, logFileOpened)) return false;

    if (logFileOpened)
    {
        m_mutex.unlock();
        emit signalNewLogFile(m_fileName);
        m_mutex.lock();
    }

    qInstallMessageHandler(qtMessageHandler);

    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVLogWriter::internalWrite(const QDateTime& dt, const QString& stdoutMsg, const QString& fileMsg,
                                bool& logFileOpened)
{
    logFileOpened = false;
    if (m_logToFile)
    {
        // check if a new file is needed
        bool new_file = recomputeFilename(dt);
        if (new_file || !m_file.isOpen())
        {
            openLogFile(dt);
            logFileOpened = true;
        }

        // return if logfile still not open
        if (!m_file.isOpen()) return false;

        m_outStream << fileMsg;

        // flush logfile output (might want to use "commit" mode under windows
        // and something similar under unix)
        if (m_alwaysFlush)
        {
            // note that it would be an error to flush the file here instead, because the qt5 text stream has an internal write buffer.
            m_outStream << flush;
        }
    }

    if (m_echoStdout)
    {
        if (m_prependCrOnStdout)
        {
            // There is room for optimization here, we could scan the message for \n instead
            // of replacing it.
            QString replace_str = stdoutMsg;
            replace_str.replace("\n", "\r\n");
            cout << replace_str.toUtf8().constData();
        } else
        {
            cout << stdoutMsg.toUtf8().constData();
        }
        if (m_alwaysFlush)
            cout << flush;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

void AVLogWriter::slotReopenLogfile()
{
    // don't hold the lock when emitting signalNewLogFile...
    bool new_logfile_opened = false;
    {
        AVASSERT(m_openNewLogfileTimer != nullptr);
        QMutexLocker locker(&m_mutex);
        QDateTime now(AVDateTime::currentDateTimeUtc());
        uint msecsToNextLogfile;
        // in case of race conditions, the log file may already have been opened...
        if (recomputeFilename(now, &msecsToNextLogfile))
        {
            openLogFile(now);
            new_logfile_opened = true;
        }
        m_openNewLogfileTimer->start(msecsToNextLogfile);
    }

    if (new_logfile_opened)
    {
        emit signalNewLogFile(m_fileName);
    }
}

///////////////////////////////////////////////////////////////////////////////

bool AVLog::isLevelSuppressed(const int lev) const
{
    int llev = lev & ~LOG_ERRNO_FLAG;
    // insure processing of FATAL logs because of exit handling
    return (llev < m_minLevel && llev != LOG__FATAL);
}

///////////////////////////////////////////////////////////////////////////////

void AVLog::writeCheckingRepeatSuppression(int line, const QString &file, int lev, const QString &log_message,
                          uint duration, bool log_periodically, bool ignore_contents)
{
    AVASSERT(duration != 0);

    QMutexLocker locker(&m_mutex);

    QMap<QString, SuppressedRepeatedMessageInfo>& suppressions = ignore_contents? m_messages_suppressed_by_position : m_messages_suppressed_by_text;
    QString key = SuppressedRepeatedMessageInfo::createKey(line, file, log_message, ignore_contents);

    if(!suppressions.contains(key))
    {
        SuppressedRepeatedMessageInfo suppression = SuppressedRepeatedMessageInfo(
                    AVDateTime::currentDateTimeUtc(), duration, log_periodically, ignore_contents, log_message, line, file, lev);
        // do this before updating our suppression data structure to avoid a summary in case this is the first thing logged (relevant for unit tests...)
        logNewRepeatedSuppressedMessage(suppression, duration);
        suppressions[key] = suppression;
    } else
    {
        SuppressedRepeatedMessageInfo& cur_info = suppressions[key];
        // TODO encapsulate this in SuppressedRepeatedMessageInfo
        if (!cur_info.m_log_periodically) cur_info.m_expiration_time = AVDateTime::currentDateTimeUtc().addSecs(duration);
    }
}

///////////////////////////////////////////////////////////////////////////////

bool AVLog::Write(int line, const QString& file,
                  int lev, const char *fmt, ...)
{
    bool forceStdout = lev & FORCE_STDOUT_FLAG;
    // if no logging is required -> return
    // insure processing of FATAL logs because of exit handling
    if (isLevelSuppressed(lev) && !forceStdout) return false;
     // Build the message now. The fixed stack buffer will suffice in most of the cases. If it does
     // not, allocate space for string dynamically.
     // Don't allocate extra space on stack to avoid stack overflow for really big log output.
     // Don't use static buffer because this function is called from different threads.
    QVarLengthArray<char, 1024> buffer(1024);

     va_list ap;
     va_start(ap, fmt);
     int str_len = vsnprintf(buffer.data(), buffer.size(), fmt, ap);
     va_end(ap);

     if (str_len >= buffer.size())
     {
         // Now we know the actual size required -> create a buffer on the heap
         // Terminating '\0' is not included in the size, add 1
         buffer.resize(str_len + 1);
         AVASSERTNOLOGGER(buffer.size() == str_len + 1);

         va_start(ap, fmt);
         AVASSERTNOLOGGER(vsnprintf(buffer.data(), buffer.size(), fmt, ap) == str_len);
         va_end(ap);
     } else if (str_len == -1)
     {
 #if defined (Q_OS_WIN32)
         // MSVC's vsnprintf won't tell us the required size of the buffer, but simply returns -1.
         // Try ever larger buffers until finding a fit.
         do
         {
             const int newSize = buffer.size() * 2;
             buffer.resize(newSize);
             AVASSERTNOLOGGER(buffer.size() == newSize);

             va_start(ap, fmt);
             // Leave room for terminating zero - see below.
             str_len = vsnprintf(buffer.data(), buffer.size() - 1, fmt, ap);
             AVASSERTNOLOGGER(str_len == -1 || str_len < buffer.size());
             va_end(ap);
         } while (str_len == -1);

         // In addition to returning -1 for too short a buffer, vsnprintf won't zero-terminate
         // the written string if it is an exact match - do it ourselves.
         buffer[str_len] = '\0';
 #else
         // Can this ever happen? Handle it anyway...
         strcpy(buffer.data(), "WARNING: vsnprintf I/O error!!");
 #endif
     }

    QString formatted_msg(buffer.data());
    buffer.clear();
    return this->Write(line,file,lev,formatted_msg);
}

///////////////////////////////////////////////////////////////////////////////
bool AVLog::Write(int line, const QString& file,
                  int lev, const QString &fmt)
{
    // first of all: save the error code and clear LOG_ERRNO_FLAG from lev.
    bool logErrno    = lev & LOG_ERRNO_FLAG;
    bool forceStdout = lev & FORCE_STDOUT_FLAG;
    lev &= ~LOG_ERRNO_FLAG;
    lev &= ~FORCE_STDOUT_FLAG;
    unsigned long err = 0;
#if defined(Q_OS_WIN32)
    if (logErrno) err = GetLastError();
#else
    if (logErrno) err = errno;
#endif

    // Working with the listeners has to be protected by a mutex. Only use that protection
    // if the listener container was not empty on first try.
    // This ensures that following access to the list is secured, no matter if the list was cleared
    // between the check here and the acquiring of the lock. Thus listeners can not be unregistered
    // after the call to buildLogHeader and before the listeners receive the log message.
    // Following the listener container will only be accessed if isMutexLocked is true
    bool isMutexLocked = false;
    QMutex *mutex = 0;
    if (!m_listener.isEmpty()) {
        mutex = &m_mutex;
        isMutexLocked = true;
    }
    QMutexLocker locker(mutex);

    // if no logging is required -> return
    // insure processing of FATAL logs because of exit handling
    bool logToFile  = m_logToFile;
    bool echoStdout = m_echoStdout;

    QString formatted_msg = fmt;

    QString env_str = AVEnvironment::getEnv(LOGGER_VARNAME_FATAL_ERRORS, false, AVEnvironment::NoPrefix);
    bool error_as_fatal = !env_str.isEmpty() && env_str != "0";
    if ((lev == AVLog::LOG__ERROR) && error_as_fatal)
    {
        const auto & whitelist = AVLog::getFatalErrorWhitelist();
        if (std::none_of(whitelist.begin(), whitelist.end(),
                         [&](const QRegularExpression& regex) -> bool { return regex.match(formatted_msg).hasMatch(); }))
        {
            lev = AVLog::LOG__FATAL;
            formatted_msg = "(errors are fatal) " + formatted_msg;
        }
    }

    // Make sure fatal messages are always sent to stdout
    // ugly design - why do both AVLog and AVLogWriter need the stdout flag?
    if ((lev & ~LOG_ERRNO_FLAG) == LOG__FATAL)
    {
        echoStdout = true;
        (*m_logWriter)->m_echoStdout = true;
    }
    // show qDebug messages even if debug messages are suppressed
    // override suppression with FORCE_STDOUT_FLAG
    if (isLevelSuppressed(lev))
    {
        if (forceStdout)
        {
            logToFile  = false;
            echoStdout = true;
        } else return false;
    }


    QDateTime dt = AVDateTime::currentDateTimeUtc();
    QString stdoutHeader, fileHeader;
    buildLogHeader(dt, lev, echoStdout ? &stdoutHeader : 0, logToFile ? &fileHeader : 0, isMutexLocked);


    if (m_indent != 0)
    {
        QString indent;
        indent.fill(' ', m_indent * 3 );
        formatted_msg = indent + formatted_msg;
        // Extra spaces to account for "INFO:      "
        formatted_msg.replace("\n", "\n" + indent + "                  ");
    }

    // Initally, all 3 messages share their string.
    QString fileMsg, stdoutMsg, guiMsg;
    if (logToFile)          fileMsg   = formatted_msg;
    if (echoStdout)         stdoutMsg = formatted_msg;
    if (m_guiOutFlags[lev]) guiMsg    = formatted_msg;

    // output file data and the system error message if requested
    if (logErrno              || (lev == LOG__WARNING) ||
        (lev == LOG__ERROR)   || (lev == LOG__FATAL)) {
        if (logErrno) {
            QString emsg = errnoMsg(err);
            if (!emsg.isEmpty()) {
                if (logToFile)          fileMsg   += "\n         " + emsg;
                if (echoStdout)         stdoutMsg += "\n         " + emsg;
                if (m_guiOutFlags[lev]) guiMsg    += "\n         " + emsg;
            }
        }
        QString fileName(file);
        QString ln= "\n         at line=" + QString::number(line) + ", file=" + fileName;

        if (m_logErrorLocation && logToFile) fileMsg += ln;
        if (echoStdout && (m_echoStdoutLn || lev == LOG__FATAL))
            stdoutMsg += ln;
        if (m_guiOutLnFlags[lev]) guiMsg += ln;
    }

    // terminate the line
    if (logToFile)  fileMsg   += "\n";
    if (echoStdout) stdoutMsg += "\n";

    // forward messages to log writer for thread-safe writing
    bool ret = m_logWriter->getShare().Write(dt, stdoutHeader, fileHeader, stdoutMsg, fileMsg);

//TODO_QT3     // gui output to log dialog or message box if requested
//TODO_QT3     // main thread logs directly, others need to post an even so the main thread
//TODO_QT3     // can do it for them (GUI can only be accessed by main thread)
//TODO_QT3     if (m_guiOutFlags[lev]) {
//TODO_QT3         if (!AVThread::isMainThread()) {
//TODO_QT3             GUILogEvent* logEvent = new GUILogEvent(lev, guiMsg);
//TODO_QT3             AVASSERT(logEvent != 0);
//TODO_QT3
//TODO_QT3             qApp->postEvent(this, logEvent);
//TODO_QT3         } else doGuiLog(lev, guiMsg);
//TODO_QT3     }

    // Calls all the listeners
    if (isMutexLocked) {
        ListenerContainer::iterator it;
        for (it = m_listener.begin(); it != m_listener.end(); ++it) {
            it.key()->addLogMsg(lev, formatted_msg);
        }
    }


    // exit if this is a fatal error
    if (lev == LOG__FATAL)
    {
        emit signalFatalLog();
        // use exit handler if available
        if (m_exitHandler)
        {
            m_exitHandler->handleExit();
            return ret;
        }

        // segfault if this is a fatal error
        if (m_fatalSegfault)
        {
            std::abort();
        }

        if (m_fatalExit) exit(1);
    }

    return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool AVLog::setMinLevel(const int lev)
{
    // get debugging level from environment if set
    QString dlvl = AVEnvironment::getEnv("DEBUG", false);
    if (!dlvl.isEmpty()) {
        m_minLevel = dlvl.toInt();
    } else {
        m_minLevel = lev;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVLog::setStdOut(const bool onoff, const bool addlinenumbers)
{
    m_echoStdout    = onoff;
    m_echoStdoutLn  = addlinenumbers;
    (*m_logWriter)->m_echoStdout = onoff; // also forward to log writer
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVLog::setGuiOut(const bool onoff, const int level,
                      const bool addlinenumbers)
{
    int lev = level & ~LOG_ERRNO_FLAG;
    if (lev < 0 || lev >= LOG__OFF) return false;
    m_guiOutFlags[lev]   = onoff;
    m_guiOutLnFlags[lev] = addlinenumbers;
    return true;
}

///////////////////////////////////////////////////////////////////////////////

QString AVLog::curFileName() const
{
    return m_logWriter->getShare().m_file.fileName();
}

///////////////////////////////////////////////////////////////////////////////

QString AVLog::fileNamePrefix() const
{
    return m_logWriter->getShare().m_fileNamePrefix;
}

///////////////////////////////////////////////////////////////////////////////

QString AVLog::fileNameOverride() const
{
    return m_logWriter->getShare().m_fileNameOverride;
}

///////////////////////////////////////////////////////////////////////////////

int AVLog::closeInterval() const
{
    return m_logWriter->getShare().m_closeIntervalSecs;
}

///////////////////////////////////////////////////////////////////////////////

void AVLog::incIndent()
{
    QMutexLocker locker(&m_indentMutex);
    ++m_indent;
}

///////////////////////////////////////////////////////////////////////////////

void AVLog::decIndent()
{
    QMutexLocker locker(&m_indentMutex);
    AVASSERT(m_indent != 0);
    --m_indent;
}

///////////////////////////////////////////////////////////////////////////////

AVLog::ExecutionPhase AVLog::getExecutionPhase()
{
    return m_execution_phase;
}

///////////////////////////////////////////////////////////////////////////////

void AVLog::setExecutionPhase(AVLog::ExecutionPhase phase)
{
    m_execution_phase = phase;
    AVLogInfo << "AVLog::setExecutionPhase: Phase now is " << enumToString(phase);
}

///////////////////////////////////////////////////////////////////////////////

void AVLog::startLogPreamble(const QStringList &arguments)
{
     AVCONNECT(&(m_logWriter->getShare()), SIGNAL(signalNewLogFile(const QString&)), this, SLOT(slotRepeatLogPreamble()), Qt::UniqueConnection);
     logPreamble(arguments);
}

///////////////////////////////////////////////////////////////////////////////

void AVLog::logPreamble(const QStringList &arguments)
{
    AVLogInfoTo(*this) << "Package "     << AVPackageInfo::getName()
                       << ", version "   << AVPackageInfo::getVersion()
                       << ", revision "  << AVPackageInfo::getRevision()
                       << ", build key " << AVPackageInfo::getBuildKey();
    AVLogInfoTo(*this) << "Arguments: " << arguments;
    AVLogInfoTo(*this) << "Running as user " << AVToString(AVGetSystemUsername());
}

///////////////////////////////////////////////////////////////////////////////
//                               I N T E R N A L
///////////////////////////////////////////////////////////////////////////////

bool AVLogWriter::recomputeFilename(const QDateTime& dt, uint * msecsToNextLogfile)
{
    AVASSERT(m_logToFile);

    // if there is an override filename set, don't do anything - it was adopted right at startup.
    if (!m_fileNameOverride.isEmpty()) return false;

    // Optimization to avoid recomputing the file name for each line of log output - only do it every minute.
    // We could optimize further and do this daily if no close interval is set, but it's not really worth it...
    if (msecsToNextLogfile == nullptr)
    {
        AVDateTime currentMinutes(dt.date(), QTime(dt.time().hour(), dt.time().minute()));
        if (currentMinutes == m_fileNameDateTime && msecsToNextLogfile == 0) return false;
        m_fileNameDateTime = currentMinutes;
    }

    // The time actually changed - recompute the file name.
    AVDateTime filenameDateTime;
    computeIntervalTime(dt, m_closeIntervalSecs, filenameDateTime, msecsToNextLogfile);

    QString timeString; // only compute this if the log file is regularly closed. Otherwise, hour/minutes are omitted from the log file name.
    if (m_closeIntervalSecs != 0)
    {
        if (m_closeIntervalSecs % 60 != 0)
        {
            AVsprintf(timeString, "-%02d%02d%02d", filenameDateTime.time().hour(), filenameDateTime.time().minute(), filenameDateTime.time().second());
        } else
        {
            AVsprintf(timeString, "-%02d%02d", filenameDateTime.time().hour(), filenameDateTime.time().minute());
        }
    }

    QString dateString;
    AVsprintf(dateString, "-%04d%02d%02d", filenameDateTime.date().year(), filenameDateTime.date().month(), filenameDateTime.date().day());

    // adopt new file name if it is different
    QString newFileName = m_logDir + '/' + m_fileNamePrefix + dateString + timeString + ".log";
    if (newFileName == m_fileName) return false;

    m_fileName = newFileName;

    return true;
}

///////////////////////////////////////////////////////////////////////////////

void AVLogWriter::computeIntervalTime(const QDateTime &now, uint secsInterval, AVDateTime &baseTime, uint *msecsUntilNext)
{
    baseTime.setDate(now.date());

    QTime midnight(0,0);
    if (secsInterval != 0)
    {
        uint secsSinceMidnight = midnight.secsTo(now.time());
        baseTime.setTime(QTime(midnight.addSecs(secsSinceMidnight - secsSinceMidnight % secsInterval)));
    } else
    {
        baseTime.setTime(midnight);
    }

    if (msecsUntilNext != nullptr)
    {
        AVDateTime nextTime;
        if (secsInterval == 0)
        {
            nextTime = baseTime.addDays(1);
        } else
        {
            nextTime = baseTime.addSecs(secsInterval);
        }

        *msecsUntilNext = -nextTime.msecsTo(now);
        AVASSERT(*msecsUntilNext > 0);
    }
}

///////////////////////////////////////////////////////////////////////////////

bool AVLogWriter::openLogFile(const QDateTime& dt)
{
    // don't open anything if we don't log to a file
    if (!m_logToFile) return true;

    closeLogFile(dt);

    // open the file now.
    m_file.setFileName(m_fileName);
    if (!m_file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        cerr << "** FATAL: can't open/create log-file "
             << m_fileName.toUtf8().constData() << endl;
        return false;
    } else {
        m_outStream.setDevice(&m_file);
        // don't rely on correct locale; setDevice resets the codec...
        m_outStream.setCodec("UTF-8");
    }

    // output a date/time stamp in the first line of the logfile
    QString timestr, datestr;
    AVsprintf(timestr, "%02d:%02d:%02d INFO:    ",
              dt.time().hour(), dt.time().minute(), dt.time().second());
    AVsprintf(datestr, "LOG FILE OPENED, DATE: %04d%02d%02d",
              dt.date().year(), dt.date().month(), dt.date().day());
    m_outStream << timestr << datestr << endl;

    // make sure that content is actually written into the new log file.
    m_outStream << flush;

    return true;
}

///////////////////////////////////////////////////////////////////////////////

void AVLogWriter::closeLogFile(const QDateTime& dt)
{
    if (m_file.isOpen())
    {
        QString timestr;
        AVsprintf(timestr, "%02d:%02d:%02d INFO:    ",
                  dt.time().hour(), dt.time().minute(), dt.time().second());
        m_outStream << timestr << "LOG FILE CLOSED" << endl;
        m_outStream << timestr << "EOF" << endl;
        m_file.close();
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVLog::customEvent(QEvent* event)
{
    AVASSERT(event != 0);

    switch (static_cast<CustomEventType>(event->type()))
    {
    case CustomEventType::START_TIMERS:
        startTimers();
        break;
        /* TODO qt5 porting
    case CustomEventType::LOG_GUI:
    {
        GUILogEvent* logEvent = dynamic_cast<GUILogEvent*>(event);
        AVASSERT(logEvent != 0);
        doGuiLog(logEvent->level(), logEvent->message());
        break;
    }
    */
    default:
        AVLogError << "AVLog::customEvent: unknown event type " << event->type();
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVLog::buildLogHeader(const QDateTime& dt, int lev, QString *stdoutHeader, QString *fileHeader, bool isMutexLocked)
{
    // TODO now that timestamping is enabled for stdout, there probably is no reason to
    // handle file and stdout header differently. Remove m_logStdoutTimeStamps?

    // output date/time stamp
    QTime t = dt.time();
    QString timestr;
    if (m_logMSecTimeStamps)
    {
        AVsprintf(timestr, "%02d:%02d:%02d.%03d ", t.hour(), t.minute(), t.second(), t.msec());
    } else
    {
        AVsprintf(timestr, "%02d:%02d:%02d ",      t.hour(), t.minute(), t.second());
    }

    if (fileHeader)                            *fileHeader   += timestr;
    if (stdoutHeader && m_logStdoutTimeStamps) *stdoutHeader += timestr;

    // output the header if it is set and not written yet
    if (!m_header.isEmpty() && !m_headerWritten) {
        m_headerWritten = true;
        if (fileHeader)   *fileHeader   += "HEADER:  " + m_header + "\n";
        if (stdoutHeader) *stdoutHeader += "HEADER:  " + m_header + "\n";
        if (fileHeader)                            *fileHeader   += timestr;
        if (stdoutHeader && m_logStdoutTimeStamps) *stdoutHeader += timestr;
    }

    // Let the log listeners modify the header. This is only done if the mutex is locked.
    if (isMutexLocked) {
        ListenerContainer::const_iterator it;
        ListenerContainer::const_iterator itEnd = m_listener.constEnd();
        for (it = m_listener.constBegin(); it != itEnd; ++it) {
            const QString &prefix = *it;
            if (!prefix.isEmpty()) {
                if (fileHeader)
                    fileHeader->prepend(prefix);
                if (stdoutHeader)
                    stdoutHeader->prepend(prefix);
            }
        }
    }

    // output level
    QString levelString = levelToPrefixString(lev);
    if (fileHeader)   *fileHeader   += levelString;
    if (stdoutHeader) *stdoutHeader += levelString;
}

///////////////////////////////////////////////////////////////////////////////

QString AVLog::levelToString(int level)
{
    int lev = level & ~LOG_ERRNO_FLAG;

    switch (lev) {
        case LOG__FATAL:   return "FATAL";
        case LOG__ERROR:   return "ERROR";
        case LOG__WARNING: return "WARNING";
        case LOG__INFO:    return "INFO";
        case LOG__DEBUG:   return "DEBUG";
        case LOG__DEBUG1:  return "DEBUG1";
        case LOG__DEBUG2:  return "DEBUG2";
    }

    return "UNKNOWN";
}

///////////////////////////////////////////////////////////////////////////////

QString AVLog::levelToPrefixString(int level)
{
    int lev = level & ~LOG_ERRNO_FLAG;

    switch (lev) {
        case LOG__FATAL:   return "FATAL:   ";
        case LOG__ERROR:   return "ERROR:   ";
        case LOG__WARNING: return "WARNING: ";
        case LOG__INFO:    return "INFO:    ";
        case LOG__DEBUG:   return "DEBUG:   ";
        case LOG__DEBUG1:  return "DEBUG1:  ";
        case LOG__DEBUG2:  return "DEBUG2:  ";
    }

    return "UNKNOWN: ";
}

///////////////////////////////////////////////////////////////////////////////

QString AVLog::errnoMsg(unsigned long err)
{
    QString emsg;
#if defined(Q_OS_WIN32)
    LPVOID p;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                  NULL, err,
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (LPTSTR) &p,
                  0, NULL);
    if (p != (LPVOID) NULL) {
        emsg = (char*) p;
        emsg = emsg.left(emsg.length() - 1);
        LocalFree(p);
    }
#else
    char *p = strerror(err);
    if (p != NULL) emsg = p;
#endif
    return emsg;
}

///////////////////////////////////////////////////////////////////////////////

void AVLog::setExitHandler(AVExitHandler* handler)
{
    delete m_exitHandler;
    m_exitHandler = handler;
}

///////////////////////////////////////////////////////////////////////////////

//parasoft suppress item oop-04
AVLogFactory::AVLogFactory() :
        m_min_log_level(AVLog::LOG__INFO)
{
    if (AVLogger == 0)
    {
        AVLog *default_logger =
            new AVLog(AVGetProcessName(), true, m_min_log_level, true, LOGGER_ROOT_ID);
        AVASSERTNOLOGGER(default_logger != 0);
        AVLog::setRootLogger(default_logger);
    }
    else
    {
        AVLogger->m_id = LOGGER_ROOT_ID;
    }
    m_loggerRegistry.insert(LOGGER_ROOT_ID, AVLogger);
}
//parasoft on

///////////////////////////////////////////////////////////////////////////////

AVLogFactory::~AVLogFactory()
{
    QMap<QString, AVLog*>::iterator iter;

    iter = m_loggerRegistry.begin();

    while (iter != m_loggerRegistry.end())
    {
        AVLog * cur_logger = iter.value();
        // Make sure root logger is no longer accessible when it is deleted. See e.g. SWE-2100.
        if (cur_logger == AVLog::getRootLogger()) AVLog::setRootLogger(0);
        delete cur_logger;
        ++iter;
    }
}

///////////////////////////////////////////////////////////////////////////////

AVLogFactory &AVLogFactory::getInstance()
{
    if (! m_instance)       // only lock-hint check (race condition here)
    {
        QMutexLocker locker(&m_singleton_mutex);

        if (! m_instance)   // double check with resolved race condition
            m_instance = new AVLogFactory;
        AVASSERTNOLOGGER(m_instance != 0);
    }

    return *m_instance;
}

///////////////////////////////////////////////////////////////////////////////

void AVLogFactory::destroyInstance()
{
    QMutexLocker locker(&m_singleton_mutex);
    AVLogFactory* tmp = m_instance;
    m_instance = 0;
    delete tmp;
}

///////////////////////////////////////////////////////////////////////////////

bool AVLogFactory::registerLogger(AVLog* logger)
{
    QMutexLocker locker(&m_mutex);

    return registerLoggerInternal(logger);
}

///////////////////////////////////////////////////////////////////////////////

bool AVLogFactory::registerLoggerInternal(AVLog* logger)
{
    AVASSERTNOLOGGER(logger != 0);

    QString id = logger->id();

    // there can only be one root logger (which is autoregistered)
    AVASSERTNOLOGGER(! id.isEmpty());
    AVASSERTNOLOGGER(id != LOGGER_ROOT_ID);

    if(m_loggerRegistry.contains(id))
    {
        AVLogWarning << "Trying to register another logger with "
                        "id '" << id << "'";

        return false;
    }

    AVASSERTNOLOGGER(logger->m_logWriter != 0);
    if (! logger->m_logWriter->getShare().getLogBaseName().isEmpty())
    {
        // the logger potentially just wants to initialize its log writer -> mutex required
        // see AVLog::startTimers.
        QMutexLocker log_locker(&logger->m_mutex);

        QMap<QString, AVLog*>::const_iterator iter = m_loggerRegistry.begin();
        while (iter != m_loggerRegistry.end())
        {
            AVASSERTNOLOGGER(*iter != logger); // should already be caught by precheck

            // share logWriter in case of equal file names
            if (logger->m_logWriter->getShare().getLogBaseName() ==
                (*iter)->m_logWriter->getShare().getLogBaseName())
            {
                // check for inconsistency with close intervals
                if (logger->m_logWriter->getShare().m_closeIntervalSecs !=
                    (*iter)->m_logWriter->getShare().m_closeIntervalSecs)
                {
                    AVLogWarning << "Close intervals for logger "
                                 << logger->id() << " (" << logger->m_logWriter->getShare().m_closeIntervalSecs
                                 << ") and "
                                 << (*iter)->id() << " (" << (*iter)->m_logWriter->getShare().m_closeIntervalSecs
                                 << ") do not match although they are writing to the same file ("
                                 << (*iter)->curFileName()
                                 << "). Using common close interval "
                                 << (*iter)->m_logWriter->getShare().m_closeIntervalSecs;
                }

                // share the log writer
                *(logger->m_logWriter) = *((*iter)->m_logWriter);

                break;
            }

            ++iter;
        }
    }

    m_loggerRegistry.insert(id, logger);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVLogFactory::unregisterLogger(const QString& id)
{
    QMutexLocker locker(&m_mutex);

    if(! m_loggerRegistry.contains(id))
        return false;

    m_loggerRegistry.remove(id);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

AVLog& AVLogFactory::getLogger(const QString& id)
{
    QMutexLocker locker(&m_mutex);

    QMap<QString, AVLog*>::const_iterator iter = m_loggerRegistry.find(id);
    if (iter != m_loggerRegistry.end())
        return *(*iter);

    AVLog* logger = new AVLog(AVEnvironment::getProcessName(), true, m_min_log_level, true, id);
    AVASSERTNOLOGGER(logger != 0);
    AVASSERTNOLOGGER(registerLoggerInternal(logger));

    return *logger;
}

///////////////////////////////////////////////////////////////////////////////

bool AVLogFactory::hasLogger(const QString& id) const
{
    QMutexLocker locker(&m_mutex);

    return (m_loggerRegistry.contains(id));
}

///////////////////////////////////////////////////////////////////////////////

QStringList AVLogFactory::registeredLoggers() const
{
    QMutexLocker locker(&m_mutex);

    QStringList ret;
    QMap<QString, AVLog*>::const_iterator iter = m_loggerRegistry.begin();
    while (iter != m_loggerRegistry.end())
    {
        ret += iter.key();
        ++iter;
    }
    return ret;
}

///////////////////////////////////////////////////////////////////////////////

void AVLogFactory::setMinLogLevel(int level)
{
    m_min_log_level = level;
}

///////////////////////////////////////////////////////////////////////////////

AVLogStream::ManipulatorVoid AVLogStream::SuppressTransientProblems = &AVLogStream::setSuppressTransientProblems;

///////////////////////////////////////////////////////////////////////////////

AVLogStream::AVLogStream(int source_line, const QString& source_file, int level, AVLog* logger) :
    m_source_line(source_line),
    m_source_file(source_file),
    m_level(level),
    m_logger(logger),
    m_suppress_repeated(false),
    m_suppression_duration(0),
    m_log_periodically(false),
    m_ignore_content(false)
{
    AVASSERT(logger != 0);
}

///////////////////////////////////////////////////////////////////////////////

AVLogStream::~AVLogStream()
{
    if (!m_metadata.isEmpty())
    {
        QStringList metadata_list;
        for (const AVLogMetadata& metadata : qAsConst(m_metadata))
        {
            metadata_list += "@@ " + metadata.getName() + " : " + metadata.getValueAsString();
        }
        m_string = metadata_list.join("\n") + "\n" + m_string;
    }

    if(m_suppress_repeated)
    {
        m_logger->writeCheckingRepeatSuppression(m_source_line, m_source_file, m_level, m_string,
                                                 m_suppression_duration, m_log_periodically, m_ignore_content);
    }
    else
    {
        m_logger->Write(m_source_line, m_source_file, m_level, m_string);
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVLogStream::appendString(const QString& string)
{
    m_string += string;
}

///////////////////////////////////////////////////////////////////////////////

AVLogStream& AVLogStream::getStream()
{
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

AVLogStream& AVLogStream::setSuppressTransientProblems()
{
    if (AVLog::getExecutionPhase() != AVLog::EP_RUNNING &&
        (m_level == AVLog::LOG__WARNING || m_level == AVLog::LOG__ERROR))
    {
        m_level = AVLog::LOG__INFO;
        QString phase = AVLog::getExecutionPhase() == AVLog::EP_STARTING_UP ?
            "startup" : "shutdown";
        m_string = "(message reduced to INFO during " + phase + " phase) " + m_string;
    }
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

AVLogStream &AVLogStream::setSuppressRepeated(uint duration, bool log_periodically, bool ignore_content)
{
    m_suppress_repeated    = true;
    m_suppression_duration = duration;
    m_log_periodically     = log_periodically;
    m_ignore_content       = ignore_content;
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

void AVLogStream::addMetadata(const AVLogMetadata &metadata)
{
    m_metadata.push_back(metadata);
}

///////////////////////////////////////////////////////////////////////////////

AVLogMetadata AVLogStream::Metadata::callsign(const QString &cs)
{
    return AVLogMetadata("CALLSIGN", cs);
}

///////////////////////////////////////////////////////////////////////////////

AVLogMetadata AVLogStream::Metadata::stand(const QString &stand)
{
    return AVLogMetadata("STAND", stand);
}

///////////////////////////////////////////////////////////////////////////////

AVLogMetadata AVLogStream::Metadata::updateSource(const QString &source)
{
    return AVLogMetadata("UPDATE_SOURCE", source);
}

///////////////////////////////////////////////////////////////////////////////

AVLog::SuppressedRepeatedMessageInfo::SuppressedRepeatedMessageInfo(
        QDateTime time, uint duration, bool log_periodically, bool ignoreContent, QString msg, int line, const QString &file, int lev)
    : m_log_periodically(log_periodically),
      m_msg(msg),
      m_ignore_content(ignoreContent),
      m_line(line),
      m_file(file),
      m_level(lev)
{
    m_expiration_time = time.addSecs(duration);
}

///////////////////////////////////////////////////////////////////////////////

QString AVLog::SuppressedRepeatedMessageInfo::createKey(int line, const QString &file, const QString &msg, bool ignore_content)
{
    const static QString pattern("line %1, file %2");
    if (ignore_content)
    {
        return pattern.arg(line).arg(file);
    } else return msg;
}

///////////////////////////////////////////////////////////////////////////////

QString AVLog::SuppressedRepeatedMessageInfo::key() const
{
    return createKey(m_line, m_file, m_msg, m_ignore_content);
}

// end of file
