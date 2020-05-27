//////////////////////////////////////////////////////////////////////////////
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
    \author    QT4-PORT: Matthias Fuchs m.fuchs@avibit.com
    \brief     Message logging class definition.

    This class provides basic message logging facilities.
*/

#if !defined(AVLOG_H_INCLUDED)
#define AVLOG_H_INCLUDED

#if !defined(QT_THREAD_SUPPORT)
#define QT_THREAD_SUPPORT
#endif

#include <memory>

#include <QDebug>
#include <QObject>
#include <QStringList>
#include <QThread>
#include <QFile>
#include <QTextStream>
#include <QMap>
#include <QList>
#include <QVector>
#include <QPointer>
#include <QTimer>

// local includes
#include "avlib_export.h"
#include "avlog/avlogmetadata.h"
#include "avlog/avlogindent.h"
#include "avdatetime.h"
#include "avshare.h"
#include "avmacros.h"
#include "avmisc.h"
#include "avfromtostring.h"

/////////////////////////////////////////////////////////////////////////////
//                                   globals
/////////////////////////////////////////////////////////////////////////////

class AVExitHandler;
class AVLog;
// template <typename T, typename AVShareLoggerPolicy> class AVShare;
class AVDateTime;

//! AVLogger is a global class for message logging.
/*! It can be accessed from any program part for message logging purposes.
    See also AVLog for more informations.
 */
#define AVLogger AVLog::getRootLogger()

//! Strips return type and function argument from given function name (see Q_FUNC_INFO)
AVLIB_EXPORT QByteArray avCleanupFuncinfo(QByteArray info);

//! AV_FUNC_INFO is a macro similar to Q_FUNC_INFO but only contains expands the pure function name
#define AV_FUNC_INFO avCleanupFuncinfo(Q_FUNC_INFO)

//! AV_FUNC_PREAMBLE can be used with log streams and expands to the function name followed by colon and space
#define AV_FUNC_PREAMBLE AV_FUNC_INFO << ": "

/////////////////////////////////////////////////////////////////////////////
//                                  constants
/////////////////////////////////////////////////////////////////////////////

//! internal error level mask to flag error levels which require that the
//! errno variable is supposed to be used to append the error text to the
//! main error text.
const int LOG_ERRNO_FLAG=0x0080;
//! Mask to force logging via stdout, regardless of the currently set log level.
//! If the log level is enabled, this flag is ignored (output is done to file and/or stdout
//! depending on the logger configuration).
//! This is used for easy use of the qDebug macro.
const int FORCE_STDOUT_FLAG=0x0100;

/////////////////////////////////////////////////////////////////////////////
//                                   macros
/////////////////////////////////////////////////////////////////////////////

//! undefine some macros in the event that they are already defined by
//! system headers
#if defined(LOG_INFO)
#undef LOG_INFO
#endif
#if defined(LOG_WARNING)
#undef LOG_WARNING
#endif
#if defined(LOG_ERROR)
#undef LOG_ERROR
#endif
#if defined(LOG_DEBUG)
#undef LOG_DEBUG
#endif

//! macros for error levels.
#define LOG_HERE          __LINE__, __FILE__
#define LOG_FATAL         LOG_HERE, AVLog::LOG__FATAL
#define LOG_ERROR         LOG_HERE, AVLog::LOG__ERROR
#define LOG_WARNING       LOG_HERE, AVLog::LOG__WARNING
#define LOG_INFO          LOG_HERE, AVLog::LOG__INFO
#define LOG_DEBUG         LOG_HERE, AVLog::LOG__DEBUG
#define LOG_DEBUG1        LOG_HERE, AVLog::LOG__DEBUG1
#define LOG_DEBUG2        LOG_HERE, AVLog::LOG__DEBUG2
#define LOG_FATAL_ERRNO   LOG_HERE, AVLog::LOG__FATAL_ERRNO
#define LOG_ERROR_ERRNO   LOG_HERE, AVLog::LOG__ERROR_ERRNO
#define LOG_WARNING_ERRNO LOG_HERE, AVLog::LOG__WARNING_ERRNO
#define LOG_INFO_ERRNO    LOG_HERE, AVLog::LOG__INFO_ERRNO
#define LOG_DEBUG_ERRNO   LOG_HERE, AVLog::LOG__DEBUG_ERRNO
#define LOG_DEBUG1_ERRNO  LOG_HERE, AVLog::LOG__DEBUG1_ERRNO
#define LOG_DEBUG2_ERRNO  LOG_HERE, AVLog::LOG__DEBUG2_ERRNO

//! unique IDs for loggers in factory
#define LOGGER_ROOT_ID                    "ROOT"
#define LOGGER_AVCOMMON_ID                "AVCOMMON"
#define LOGGER_AVCOMMON_AVCONFIGLIB_ID    "AVCOMMON_AVCONFIGLIB"
#define LOGGER_AVCOMMON_AVLIB_ID          "AVCOMMON_AVLIB"
#define LOGGER_AVCOMMON_AVLIB_EXPR_ID     "AVCOMMON_AVLIB_EXPRESSIONS"
#define LOGGER_AVCOMMON_AVLIB_MSGS_ID     "AVCOMMON_AVLIB_MESSAGES"
#define LOGGER_AVCOMMON_AVCOMLIB_ID       "AVCOMMON_AVCOMLIB"
#define LOGGER_AVCOMMON_AVDLIB_ID         "AVCOMMON_AVDLIB"
#define LOGGER_AVCOMMON_AVUILIB_ID        "AVCOMMON_AVUILIB"
#define LOGGER_AVCOMMON_AVIMAGELIB_ID     "AVCOMMON_AVIMAGELIB"
#define LOGGER_AVCOMMON_AVMAPEDLIB_ID     "AVCOMMON_AVMAPEDLIB"

//! environment variable names (without app/process name)
//! Note: Prefix application followed by underline to this environment variables
//! (E.g.: To enable for log output with milliseconds for Infomax set: "IM2_LOG_MSEC_TIMESTAMPS")
#define LOGGER_VARNAME_LEVEL             "LOG_LEVEL"
#define LOGGER_VARNAME_TO_FILE           "LOG_TO_FILE"
#define LOGGER_VARNAME_FILE_PREFIX       "LOG_FILE_PREFIX"
#define LOGGER_VARNAME_CLOSE_INTERVAL    "LOG_CLOSE_INTERVAL"
#define LOGGER_VARNAME_FILENAME          "LOG_FILENAME"
#define LOGGER_VARNAME_STDOUT            "LOG_STDOUT"
#define LOGGER_VARNAME_STDOUT_LINES      "LOG_STDOUT_LINES"
#define LOGGER_VARNAME_STDOUT_TIMESTAMPS "LOG_STDOUT_TIMESTAMPS"
#define LOGGER_VARNAME_MSEC_TIMESTAMPS   "LOG_MSEC_TIMESTAMPS"
/**
 * This environment variable is global (no application name prefix).
 * It is supposed to be set in development / testing scenarios, to ensure errors are detected, and a backtrace is available
 * for problem analysis.
 * Assigning any value but "0" enables this setting.
 */
#define LOGGER_VARNAME_FATAL_ERRORS      "AVLOG_FATAL_ERRORS"

//! macros for short access to loggers from factory
#define LOGGER_ROOT \
        AVLogFactory::getInstance().getLogger(LOGGER_ROOT_ID)
#define LOGGER_AVCOMMON \
        AVLogFactory::getInstance().getLogger(LOGGER_AVCOMMON_ID)
#define LOGGER_AVCOMMON_AVCONFIGLIB \
        AVLogFactory::getInstance().getLogger(LOGGER_AVCOMMON_AVCONFIGLIB_ID)
#define LOGGER_AVCOMMON_AVLIB \
        AVLogFactory::getInstance().getLogger(LOGGER_AVCOMMON_AVLIB_ID)
#define LOGGER_AVCOMMON_AVLIB_EXPR \
        AVLogFactory::getInstance().getLogger(LOGGER_AVCOMMON_AVLIB_EXPR_ID)
#define LOGGER_AVCOMMON_AVLIB_MSGS \
        AVLogFactory::getInstance().getLogger(LOGGER_AVCOMMON_AVLIB_MSGS_ID)
#define LOGGER_AVCOMMON_AVCOMLIB \
        AVLogFactory::getInstance().getLogger(LOGGER_AVCOMMON_AVCOMLIB_ID)
#define LOGGER_AVCOMMON_AVDLIB \
        AVLogFactory::getInstance().getLogger(LOGGER_AVCOMMON_AVDLIB_ID)
#define LOGGER_AVCOMMON_AVUILIB \
        AVLogFactory::getInstance().getLogger(LOGGER_AVCOMMON_AVUILIB_ID)
#define LOGGER_AVCOMMON_AVIMAGELIB \
        AVLogFactory::getInstance().getLogger(LOGGER_AVCOMMON_AVIMAGELIB_ID)
#define LOGGER_AVCOMMON_AVMAPEDLIB \
        AVLogFactory::getInstance().getLogger(LOGGER_AVCOMMON_AVMAPEDLIB_ID)

/////////////////////////////////////////////////////////////////////////////
//                           interface for log views
/////////////////////////////////////////////////////////////////////////////

class AVLIB_EXPORT AVLogViewIface
{
public:
    //! Adds a new log message to the view
    /*! Adds the message to the view area. It is formatted differently
        according to it's log level.
        - AVLog::LOG__INFO is displayed unchanged.
        - AVLog::LOG__WARNING is displayed bold
        - AVLog::LOG__ERROR is displayed bold and red
        After the messages is appended to the view, the signal signalNewMsg is
        emitted, carrying the message's log level.

        \param logLevel the message's level as specified in avlog.h
        \param message the log message
    */
    virtual void addLogMsg(int logLevel, const QString& message) = 0;
    virtual ~AVLogViewIface () {};
};

/////////////////////////////////////////////////////////////////////////////
//                           Log Writer Module
/////////////////////////////////////////////////////////////////////////////

//! Class that takes care of writing to file and stdout (thread-safe)
/*! \sa AVLog
 */
class AVLIB_EXPORT AVLogWriter : public QObject
{
    Q_OBJECT
    AVDISABLECOPY(AVLogWriter);

public:
    friend class AVLog;
    friend class AVLogFactory;
    friend class TstAVLogWriter;
    friend class AVLogTest;

    /**
     * Format of the file name is
     * QString::null if file logging is disabled,
     * <override_name> if an override is specified (it has to include full path and suffix),
     * <logdir>/<prefix>-<open date>-<open time>.log if a close interval is given,
     * <logdir>/<prefix>-<open date>-<open time>.log otherwise.
     *
     * @param fileNamePrefix   First part of the log file name (if no fileNameOverride is given).
     * @param fileNameOverride If an override is set, the log file name will not be computed to have date/time information,
     *                         but the override is unconditionally used (also its extension). prefix and close interval are
     *                         irrelevant in this case.
     * @param logDir           Directory where to write log file(s).
     * @param closeIntervalSecs Log file close interval in seconds.
     * @param logToFile        Whether to do any file logging at all. If false, parameters above are irrelevant.
     * @param echoStdout       Note that the ologger mechanism requires that processes write to stdout.
     */
    AVLogWriter(const QString& fileNamePrefix, const QString& fileNameOverride,
                const QString& logDir, int closeIntervalSecs,
                bool logToFile, bool echoStdout);

    //! Destructor
    ~AVLogWriter() override;

    //! writes the messages to standard out and to the logfile (thread-safe)
    bool Write(const QDateTime& dt,
               const QString& stdoutHeader,
               const QString& fileHeader,
               QString stdoutMsg,
               QString fileMsg);

    //! Returns the base log filename used by this log writer. Does not directly correspond to a real filename,
    //! useful only to compare log writer configurations (do they basically write to the same file(s)?)
    /*! the filename is one of the following:
        1: QString::null if file logging is disabled
        2: overridden log file name if available
        3: logdir + filename prefix
     */
    QString getLogBaseName() const;

    //! Per default, the log is flushed after every log statement, so no log output is lost in the event a process crashes.
    //! For usages where this is not important (e.g. the sys profile logger), automatic flushing can be disabled.
    void setAlwaysFlush(bool flush);
    //! Per default, thread context changes are logged. For processes with excessive context changes, or for loggers without
    //! this requirement (e.g. sys profiling), this feature can be disabled.
    void setLogThreadContextChanges(bool b);
    bool getLogThreadContextChanges() const;

    //! To be used for unit tests only. Deletes the current log file and opens a new one.
    //! This is a no-op returning true if file logging is not active.
    bool truncateLogfile();

    //! Start the timer which triggers periodic logfile opening based on the close interval. Must be called only
    //! if the QApplication exists.
    void startReopenLogfileTimer();

    // see member docs.
    static void setPrependCrOnStdout(bool a);
    // see member docs.
    static bool getPrependCrOnStdout();

protected: // methods
    /**
     * @brief     Computes the current log file name and stores it in m_fileName.
     *
     * TODO CM in the future, avoid calling this method for every single log statement, and rely on timer-based log file opening.
     *         First ensure that there no longer are processes without a main event loop - see ACEMAX-2042.
     *
     * @param  dt The current time, passed for easier unit testing. Used for the date/time suffix if the close interval is not 0.
     * @return    Whether the log file name changed since the last call.
     */
    bool recomputeFilename(const QDateTime& dt, uint *msecsToNextLogfile = 0);
    /**
     * @brief computeIntervalTime computes the time used for log file naming, based on the close interval.
     * @param now            The current time. Passed for easier unit testing.
     * @param secsInterval   The log close interval in seconds. 0 means "close every day".
     * @param baseTime       [out] the time stamp for the log file name.
     * @param msecsUntilNext [out] if != 0, will be set to the number of msecs until the next file should be opened.
     */
    static void computeIntervalTime(const QDateTime& now, uint secsInterval, AVDateTime& baseTime, uint * msecsUntilNext = 0);
    //! open the current logfile
    bool openLogFile(const QDateTime& dt);
    //! Log an EOF marker and close the logfile (if it is open).
    void closeLogFile(const QDateTime& dt);

    //! Does the actual log writing. Called by Write() and flushLogEntries()
    bool internalWrite(const QDateTime& dt, const QString& stdoutMsg, const QString& fileMsg, bool& logFileOpened);

protected slots:

    //! Opens the new log file and schedules the next call of this method.
    void slotReopenLogfile();

signals:
    //! signal emitted when a new logfile was created
    void signalNewLogFile(const QString& fileName);

protected: // members
    mutable QMutex m_mutex;                //!< locking mutex
    QFile       m_file;                    //!< log file object
    QTextStream m_outStream;               //!< output text stream
    QString     m_fileNamePrefix;          //!< file name prefix
    QString     m_fileNameOverride;        //!< Log file override name. If set, this will disable any automatic log file naming based on date/time.
    QString     m_fileName;                //!< current log file name
    //! Timestamp that was used for computing the current log file name, with minute granularity.
    //! Cached to avoid recomputing the log file name for each log statement.
    AVDateTime m_fileNameDateTime;
    //! Ensures that log files are opened even if nothing is written to them (ACEMAX-1914).
    //! This is always created in the main thread context, even if the logger is lazily initialized in a thread. See SWE-3425.
    QPointer<QTimer> m_openNewLogfileTimer;
    QString     m_logDir;                  //!< logging directory
    int         m_closeIntervalSecs;       //!< file close interval in seconds. If this is 0, the log file still will be reopened every day!
    bool        m_logToFile;               //!< don't log to a file anyway
    bool        m_echoStdout;              //!< echo to stdout flag
    bool        m_alwaysFlush;             //!< always flush after each line of log output

    //! The thread for which the last log entry was written to file.
    //! Used to log changes in thread context.
    QString        m_lastThreadContextFile;
    //! The thread for which the last log entry was written to stdout.
    //! Used to log changes in thread context.
    QString        m_lastThreadContextStdout;
    bool           m_logThreadContextChanges;

    //! If using a raw terminal, we need to replace "\n" with "\r\n" when echoing to stdout.
    //! \sa AVConsoleStdinoutConnection.
    static bool m_prependCrOnStdout;
};

/////////////////////////////////////////////////////////////////////////////
//                           message logging class
/////////////////////////////////////////////////////////////////////////////

//! Main message and error logger class.
/*! This class provides centralized means for logging messages and errors
    to log-files. Log-files are stored in a special directory. For each
    day a new log-file is created. The name of the log-file consists of
    a specific prefix and the current date as follows:

                   logdir/prefix-yyyymmdd.log

    where logdir is the logging directory, prefix is the process specific
    prefix (if initialized through AVDaemonInit) and yyyymmdd is the current
    date.

    The filename prefix and the log-directory can be specified to the
    constructor of the class. If no log-directory is specified, the
    directory $APP_HOME/log is used.

    The root logger AVLogger is created in AVDaemonInit. Alternatively it can
    be instantiated manually with "new" or dynamically by accessing it through
    the AVLogFactory which uses lazy creation and registration. If accessed
    through the AVLogFactory and no AVDaemonDeinit is called the factory shall
    shall be manually destroyed through AVLogFactory::destroyInstance().

    Accessing loggers through the factory also ensures that different loggers
    which need to write to the same file share a common file-handle. Additionally
    inconsistencies between different close intervals in case of equal file
    names are also handled when registering the logger. So it is strongly
    recommended to have all used loggers registered in the AVLogFactory.

    Be aware that the access to a specific logger via
    AVLogFactory::getInstance()->getLogger(LOGGER_ID) requires a string dictionary
    lookup. This may be too time consuming if performance is an issue.
    Refer to the loggers cache code example in the AVLogFactory description.

    The core parameters of a logger can be specified in its constructor. The
    constructor parameters plus some additional variables can be overridden
    through environment variables. The following environment variables are
    available:

    LOG_LEVEL => min log level. 0: DEBUG2, 3: INFO, 4: WARNING, 5: ERROR, 6: FATAL (default: 3).
    LOG_TO_FILE => en/disable file logging (default: 1)
    LOG_FILE_PREFIX => prefix for file before date part (default: process-name)
    LOG_CLOSE_INTERVAL => file close interval in minutes (default: every midnight)
    LOG_FILENAME => complete file name override
    LOG_STDOUT => log to standard out (default: 1)
    LOG_STDOUT_LINES => include source file line numbers in stdout log (default: 0)
    LOG_STDOUT_TIMESTAMPS => include timestamps in stdout log (default: 0)
    LOG_MSEC_TIMESTAMPS => timestamps with msecs (default: 0)

    the complete environment variable must be preceded by one of the
    following (in ascending priority order):

    \code
    APPLICATION_NAME_ (e.g. ASTOS_)
    PROCESS_NAME_ (e.g. DAQ_SSR_HB_)
    APPLICATION_NAME_LOGGER_NAME_ (e.g. ASTOS_AVCOMMON_AVLIB_MESSAGES_ or
                                   ASTOS_ASTOS_AVASLIB_DATA_LOG_LEVEL)
    PROCESS_NAME_LOGGER_NAME_ (e.g. DAQ_SSR_HB_AVCOMMON_AVLIB_MESSAGES_)
    \endcode

    When using the PROCESS_NAME make sure that it has been set correctly.
    This is usually done in AVDaemonInit(). If AVDaemonInit is not used, the
    process name must be set manually by calling AVEnvironment::setProcessName().

    The LOGGER_NAME by itself is also of hierarchical structure. E.g. the
    variable DAQ_ML_AVCOMMON_LOG_LEVEL will be overridden by the variable
    DAQ_ML_AVCOMMON_AVLIB_MESSAGES_LOG_LEVEL regarding all message based
    logging in AVLIB.

    Some usage examples:

    \code

        // no previous AVDaemonInit() => lazy creation through factory
        LOGGER_ROOT.Write(LOG_INFO, "First time access to root AVLogger");
        AVLogger->Write(LOG_INFO, "Global AVLogger now exists for sure");

        // some other loggers are also available in factory by default
        LOGGER_AVCOMMON.Write(LOG_INFO, "Log message with param1=%d", param1);
        LOGGER_AVCOMMON_AVLIB.Write(LOG_INFO, "Log message with param2=%d", param2);
        AVLogInfo(LOGGER_AVCOMMON, "Alternate log message with no params");

        // dynamic creation and registration of a new logger instance
        AVLog* myLogger = AVLogFactory::getInstance()->
                          getLogger(SOME_NEW_LOGGER_ID);
        AVLogInfo(myLogger, "Message without params");

        MyClass* a = new MyClass();
        AVASSERT(a != 0);
        if (a == 0)
        {
            AVLogFatalErrno(AVLogger, "cannot allocate memory");
        }

        // or simpler.

        MyClass* b = new MyClass();
        AVASSERT(b != 0);

    \endcode

    \sa AVLogFactory
*/

class AVLogListener;

class AVLIB_EXPORT AVLog : public QObject
{
    Q_OBJECT
    AVDISABLECOPY(AVLog);

public:
    friend class AVLogFactory;
    friend class AVLogTest;

    //! defines the supported logging levels.
    /*! The logging level defines the severity of the log message. The
        level may be DEBUG2, DEBUG1, INFO, WARNING, ERROR and FATAL
        where FATAL messages cause the program to stop immediately after
        the message is written to the log-file. If the ERRNO mask is set,
        the message will be appended an additional system error message
        which depends on the current value of the errno variable.
    */
    enum {
        LOG__DEBUG2 = 0, LOG__DEBUG1, LOG__DEBUG, LOG__INFO, LOG__WARNING,
        LOG__ERROR, LOG__FATAL, LOG__OFF, LOG__DEBUG2_ERRNO = LOG_ERRNO_FLAG,
        LOG__DEBUG1_ERRNO, LOG__DEBUG_ERRNO, LOG__INFO_ERRNO,
        LOG__WARNING_ERRNO, LOG__ERROR_ERRNO, LOG__FATAL_ERRNO
    };

    enum
    {
        HOUSEKEEP_TIMER_INTERVAL_MS = 1000
    };

    enum class CustomEventType
    {
        START_TIMERS = QEvent::User,
        LOG_GUI
    };

    //! construct a message logger class.
    /*! The message logger class is usually assigned to the global AVLogger
        variable. Of course private, non-global instantiations of this
        class are possible as well.
        With this constructor echoing to stdout is disabled.
        \param filePrefix log-file name prefix or NULL for no prefix.
        \param logDir log-file directory or NULL to place them
                      in APP_HOME/log or the the current directory. Note that
                      when a relative directory path is specified here
                      (e.g. "tmp/") the path is appended to the current
                      application home directory.
        \param loggerId ID of the logger when registering it in the logger
               factory
        \sa AVLogFactory
    */
    explicit AVLog(const QString& filePrefix, const QString& logDir = 0,
                   const QString& loggerId = 0);

    //! alternate constructor which also sets the stdout flag, the
    //! min. logging level and the file-output flag
    /*! \sa setStdOut  \sa setMinLevel
     */
    AVLog(const QString& filePrefix, const bool stdo, const int level = LOG__INFO,
          const bool filelog = true, const QString& loggerId = 0);

    //! close the log-file and destroy the class.
    ~AVLog() override;

    //! Sets the logger which is referenced by the AVLogger macro.
    //! Note that most probably, you don't want to call this method. Use AVDaemonInit instead.
    //! The root logger is owned by the AVLogFactory.
    static void setRootLogger(AVLog* logger);
    //! Returns the previously set root logger.
    static AVLog *getRootLogger();

    //! Adds a regex for error messages which should not lead to a fatal error
    static void addFatalErrorWhitelistRegex(const QRegularExpression& regex);
    //! Gets all fatal error whitelist regex
    static const QList<QRegularExpression> getFatalErrorWhitelist();

    /**
     * @brief startTimers is called for the root logger after the QApplication object is available to start any required timers.
     *
     * The root logger is created before the QApplication object. This is a problem for any QTimers, because they cannot be
     * started if QApplication doesn't exit.
     */
    void startTimers();

    //! write a single log message to the file.
    /*! the parameters line, file, date, tim and lev are replaced by the
        LOG_XXXX macros with the respective program line, program file,
        compilation date, compilation time and logging level. A typical
        usage of this method is this:

        \code

        if (f == 0) {
            AVLogger->Write(LOG_ERROR_ERRNO, "cannot open file");
        }

        \endcode

        where the LOG_ERROR_ERRNO macro replaces line, file, date, tim
        and lev. The fmt parameter is set to "cannot open file".

        \param line line number where the log message was produced
        \param file filename where the log message was produced
        \param lev the log level to be logged on. The level LOG_FATAL will
               always be logged, also when the minimum log level is set to
               LOG_OFF.
               Use the \see LOG_ERRNO_FLAG to additionally log the current error number.
               Use the \see FORCE_STDOUT_FLAG to log the error regardless of the set log level.
        \param fmt the message text format to be written
        \param ... the message format parameters to be used.
        \return true if successful, false if not.

        \sa minLevel()
    */
    bool Write(int line, const QString& file,
               int lev, const QString& logMessage);

    //! Overwriting method to allow old behaviour of passing const chars using QString as filename
    bool Write(int line, const QString& file,
               int lev, const char* logMessage, ...)

   // this GCC attribute checks for format/argument mismatches -- it only
   // works with GCC
#if defined(Q_CC_GNU) && !defined(__INSURE__) && !defined(__INTEL_COMPILER)
               __attribute__ ((format (printf, 5, 6)))
#endif
    ;

    //! Answer whether Write will suppress the specified level.
    /*! This can be used to avoid creating debug output that is suppressed,
        to improve performance. Typical usage:
        \code
        if (!AVLogger->isLevelSuppressed(AVLog::LOG__ERROR))
            AVLogger->Write(LOG_ERROR, "foo = %d", expensiveToCompute());
        \endcode
    */
    bool isLevelSuppressed(const int lev) const;

    /*!
     * \brief writeCheckingRepeatSuppression writes a message to log, if it is not suppressed and
     * performs message suppression for the next occurrence of the message.
     */
    void writeCheckingRepeatSuppression(int line, const QString& file, int lev,  const QString& log_message,
                      uint duration, bool log_periodically, bool ignore_contents);

    //! Returns the loggers' ID
    inline QString id() const { return m_id; }

//     //! Returns the parent logger as determinded by the logger factory
//     /*! Use this method instead of QObject::parent(). The latter will always
//         return NULL in case of AVLog.
//      */
//     inline AVLog* parentLogger() { return m_parentLogger; }

//     //! Returns the child loggers as determinded by the logger factory
//     /*! Use this method instead of QObject::children(). The latter will always
//         return an empty list in case of AVLog.
//      */
//     inline const QPtrList<AVLog>& childLoggers() { return m_childLoggers; }

    //! set the minimum logging level
    /*! under certain circumstances it might be necessary to suppress
        certain logging levels. This routine sets the minimum level to
        be written to the output file. The default is LOG_INFO meaning
        that the DEBUG2, DEBUG1 and DEBUG logging levels are suppressed.
        In order to also log DEBUG messages set the min. logging level
        to AVLog::LOG__DEBUG. In oder to log all debug messages as well,
        set the level to AVLog::LOG__DEBUG2.

        The environment variable XXXXX_DEBUG can be set to the minimum
        desired debugging level. As soon as this environment variable
        is set, it overrides any calls to setMinLevel.

        \param lev the minimum logging level to set
        \return true if successful, false if not.
    */
    bool setMinLevel(const int lev);

    //! return the minimum logging level
    /*! returns the current minimum logging level. \sa setMinLevel
        \return current min. logging level
    */
    int minLevel() const { return m_minLevel; }

    //! returns the log level converted to the corresponding string
    /*! e.g. AVLog::LOG_WARNING --> "WARNING"
        \param level the log level which to return as string
        \return The log level in string form, UNKNOWN if the level is not known.
     */
    static QString levelToString(int level);

    //! returns the log level converted to the corresponding string as it is outputed in the log
    /*! e.g. AVLog::LOG_WARNING --> "WARNING: "
        \param level the log level which to return as string
        \return log level in string form as outputed in the log
     */
    static QString levelToPrefixString(int level);

    //! enable/disable messages to be mirrored to stdout
    /*! normally messages are only written into the log-file. In order
        to see the message also on stdout, this method can be called with
        true as the onoff parameter. Also it is possible to influence logging
        of messages to stdout by setting the LOG_STDOUT environment
        variable.
        \param onoff true to echo messages to stdout, false to be quiet
        \param addlinenumbers if onoff is true and addlinenumbers is true,
               the source code linenumbers are added to the error messages
               on stdout as well.
    */
    bool setStdOut(const bool onoff, const bool addlinenumbers = false);

    //! returns whether echoing to stdout is enabled
    inline bool stdOut() const { return m_echoStdout; }

    //! returns whether messages are logged to file
    inline bool logToFile() const { return m_logToFile; }

    //! enable/disable messages to cause pop-up dialog boxes
    /*! normally messages are only written into the log-file or to stdout.
        This function can be used for GUI programs to also cause pop-up
        dialog boxes with the messages for certain message levels.

        \param onoff true for GUI popup dialog boxes
        \param level message level for which to turn GUI boxes on/off
               This is one of the AVLog::LOG__ERROR etc. constants
        \param addlinenumbers if onoff is true and addlinenumbers is true,
               the source code linenumbers are added to the error messages
               on stdout as well.
    */
    bool setGuiOut(const bool onoff, const int level,
                   const bool addlinenumbers = false);

    //! return the current setting of setGuiOut
    bool guiOut(const int level) const {
        int lev = level & ~LOG_ERRNO_FLAG;
        if (lev < 0 || lev >= LOG__OFF) return false;
        return m_guiOutFlags[lev];
    }

    //! sets the view for logging output
    /*! normally messages are only written into log-file or to stdout
        If a log view is set with this function and GUI logging is enabled
        by setGuiOut the messages will be appended to the view instead of
        being displayed as messages boxes.
        If the view is 0 GUI Output reverts to message boxes.

        \param view to send messages to or 0 if GUI logging should use
               message boxes.
    */
    void setLogView(AVLogViewIface* view);

    //! Registers a log listener with a given prefix
    /*! The listener will receive all messages of AVLog. All messages sent to
        AVLogWriter will get the specified prefix. Thus the prefix is independent of the
        AVLogListener::minLevel.
        \param listener listener to register for log input
        \param writerPrefix prefix to add to messages written to stdout and file out.
               This allows you to mark messages as expected.
        \note do not modify the listener from outside once this method is called
        \note if the same listener was already registered the writerPrefix of the
              already registered listener will be overwritten
        \see unregisterListener
     */
    void registerListener(AVLogListener *listener, const QString &writerPrefix = QString());

    //! unregister a log listener
    /*! Afterwards you can modify the listener again.
        \see registerListener
     */
    void unregisterListener(AVLogListener *listener);

    //! set the application name to appear in dialog error boxes title line.
    inline void setAppName(const QString &an) { m_appname = an; }

    //! set the message header.
    /*! This can be used to add certain header marks in the log file.
        For each call to setHeader, the next log message will have a
        special header line which contains this header. However, the
        header line will only be written once for each call to setHeader.
        To use another header line, you need to call setheader again.
    */
    inline void setHeader(const QString &h)
    { m_header = h; m_headerWritten = false; }

    //! specifies whether it's allowed to segfault on fatal errors.
    /*! By default, the program segfaults if a fatal error is logged.
        This can be changed by calling setFatalSegfault(false);
    */
    static void setFatalSegfault(bool fs) { m_fatalSegfault = fs; }

    //! specifies whether it's allowed to exit on fatal errors.
    /*! By default, the program exits if a fatal error is logged. This
        can be avoided by calling setFatalExit(false);
    */
    static void setFatalExit(bool fe) { m_fatalExit = fe; }

    //! specifies whether the error location should be logged in the log file.
    /*! By default, the class also logs the line number, file number etc.
        where the error was logged. This can be turned off with this function.
    */
    inline void setErrorLocation(bool el) { m_logErrorLocation = el; }

    //! specifies an exit handler that shall be notified on fatal errors
    /*! The handler's ownership is taken over.
        \param handler The handler to be notified synchronously
     */
    static void setExitHandler(AVExitHandler* handler);

    //! returns the current full name of the logfile
    /*! If nothing has been written with this logger yet, the filename will
        be QString::null
     */
    QString curFileName() const;

    //! returns the file name prefix
    QString fileNamePrefix() const;

    //! returns the overridden file name (QString::null if not overridden)
    QString fileNameOverride() const;

    //! returns the close interval of the log file in seconds
    int closeInterval() const;

    //! returns a pointer to the (possibly) shared Log-Writer instance
    inline const AVShare<AVLogWriter, AVShareEmptyLoggerPolicy>* logWriter() const
    { return m_logWriter; }

    //! returns a pointer to the (possibly) shared Log-Writer instance
    inline AVShare<AVLogWriter, AVShareEmptyLoggerPolicy>* logWriter()
    { return m_logWriter; }

    void incIndent();
    void decIndent();

#define AVLOG_EXECUTION_PHASES(enum_value) \
    enum_value(EP_STARTING_UP  ,) \
    enum_value(EP_RUNNING      ,) \
    enum_value(EP_SHUTTING_DOWN,)

    DEFINE_ENUM(ExecutionPhase, AVLOG_EXECUTION_PHASES)

    static ExecutionPhase getExecutionPhase();
    static void setExecutionPhase(ExecutionPhase phase);

    //! Writes revision info, arguments and username as info and ensures
    //! it will be written to everey logfile
    void startLogPreamble(const QStringList& arguments);

signals:
    //! signal emitted when a new logfile is created.
    //! Note that this signal is emitted in whichever thread context that caused the log!
    void signalNewLogFile(const QString& fileName);
    //! signal emitted for synchronous exit handlers
    void signalFatalLog();

protected:
    void customEvent(QEvent* event) override;

protected slots:
    //! Housekeeping for suppression of periodic logging.
    void slotHousekeeping();
    void slotRepeatedSuppressionsOverview();
    void slotRepeatLogPreamble();

private:
    //! build the log header, that is the prefix of the log message
    /*! build the log header, that is the prefix of the log message
        \param dt the current timestamp
        \param lev the level of the log message
        \param stdoutHeader header for stdout output, can be 0
        \param fileHeader header for file output, can be 0
        \param isMutexLocked true when m_mutex is locked, false otherwise. Some operations depend
               on the mutex to be locked and are otherwise omitted
     */
    void buildLogHeader(const QDateTime& dt, int lev, QString *stdoutHeader, QString *fileHeader, bool isMutexLocked);
    //! get the system message belonging to a certain errno value
    QString errnoMsg(unsigned long err);
    //! internal init routine.
    void    internalInit(const QString& pf, const QString& name, const QString& ld,
                         bool stdo, bool filelog, int level);

    //! parses all supported environment variables
    void parseEnvironVars(int& level, bool& toFile, QString& filePrefix,
                          int& closeInterval, QString& fileNameOverride,
                          bool& stdo, bool& stdoutLines,
                          bool& stdoutTimestamps,  bool& logMSecTimeStamps) const;

    //! finds environment variables in the hierarchical structure
    /*! \param varName The environment var name to be read
        \param varValue The found value of the environment var
        \param foundAtLevel level in the hierarchical structure at which a
               value could be found
     */
    bool findEnvironVar(const QString& varName,
                        QString& varValue, int& foundAtLevel) const;

    //! performs the actual GUI log output
    /*! Depending on whether #mlog_View view is set, it will write to it or use
        standard Qt message boxes.

        \warning Since this interacts with GUI it has to be called from the GUI thread

        \param level the log level of the message
        \param message the log message
    */
    void doGuiLog(int level, const QString& message);

    struct SuppressedRepeatedMessageInfo;
    /**
     * @brief logNewRepeatedSuppressedMessage logs a message, but with a prefix indicating that it will be suppressed from now on.
     *
     * @param info     Contains the message itself, and additional metadata.
     * @param duration Used for output if the suppressed message should be periodically logged.
     */
    void logNewRepeatedSuppressedMessage(const AVLog::SuppressedRepeatedMessageInfo &info, uint duration);
    void deleteRepeatedSuppression(const AVLog::SuppressedRepeatedMessageInfo& info);

    //! Writes revision info, arguments and username as info
    void logPreamble(const QStringList& arguments);

    QString m_id;                      //!< logger ID
    QString m_appname;                 //!< application name for dialog boxes
    QString m_header;                  //!< log message header to be written
                                       //!< in front of each log message
                                       //!< until a new header is set.
    //! The log writer can be shared between multiple logs if they are registered in the
    //! log factory. see AVLogFactory::registerLoggerInternal.
    AVShare<AVLogWriter, AVShareEmptyLoggerPolicy>* m_logWriter;
    bool    m_logToFile;               //!< don't log to a file anyway
    bool    m_headerWritten;           //!< header written yet?
    int     m_minLevel;                //!< min. logging level
    bool    m_echoStdout;              //!< echo to stdout flag
    bool    m_echoStdoutLn;            //!< echo linenumbers to stdout
    bool    m_guiOutFlags[LOG__OFF];   //!< GUI output flags for msgs.
    bool    m_guiOutLnFlags[LOG__OFF]; //!< GUI output linenumber flags.
    //! The current level of indentation. \see AVLOG_ENTER.
    uint m_indent;
    //! Needed to avoid race conditions when increasing / decreasing the log indentation.
    //! TODO CM keep track of indentation per thread; find a good way to log the thread name
    QMutex m_indentMutex;
    QMutex m_mutex;
    static bool m_fatalSegfault;       //!< segfault when a fatal error is
                                       //!< logged.
    static bool m_fatalExit;           //!< exit when a fatal error is logged.
    bool    m_logErrorLocation;        //!< log the error location as well
    //! Use timestamps on stdout as well. See AVLog::buildLogHeader. Remove?
    bool    m_logStdoutTimeStamps;
    bool    m_logMSecTimeStamps;       //!< use timestamps with msecs
    AVLogViewIface* m_logView;         //!< View for log output
    static AVExitHandler* m_exitHandler; //!< optional handler for fatal errors

    //! Note that the root logger is owned by the AVLogFactory.
    //! This formerly was the AVLogger global variable.
    static AVLog *m_root_logger;

    typedef QMap<AVLogListener*, QString> ListenerContainer;
    //! Maps listener object to the prefix which was registered for this listener.
    ListenerContainer m_listener;

    //! forward declaration of the custom event for logging to GUI from a non-GUI thread
//TODO_QT3    class GUILogEvent;

    //! See AC-554 and https://confluence.avibit.com:2233/x/ZYDa.
    //! The default execution phase is EP_RUNNING -> don't change behaviour in operational systems.
    //! In a testing context, the execution phase initially is set to EP_STARTING_UP and can optionally be set to
    //! EP_RUNNING.
    static ExecutionPhase m_execution_phase;

    static QList<QRegularExpression> m_fatal_error_whitelist;
    static QMutex m_fatal_error_whitelist_mutex;

//     AVLog*  m_parentLogger;            //!< Parent Logger
//     QPtrList<AVLog> m_childLoggers;    //!< Child Loggers

    struct AVLIB_EXPORT SuppressedRepeatedMessageInfo
    {
        SuppressedRepeatedMessageInfo() {}

        SuppressedRepeatedMessageInfo(QDateTime time, uint duration, bool log_periodically,
                                  bool ignoreContent, QString msg, int line, const QString &file, int lev);

        static QString createKey(int line, const QString &file, const QString& msg, bool ignore_content);
        QString key() const;

        AVDateTime m_expiration_time;
        bool m_log_periodically;
        QString m_msg;
        bool m_ignore_content;

        int     m_line;
        QString m_file;
        int     m_level;
    };

    //! Schedules housekeeping of suppression of periodic log output
    //! This is always created in the main thread context, even if the logger is lazily initialized in a thread. See SWE-3425.
    QPointer<QTimer> m_housekeeping_timer;
    // TODO CM probably those two maps could be merged...
    QMap<QString, SuppressedRepeatedMessageInfo> m_messages_suppressed_by_text;
    QMap<QString, SuppressedRepeatedMessageInfo> m_messages_suppressed_by_position;
};


//! AVLogListener provides facilities to listen to log output done via AVLog.
/*! Additionally, it is possible to specify the log levels for which to listen.
    Register listeners with AVLog::registerListener() and remove registration with
    AVLog::unregisterListener().
    \note by default no method is thread safe. Thus it depends on the caller to ensure
          that methods are called a thread safe way and on the implementor to ensure
          thread safety where needed.
    \note if AVLog::minLevel is more restrictive than your AVLogListener::minLevel,
          e.g. AVLog::minLevel > AVLogListener::minLevel
          then any level < AVLog::minLevel will be ignored
 */
class AVLIB_EXPORT AVLogListener
{
public:
    virtual ~AVLogListener();

    //! Adds a new log message to the listener.
    /*! This is called by the logger for every message logged.
        doAddLogMsg is called if the log level is accepted
     */
    virtual void addLogMsg(int level, const QString &message);

    //! retrieve the minimum log level for which to listen to log messages
    /*! \note default is AVLog::LOG__DEBUG2
     */
    int minLevel() const;

    //! set the minimum log level for which to listen to log messages
    /*! \param listens for all messages of a log level >= level, all the others will not
               listened
        \return true if setting was successful, false otherwise e.g. if the level was not correct
        \note default is AVLog::LOG__DEBUG2
     */
    void setMinLevel(int level);

protected:
    AVLogListener();

    /*! Adds a new log message. You have to implement this method for actually handling the log
        message.
        The given message does not contain any pre or postfixes, only the message.
        Use AVLog::levelToPrefixString() if you need to acquire the prefix for your output.

        For the base class implementation, this method will be called in the logging thread's
        context. If you want to handle log messages in the context of the main thread, subclass
        AVLogMainThreadListener.

        \see addLogMsg
     */
    virtual void doAddLogMsg(int level, const QString &message) = 0;

    //! retrieve whether messages of a level should be listened to or not
    /*! \see minLevel
        \return true if messages of level are listened
     */
    bool logLevel(int level) const;

private:
    int     m_minLevel; //!< minimum level to hand over to subclasses
};

class AVLogMainThreadListener;

///////////////////////////////////////////////////////////////////////////////

//! Helper class AVLogMainThreadListener
/*! This helper class ensures that
    AVLogMainThreadListener::processLogMsg is called in main thread context.
    This class only exists so AVLogMainThreadListener does not need to subclass
    QObject.
 */
class AVLIB_EXPORT AVLogMainThreadListenerHelper : public QObject
{
    AVDISABLECOPY(AVLogMainThreadListenerHelper);
    Q_OBJECT

public:
    explicit AVLogMainThreadListenerHelper(AVLogMainThreadListener *listener);
    ~AVLogMainThreadListenerHelper() override;

    void scheduleProcessing();

    void timerEvent(QTimerEvent* event) override;

private:
    //! listener on which processLogMessages is called in the main thread context (when the timer event arrives).
    AVLogMainThreadListener *m_listener;
};

///////////////////////////////////////////////////////////////////////////////

//! This calls ensures that all messages are received in the main thread context
/*! The messages are stored in m_logEntries; doAddLogMsg schedules a call of processLogMsg
 *  in the main thread context.
 */
class AVLIB_EXPORT AVLogMainThreadListener : public AVLogListener
{
public:

    AVLogMainThreadListener();
    ~AVLogMainThreadListener() override;

    //! Overrides AVLogListener. Add the message to our stored entries, schedule a main thread call
    //! to processLogMessages (indirectly via m_helper).
    void addLogMsg(int logLevel, const QString& message) override;

    //! Method to process the received log messages.
    /*! This method must be called from main thread context. Calls doAddLogMsg for every message
     *  in m_logEntries, and clears the queue afterwards.
     */
    void processLogMessages();

protected:
    struct LogMessage
    {
        int     logLevel;
        QString message;
    };

    QMutex                        m_logListenerMutex;  //!< mutex used for modifying the log entries
                                                       //!< list
    QList<LogMessage>             m_logEntries;        //!< list of log entries

private:
    //! Helper class used for calling processLogMsg in the main thread context
    AVLogMainThreadListenerHelper m_helper;
};

/////////////////////////////////////////////////////////////////////////////
//                           logger factory
/////////////////////////////////////////////////////////////////////////////

//! Factory holding global loggers
/*!
    TODO it is not clear why this isn't an explicit singleton. Initialization and
    destruction happen in avdaemon.cpp. Is the idea to allow logging in static initialization
    code? Do we really want that? Construction of root logger is different in avdaemon.cpp and
    AVLogFactory::AVLogFactory().

    This factory is implemented as a singleton. All operations are thread-safe.
    The factory is created when calling AVDaemonInit (or when first calling the
    static AVLogFactory::getInstance() method). On AVDaemonDeinit() the factory
    is destroyed again together with all registered loggers (including the root
    logger AVLogger).

    If AVDaeminDeinit() is not called (e.g. because is was never initialized)
    the factory and registered loggers can be cleaned up by calling
    AVLogFactory::destroyInstance().

    Loggers can be registered in the factory either by calling getLogger() which
    uses lazy creation and registration (creation is done with some default
    constructor parameters) or by creating the AVLog instance manually (e.g.
    if special constructor parameters are needed) and then registering it in
    the factory through registerLogger().

    Accessing loggers through the factory ensures that different loggers which
    need to write to the same file share a common file-handle. Additionally
    inconsistencies between different close intervals in case of equal file
    names are also handled when registering the logger. So it is strongly
    recommended to have all used loggers registered in the AVLogFactory.

    In case performance is an issue it is advised to use a cache for all registered loggers
    to avoid frequent and relatively time consuming string dictionary lookup.

    The following code example illustrates the definition of multiple loggers and a caching
    method:

    \code

    // --- myliblog.h

    #include "avlog.h"
    #include "avsingleton.h"

    // unique IDs for the loggers in factory
    #define AVTRKLIB_MONO_UPDATE_LOGGER_ID \
        "AVTRKLIB_MONO_UPDATE"  // envvar: ASTOS_AVTRKLIB_MONO_UPDATE_LOG_LEVEL
    #define AVTRKLIB_MONO_INOUT_LOGGER_ID \
        "AVTRKLIB_MONO_INOUT"   // envvar: ASTOS_AVTRKLIB_MONO_INOUT_LOG_LEVEL

    // macros for short access to loggers from factory -- time consuming !!
    #define LOGGER_ASTOS_AVTRKLIB_MONO_UPDATE \
        AVLogFactory::getInstance().getLogger(AVTRKLIB_MONO_UPDATE_LOGGER_ID)
    #define LOGGER_ASTOS_AVTRKLIB_MONO_INOUT \
        AVLogFactory::getInstance().getLogger(AVTRKLIB_MONO_INOUT_LOGGER_ID)

    // logger cache class
    class AVTrkLibLoggers
    {
    public:

        AVTrkLibLoggers() :
            LoggerAvtrklibMonoUpdate(LOGGER_ASTOS_AVTRKLIB_MONO_UPDATE),
            LoggerAvtrklibMonoInout(LOGGER_ASTOS_AVTRKLIB_MONO_INOUT) {}

        AVLog& LoggerAvtrklibMonoUpdate;     // LOGGER_ASTOS_AVTRKLIB_MONO_UPDATE
        AVLog& LoggerAvtrklibMonoInout;      // LOGGER_ASTOS_AVTRKLIB_MONO_INOUT
    };

    // singleton logger cache class
    class AVTrkLibLoggersSingleton : public AVSingleton<AVTrkLibLoggers>
    {
    protected:
        AVTrkLibLoggersSingleton() {}
        virtual ~AVTrkLibLoggersSingleton() {}
    };

    // macro for short access to loggers cache -- fast !!
    #define AVTRKLIB_LOGGERS AVTrkLibLoggersSingleton::getInstance()


    // --- useloggers.cpp

    #include "avtrkliblog.h"

    // ...

    if (AVTRKLIB_LOGGERS->LoggerAvtrklibMonoUpdate.minLevel() <= AVLog::LOG__DEBUG)
        AVTRKLIB_LOGGERS->LoggerAvtrklibMonoUpdate.Write(LOG_DEBUG, "Logging text...");

    \endcode


    \sa AVLog
 */
class AVLIB_EXPORT AVLogFactory
{
    AVDISABLECOPY(AVLogFactory);

public:

    //! Returns a pointer to the instance of the singleton class.
    /*!
        \return A pointer to the instance of the singleton class.
        If there is no instance of this class yet, one will be created.
    */
    static AVLogFactory& getInstance();

    //! Destroys the singleton class instance.
    /*!
        Be aware that all references to the single class instance will be
        invalid after this method has been called!
    */
    static void destroyInstance();

    //! Registers a new logger in factory
    /*! The loggers ownership is taken over. If the logger uses the same
        logging filename as a previously registered logger the filehandle
        of the registered logger is also transferred to the new logger to
        allow a thread safe file writing.
        Basically this method is only the thread-safe public interface to
        registerLoggerInternal().
        \return true if successfully registered, false if registration failed
                (e.g. because logger has already been registered previously or
                the logger's id was empty)
       \sa registerLoggerInternal()
     */
    bool registerLogger(AVLog* logger);

    //! Unregisters a previously registered logger
    /*! \return true if successfully unregistered, false if unregistration failed
                (e.g. because logger had never been registered or the name was
                empty)
     */
    bool unregisterLogger(const QString& name);

    //! Returns the logger with the given name
    /*! If the logger is not registered yet a new logger with default settings is created for name.
     *  By default the created loggers log to stdout and to the process's standard log file.
     */
    AVLog& getLogger(const QString& name);

    //! checks whether a logger with the given name is registered
    bool hasLogger(const QString& name) const;

    //! Returns the name of all registered loggers
    QStringList registeredLoggers() const;

    //! Set the min log level for all newly created loggers.
    /*!
     *  Used to disable all logging if e.g. "-help" is used.
     */
    void setMinLogLevel(int level);

protected: // methods

    // shield the constructor and destructor to prevent outside sources
    // from creating or destroying a AVSingleton instance.

    //! Standard Constructor
    /*! Creates the root logger (if not already created and assigned to global
        AVLogger instance) and registers it in the factory. Sets the logger
        Root ID to AVLogger if necessary.
     */
    AVLogFactory();

    //! Destructor
    /*! All registered loggers are deleted
     */
    virtual ~AVLogFactory();

    //! Registers a new logger in factory
    /*! The loggers ownership is taken over. If the logger uses the same
        logging filename as a previously registered logger the filehandle
        of the registered logger is also transferred to the new logger to
        allow a thread safe file writing.
        This method by itself is not thread-safe. It shall be called only
        through the public method registerLogger() or when already holding
        the lock.
        \return true if successfully registered, false if registration failed
                (e.g. because logger has already been registered previously or
                the logger's id was empty)
       \sa registerLogger()
     */
    bool registerLoggerInternal(AVLog* logger);

protected: // members
    QMap<QString, AVLog*> m_loggerRegistry;  //!< map holding all registered loggers
    mutable QMutex m_mutex; //!< mutex for factory operations

private:
    /* NOTE:
     * Double checked locking is very problematic on many compilers/cpu architectures.
     * e.g. it does not work on MS VC++ prior to 2005, and possibly many others.
     * http://en.wikipedia.org/wiki/Double-checked_locking
     */
    static AVLogFactory* volatile m_instance; //!< singleton class instance
    static QMutex         m_singleton_mutex;
                               //!< mutex for making getInstance() thread safe
    int m_min_log_level;
};

/////////////////////////////////////////////////////////////////////////////
//! Macros for short writing of log entries using the AVLogStream class
/*!
 * Remark: the for loop construct prevents warnings when used in if/else without braces. e.g.
 * if (foo) AVLogInfo << "xyz"; else bar();
 * would produce a compiler warning
 */

#define AVLogFatal          AVLogStream(LOG_HERE, AVLog::LOG__FATAL, AVLogger).getStream()
#define AVLogError          for(bool av_log_level_suppressed = AVLogger->isLevelSuppressed(AVLog::LOG__ERROR); \
                                !av_log_level_suppressed; av_log_level_suppressed = true) \
                                    AVLogStream(LOG_HERE, AVLog::LOG__ERROR, AVLogger).getStream()
#define AVLogWarning        for(bool av_log_level_suppressed = AVLogger->isLevelSuppressed(AVLog::LOG__WARNING); \
                                !av_log_level_suppressed; av_log_level_suppressed = true) \
                                    AVLogStream(LOG_HERE, AVLog::LOG__WARNING, AVLogger).getStream()
#define AVLogInfo           for(bool av_log_level_suppressed = AVLogger->isLevelSuppressed(AVLog::LOG__INFO); \
                                !av_log_level_suppressed; av_log_level_suppressed = true) \
                                    AVLogStream(LOG_HERE, AVLog::LOG__INFO, AVLogger).getStream()
#define AVLogDebug          for(bool av_log_level_suppressed = AVLogger->isLevelSuppressed(AVLog::LOG__DEBUG); \
                                !av_log_level_suppressed; av_log_level_suppressed = true) \
                                    AVLogStream(LOG_HERE, AVLog::LOG__DEBUG, AVLogger).getStream()
#define AVLogDebug1         for(bool av_log_level_suppressed = AVLogger->isLevelSuppressed(AVLog::LOG__DEBUG1); \
                                !av_log_level_suppressed; av_log_level_suppressed = true) \
                                    AVLogStream(LOG_HERE, AVLog::LOG__DEBUG1, AVLogger).getStream()
#define AVLogDebug2         for(bool av_log_level_suppressed = AVLogger->isLevelSuppressed(AVLog::LOG__DEBUG2); \
                                !av_log_level_suppressed; av_log_level_suppressed = true) \
                                    AVLogStream(LOG_HERE, AVLog::LOG__DEBUG2, AVLogger).getStream()
#define AVLogAssert(CONDITION)  for(bool av_log_condition = (CONDITION); !av_log_condition; av_log_condition = true) \
                                    AVLogStream(LOG_HERE, AVLog::LOG__FATAL, AVLogger).getStream()

#define AVLogFatalTo(LOGGER)          AVLogStream(LOG_HERE, AVLog::LOG__FATAL, &LOGGER).getStream()
#define AVLogErrorTo(LOGGER)          for(bool av_log_level_suppressed = (LOGGER).isLevelSuppressed(AVLog::LOG__ERROR); \
                                          !av_log_level_suppressed; av_log_level_suppressed = true) \
                                              AVLogStream(LOG_HERE, AVLog::LOG__ERROR, &(LOGGER)).getStream()
#define AVLogWarningTo(LOGGER)        for(bool av_log_level_suppressed = (LOGGER).isLevelSuppressed(AVLog::LOG__WARNING); \
                                          !av_log_level_suppressed; av_log_level_suppressed = true) \
                                              AVLogStream(LOG_HERE, AVLog::LOG__WARNING, &(LOGGER)).getStream()
#define AVLogInfoTo(LOGGER)           for(bool av_log_level_suppressed = (LOGGER).isLevelSuppressed(AVLog::LOG__INFO); \
                                          !av_log_level_suppressed; av_log_level_suppressed = true) \
                                              AVLogStream(LOG_HERE, AVLog::LOG__INFO, &(LOGGER)).getStream()
#define AVLogDebugTo(LOGGER)          for(bool av_log_level_suppressed = (LOGGER).isLevelSuppressed(AVLog::LOG__DEBUG); \
                                          !av_log_level_suppressed; av_log_level_suppressed = true) \
                                              AVLogStream(LOG_HERE, AVLog::LOG__DEBUG, &(LOGGER)).getStream()
#define AVLogDebug1To(LOGGER)         for(bool av_log_level_suppressed = (LOGGER).isLevelSuppressed(AVLog::LOG__DEBUG1); \
                                          !av_log_level_suppressed; av_log_level_suppressed = true) \
                                              AVLogStream(LOG_HERE, AVLog::LOG__DEBUG1, &(LOGGER)).getStream()
#define AVLogDebug2To(LOGGER)         for(bool av_log_level_suppressed = (LOGGER).isLevelSuppressed(AVLog::LOG__DEBUG2); \
                                          !av_log_level_suppressed; av_log_level_suppressed = true) \
                                              AVLogStream(LOG_HERE, AVLog::LOG__DEBUG2, &(LOGGER)).getStream()

#define AVLogFatalErrno     AVLogStream(LOG_HERE, AVLog::LOG__FATAL_ERRNO, AVLogger).getStream()
#define AVLogErrorErrno     for(bool av_log_level_suppressed = AVLogger->isLevelSuppressed(AVLog::LOG__ERROR_ERRNO); \
                                !av_log_level_suppressed; av_log_level_suppressed = true) \
                                    AVLogStream(LOG_HERE, AVLog::LOG__ERROR_ERRNO, AVLogger).getStream()
#define AVLogWarningErrno   for(bool av_log_level_suppressed = AVLogger->isLevelSuppressed(AVLog::LOG__WARNING_ERRNO); \
                                !av_log_level_suppressed; av_log_level_suppressed = true) \
                                    AVLogStream(LOG_HERE, AVLog::LOG__WARNING_ERRNO, AVLogger).getStream()
#define AVLogInfoErrno      for(bool av_log_level_suppressed = AVLogger->isLevelSuppressed(AVLog::LOG__INFO_ERRNO); \
                                !av_log_level_suppressed; av_log_level_suppressed = true) \
                                    AVLogStream(LOG_HERE, AVLog::LOG__INFO_ERRNO, AVLogger).getStream()
#define AVLogDebugErrno     for(bool av_log_level_suppressed = AVLogger->isLevelSuppressed(AVLog::LOG__DEBUG_ERRNO); \
                                !av_log_level_suppressed; av_log_level_suppressed = true) \
                                    AVLogStream(LOG_HERE, AVLog::LOG__DEBUG_ERRNO, AVLogger).getStream()
#define AVLogDebug1Errno    for(bool av_log_level_suppressed = AVLogger->isLevelSuppressed(AVLog::LOG__DEBUG1_ERRNO); \
                                !av_log_level_suppressed; av_log_level_suppressed = true) \
                                    AVLogStream(LOG_HERE, AVLog::LOG__DEBUG1_ERRNO, AVLogger).getStream()
#define AVLogDebug2Errno    for(bool av_log_level_suppressed = AVLogger->isLevelSuppressed(AVLog::LOG__DEBUG2_ERRNO); \
                                !av_log_level_suppressed; av_log_level_suppressed = true) \
                                    AVLogStream(LOG_HERE, AVLog::LOG__DEBUG2_ERRNO, AVLogger).getStream()

/////////////////////////////////////////////////////////////////////////////
//! Helper class to allow logging in style of qDebug()<<"bla bla";
/*!
 * This class is best used with the macros AVLogInfo, AVLogDebug, etc..
 * An example would be:
 *
 * \code
 * AVLogWarning << "The temperature is rising";
 *
 * AVLogFatal << "I can not work anymore, it is "<< 34.5<<" degrees.";
 * \endcode
 *
 * If a specific logger class other than the ROOT_LOGGER needs to be used,
 * the following macros can be used:
 *
 * \code
 * AVLog my_logger;
 * AVLogErrorTo(my_logger) << "error log entry";
 * \endcode
 *
 * There also a macro for asserting:
 * \code
 * AVLogAssert( A == B ) << "fatal log entry";
 * \endcode
 *
 * Internally, the class uses a mechanism similar to qDebug(). It creates a
 * temporary object AVLogStream which collects the message parts inserted with
 * operator<<. When its scope is gone, the destructor writes the whole message
 * to the logger class.
 */

class AVLIB_EXPORT AVLogStream
{
public:

    //! Used for manipulator syntax, eg AVLogError << AVLogStream::SuppressTransientProblems << "test"
    typedef AVLogStream& (AVLogStream::*ManipulatorVoid)();

    AVLogStream(int source_line, const QString& source_file, int level, AVLog* logger = &LOGGER_ROOT);
    virtual ~AVLogStream();

    //! Appends a string to the currently built statement in m_string.
    void appendString(const QString& string);

    //! TODO CM Work around template restrictions. I found no way to have the operator<< as free operator (which is
    //! neccessary to make specialization work) without using this method (error: "cannot convert AVLogStream to
    //! AVLogStream&")
    AVLogStream& getStream();

    //! Manipulator function. During startup and shutdown phase, Warning and Error messages are always logged as Info if
    //! this manipulator is used.
    AVLogStream& setSuppressTransientProblems();

    //! \see AVLogStream::SuppressRepeated
    AVLogStream& setSuppressRepeated(uint duration, bool log_periodically, bool ignore_content);

    //! \see AVLogStream::SuppressRepeated
    struct AVLIB_EXPORT SuppressionRepeatStreamManipulator
    {
        //! \see AVLogStream::SuppressRepeated
        SuppressionRepeatStreamManipulator(uint duration, bool log_periodically = false, bool ignore_content = false)
            : m_suppression_time(duration),
              m_log_periodically(log_periodically),
              m_ignore_content(ignore_content)
        {}
        uint m_suppression_time; // TODO AM this should be called duration for consistency
        bool m_log_periodically;
        bool m_ignore_content;
    };
    /**
     * @brief SuppressRepeated is a manipulator used to suppress repeated log messages that otherwise fill up a log file.
     *
     * Log messages are always identified by their source file and line, and optionally by their content.
     *
     * There are two modes of suppression:
     *
     * - log_periodically is false (the default):
     *   Use this if you know the interval of the log statement, and you want to have a single log statement when the logging starts,
     *   and a single log statement when the logging ends, e.g.
     *   <pre>
     *   09:55:00 INFO:    (repeated occurrences suppressed) foo
     *   09:58:00 INFO:    (no longer applies) foo
     *   </pre>
     *   Internally, the suppression duration is reset every time a log occurs, so if a message is logged repeatedly
     *   within the specified duration, it will be suppressed indefinitely.
     *
     * - log_periodically is true:
     *   Use this if you want the suppressed log statement to still occur periodically, but with a lesser frequency, e.g.
     *   <pre>
     *   09:55:00 INFO:    (suppressed for the next 2 seconds) bar
     *   09:55:02 INFO:    (suppressed for the next 2 seconds) bar
     *   09:55:04 INFO:    (suppressed for the next 2 seconds) bar
     *   </pre>
     *
     * Usage examples:
     * \code
     * AVLogError << AVLogStream::SuppressRepeated(60, true) << "xyz";
     * or
     * \code
     * AVLogError << AVLogStream::SuppressRepeated(60) << "xyz";
     *
     * Also check the confluence page https://confluence.avibit.com:2233/display/AVC/Handling+of+Repeated+Log+Messages
     *
     * @param duration         Duration to suppress the log message in seconds.
     * @param log_periodically See method documentation.
     * @param ignore_content   If true, the actual content of the message is ignored, everything logged from the same source file/line
     *                         will be suppressed. If false, the message is logged again if the content is different.
     * @return
     */
    typedef SuppressionRepeatStreamManipulator SuppressRepeated;

    /**
     * @brief addMetadata adds metadata for the current log statement. This metadata can be processed e.g. by logstash.
     *
     *  Don't call this method directly, use AVLogStream::Metadata() instead.
     *
     *  Metadata currently is parsed from the log file via markup; in the future a direct communication between avlog and the log server
     *  is envisioned; for this reason AVLogMetadata already supports arbitrary data types.
     */
    void addMetadata(const AVLogMetadata& metadata);

    //! \see setSuppressTransientProblems.
    static ManipulatorVoid SuppressTransientProblems;

    /**
     * @brief The Metadata class ensures that metadata is consistently named across processes.
     *
     * \see addMetadata.
     * Usage example:
     *
     * AVLogInfo << AVLogStream::Metadata::callsign("AUA123") << "log message";
     */
    class AVLIB_EXPORT Metadata
    {
    public:
        static AVLogMetadata callsign    (const QString& cs);
        static AVLogMetadata stand       (const QString& stand);
        static AVLogMetadata updateSource(const QString& source);
    };

private:
    int m_source_line;
    QString m_source_file;
    int m_level;
    QString m_string;
    AVLog* m_logger;
    QVector<AVLogMetadata> m_metadata;

    bool m_suppress_repeated;
    uint m_suppression_duration;
    bool m_log_periodically;
    bool m_ignore_content;
};

/////////////////////////////////////////////////////////////////////////////
//! Ordinary parameters are converted via AVToString when logging.
template<class LOG_ENTRY_TYPE>
AVLogStream& operator<<(AVLogStream& stream, const LOG_ENTRY_TYPE& log_entry)
{
    stream.appendString(AVToString(log_entry));
    return stream;
}

/////////////////////////////////////////////////////////////////////////////
//! Template specialization for manipulators without argument.
template<>
inline AVLogStream& operator<<(AVLogStream& stream, const AVLogStream::ManipulatorVoid& manipulator)
{
    return (stream.*manipulator)();
}

/////////////////////////////////////////////////////////////////////////////
template<>
inline AVLogStream& operator<<(AVLogStream& stream, const AVLogStream::SuppressRepeated& repeatSuppressor)
{
    stream.setSuppressRepeated(repeatSuppressor.m_suppression_time, repeatSuppressor.m_log_periodically, repeatSuppressor.m_ignore_content);
    return stream;
}

/////////////////////////////////////////////////////////////////////////////

//! Template specialization for QVariant instances.

template<>
inline AVLogStream& operator<<(AVLogStream& stream, const QVariant& variant)
{
    QString logstring;
    QDebug debug(&logstring);
    debug << variant;
    stream << logstring;
    return stream;
}

/////////////////////////////////////////////////////////////////////////////

template<>
inline AVLogStream& operator<<(AVLogStream& stream, const AVLogMetadata& metadata)
{
    stream.addMetadata(metadata);
    return stream;
}

/////////////////////////////////////////////////////////////////////////////
// this is to cover the case that some LOG_xxx macros were redefined in other
// system include files (e.g. syslog.h) *AFTER* avlog.h was already included

#else

#undef LOG_INFO
#undef LOG_WARNING
#undef LOG_ERROR
#undef LOG_DEBUG

//! macros for error levels.
#define LOG_INFO          LOG_HERE, AVLog::LOG__INFO
#define LOG_WARNING       LOG_HERE, AVLog::LOG__WARNING
#define LOG_ERROR         LOG_HERE, AVLog::LOG__ERROR
#define LOG_DEBUG         LOG_HERE, AVLog::LOG__DEBUG

#endif

// End of file
