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

#ifndef AVSYSPROF_H
#define AVSYSPROF_H

// AVLib includes
#include "avlib_export.h"
#include "avconsole.h"
#include "avexplicitsingleton.h"
#include "avfromtostring.h"

class AVSysProfConfig;
class AVLog;

///////////////////////////////////////////////////////////////////////////////

//! System profiling is done in nanoseconds resolution, so that the sequence
//! of measurements from different processes is observable.

typedef qint64 AVSysProfNanoseconds;

//! This class implements the API for system profiling. System profiling
//! captures events and writes them to a file (a separate log file).
//! In addition to the timestamps of the events, arbitrary additional
//! values can be recorded and evaluated (e.g. message size, number of flight plans).
//! There are tools to combine these files from multiple processes (even
//! from multiple machines) and to make statistics (sysproftool and sysprofstat).
//! Profiling is disabled per default (see AVSysProfConfig).
//! The class is a singleton and needs to be initialized at program start using
//! \code AVSysProf::createInstance("process name"); \endcode

class AVLIB_EXPORT AVSysProf : public QObject, public AVExplicitSingleton<AVSysProf>
{
    Q_OBJECT
public:
    //! Record the start of an activity.
    //! \param activity an identifier or description for the event or activity to be measured.
    //!                 It should be unique within the process. It is good practice to use
    //!                 "class::method".
    //! \param id an additional identifier for this occurrence of the event or activity.
    //!           If the id is used consistently, it can be used to measure activities that
    //!           span multiple processes (e.g. the time between "send message" and
    //!           "receive message" events).
    //! \param process_instance If there are multiple logical "instances" within a process that
    //!                         should be measured separately, this parameter can be used.
    //!                         This is currently used for eclient sessions.
    //! TODO fdp2 would like to use a QString to represent interface instances
    //! \return the measured time in nanoseconds (measured from power-up of the machine)
    static AVSysProfNanoseconds start(const QString& activity, const QString& id=EmptyQString, int process_instance=0)
    {
        return singleton().startActivity(activity, id, process_instance);
    }

    //! Record the stop of an activity.
    //! \param activity the same identifier as in AVSysProf::start
    //! \param id the same identifier as in AVSysProf::start
    //! \param process_instance the same identifier as in AVSysProf::start
    //! \param start the result of the corresponding AVSysProf::start, or -1 if not available.
    //!              Please note that the results produced by sysproftool are correct even
    //!              if start is not provided here.
    //! \return Answer the elapsed time in nanoseconds if start has been specified, -1 otherwise.
    static AVSysProfNanoseconds stop(
        const QString& activity, const QString& id=EmptyQString, int process_instance=0, AVSysProfNanoseconds start=-1)
    {
        return singleton().stopActivity(activity, id, process_instance, start);
    }

    //! Record a value. The recorded value is associated with the previous (most recent)
    //! call to AVSysProf::start or AVSysProf::stop.
    //! \param key an identifier or description for the value
    //! \param value the value to be recorded
    //! \param process_instance the same identifier as in AVSysProf::start
    template<class TYPE>
    static void record(const QString& key, const TYPE& value, int process_instance=0)
    {
        singleton().recordString(key, AVToString(value), process_instance);
    }

    //! Record the message size.
    //! This method is provided so that the same key is used throughout all AviBit products
    //! to record a message size.
    static void recordMsgSize(int msgSize, int process_instance=0)
    {
        record("msgSize", msgSize, process_instance);
    }

    //! Record the number of flight plans.
    //! This method is provided so that the same key is used throughout all AviBit products
    //! to record the number of flight plans.
    static void recordFpls(int fpls, int process_instance=0)
    {
        record("fpls", fpls, process_instance);
    }

    //! Record the number of service records.
    //! This method is provided so that the same key is used throughout all AviBit products
    //! to record the number of service records.
    static void recordSvcRecs(int svcRecs, int process_instance=0)
    {
        record("svcRecs", svcRecs, process_instance);
    }

    //! Format the specified time in "YYYY-MM-DDTHH:MM:SS.MMM" (ISO with milliseconds) format
    static QString formatDateTime(const QDateTime& dt);

    //! create instance
    static AVSysProf& initializeSingleton()
    {
        return setSingleton(new AVSysProf());
    }

    // Answer the class name
    virtual const char *className() const { return "AVSysProf"; }

public:
    AVSysProf();
    ~AVSysProf() override;

protected:
    //! Record the start of an activity. Answer the measured time in nanoseconds.
    AVSysProfNanoseconds startActivity(const QString& activity, const QString& id=EmptyQString, int process_instance=0);

    //! Record the stop of an activity. Answer the elapsed time in nanoseconds.
    AVSysProfNanoseconds stopActivity(
        const QString& activity, const QString& id=EmptyQString, int process_instance=0, AVSysProfNanoseconds start=-1);

    //! Record AVToString(value)
    template<class TYPE>
    void recordValue(const QString& key, const TYPE& value, int process_instance=0)
    {
        recordString(key, AVToString(value), process_instance);
    }
protected:
    //! Answer the current date and time using formatDateTime
    QString currentDateTime() const;
    //! Answer the current time in nanoseconds
    AVSysProfNanoseconds nanoseconds() const;
    //! Record a value
    void recordString(const QString& key, const QString& valueToString, int process_instance);
protected slots:
    //! Execute a console command. Valid commands are:
    //! AVSysProf START activity
    //! AVSysProf STOP activity
    //! No spaces are allowed in the activity name.
    void slotConsoleExecuteCommand(AVConsoleConnection& connection, const QStringList& params);
    //! Command completion for console commands
    void slotCompleteCommand(QStringList& completions, const QStringList& args);
private:
    QString          m_process_name;
    //! keep the config to make -save work
    //! TODO there is an API to achieve the same effect
    AVSysProfConfig *m_config;
    QString          m_host_name;
    AVLog           *m_logger;
    QString          m_separator;

    AVDISABLECOPY(AVSysProf);
};

///////////////////////////////////////////////////////////////////////////////

//! This class provides an API to profile a section of code (scope).
//! It calls AVSysProf::start in its constructor and AVSysProf::stop in its destructor.
//! Example usage:
//! \code
//! void Foo::bar()
//! {
//!     AVSysProfScope activity("Foo::bar");
//!     ... implementation of method
//! }
//! \endcode

class AVLIB_EXPORT AVSysProfScope
{
public:
    //! Record the start of an activity.
    //! \param activity an identifier or description for the event or activity to be measured.
    //!                 It should be unique within the process. It is good practice to use
    //!                 "class::method".
    //! \param process_instance If there are multiple logical "instances" within a process that
    //!                         should be measured separately, this parameter can be used.
    //!                         This is currently used for eclient sessions.
    //! \param elapsed pointer to a variable that will receive the elapsed time when this instance
    //!                is destroyed
    explicit AVSysProfScope(const QString& activity, int process_instance=0, AVSysProfNanoseconds *elapsed=0);

    //! Record the start of an activity.
    //! \param activity an identifier or description for the event or activity to be measured.
    //!                 It should be unique within the process. It is good practice to use
    //!                 "class::method".
    //! \param id an additional identifier for this occurrence of the event or activity.
    //!           If the id is used consistently, it can be used to measure activities that
    //!           span multiple processes (e.g. the time between "send message" and
    //!           "receive message" events).
    //! \param process_instance If there are multiple logical "instances" within a process that
    //!                         should be measured separately, this parameter can be used.
    //!                         This is currently used for eclient sessions.
    //! \param elapsed pointer to a variable that will receive the elapsed time when this instance
    //!                is destroyed
    AVSysProfScope(const QString& activity, const QString& id, int process_instance=0, AVSysProfNanoseconds *elapsed=0);

    virtual ~AVSysProfScope();
private:
    QString m_activity;
    QString m_id;
    int     m_process_instance;
    AVSysProfNanoseconds *m_elapsed;
    AVSysProfNanoseconds  m_start;

    AVDISABLECOPY(AVSysProfScope);
};

#endif // AVSYSPROF_H

// End of file
