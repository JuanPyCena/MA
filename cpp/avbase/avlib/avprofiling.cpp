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
    This class provides profiling facilities.
*/


// QT includes
#include <QtGlobal>

// local includes
#include "avconsole.h"
#include "avlog.h"
#include "avprofiling.h"


///////////////////////////////////////////////////////////////////////////////

AVProfTask::AVProfTask(const QString& name) :
    m_isActivated(true),
    m_name(name)
{
    AVASSERT(!m_name.isEmpty());
}

///////////////////////////////////////////////////////////////////////////////

AVProfTask::~AVProfTask()
{
    Q_FOREACH(ActivityList* lst, m_activities)
    {
        Q_FOREACH(AVProfActivity* act, *lst)
            delete act;
        delete lst;
    }
}

///////////////////////////////////////////////////////////////////////////////

QString AVProfTask::name() const
{
    return m_name;
}

///////////////////////////////////////////////////////////////////////////////

bool AVProfTask::isActivated() const
{
    return m_isActivated;
}

///////////////////////////////////////////////////////////////////////////////

void AVProfTask::setActivated(const bool activated)
{
    m_isActivated = activated;
}

///////////////////////////////////////////////////////////////////////////////

bool AVProfTask::addActivity(AVProfActivity* activity)
{
    AVASSERT(activity != 0);

    ActivityList* actList = m_activities.value(activity->name(), 0);

    if (actList == 0)
    {
        // no list for this activity name exists, create one
        actList = new ActivityList();
        AVASSERT(actList != 0);
        m_activities.insert(activity->name(), actList);
    }
    else
    {
        for (ActivityListIterator iter=actList->begin();
                iter != actList->end();
                ++iter)
        {
            AVProfActivity* loopAct=*iter;

            if (loopAct->createdByThisThread())
            {
                AVLogError << "activity with name '%s' already exists for this thread"
                           << loopAct->name();

                return false;
            }
        }
    }

    actList->append(activity);

    return true;
}

///////////////////////////////////////////////////////////////////////////////

AVProfActivity* AVProfTask::activity(const QString& activityName) const
{
    const ActivityList* actList = m_activities.value(activityName, 0);

    if (actList == 0) return 0;

    for (ConstActivityListIterator iter=actList->begin();
            iter != actList->end();
            ++iter)
    {
        AVProfActivity* loopAct=const_cast<AVProfActivity*>(*iter);
        if (loopAct->createdByThisThread()) return loopAct;
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////

const AVProfTask::ActivityListDict& AVProfTask::activities() const
{
    return m_activities;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

QThreadStorage<QString*> AVProfActivity::m_threadNames;
QThreadStorage<AVProfActivitySet*> AVProfActivity::m_threadActivities;

///////////////////////////////////////////////////////////////////////////////

AVProfActivity::AVProfActivity(AVProfTask* task,
                               const QString& name,
                               int sourcelineNumber,
                               const char* sourcefileName)
    : m_task(task),
      m_name(name),
      m_sourcelineNumber(sourcelineNumber),
      m_sourcefileName(sourcefileName),
      m_numRuns(0),
      m_msecsTotal(0),
      m_msecsMinRuntime(0),
      m_msecsMaxRuntime(0),
      m_running(false),
      m_creationTime(QDateTime::currentDateTimeUtc())
{
    AVASSERT(m_task != 0);
    AVASSERT(!m_name.isEmpty());

    static int numThreads = 0;
    if (!m_threadNames.hasLocalData())
    {
        // thread is creating its first AVProfActivity instance, generate a unique name for it
        // and store it in its local storage and as an attribute
        m_threadName = "Thread " + QString::number(numThreads);
        if (!AVThread::threadName().isEmpty()) m_threadName += " (" + AVThread::threadName() + ")";

        QString* localData = new QString(m_threadName);
        AVASSERT(localData != 0);
        m_threadNames.setLocalData(localData);
        ++numThreads;
    }
    else
    {
        // this is not the first instance created by this thread,
        // copy its name from its local storage to the attribute
        m_threadName = *(m_threadNames.localData());
    }

    if (!m_threadActivities.hasLocalData())
    {
        // create new set for the thread to store all activities created by it
        AVProfActivitySet* actSet = new AVProfActivitySet();
        AVASSERT(actSet != 0);
        m_threadActivities.setLocalData(actSet);
    }

    // add this new instance to the threads instance set
    m_threadActivities.localData()->insert(this);

    // store handle to instance set to be able to remove this instance from another thread
    // than the one which called this constructor
    m_activitySetHandle = m_threadActivities.localData();
}

///////////////////////////////////////////////////////////////////////////////

AVProfActivity::~AVProfActivity()
{
    if (m_running)
    {
        AVLogWarning << "Profiling: activity '" << m_name << "s' of task '"
                     << m_task->name() << "' to be destroyed is still running!";
    }

    // remove this from activityset of the creating thread if thread has not cleared its
    // local storage yet which means the thread is still alive
    if (!m_activitySetHandle.isNull())
    {
        AVLogDebug << "Profiling: removing activity '" << m_name << "' of task '"
                   << m_task->name() << "' from thread storage of '" << m_threadName << "'";
        m_activitySetHandle->remove(this);
    }
}

///////////////////////////////////////////////////////////////////////////////

const AVProfTask* AVProfActivity::task() const
{
    return m_task;
}

///////////////////////////////////////////////////////////////////////////////

QString AVProfActivity::name() const
{
    return m_name;
}

///////////////////////////////////////////////////////////////////////////////

int AVProfActivity::numRuns() const
{
    return m_numRuns;
}

///////////////////////////////////////////////////////////////////////////////

int AVProfActivity::msecsTotalRuntime() const
{
    return m_msecsTotal;
}

///////////////////////////////////////////////////////////////////////////////

int AVProfActivity::msecsMinRuntime() const
{
    return m_msecsMinRuntime;
}

///////////////////////////////////////////////////////////////////////////////

int AVProfActivity::msecsMaxRuntime() const
{
    return m_msecsMaxRuntime;
}

///////////////////////////////////////////////////////////////////////////////

int AVProfActivity::msecsCurrentRuntime() const
{
    return m_timer.elapsed();
}

///////////////////////////////////////////////////////////////////////////////

int AVProfActivity::sourcelineNumber() const
{
    return m_sourcelineNumber;
}

///////////////////////////////////////////////////////////////////////////////

const char* AVProfActivity::sourcefileName() const
{
    return m_sourcefileName;
}

///////////////////////////////////////////////////////////////////////////////

bool AVProfActivity::isRunning() const
{
    return m_running;
}

///////////////////////////////////////////////////////////////////////////////

QDateTime AVProfActivity::creationTime() const
{
    return m_creationTime;
}

///////////////////////////////////////////////////////////////////////////////

void AVProfActivity::start()
{
    AVASSERT(createdByThisThread());

    if (m_running)
    {
        AVLogWarning << "Profiling: activity '" << m_name << "' of task '"
                     << m_task->name() << "' already running, skipping start";
        return;
    }

    m_timer.start();
    m_running = true;
}

///////////////////////////////////////////////////////////////////////////////

void AVProfActivity::stop()
{
    AVASSERT(createdByThisThread());

    if (!m_running)
    {
        AVLogWarning << "Profiling: activity '" << m_name << "s' of task '"
                     << m_task->name() << "' not running, skipping stop";
        return;
    }

    const int runtime = m_timer.elapsed();
    m_timer = QElapsedTimer();
    m_running = false;

    m_msecsTotal += runtime;
    ++m_numRuns;

    if (runtime < m_msecsMinRuntime) m_msecsMinRuntime = runtime;

    if (runtime > m_msecsMaxRuntime) m_msecsMaxRuntime = runtime;
}

///////////////////////////////////////////////////////////////////////////////

bool AVProfActivity::createdByThisThread()
{
    if(!m_threadActivities.hasLocalData())
        return false;

    return m_threadActivities.localData()->contains(this);
}

///////////////////////////////////////////////////////////////////////////////

QString AVProfActivity::threadName() const
{
    return m_threadName;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

AVProfDataManager::AVProfDataManager(AVProfDataReporter* reporter) :
    m_profileLevel(AVProfileLevel::AllTasks),
    m_reporter(reporter)
{
    registerConsoleSlots();
}

///////////////////////////////////////////////////////////////////////////////

AVProfDataManager::~AVProfDataManager()
{
    clearAll();

    delete m_reporter;
}

///////////////////////////////////////////////////////////////////////////////

void AVProfDataManager::registerConsoleSlots()
{
    if (!AVConsole::isSingletonInitialized())
        return;

    AVConsole &c = AVConsole::singleton();

    c.registerSlot("getProfileLevel", this,
                   SLOT(slotConsoleGetProfileLevel(AVConsoleConnection&, const QStringList&)),
                   "Gets the current profile level.");

    c.registerSlot("setProfileLevel", this,
                   SLOT(slotConsoleSetProfileLevel(AVConsoleConnection&, const QStringList&)),
                   "Specify the profile level, valid values are: "
                   + AVToString(AVProfileLevel::getEnumStrings()));

    c.registerSlot("listKnownProfileTasks", this,
                   SLOT(slotConsoleListKnownTasks(AVConsoleConnection&, const QStringList&)),
                   "List the known profiling tasks.");

    c.registerSlot("profileTaskOnDemand", this,
                   SLOT(slotConsoleActivateTaskOnDemand(AVConsoleConnection&, const QStringList&)),
                   "Activates the given task for profiling. Needs to have TasksOnDemand profile state.");

    c.registerSlot("dontProfileTaskOnDemand", this,
                   SLOT(slotConsoleDeactivateTaskOnDemand(AVConsoleConnection&, const QStringList&)),
                   "Deactivates the given task for profiling. Needs to have TasksOnDemand profile state.");
}

///////////////////////////////////////////////////////////////////////////////

void AVProfDataManager::setProfileLevel(const AVProfileLevel::Enum profileLevel)
{
    QMutexLocker lock(&m_accessMutex);
    if (m_profileLevel == profileLevel)
        return;

    m_profileLevel = profileLevel;
    m_onDemandTasks.clear();

    // update active status of tasks
    const bool activated = (m_profileLevel == AVProfileLevel::AllTasks);

    TaskDictIterator it;
    TaskDictIterator itEnd = m_tasks.end();
    for (it = m_tasks.begin(); it != itEnd; ++it)
        (*it)->setActivated(activated);
}

///////////////////////////////////////////////////////////////////////////////

AVProfActivity* AVProfDataManager::startActivity(const QString& taskName,
                                                 const QString& activityName,
                                                 int sourcelineNumber,
                                                 const char* sourcefileName)
{
    return startActivityInternal(taskName, activityName, sourcelineNumber, sourcefileName);
}

///////////////////////////////////////////////////////////////////////////////

AVProfTask *AVProfDataManager::getOrCreateTask(const QString& taskName)
{
    AVProfTask* task = m_tasks.value(taskName, 0);
    if (task == 0)
    {
        task = new AVProfTask(taskName);
        AVASSERT(task != 0);
        m_tasks.insert(taskName, task);
        m_knownTasks.insert(taskName);

        if (m_profileLevel == AVProfileLevel::TasksOnDemand)
        {
            task->setActivated(m_onDemandTasks.contains(taskName));
        }
    }

    return task;
}

///////////////////////////////////////////////////////////////////////////////

AVProfActivity* AVProfDataManager::startActivity(const char* taskName,
                                                 const char* activityName,
                                                 int sourcelineNumber,
                                                 const char* sourcefileName)
{
    return startActivityInternal(taskName, activityName, sourcelineNumber, sourcefileName);
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
AVProfActivity *AVProfDataManager::startActivityInternal(const T& taskName,
                                                         const T& activityName,
                                                         int sourcelineNumber,
                                                         const char* sourcefileName)
{
    QMutexLocker mutexLocker(&m_accessMutex);
    if (m_profileLevel == AVProfileLevel::Off)
        return 0;

    AVProfTask* task = getOrCreateTask(taskName);
    if (!task->isActivated())
        return 0;

    AVProfActivity* activity = task->activity(activityName);
    if (activity == 0)
    {
        activity = new AVProfActivity(task, activityName, sourcelineNumber, sourcefileName);
        AVASSERT(activity != 0);
        task->addActivity(activity);
    }

    activity->start();

    if (m_reporter != 0) m_reporter->onActivityStart(activity, m_tasks);

    return activity;
}

///////////////////////////////////////////////////////////////////////////////

void AVProfDataManager::stopActivity(const QString& taskName, const QString& activityName)
{
    QMutexLocker mutexLocker(&m_accessMutex);

    AVProfTask* task = m_tasks.value(taskName, 0);
    if (task == 0)
    {
        AVLogError << "Profiling: stopActivity failed, unknown taskname '" << taskName << "'";
        return;
    }

    AVProfActivity* activity = task->activity(activityName);
    if (activity == 0)
    {
        AVLogError << "Profiling: stopActivity failed, unknown activityname '"
                   << activityName << "' of task '" << taskName << "'";
        return;
    }

    activity->stop();

    if (m_reporter != 0) m_reporter->onActivityStop(activity, m_tasks);
}

///////////////////////////////////////////////////////////////////////////////

void AVProfDataManager::stopActivity(AVProfActivity* activity)
{
    if (activity == 0)
        return;

    QMutexLocker mutexLocker(&m_accessMutex);

    activity->stop();

    if (m_reporter != 0) m_reporter->onActivityStop(activity, m_tasks);
}

///////////////////////////////////////////////////////////////////////////////

void AVProfDataManager::taskReport(const QString& taskName)
{
    QMutexLocker mutexLocker(&m_accessMutex);

    AVProfTask* task = m_tasks.value(taskName, 0);
    if (task == 0)
    {
        AVLogError << "Profiling: taskReport failed, unknown taskname '" << taskName << "'";
        return;
    }

    if (m_reporter != 0)
    {
        m_reporter->taskReport(task);
    }
    else
    {
        AVLogWarning << "Profiling: taskReport failed, no reporter set";
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVProfDataManager::fullReport()
{
    QMutexLocker mutexLocker(&m_accessMutex);

    if (m_reporter != 0)
    {
        m_reporter->fullReport(m_tasks);
    }
    else
    {
        AVLogWarning << "Profiling: fullReport failed, no reporter set";
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVProfDataManager::clearAll()
{
    QMutexLocker mutexLocker(&m_accessMutex);

    AVLogInfo << "Profiling: clearing all profiling data";
    qDeleteAll(m_tasks);
    m_tasks.clear();
}

///////////////////////////////////////////////////////////////////////////////

void AVProfDataManager::setReporter(AVProfDataReporter* reporter)
{
    QMutexLocker mutexLocker(&m_accessMutex);

    if (m_reporter == reporter) return;

    delete m_reporter;
    m_reporter = reporter;
}

///////////////////////////////////////////////////////////////////////////////

void AVProfDataManager::slotConsoleGetProfileLevel(AVConsoleConnection& connection,
                                                   const QStringList&)
{
    QMutexLocker lock(&m_accessMutex);
    connection.print(AVProfileLevel::enumToString(m_profileLevel));
}

///////////////////////////////////////////////////////////////////////////////

void AVProfDataManager::slotConsoleSetProfileLevel(AVConsoleConnection& connection,
                                                   const QStringList& args)
{
    if (args.size() != 1)
    {
        connection.printError("Just one parameter is allowed.");
        return;
    }

    AVProfileLevel::Enum profileLevel;
    if (!AVProfileLevel::enumFromString(args[0], profileLevel))
    {
        connection.printError("Unknown parameter '" + args[0] + "' given.");
        return;
    }

    connection.print("Setting profile level to '" + args[0] + "'");
    setProfileLevel(profileLevel);
}

///////////////////////////////////////////////////////////////////////////////

void AVProfDataManager::slotConsoleListKnownTasks(AVConsoleConnection& connection,
                                                  const QStringList&)
{
    QMutexLocker lock(&m_accessMutex);
    connection.print(AVToString(m_knownTasks));
}

///////////////////////////////////////////////////////////////////////////////

void AVProfDataManager::slotConsoleActivateTaskOnDemand(AVConsoleConnection& connection,
                                                        const QStringList& args)
{
    setTaskActivated(connection, args, true);
}

///////////////////////////////////////////////////////////////////////////////

void AVProfDataManager::slotConsoleDeactivateTaskOnDemand(AVConsoleConnection& connection,
                                                          const QStringList& args)
{
    setTaskActivated(connection, args, false);
}

///////////////////////////////////////////////////////////////////////////////

void AVProfDataManager::setTaskActivated(AVConsoleConnection& connection,
                                         const QStringList& args,
                                         const bool activated)
{
    const QString taskName = args.join(" ");

    QString log = (activated ? "Activating" : "Deactivating");
    log += " task '" + AVToString(taskName) + "'";

    if (setTaskActivated(taskName, activated))
        connection.print(log);
    else
        connection.printError(log + " failed!");
}

///////////////////////////////////////////////////////////////////////////////

bool AVProfDataManager::setTasksActivated(const QStringList& taskNames,
                                          const bool activated)
{
    QMutexLocker lock(&m_accessMutex);

    AVLogDebug << "AVProfDataManager::setTasksActivated: Setting tasks "
               << AVToString(taskNames) << " to activated=" << activated;

    if (m_profileLevel != AVProfileLevel::TasksOnDemand)
    {
        AVLogWarning << "AVProfDataManager::setTasksActivated: Wrong profile level, expected '"
                     << AVProfileLevel::enumToString(AVProfileLevel::TasksOnDemand) << "'";
        return false;
    }

    QStringList::const_iterator it;
    QStringList::const_iterator itEnd = taskNames.constEnd();
    for (it = taskNames.constBegin(); it != itEnd; ++it)
    {
        if (activated)
            m_onDemandTasks.insert(*it);
        else
            m_onDemandTasks.remove(*it);

        AVProfTask *task = m_tasks.value(*it, 0);
        if (task != 0)
            task->setActivated(activated);
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVProfDataManager::setTaskActivated(const QString &taskName, const bool activated)
{
    return setTasksActivated(QStringList(taskName), activated);
}

///////////////////////////////////////////////////////////////////////////////

void AVProfDataManager::slotConsoleListOnDemandTasks(AVConsoleConnection& connection,
                                                     const QStringList&)
{
    connection.print(AVToString(m_onDemandTasks));
}

///////////////////////////////////////////////////////////////////////////////

AVProfScopeActivityRunner::AVProfScopeActivityRunner(const QString& taskName,
                                                     const QString& activityName,
                                                     int sourcelineNumber,
                                                     const char* sourcefileName)
    : m_activity()
{
    if (AVProfDataManager::hasInstance())
    {
        m_activity = AVProfDataManager::getInstance()->startActivity(taskName,
                                                                     activityName,
                                                                     sourcelineNumber,
                                                                     sourcefileName);
    }
}

///////////////////////////////////////////////////////////////////////////////

AVProfScopeActivityRunner::AVProfScopeActivityRunner(const char* taskName,
                                                     const char* activityName,
                                                     int sourcelineNumber,
                                                     const char* sourcefileName)
    : m_activity()
{
    if (AVProfDataManager::hasInstance())
    {
        m_activity = AVProfDataManager::getInstance()->startActivity(taskName,
                                                                     activityName,
                                                                     sourcelineNumber,
                                                                     sourcefileName);
    }
}

///////////////////////////////////////////////////////////////////////////////

AVProfScopeActivityRunner::~AVProfScopeActivityRunner()
{
    if (AVProfDataManager::hasInstance())
        AVProfDataManager::getInstance()->stopActivity(m_activity);
}

// End of file
