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

    \brief

    Provides profiling data reporting classes.
*/

#if !defined AVPROFILINGREPORTERS_H_INCLUDED
#define AVPROFILINGREPORTERS_H_INCLUDED

// Qt includes
#include <QDateTime>

// local includes
#include "avlib_export.h"
#include "avlog.h"
#include "avmacros.h"
#include "avprofiling.h"

///////////////////////////////////////////////////////////////////////////////

//! Basic implementation of the AVProfDataReporter interface to log profiling data
class AVLIB_EXPORT AVProfBasicLoggingDataReporter : public AVProfDataReporter
{
    AVDISABLECOPY(AVProfBasicLoggingDataReporter);
public:

    /*! Constructor
     * \param logger Logger to use for logging with loglevel LOG_INFO
     * \param printAtActStartInterval Interval for logging a message on an activity start
     * \param printAtActStopInterval Interval for logging a message on an activity stop
     */
    explicit AVProfBasicLoggingDataReporter(AVLog& logger = LOGGER_ROOT,
                                   int printAtActStartInterval = 0,
                                   int printAtActStopInterval = 1);

    //! Destructor
    ~AVProfBasicLoggingDataReporter() override;

    //! Logs a message in case the interval is reached
    void onActivityStart(const AVProfActivity* activity, const AVProfDataManager::TaskDict& tasks) override;

    //! Logs a message in case the interval is reached
    void onActivityStop(const AVProfActivity* activity, const AVProfDataManager::TaskDict& tasks) override;

    //! Logs a task report by calling logActivityAsCSV() for all its activities
    void taskReport(const AVProfTask* task) override;

    //! Logs a full report by logging a header, calling taskReport() for all tasks, and a footer
    void fullReport(const AVProfDataManager::TaskDict& tasks) override;

    //! Sets the name of the file (absolute path) the report is written to
    void setReportFileName(const QString& fileName);

    //! Sets the character used as field separator in the CSV report output
    void setReportFieldSeparator(char fieldSeparator);

protected:

    /*! Log the following information of the given activity on loglevel INFO in CSV syntax using
     *  the specified field separator character:
     *  task: Name of the Task the given Activity belongs to
     *  activity: Name of the given Activity
     *  sourceline: Number of the sourceline the given Activity has been created by
     *  sourcefile: Name of the sourceline the given Activity has been created by
     *  thread: Identifier of the thread which created and run the activity
     *  #runs: Number of start/stop recording cycles of the activity
     *  avg #runs/sec[1/s]: Average number of start/stop recording cycles per second.
     *  This is computed as (#runs / seconds elapsed since the creation of the activity).
     *  totalruntime[ms]: Total runtime of all start/stop cycles considering the
     *  given start/stop delays.
     *  avg msecs/run[ms]: Average runtime of a start/stop cycle of the activity. This is
     *  computed as (totalruntime[ms] / #runs).
     *  usage[%]: Utilization of runtime since the creation of the Activity. This is computed
     *  as (((avg msecs/run[ms] / 1000.0) * avg #runs/sec[1/s]) * 100.0.
     *
     *  Note: If a file name was specified, the CSV output is additionally written to a file
     *  \param Activity to log the information of
     */
    virtual void logActivityAsCSV(const AVProfActivity* activity);

    //! Logs the column headers for the data printed with logActivityAsCSV() on loglevel info.
    //! Note: If a file name was specified, the CSV output is additionally written to a file
    virtual void logActivityCSVHeader();

private:
    AVLog& m_logger;
    int m_printAtActStartInterval;
    int m_printAtActStopInterval;
    const QDateTime m_initTime;
    //! The name of the file (absolute path) the report is written to.
    //! Note: If empty, no file is written.
    QString m_reportFileName;
    //! The character used as field separator in the CSV report output
    char m_reportFieldSeparator;
    //! The file handle to the report output file (only valid if a report file name was set)
    QFile m_reportFileHandle;
    //! The text stream used to write to the report file
    QTextStream m_reportFileStream;
};

#endif

// End of file
