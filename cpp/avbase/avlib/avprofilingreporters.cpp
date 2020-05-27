///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2011
//
// Module:     AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Peter Bauer, p.bauer@avibit.com
    \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com

    This class provides profiling reporters.
*/


// QT includes
#include <QtGlobal>

// local includes
#include "avprofilingreporters.h"


///////////////////////////////////////////////////////////////////////////////

AVProfBasicLoggingDataReporter::AVProfBasicLoggingDataReporter(AVLog& logger,
                                                               int printAtActStartInterval,
                                                               int printAtActStopInterval)
    : m_logger(logger),
      m_printAtActStartInterval(printAtActStartInterval),
      m_printAtActStopInterval(printAtActStopInterval),
      m_initTime(QDateTime::currentDateTimeUtc()),
      m_reportFileName(QString()),
      m_reportFieldSeparator(';')
{
}

///////////////////////////////////////////////////////////////////////////////

AVProfBasicLoggingDataReporter::~AVProfBasicLoggingDataReporter()
{
}

///////////////////////////////////////////////////////////////////////////////

void AVProfBasicLoggingDataReporter::onActivityStart(const AVProfActivity* activity,
                                                     const AVProfDataManager::TaskDict& tasks)
{
    Q_UNUSED(tasks);
    AVASSERT(activity != 0);

    // Check if file writing is required and open report file if not already opened
    if (!m_reportFileName.isEmpty())
    {
        if (!m_reportFileHandle.isOpen())
        {
            m_reportFileHandle.setFileName(m_reportFileName);
            if (!m_reportFileHandle.open(QIODevice::WriteOnly))
            {
                AVLogErrorTo(m_logger) <<
                               "AVProfBasicLoggingDataReporter::AVProfBasicLoggingDataReporter: "
                               "unable to open file " << m_reportFileName;
            }
            else
            {
                AVLogInfo << "AVProfBasicLoggingDataReporter::onActivityStart: writing report to "
                        "file " << m_reportFileName;
                m_reportFileStream.setDevice(&m_reportFileHandle);
            }
        }
    }

    if (m_printAtActStartInterval <= 0) return;

    if ( (activity->numRuns() % m_printAtActStartInterval) == 0)
    {
        AVLogDebugTo(m_logger) <<  "started recording run #" << activity->numRuns()
                               <<  " for activity '" << activity->name() << "'";
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVProfBasicLoggingDataReporter::onActivityStop(const AVProfActivity* activity,
                                                    const AVProfDataManager::TaskDict& tasks)
{
    Q_UNUSED(tasks);
    AVASSERT(activity != 0);

    if (m_printAtActStopInterval <= 0) return;

    if ( (activity->numRuns() % m_printAtActStopInterval) == 0)
    {
        logActivityCSVHeader();
        logActivityAsCSV(activity);
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVProfBasicLoggingDataReporter::taskReport(const AVProfTask* task)
{
    if (task->activities().count() == 0) return;

    for (AVProfTask::ConstActivityListDictIterator actIt = task->activities().begin();
        actIt!= task->activities().end(); ++actIt)
    {
        const AVProfTask::ActivityList *act_list = *actIt;

        AVProfTask::ConstActivityListIterator actListIt=act_list->begin();

        for (; actListIt != act_list->end(); ++actListIt)
        {
            logActivityAsCSV(*actListIt);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVProfBasicLoggingDataReporter::fullReport(const AVProfDataManager::TaskDict& tasks)
{
    AVLogInfoTo(m_logger) << "-- Begin Profiling report for " << tasks.count() << " tasks running for "
                          << m_initTime.secsTo(QDateTime::currentDateTimeUtc()) << "sec --";

    logActivityCSVHeader();
    for (AVProfDataManager::ConstTaskDictIterator it=tasks.begin(); it != tasks.end() ; ++it)
    {
        taskReport(*it);
    }

    AVLogInfoTo(m_logger) << "-- End Profiling report for " << tasks.count() << " tasks --";
}

///////////////////////////////////////////////////////////////////////////////

void AVProfBasicLoggingDataReporter::setReportFileName(const QString& fileName)
{
    m_reportFileName = fileName;
}

///////////////////////////////////////////////////////////////////////////////

void AVProfBasicLoggingDataReporter::setReportFieldSeparator(char fieldSeparator)
{
    m_reportFieldSeparator = fieldSeparator;
}

///////////////////////////////////////////////////////////////////////////////

void AVProfBasicLoggingDataReporter::logActivityAsCSV(const AVProfActivity* activity)
{
    const qint64 secsActCreationToNow = activity->creationTime().secsTo(QDateTime::currentDateTimeUtc());
    const int actNumRuns = activity->numRuns();
    const double runsPerSecond = (secsActCreationToNow == 0 ?
                                      0.0 : static_cast<double>(activity->numRuns()) / secsActCreationToNow);
    const double avgMsecsPerRun = (actNumRuns == 0 ?
                                       0.0 : static_cast<double>(activity->msecsTotalRuntime()) / actNumRuns);

    const double usage = ((avgMsecsPerRun / 1000.0) * runsPerSecond) * 100.0;

    QString reportText;
    QTextStream reportStream(&reportText, QIODevice::WriteOnly);
    reportStream <<
            activity->task()->name() <<
            m_reportFieldSeparator <<
            activity->name() <<
            m_reportFieldSeparator <<
            activity->sourcelineNumber() <<
            m_reportFieldSeparator <<
            activity->sourcefileName() <<
            m_reportFieldSeparator <<
            activity->threadName() <<
            m_reportFieldSeparator <<
            actNumRuns <<
            m_reportFieldSeparator <<
            runsPerSecond <<
            m_reportFieldSeparator <<
            activity->msecsTotalRuntime() <<
            m_reportFieldSeparator <<
            avgMsecsPerRun <<
            m_reportFieldSeparator <<
            usage;

    AVLogInfoTo(m_logger) << reportText;

    // If file name is set, write report to given file
    if (m_reportFileHandle.isOpen())
    {
        m_reportFileStream << reportText;
        m_reportFileStream << endl;
        m_reportFileStream.flush();
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVProfBasicLoggingDataReporter::logActivityCSVHeader()
{
    QString reportHeader;
    QTextStream headerStream(&reportHeader, QIODevice::WriteOnly);
    headerStream << "task" <<
            m_reportFieldSeparator <<
            "activity" <<
            m_reportFieldSeparator <<
            "sourceline#" <<
            m_reportFieldSeparator <<
            "sourcefile" <<
            m_reportFieldSeparator <<
            "thread" <<
            m_reportFieldSeparator <<
            "#runs" <<
            m_reportFieldSeparator <<
            "avg #runs/sec[1/s]" <<
            m_reportFieldSeparator <<
            "totalruntime[ms]" <<
            m_reportFieldSeparator <<
            "avg msecs/run[ms]" <<
            m_reportFieldSeparator <<
            "usage[%]";

    AVLogInfoTo(m_logger) << reportHeader;

    // If file name is set, write report to given file
    if (m_reportFileHandle.isOpen())
    {
        m_reportFileStream << reportHeader;
        m_reportFileStream << endl;
        m_reportFileStream.flush();
    }
}

// End of file
