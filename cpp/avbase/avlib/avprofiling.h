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

    Provides simple profiling facilities
*/

#if !defined AVPROFILING_H_INCLUDED
#define AVPROFILING_H_INCLUDED

// Qt includes
#include <QDateTime>
#include <QElapsedTimer>
#include <QMutex>
#include <QString>
#include <QPointer>
#include <QSet>

// AVLib includes
#include "avlib_export.h"
#include "avmacros.h"
#include "avsingleton.h"
#include "avthread.h"

// forward declarations
class AVConsoleConnection;
class AVProfActivity;
class AVProfDataReporter;
class QStringList;

///////////////////////////////////////////////////////////////////////////////

//! This class represents a task to be profiled.
/*! Represents a task of an application to be profiled.
 *  A task contains an arbitrary number of activities. The profiling data is recorded
 *  per activity. A task, e.g. SMR Image Drawing could consist of
 *  the activities 'compose' and 'draw' which shall be profiled separately.
 */
class AVLIB_EXPORT AVProfTask
{
    AVDISABLECOPY(AVProfTask);
public:
    typedef QList<AVProfActivity*> ActivityList;
    typedef QHash<QString, ActivityList*> ActivityListDict;
    typedef ActivityList::iterator ActivityListIterator;
    typedef ActivityList::const_iterator ConstActivityListIterator;
    typedef ActivityListDict::iterator ActivityListDictIterator;
    typedef ActivityListDict::const_iterator ConstActivityListDictIterator;


public:
    /*! Constuctor with name of the task as parameter.
     *  \param name Name of the task
     */
    explicit AVProfTask(const QString& name);
    virtual ~AVProfTask();

    /*! Returns the name of the task
     *  \return name of the task
     */
    QString name() const;

    /*! Returns true if the task and all its activities are activated for profiling.
     *  \return true if the task and all its activities are activated for profiling.
     */
    bool isActivated() const;

    /*! Mark this task and its activies as activated or deactivated.
     *
     *  \note This just acts as a flag that can be acted on externally.
     */
    void setActivated(const bool activated);

    /*! Adds the given activity to the task if no activity with the same name exists
     *  for the calling thread.
     *  \param activity Activity to add to the task.
     *  \return true if the activity could be added, false if its name is already used.
     */
    bool addActivity(AVProfActivity* activity);

    /*! Returns the activity with the given name and created by the calling thread.
     *  \param activityName Name of the activity to return
     *  \return The activity with the given name created by the calling thread or 0
     *  if such an activity could not be found.
     */
    AVProfActivity* activity(const QString& activityName) const;

    /*! Returns all activities of the task regardless of the creating thread.
     *  \return All activities of the task regardless of the creating thread.
     */
    const ActivityListDict& activities() const;

private:
    bool m_isActivated;
    QString m_name;
    ActivityListDict m_activities;
};

///////////////////////////////////////////////////////////////////////////////

/*! Container class for storing AVProfActivity pointers in the local storage
 *  of the threads creating AVProfActivity instances. It is derived from QObject
 *  to be usable in a QGuardedPtr.
 */
class AVLIB_EXPORT AVProfActivitySet : public QObject, public QSet<AVProfActivity*>
{
    Q_OBJECT
};

///////////////////////////////////////////////////////////////////////////////

//! Represents an activity to be profiled. Every activity is assigned to a task.
class AVLIB_EXPORT AVProfActivity
{
    AVDISABLECOPY(AVProfActivity);
public:
    /*! Constructor
     * \param task Task the activity belongs to
     * \param name Name of the activity
     * \param sourcelineNumber Number of the line in the sourcefile which caused the creation
     *  of the activity
     * \param sourcefileName Name of the sourcefile which caused the creation
     *  of the activity
     */
    AVProfActivity(AVProfTask* task,
                   const QString& name,
                   int sourcelineNumber,
                   const char* sourcefileName);

    //! Destructor
    virtual ~AVProfActivity();

    /*! Returns the name of the activity
     *  \return Name of the activity
     */
    QString name() const;

    /*! Returns the task the activity belongs to.
     * \return The task the activity belongs to.
     */
    const AVProfTask* task() const;

    /*! Returns the number of start/stop cycles of this activity.
     * \return the number of start/stop cycles of this activity.
     */
    int numRuns() const;

    /*! Returns the total duration in ms of all start/stop cycles of this activity.
     * \return the total duration in ms of all start/stop cycles of this activity.
     */
    int msecsTotalRuntime() const;

    /*! Returns the duration in ms of the shortest start/stop cycle of this activity.
     *  \return the duration in ms of the shortest start/stop cycle of this activity.
     */
    int msecsMinRuntime() const;

    /*! Returns the duration in ms of the longest start/stop cycle of this activity.
     *  \return the duration in ms of the longest start/stop cycle of this activity.
     */
    int msecsMaxRuntime() const;

    /*! Returns the duration in ms of the current start/stop cycle of this activity.
     *  \return the duration in ms of the current start/stop cycle of this activity.
     */
    int msecsCurrentRuntime() const;

    /*! Returns the line in the sourcefile which caused the creation of the activity.
     *  \return the line in the sourcefile which caused the creation of the activity.
     */
    int sourcelineNumber() const;

    /*! Returns the name of the sourcefile which caused the creation of the activity.
     *  \return the name of the sourcefile which caused the creation of the activity.
     */
    const char* sourcefileName() const;

    /*! Returns wether this activity is currently in a start/stop cycle.
     * \return wether this activity is currently in a start/stop cycle.
     */
    bool isRunning() const;

    /*! Returns the time this activity has been created at.
     *  \return the time this activity has been created at.
     */
    QDateTime creationTime() const;

    /*! Starts a recording cycle. If a cycle is already running, no actions are performed.
     */
    void start();

    /*! Stops/finishes a recording cycle. If no cycle is running, no actions are performed.
     */
    void stop();

    /*! Returns wether this activity has been created by the calling thread
     *  \return Wether this activity has been created by the calling thread
     */
    bool createdByThisThread();

    /*! Returns the identifier of the thread which created this instance
     *  \return The identifier of the thread which created this instance
     */
    QString threadName() const;

private:
    AVProfTask* m_task;
    QString m_name;
    int m_sourcelineNumber;
    const char* m_sourcefileName;
    int m_numRuns;
    int m_msecsTotal;
    int m_msecsMinRuntime;
    int m_msecsMaxRuntime;
    QElapsedTimer m_timer;
    bool m_running;
    const QDateTime m_creationTime;
    QString m_threadName;
    static QThreadStorage<QString*> m_threadNames;
    static QThreadStorage<AVProfActivitySet*> m_threadActivities;
    QPointer<AVProfActivitySet> m_activitySetHandle;
};

#define AVProfileLevelEnums(enum_value)     \
    enum_value(AllTasks)                    \
    enum_value(Off)                         \
    enum_value(TasksOnDemand)

DEFINE_ENUM_IN_STRUCT(AVProfileLevel, Enum, AVProfileLevelEnums);

///////////////////////////////////////////////////////////////////////////////

//! Global profiling data management.
/*! This singleton class stores the recored profiling data and calls its reporter
 *  to report the result on the various events.
 */
class AVLIB_EXPORT AVProfDataManager : public QObject, public AVSingleton<AVProfDataManager>
{
    Q_OBJECT
    AVDISABLECOPY(AVProfDataManager);
public:
    typedef QHash<QString, AVProfTask*> TaskDict;
    typedef TaskDict::iterator TaskDictIterator;
    typedef TaskDict::const_iterator ConstTaskDictIterator;
    typedef QSet<QString> OnDemandTasks;
    typedef QSet<QString> KnownTasks;

public:
    /*! Construtor, takes ownership over the given reporter. If reporter == 0, no reporting
     *  actions are performed on recording or reporting events.
     * \param reporter Reporter instance to be called for reporting on recording events.
     */
    explicit AVProfDataManager(AVProfDataReporter* reporter = 0);
    ~AVProfDataManager() override;

    void setProfileLevel(const AVProfileLevel::Enum tasksToProfile);

    /*! Marks the given task as activated for profiling with AVProfileLevel::TasksOnDemand.
     *
     *  \param taskName name of the task to mark as activated for profiling
     *  \param activate if true the task will be activated, on false deactivated
     *         for profiling.
     *  \retval true on success
     *  \retval false if the profile level is not AVProfileLevel::TasksOnDemand.
     *
     *  \note Profile level needs to be AVProfileLevel::TasksOnDemand.
     */
    bool setTaskActivated(const QString& taskName, const bool activated);

    /*! Same as setTaskActivated with multiple tasks.
     *
     * \see setTaskActivated
     */
    bool setTasksActivated(const QStringList& taskNames, const bool activated);

    /*! Starts the activity of the given task. If no task or activity with the given names exist,
     * new ones are created.
     * \param taskName Name of the task the activity belongs to.
     * \param activityName Name of the activity to start.
     * \param sourcelineNumber Number of the source line calling the method.
     * \param sourcefileName Name of the sourcefile calling the method.
     * \return the activity matching the parameters
     * \retval 0 when profiling is turned off (AVProfileLevel::Off) or when
     *         the given task was not manually activated (AVProfileLevel::TasksOnDemand).
     */
    AVProfActivity* startActivity(const QString& taskName,
                                  const QString& activityName,
                                  int sourcelineNumber,
                                  const char* sourcefileName);

    /*! Overloaded function to avoid unnecessary QString creations.
     *
     *  When there is no profiling there is no need to create QStrings
     *  from the const chars.
     *
     *  \see AVProfActivity* startActivity(const QString&, const QString&, int, const char*)
     */
    AVProfActivity* startActivity(const char* taskName,
                                  const char* activityName,
                                  int sourcelineNumber,
                                  const char* sourcefileName);

    /*! Stops the activity of the given task. If no task or activity with the given names exist,
     * the error is logged but no further actions are performed.
     * \param taskName Name of the task the activity belongs to.
     * \param activityName Name of the activity to stop.
     */
    void stopActivity(const QString& taskName,
                      const QString& activityName);

    /*! Stops the given activity.
     *
     *  \param activity to stop, the activity either has to exist or be 0.
     */
    void stopActivity(AVProfActivity* activity);

    /*! Triggers a report of the data recorded for the given task using the reporter.
     *  If no reporter is set, no actions are performed.
     *  \param taskName Name of the task for reporting.
     */
    void taskReport(const QString& taskName);

    /*! Triggers a report of all recorded data by the reporter.
     *  If no reporter is set, no actions are performed.
     */
    void fullReport();

    /*! Removes all tasks thus clears all the collected profiling data.
     */
    void clearAll();

    /*! Replaces the current reporter with the given one. The former reporter is deleted.
     * \param reporter Reporter to set.
     */
    void setReporter(AVProfDataReporter* reporter);

public slots:
    void slotConsoleGetProfileLevel(AVConsoleConnection& connection, const QStringList&);
    void slotConsoleSetProfileLevel(AVConsoleConnection& connection, const QStringList& args);
    void slotConsoleListKnownTasks(AVConsoleConnection& connection, const QStringList&);
    void slotConsoleActivateTaskOnDemand(AVConsoleConnection& connection, const QStringList& args);
    void slotConsoleDeactivateTaskOnDemand(AVConsoleConnection& connection, const QStringList& args);
    void slotConsoleListOnDemandTasks(AVConsoleConnection&, const QStringList&);

private:
    void registerConsoleSlots();
    void setTaskActivated(AVConsoleConnection& connection, const QStringList& args, const bool activated);

    AVProfTask *getOrCreateTask(const QString& taskName);

    template<typename T>
    AVProfActivity *startActivityInternal(const T& taskName,
                                          const T& activityName,
                                          int sourcelineNumber,
                                          const char* sourcefileName);

private:
    AVProfileLevel::Enum m_profileLevel;
    TaskDict m_tasks;
    OnDemandTasks m_onDemandTasks;
    KnownTasks    m_knownTasks;
    AVProfDataReporter* m_reporter;
    mutable QMutex m_accessMutex;
};

///////////////////////////////////////////////////////////////////////////////

/*!
 * Interface for all profiling data reporters. The interface methods are called
 * by the profiling data manager in case of profiling events.
 */
class AVLIB_EXPORT AVProfDataReporter
{
    AVDISABLECOPY(AVProfDataReporter);
public:
    AVProfDataReporter() { }
    virtual ~AVProfDataReporter() { }

    /*! Called by the data manager after the given activity has been started.
     *  \param activity Activity that has been started.
     *  \param tasks All tasks stored in the data manager.
     */
    virtual void onActivityStart(const AVProfActivity* activity,
                                 const AVProfDataManager::TaskDict& tasks) = 0;

    /*! Called by the data manager after the given activity has been stopped.
     *  \param activity Activity that has been stopped.
     *  \param tasks All tasks stored in the data manager.
     */
    virtual void onActivityStop(const AVProfActivity* activity,
                                const AVProfDataManager::TaskDict& tasks) = 0;

    /*! Reports information recorded about the given task.
     * \param task Task to report information about.
     */
    virtual void taskReport(const AVProfTask* task) = 0;

    /*! Reports information about all tasks.
     *  \param tasks All tasks stored by the data manager.
     */
    virtual void fullReport(const AVProfDataManager::TaskDict& tasks) = 0;
};

///////////////////////////////////////////////////////////////////////////////

/*!
 * Runs an activity during the its lifetime following the RAII principle.
 * The activity is started/stopped in the Constructor/Destructor using the
 * global profiling data manager.
 */
class AVLIB_EXPORT AVProfScopeActivityRunner
{
    AVDISABLECOPY(AVProfScopeActivityRunner);
public:
    /*! Starts the activity with the given name.
     *  \param taskName Name of the task the activity belongs to.
     *  \param activityName Name of the activity to start.
     *  \param sourcelineNumber Number of the source line calling the constructor.
     *  \param sourcefileName Name of the source file calling the constructor.
     */
    AVProfScopeActivityRunner(const QString& taskName,
                              const QString& activityName,
                              int sourcelineNumber,
                              const char* sourcefileName);

    AVProfScopeActivityRunner(const char* taskName,
                              const char* activityName,
                              int sourcelineNumber,
                              const char* sourcefileName);

    //! Stops the activity started with the constructor.
    ~AVProfScopeActivityRunner();

private:
    AVProfActivity *m_activity;
};

///////////////////////////////////////////////////////////////////////////////

// Check if AVPROFILING is defined to enable all profiling macros
#if defined(AVPROFILING)
    /** Adds the line number to the specified string to generate a unique name (for this line)
     * \param str String to decorate.
     */
    #define __AVPROF_MAKE_UNIQUE_NAME( str ) AVCONCATENATE2( str, __LINE__ )

    /*! \def AVPROFRECORD_SCOPE(task,activity)
     *  Creates an AVProfScopeActivityRunner with a unique name (per line) on the stack, thus
     *  record activity profiling data until the end of the scope is reached.
     *  The line number and file name are determined using the __LINE__ and __FILE__ macros.
     *  \param task Name of the task the activity belongs to
     *  \param activity Name of the activity to run
     */
    #define AVPROFRECORD_SCOPE(task,activity) \
        AVProfScopeActivityRunner __AVPROF_MAKE_UNIQUE_NAME( _avprofscopeactivityrunner_line ) \
        ((task), (activity), __LINE__, __FILE__);

    /*! \def AVPROFRECORD_START(task,activity)
     *  Starts recording data for the given activity using the data manager singleton.
     *  \param task Name of the task the activity belongs to.
     *  \param activity Name of the activity to start.
     */
    #define AVPROFRECORD_START(task,activity) \
        AVProfDataManager::getInstance()->startActivity((task), (activity), __LINE__, __FILE__);

    /*! \def AVPROFRECORD_STOP(task,activity)
     *  Stops recording data for the given activity using the data manager singleton.
     *  \param task Name of the task the activity belongs to.
     *  \param activity Name of the activity to start.
     */
    #define AVPROFRECORD_STOP(task,activity) \
        AVProfDataManager::getInstance()->stopActivity((task), (activity));
#else
    // AVPROFILING is not defined, disable macros
    #define AVPROFRECORD_SCOPE(task,activity)
    #define AVPROFRECORD_START(task,activity)
    #define AVPROFRECORD_STOP(task,activity)
#endif

///////////////////////////////////////////////////////////////////////////////

/*! \page page_avprofiling Collecting performance related information

    This page describes how to use the profiling functionality offered by the AVProf*
    classes and macros.

    \section usage Usage
    To make use of the profiling facilities, the following steps have to be performed:
    - Properly initialize/destroy the global profiling data manager (AVProfDataManager)
      with a suitable data reporter at startup/shutdown of your application, e.g.
      \code
      // startup
      AVProfDataReporter* reporter = new AVProfBasicLoggingDataReporter(LOGGER_ASTOS, 0, 0);
      AVASSERT(reporter != 0);
      AVProfDataManager::createInstance(reporter);
      ...
      // shutdown
      AVProfDataManager::destroyInstance();
      \endcode
      If you want to report the collected profiling data in another way, you need to
      implement your own data reporter (AVProfDataReporter interface).
    - Add the AFPROF* macros to the "interesting" places like methods or loops which are e.g.
      under suspicion of eating up all the CPU cycles. Tasks and Activities are identified
      by their names, so use practical names for them, e.g.
      \code
      bool AstosHMICore::applicationStartup()
      {
        AVPROFRECORD_SCOPE("HMI Core Startup", "Core Application Startup");
        ...
      }
      \endcode
      If an activity spans over multiple places in the code, you can use the recording macros
      with the same task- and activityname and the profiling data will be accumulated accordingly.
    - Call the reporting/control methods of the data manager to trigger reporting or clearing of
      the collected information, e.g.
      \code
      // handle an event to trigger a full report
      AVProfDataManager::getInstance()->fullReport();
      \endcode
      This causes the data manager to produce the report output using the data reporter instance
      it has been initialized with.
      To clear the previously collected data during runtime, e.g. after a configuration change
      occurred which affects the runtime behaviour, use the clearAll() method of the data manager.
      \code
      // handle an event to clear the collected data
      AVProfDataManager::getInstance()->clearAll();
      \endcode
    - Make sure that the pre-processor define "AVPROFILING" is defined when compiling the code
      for profiling purposes, e.g. debug build. If its not defined, all AVPROF* macros will be
      defined empty so no profiling data will be recorded by the data manager.

      e.g. put in your CMakeLists.txt
      \verbatim
#! Macros to be set in case of a debug build
if (CMAKE_BUILD_TYPE MATCHES "Debug")
    add_definitions(-DAVDEBUG -DAVPROFILING)
endif (CMAKE_BUILD_TYPE MATCHES "Debug")
      \endverbatim

      or when calling cmake use:
      \verbatim
cmake ../src5 -DCMAKE_CXX_FLAGS_DEBUG="-DAVPROFILING"
      \endverbatim
    - Set the appropriate profile level via the API
      \code
      // profiles only tasks that were manually activated
      AVProfDataManager::getInstance()->setProfileLevel(AVProfileLevel::TasksOnDemand);
      \endcode
      or via AVConsole using
      \code
      setProfileLevel TasksOnDemand
      \endcode
      The following profile levels are available:
        + AVProfileLevel::AllTasks is default, every task will profiled.
        + AVProfileLevel::Off no task will be profiled anymore.
          The profiling classes incure negliable overhead in this mode.
        + AVProfileLevel::OnDemandTasks will only profile tasks that were
          manually activated using either the API
          \code
          // one task
          AVProfDataManager::getInstance()->setTaskActivated("My Task", true);

          // multiple tasks
          QStringList taskNames = ...;
          AVProfDataManager::getInstance()->setTaskActivated(taskNames, true);
          \endcode
          or AVConsole
          \code
          // activates profiling for the given task
          profileTaskOnDemand My Task

          // deactivates profiling for the given task
          dontProfileTaskOnDemand My Task
          \endcode
          Keep in mind that profiling for tasks can only be turned on/off
          when AVProfileLevel::OnDemandTasks is used.
      Changing the profile level allows to dynamically investigate processes
      that were compiled with AVPROFILING, while having profiling turned off
      most of the time.

    \subsection outputexample Example profiling output
    \verbatim
-- Begin Profiling report for 3 tasks running for 93sec --
task;activity;line#;file;#runs;avg #runs/sec[1/s];totalruntime[ms];avg msecs/run[ms];usage[%]
HMI Core Startup;Core Constructor;190;astoshmicore.cpp;1;0.0;2;2.0;0.0
HMI Core Startup;Init Main Window;615;astoshmicore.cpp;1;0.0;197;197.0;0.2
HMI Core Startup;Core Application Startup;392;astoshmicore.cpp;1;0.0;3919;3919.0;4.2
HMI Core Startup;Delayed Init;1866;astoshmicore.cpp;1;0.0;700;700.0;0.8
Draw Layers of MainWindowLayerView;Alarm Layer;80;layers/avdlayeralarm.cpp;1102;12.4;13;0.0;0.0
Draw Layers of MainWindowLayerView;Label Layer;197;layers/avdlayerlabels.cpp;551;6.2;9967;18.1;11.2
Draw Layers of MainWindowLayerView;Image Layer;132;avlayerimage.cpp;1102;12.4;7;0.0;0.0
Draw Layers of MainWindowLayerView;DF Layer;74;layers/avdlayerdf.cpp;551;6.2;2;0.0;0.0
Draw Layers of MainWindowLayerView;VMap Layer;61;avlayervmap.cpp;1102;12.4;14071;12.8;15.8
Draw Layers of MainWindowLayerView;Stopbar Layer;67;layers/avdlayerstopbar.cpp;551;6.2;2;0.0;0.0
SMR Image Composing;Copy patch to Composer Thread;185;avimagecomposerthread.cpp;703;8.0;0;0.0;0.0
-- End Profiling report for 3 tasks --
    \endverbatim
    \section Definitions
    \par Activities (AVProfActivities)
    are actions performed by the application for which performance data can
    be collected. An activity has the following attributes:
    - Name (usually a short description of the action performed by the application)
    - Running state (is the runtime recording currently on?)
    - Time of the creation of the activity (first use) as QTime
    - Number of start/stop (recording) cyles as int
    - Total runtime of all recording cycles as int in milliseconds
    - Minimum and Maximum runtime of all recording cycles as int in milliseconds
    - Sum of all start- and stop-delays as int in milliseconds
    - Name of the sourcefile the first recording cycle has been started
    - Linenumber of the statement in the sourcefile the first recording cycle has been started in
    - A pointer to the task the activity belongs to
    - Identifier of the thread which created the instance as QString

    \par Tasks (AVProfTask)
    are used to group activities. Therefore a task has the following attributes
    - Name (usually a short description of the task performed by its activities)
    - an arbitrary number of activities

    \par Profiling Data Manager (AVProfDataManager)
    is used to collect the performance data by creating the Tasks and Activities as
    needed and provide a central point to retrieve the data. Tasks and Activities are
    identified by their names.

    \par Profiling Data Reporter (AVProfDataReporter interface)
    When the global Profiling Data Manager is initialized, a Profiling Data Reporter instance
    has to be provided. It is used by the Data Manager to report the collected data.
    AVProfBasicLoggingDataReporter offers a basic implementation of the interface
    which loggs the collected information using an AVLog with Loglevel INFO in a CSV-like format.

    \section Multithreading
    Activities are stored on a per-thread basis, thus if the same activityname is used by
    concurrent threads to record profiling data, an activity is created/managed for each thread.
    It is guaranteed that an activity is only started/stopped by the thread which created it
    using AVASSERT. Every activity instance stores an identifier of the thread which created it
    for logging and reporting purposes.
*/

#endif

// End of file
