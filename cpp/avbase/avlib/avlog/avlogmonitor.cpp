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
 */


// QT includes
#include <QMutex>

// avlib includes

// local includes
#include "avlog/avlogmonitor.h"
#include "avlog/avlogmatcher.h"


///////////////////////////////////////////////////////////////////////////////

AVLogMonitor::AVLogMonitor(AVLog *log, bool alwaysShowReport, bool check_for_other_problems)
  : m_isRunning(false),
    m_alwaysShowReport(alwaysShowReport),
    m_line(-1),
    m_log(log),
    m_check_for_other_problems(check_for_other_problems)
{
}

///////////////////////////////////////////////////////////////////////////////

AVLogMonitor::~AVLogMonitor()
{
    if (m_isRunning)
        stop();
}

///////////////////////////////////////////////////////////////////////////////

void AVLogMonitor::start(int codeLine, const QString &codeFile)
{
    QMutexLocker locker(&m_mutex);

    // start can not be called if the monitor is running already or if there are still log lines
    // or error strings
    AVASSERT(!m_isRunning);
    AVASSERT(m_logLines.size() == 0);
    AVASSERT(m_errorString.isEmpty());

    m_isRunning = true;
    m_line = codeLine;
    m_file = codeFile;

    m_log->registerListener(this, "MONITORED: ");
}

///////////////////////////////////////////////////////////////////////////////

bool AVLogMonitor::stop(int logLineCount)
{
    QString report;
    bool worked = doStopAndGetReport(report, logLineCount);
    if (!worked || m_alwaysShowReport)
        m_log->Write(LOG_INFO, report);

    return worked;
}

///////////////////////////////////////////////////////////////////////////////

QString AVLogMonitor::stopAndGetReport(int logLineCount)
{
    QString report;
    bool worked = doStopAndGetReport(report, logLineCount);

    return (worked ? QString() : report);
}

///////////////////////////////////////////////////////////////////////////////

bool AVLogMonitor::doStopAndGetReport(QString &report, int logLineCount)
{
    // stop can not be called if the monitor is not running
    AVASSERT(m_isRunning);

    // unregister to not receive any messages anymore
    m_log->unregisterListener(this);

    // lock after unregister to avoid lock reversal. See SWE-4363.
    QMutexLocker locker(&m_mutex);

    // the number of logged lines is different from the expected value
    if (logLineCount >= 0 && logLineCount != static_cast<int>(m_logLines.count())) {
        m_errorString.append("AVLogMonitor: Unexpected number of log lines. Expected " +
                             QString::number(logLineCount) + " but got " +
                             QString::number(m_logLines.count()) + ".\n");
    }

    // finally check that there are no warnings, errors or fatals left in the log output
    if(m_check_for_other_problems)
    {
        AVLogNoProblemsMatcher *noProblems = new AVLogNoProblemsMatcher();
        AVASSERT(noProblems);
        doFilterOut(noProblems);
    }

    bool worked = m_errorString.isEmpty();
    clearLogLines();

    if (!m_errorString.isEmpty()) {
        // add the line and the file start was called from
        if (m_line != -1 && !m_file.isEmpty())
            m_errorString.prepend("========= AVLogMonitor Issues BEGIN from " + m_file + ":" +
                                 QString::number(m_line) + " =========\n");
        else
            m_errorString.prepend("========= AVLogMonitor Issues BEGIN =========\n");

        m_errorString.append("========= AVLogMonitor Issues END =========\n");
    }


    // create the information message
    // add the line and the file start was called from
    if (m_alwaysShowReport || !worked)
    {
        if (m_line != -1 && !m_file.isEmpty())
            m_infoString.prepend("========= AVLogMonitor Report BEGIN from " + m_file + ":" +
                                 QString::number(m_line) + " =========\n");
        else
            m_infoString.prepend("========= AVLogMonitor Report BEGIN =========\n");

        m_infoString.append("========= AVLogMonitor Report END =========\n");
    }

    // now output the messages
    if (!worked) {
        m_errorString.prepend(AVLog::levelToString(AVLog::LOG__ERROR) +  ": ");
        m_infoString.append(m_errorString);
    }
    report = m_infoString;

    m_errorString.clear();
    m_infoString.clear();
    m_line = -1;
    m_isRunning = false;

    return worked;
}

///////////////////////////////////////////////////////////////////////////////

void AVLogMonitor::doAddLogMsg(int logLevel, const QString &message)
{
    QMutexLocker locker(&m_mutex);

    // has to run to add log messages
    AVASSERT(m_isRunning);

    AVLogMatcherLogLine *line = new AVLogMatcherLogLine();
    AVASSERT(line);
    line->type = logLevel;
    line->message = message;

    m_logLines.append(line);
}

///////////////////////////////////////////////////////////////////////////////

void AVLogMonitor::filterOut(const QString &pattern, int minMatches, int maxMatches, int type)
{
    QMutexLocker locker(&m_mutex);

    // has to run, to use filtering
    AVASSERT(m_isRunning);

    AVLogMatcherBase *matcher = new AVLogStringMatcher(pattern, AVLogStringMatcher::CONTAINS, type);
    AVASSERT(matcher);
    matcher->setMatchConstraints(minMatches, maxMatches);

    doFilterOut(matcher);
}

///////////////////////////////////////////////////////////////////////////////

void AVLogMonitor::filterOut(const QRegExp &pattern, int minMatches, int maxMatches, int type)
{
    QMutexLocker locker(&m_mutex);

    // has to run, to use filtering
    AVASSERT(m_isRunning);

    AVLogMatcherBase *matcher = new AVLogRegExpMatcher(pattern, type);
    AVASSERT(matcher);
    matcher->setMatchConstraints(minMatches, maxMatches);

    doFilterOut(matcher);
}

///////////////////////////////////////////////////////////////////////////////

void AVLogMonitor::filterOut(AVLogMatcherBase *matcher)
{
    QMutexLocker locker(&m_mutex);

    // has to run, to use filtering
    AVASSERT(m_isRunning);

    doFilterOut(matcher);
}

///////////////////////////////////////////////////////////////////////////////

void AVLogMonitor::doFilterOut(AVLogMatcherBase *matcher)
{
    QList<AVLogMatcherLogLine*>::iterator it;
    for (it = m_logLines.begin(); it != m_logLines.end(); ) {
        if (matcher->match(**it)) {
            delete *it;
            it = m_logLines.erase(it);
        }
        else
            ++it;
    }

    if (!matcher->fullfilledConstraints())
        m_errorString.append(matcher->errorString());

    m_infoString.append(matcher->matchedLines());

    delete matcher;
}

///////////////////////////////////////////////////////////////////////////////

void AVLogMonitor::clearLogLines()
{
    qDeleteAll(m_logLines.begin(), m_logLines.end());
    m_logLines.clear();
}

// End of file
