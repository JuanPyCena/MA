///////////////////////////////////////////////////////////////////////////////
//
// Package:   ADMAX - Avibit AMAN/DMAN Libraries
// QT-Version: QT4
// Copyright: AviBit data processing GmbH, 2001-2011
//
// Module:    AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Matthias Fuchs, m.fuchs@avibit.com
    \author  QT4-PORT: Matthias Fuchs, m.fuchs@avibit.com
    \brief   Class making it possible to monitor the log output for specific patterns

     This class makes it possible to monitor the log output for specific patterns.
     This can be useful for unit testing when you expect certain warnings and want to check
     for these.
     All the output that is monitored is marked as such, making it easier to grep for
     issues that actually were not expect and to ignore messages that were expected.
 */

#if !defined AVLOGMONITOR_H_INCLUDED
#define AVLOGMONITOR_H_INCLUDED

// QT includes
#include <QList>

// avlib includes
#include "avlib_export.h"
#include "avlog.h"
#include "avmacros.h"
#include "avlib_export.h"

// foward declarations
class AVLogMatcherBase;
struct AVLogMatcherLogLine;
class QMutex;

//! macro for starting an AVLogMonitor
/*! macro for starting an AVLogMonitor
    this macro ensures that the AVLogMonitor is started with the correct line number and
    source file name
 */
#define AVLOG_MONITOR_START(monitor) monitor.start(LOG_HERE)

//! macros for stopping AVLogMonitor when used in a unit test
/*! stops the monitor and asserts on problems, the problem
    report is printed at the test summary
 */
#define AVLOG_MONITOR_STOP_NUM(monitor, logLineCount)       \
    QCOMPARE(monitor.stopAndGetReport(logLineCount), QString())

#define AVLOG_MONITOR_STOP(monitor) AVLOG_MONITOR_STOP_NUM(monitor, -1)


//! macros for stopping AVLogMonitor when used in a unit test (QTEST framework)
/*! stops the monitor and asserts on problems, the problem
    report is printed at the test summary
 */
#define AVLOG_MONITOR_QSTOP_NUM(monitor, logLineCount)      \
    QCOMPARE(monitor.stopAndGetReport(logLineCount),        \
    QString())

#define AVLOG_MONITOR_QSTOP(monitor) AVLOG_MONITOR_QSTOP_NUM(monitor, -1)

//! Class making it possible to monitor the log output for specific patterns
/*! This class makes it possible to monitor the log output for specific patterns. The log output
    you get for filtering does not contain a header, thus it will be of the form "Message"
    _instead_ of for example "00:00:00 WARNING: Message".
    This can be useful for unit testing when you expect certain warnings and want to check
    for these.
    The log output that actually gets written to the file or to stdout contains an additional
    prefix "MONITORED: " making it easy to grep for issues that actually were not expect and to
    ignore messages that were expected.

    Following are some examples.
    \code
        void testa()
        {
            // ...

            // you want to filter out all following occurrences of "WARNING: No FPLID"
            // as these are expected
            AVLogMonitor mon(AVLogger); // the AVLog you want to use
            AVLOG_MONITOR_START(mon);

            // ...

            // any log message between start an up to this point is filtered out if it
            // contains the string
            mon.filterOut(QString("No FPLID"), -1, -1, AVLog::LOG__WARNING);

            // the following output is not allowed to contain any errors or warnings
            // as the mon will detect that on stop() which is automatically called on
            // destruction of AVLogMonitor
        }

        void testb()
        {
            // ...

            // you want to filter out "WARNING: No FPLID" and
            // "WARNING: No FPLID, time \\d{2}:\\d{2}"
            AVLogMonitor mon(AVLogger); // the AVLog you want to use
            AVLOG_MONITOR_START(mon);

            // ...

            // any log message between start an up to this point is filtered out if it
            // contains the string
            mon.filterOut(QString("No FPLID"), -1, -1, AVLog::LOG__WARNING);

            // ...

            // all log lines containing the regular expression from mon.start() to
            // mon.stop() are filtered
            mon.filterOut(QRegExp("No FPLID, time \\d{2}:\\d{2}"), -1, -1, AVLog::LOG__WARNING);
            AVLOG_MONITOR_STOP(mon);    // will QCOMPARE on problems and print the
                                        // report on the end of the unit tests.

        }
    \endcode

    \note AVLogMonitor is AVLog specific. Thus you will only retrieve messages of an AVLog
          instance _not_ its associated AVLogWriter. If different AVLog are involved you have
          to use different AVLogMonitor for each unique AVLog involved you want to monitor.
    \note By default AVLogMonitor will receive all log messages with a
          log level >=  AVLog::LOG__DEBUG2 you can modify that via setMinLevel, though only if
          the monitor is not running
    \see AVLogListener
 */
class AVLIB_EXPORT AVLogMonitor : public AVLogListener
{
    AVDISABLECOPY(AVLogMonitor);

public:
    /*! \param log the AVLog to monitor
        \param alwaysShowReport on true also show the monitoring report on stop if there
               were no issues. The monitoring report contains the log messages that were matched.
        \param check_for_other_problems if to use the additional AVLogNoProblemsMatcher when creating the report
     */
    explicit AVLogMonitor(AVLog *log, bool alwaysShowReport = false, bool check_for_other_problems = true);
    ~AVLogMonitor() override;

    //! filters out any log line that contains the pattern
    /*! filters out any log line that contains the pattern,
        thus it works like logLine.contains(pattern)
        \param pattern the string that is matched
        \param minMatches there need to be at least minMatches, a minMatches of <= 0
               means that there is no minimum constraint, default on construction is 1
        \param maxMatches the match count needs to be smaller than maxMatches to fulfill the
               constraint, a maxMatches of < 0 means that there is no maximum constraint,
               default on construction is 1
        \param type the log type to accept, e.g. AVLog::LOG__DEBUG,
               -1 means every message is accepted to be matched
     */
    void filterOut(const QString &pattern, int minMatches = 1, int maxMatches = 1, int type = -1);

    //! filters out any log line that contains the pattern
    /*! filters out any log line that contains the pattern,
        thus it works like logLine.contains(pattern)
        \param pattern the regular expression that is matched
        \param minMatches there need to be at least minMatches, a minMatches of <= 0
               means that there is no minimum constraint, default on construction is 1
        \param maxMatches the match count needs to be smaller than maxMatches to fulfill the
               constraint, a maxMatches of < 0 means that there is no maximum constraint,
               default on construction is 1
        \param type the log type to accept, e.g. AVLog::LOG__DEBUG,
               -1 means every message is accepted to be matched
     */
    void filterOut(const QRegExp &pattern, int minMatches = 1, int maxMatches = 1, int type = -1);

    //! filters out any log line that matches in the matcher
    /*! filters out any log line that matches in the matcher
     */
    void filterOut(AVLogMatcherBase *matcher);

    //! start the monitoring
    /*! start the monitoring
        \param codeLine the line of code you called this method from
        \param codeFile the code file you called this method from
        \note use the AVLOG_MONITOR_START macro to start a monitor "AVLOG_MONITOR_START(monitor);"
              that way you do not have to specify the code line and code file
     */
    void start(int codeLine, const QString &codeFile);

    //! stop the monitoring and output the results
    /*! stop the monitoring and output the results
        \param logLineCount the number of lines the log should have at
               this point, i.e. after all filtering. -1 means that
               the number of lines is ignored
        \return true on success, false if problems turned up
     */
    bool stop(int logLineCount = -1);

    //! same as stop, just that the report will not be printed but returned as string
    /*! \return empty string on success, otherwise a report
        \note alwaysShowReport is ignored here
     */
    QString stopAndGetReport(int logLineCount = -1);

protected:
    //! Adds another log line to the monitor
    /*! Adds another log line to the monitor
        This method is called automatically by AVLog as it
        implemented the AVLogListener
        \note You do not need to call this method manually
        \see AVLogListener
     */
    void doAddLogMsg(int logLevel, const QString &message) override;

    //! does the actual filtering
    void doFilterOut(AVLogMatcherBase *matcher);

    bool doStopAndGetReport(QString &report, int logLineCount);

private:
    void clearLogLines();

private:
    bool    m_isRunning;                         //!< true if currently running, i.e. start was
                                                 //!< called
    bool    m_alwaysShowReport;                  //!< whether to show the report even if there
                                                 //!< were not issues (true) or only if there were
    int     m_line;                              //!< the line number start was called from
    AVLog   *m_log;                              //!< AVLog to use for retrieving the log output
    QString m_errorString;                       //!< string containing all errors
    QString m_infoString;                        //!< string containing all matched log lines
    QString m_file;                              //!< the source file start was called from
    /**
     * @brief m_mutex is required to allow calling filterOut while the monitor is running (and threads
     * possible produce additional log output).
     */
    QMutex  m_mutex;
    QList<AVLogMatcherLogLine*> m_logLines;      //!< log lines
    bool m_check_for_other_problems;             //!< if to use AVLogNoProblemsMatcher when creating the report

};

#endif

// End of file
