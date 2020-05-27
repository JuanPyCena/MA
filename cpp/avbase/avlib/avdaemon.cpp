///////////////////////////////////////////////////////////////////////////////
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
    \author    QT4-PORT: Christian Muschick, c.muschick@avibit.com
    \brief     Routines for implementing daemon (background) processes.
*/


// qt includes
#include <qglobal.h>
#include <QNetworkProxyFactory>

// system includes
#include <csignal>
#include <iostream>
#include <iterator>
#include <memory>
using namespace std;


// windows specific system includes
#if defined (_OS_WIN32_) || defined(Q_OS_WIN32)
#include <windows.h>
#endif

// unix specific system includes
#if defined(Q_OS_UNIX)
#include <unistd.h>
#endif

#define AVDAEMON_MAIN
#include "avconfig.h" // TODO CM remove after QTP-10 has been resolved
#include "avconfig2.h"
#include "avconsole.h"
#include "avdaemon.h"
#include "avdeprecate.h"
#include "avlog.h"
#include "avpackageinfo.h"
#include "avprocessstate.h"
#include "avthread.h"
#include "avshm_varnames.h"
#include "avtimereference.h"
#include "avexecstandbychecker.h"
#include "avdebughelpers.h"
#include "avtimereference/avtimereferenceadapter.h"
#include "avtimereference/avtimereferenceadapterconfig.h"
#include "avtimereference/avtimereferenceadapterserver.h"
#include "avtimereference/avtimereferenceadapterserverconfig.h"
#include "avtimereference/avtimereferenceadaptersharedmemory.h"
#include "avtimereference/avtimereferenceadapterlocal.h"
#include "avtimereference/avtimereferenceconfigdeprecated.h"
#include "avsysprof.h"
#include "avsignalhandler.h"
#include "avprocesswatchdog.h"

///////////////////////////////////////////////////////////////////////////////
// store the process name and the shm name

static QString *avshm = 0;         // shared memory name

#if defined(AVDAEMON_MAIN)

AVProcessState *AVProcState = 0;
bool AVProcessTerminated = false;      // global termination flag
#endif

///////////////////////////////////////////////////////////////////////////////

AVApplicationInitParams::AVApplicationInitParams(
        int argc, char **argv,
        const QString& default_app_name,
        const QString& default_process_name) :
    m_argc(argc), m_argv(argv),
    m_default_app_name    (default_app_name),
    m_default_process_name(default_process_name),
    m_log_to_file   (true),
    m_log_to_stdout (true),
    m_enable_console(true),
    m_enable_time_reference_adapter(true),
    m_exit_on_time_jump(true),
    m_flags         (UseEventLoop),
    m_allow_extra_cmdline(false),
    m_exec_standby_checker_enabled(false),
    m_enable_sending_replay_state_notifications(false),
    m_random_seed(0)
{
    AVASSERT(!m_default_app_name    .isEmpty());
    AVASSERT(!m_default_process_name.isEmpty());
}

///////////////////////////////////////////////////////////////////////////////
void AVApplicationInitParams::enableAllowExtraCmdLine()
{
    m_allow_extra_cmdline = true;
}

///////////////////////////////////////////////////////////////////////////////
bool AVApplicationInitParams::getAllowExtraCmdLine() const
{
    return m_allow_extra_cmdline;
}

///////////////////////////////////////////////////////////////////////////////

bool AVApplicationInitParams::isExecStandbyCheckerEnabled() const
{
    return m_exec_standby_checker_enabled;
}

///////////////////////////////////////////////////////////////////////////////

bool AVApplicationInitParams::isReplayStateNotificationSendingEnabled() const
{
    return m_enable_sending_replay_state_notifications;
}

///////////////////////////////////////////////////////////////////////////////

uint AVApplicationInitParams::getRandomSeed() const
{
    return m_random_seed;
}

///////////////////////////////////////////////////////////////////////////////

bool AVApplicationInitParams::isAbortOnDeadlockEnabled() const
{
    return m_enable_abort_on_deadlock;
}

///////////////////////////////////////////////////////////////////////////////

void AVApplicationInitParams::disableFileLogging()
{
    m_log_to_file = false;
}

///////////////////////////////////////////////////////////////////////////////

void AVApplicationInitParams::disableStdoutLogging()
{
    m_log_to_stdout = false;
}

///////////////////////////////////////////////////////////////////////////////

void AVApplicationInitParams::disableConsole()
{
    m_enable_console = false;
}

///////////////////////////////////////////////////////////////////////////////

void AVApplicationInitParams::disableTimeReferenceAdapter()
{
    m_enable_time_reference_adapter = false;
}

///////////////////////////////////////////////////////////////////////////////

void AVApplicationInitParams::disableExitOnTimeJump()
{
    m_exit_on_time_jump = false;
}

///////////////////////////////////////////////////////////////////////////////

void AVApplicationInitParams::setShmName(const QString& name)
{
    m_shm_name = name;
}

///////////////////////////////////////////////////////////////////////////////

void AVApplicationInitParams::disableEventLoop()
{
    m_flags = UseNoEventLoop;
}

///////////////////////////////////////////////////////////////////////////////

void AVApplicationInitParams::enableExecStandbyChecker()
{
    m_exec_standby_checker_enabled = true;
}

///////////////////////////////////////////////////////////////////////////////

void AVApplicationInitParams::enableSendingReplayStateNotifications()
{
    m_enable_sending_replay_state_notifications = true;
}

///////////////////////////////////////////////////////////////////////////////

void AVApplicationInitParams::setRandomSeed(uint seed)
{
    AVASSERT(seed != 0);
    m_random_seed = seed;
}

///////////////////////////////////////////////////////////////////////////////

void AVApplicationInitParams::enableAbortOnDeadlock()
{
    m_enable_abort_on_deadlock = true;
}

///////////////////////////////////////////////////////////////////////////////

int AVApplicationInitParams::argc() const
{
    return m_argc;
}

///////////////////////////////////////////////////////////////////////////////

char ** AVApplicationInitParams::argv() const
{
    return m_argv;
}

///////////////////////////////////////////////////////////////////////////////

const QString& AVApplicationInitParams::getDefaultAppName() const
{
    return m_default_app_name;
}

///////////////////////////////////////////////////////////////////////////////

const QString& AVApplicationInitParams::getDefaultProcessName() const
{
    return m_default_process_name;
}

///////////////////////////////////////////////////////////////////////////////

bool AVApplicationInitParams::isLogToFileEnabled() const
{
    return m_log_to_file;
}

///////////////////////////////////////////////////////////////////////////////

bool AVApplicationInitParams::islogToStdoutEnabled() const
{
    return m_log_to_stdout;
}

///////////////////////////////////////////////////////////////////////////////

bool AVApplicationInitParams::isConsoleEnabled() const
{
    return m_enable_console;
}

///////////////////////////////////////////////////////////////////////////////

bool AVApplicationInitParams::isTimeReferenceAdapterEnabled() const
{
    return m_enable_time_reference_adapter;
}

///////////////////////////////////////////////////////////////////////////////

bool AVApplicationInitParams::isExitOnTimeJump() const
{
    return m_exit_on_time_jump;
}

///////////////////////////////////////////////////////////////////////////////

const QString& AVApplicationInitParams::getShmName() const
{
    return m_shm_name;
}

///////////////////////////////////////////////////////////////////////////////

int AVApplicationInitParams::getFlags() const
{
    return m_flags;
}

///////////////////////////////////////////////////////////////////////////////

#if defined(Q_OS_WIN32)
void doWSAStartupForWindows()
{
    // RD-293: one call to WSAStartup required before socket functions like gethostname can be used
    // See https://msdn.microsoft.com/en-us/library/windows/desktop/ms742213%28v=vs.85%29.aspx
    WORD wVersionRequested = MAKEWORD(2, 2);
    WSADATA wsaData;
    int err = WSAStartup(wVersionRequested, &wsaData);
    AVASSERTMSG(err == 0, qPrintable(QString("AVHostName: WSAStartup() failed (error=%1)!").arg(err)));
}

void doWSACleanupForWindows()
{
    // RD-293: cleanup the WSAStartup call done in AVDaemonFunctions::AVDaemonInit and AVApplicationPreInit
    WSACleanup();  // Cleanup Winsock
}
#endif

///////////////////////////////////////////////////////////////////////////////

void AVApplicationPreInit( const AVApplicationInitParams& params )
{
    set_terminate(avTerminateHandler);
    set_unexpected(avUnexpectedHandler);

    if (qgetenv("AV_USE_SYSTEM_PROXY_CONFIGURATION")=="0")
        QNetworkProxyFactory::setUseSystemConfiguration(false);


#if defined(Q_OS_WIN32)
    doWSAStartupForWindows();

    //Clear TZ environment variable as it does not work on Windows (QTBUG-64954)
    _putenv_s("TZ", "");
    tzset();
#endif

    AVConfigBase::process_uses_avconfig2 = true;

    AVDeprecatedList::initializeSingleton();

    AVConfig2Global::initializeSingleton(
                params.argc(), params.argv(), params.getAllowExtraCmdLine(),
                params.getDefaultAppName(), params.getDefaultProcessName());
    AVConfig2Global& config = AVConfig2Global::singleton();

    int min_log_level;
    if (config.isHelpRequested() || config.isDumpConfigRequested() || config.isVersionRequested())
    {
        min_log_level = AVLog::LOG__FATAL;
    } else
    {
        min_log_level = AVLog::LOG__INFO - config.getStartupDebugLevel();
        AVASSERT(min_log_level >= 0);
    }

    AVLog::setRootLogger(new AVLog(
                             AVEnvironment::getProcessName(),
                             params.islogToStdoutEnabled(), min_log_level,
                             params.isLogToFileEnabled(), LOGGER_ROOT_ID));
    // Do this after creating the root logger because the factory constructor would do it otherwise...
    AVLogFactory::getInstance().setMinLogLevel(min_log_level);
    AVLogger->Write(LOG_INFO, "%s starting up ...........",
                    qPrintable(AVEnvironment::getProcessName().toUpper()));

    QStringList arguments;
    std::copy(params.argv(), params.argv()+params.argc(), std::back_inserter(arguments));
    AVLogger->startLogPreamble(arguments);

    initializeRandomSeed(params.getRandomSeed());

    AVConfig2Global::singleton().initialize();
}

///////////////////////////////////////////////////////////////////////////////

#if defined (Q_CC_GNU)
//must be used for SIG_IGN definition
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif

void AVApplicationInit(const AVApplicationInitParams& params)
{
    AVASSERT(AVConfigBase::process_uses_avconfig2);
    AVASSERT(qApp != 0);
    AVASSERT(AVLogger != 0);

    //Initialize here, because QApplication constructor might register own signal handler
    AVSignalHandler::initializeSingleton();

    // perform per-thread initializations
    AVThread::setMainThread();
    AVThread::fpinit();

    // start timers which could not be started in preinit because of missing QApplication
    {
        AVLogger->startTimers();
        AVDeprecatedList::singleton().startTimer();
    }

    // sync iostream output with stdout output
    ios::sync_with_stdio();

    // append $APP_HOME/bin to the library search path
    QString home = AVEnvironment::getApplicationHome();
    if (!home.isNull()) {
        QString lp = home + "/bin";
        qApp->addLibraryPath(lp);
    }

    // turn off SIGFPE if allowed
    QString afp = AVEnvironment::getEnv("ALLOW_FP_EXCEPTIONS", false);
    if (afp.isNull()) {
        AVLogger->Write(LOG_INFO, "SIGFPE disabled");
        signal(SIGFPE, SIG_IGN);
    } else {
        AVLogger->Write(LOG_INFO, "SIGFPE enabled");
    }

    //load process config if present
    AVConfig2Global::singleton().loadConfig("processconfig", true);
    //load deprecated time reference if present
    AVConfig2Global::singleton().loadConfig("timereference", true);

    QString process_name = AVEnvironment::getProcessName();
    QString shm_name = params.getShmName().isEmpty()?
                AVProcessState::defaultShmName :
                params.getShmName();

    if(AVConfig2Global::singleton().isSingletonInitialized() && !AVConfig2Global::singleton().isApplicationStartupAllowed())
    {
        const QString dummy_app_name = "DUMMY_APP_NAME";
        AVLogInfo << "Application startup is not allowed - shared memory segment will be created with the name " << dummy_app_name;
        AVProcState = AVProcessStateFactory::getInstance()->getNewProcessState(process_name, shm_name, false, dummy_app_name);
    }
    else
    {
        AVProcState = AVProcessStateFactory::getInstance()->getNewProcessState(process_name, shm_name);
    }

    AVTimeReference::initializeSingleton();

    if(params.isTimeReferenceAdapterEnabled())
    {
        std::unique_ptr<AVTimeReferenceConfigDeprecated> deprecated_config;
        std::unique_ptr<AVTimeReferenceAdapterConfig> config;

        if(AVConfig2Global::singleton().containsLoadedParameterWithPrefix("timereference"))
        {
            deprecated_config.reset(new AVTimeReferenceConfigDeprecated());
        }
        if(AVConfig2Global::singleton().containsLoadedParameterWithPrefix(AVTimeReferenceAdapterConfig::CONFIG_PREFIX))
        {
            config.reset(new AVTimeReferenceAdapterConfig());
        }

        // use old config if new is not present, but when save is called, use new config
        if(!config && deprecated_config && !AVConfig2Global::singleton().isSaveRequested())
        {
            AVLogInfo << "AVApplicationInit: Using deprecated config for time reference: timereference.cc";

            if(!deprecated_config->m_time_server.isEmpty() && deprecated_config->m_time_port > 0)
            {
                AVLogInfo << "AVApplicationInit: instantiating AVTimeReferenceAdapterServer";
                AVTimeReferenceAdapterServer* adapter =
                        new AVTimeReferenceAdapterServer(deprecated_config->m_time_server, deprecated_config->m_time_port);
                AVTimeReferenceAdapter::setSingleton(adapter);
                if(AVConfig2Global::singleton().isApplicationStartupAllowed() && !adapter->waitReceivedMessage() && !AVProcessTerminated)
                {
                    AVLogFatal << "AVApplicationInit: could not establish connection to time reference server";
                }
                AVTimeReferenceAdapter::singleton().setExitOnTimeJump(true);
            }
            else
            {
                AVLogInfo << "AVApplicationInit: instantiating AVTimeReferenceAdapterSharedMemory";
                AVTimeReferenceAdapter::setSingleton(new AVTimeReferenceAdapterSharedMemory());
                AVTimeReferenceAdapter::singleton().setExitOnTimeJump(false);
            }
            AVConfig2Global::singleton().keepOrDeleteConfigInstance(std::move(deprecated_config));
        }
        else if (config || (deprecated_config && AVConfig2Global::singleton().isSaveRequested()))// use the new config when saving
        {
            if(!config)
                config.reset(new AVTimeReferenceAdapterConfig());

            QString adapter_type = config->m_type;

            if(adapter_type == "Server")
            {
                QSharedPointer<AVTimeReferenceAdapterServerConfig> config(
                        new AVTimeReferenceAdapterServerConfig(params.isReplayStateNotificationSendingEnabled()));

                if (deprecated_config != 0) //check consistency between old and new
                {
                    if(deprecated_config->m_time_server != config->m_time_server_hostname ||
                            deprecated_config->m_time_port != config->m_time_server_distribution_port)
                    {
                        AVLogFatal << "AVApplicationInit: detected bot old and new Time reference configuration, which do not match.";
                    }
                }

                AVLogInfo << "AVApplicationInit: instantiating AVTimeReferenceAdapterServer";
                AVTimeReferenceAdapterServer* adapter = new AVTimeReferenceAdapterServer(config);
                AVTimeReferenceAdapter::setSingleton(adapter);
                if(AVConfig2Global::singleton().isApplicationStartupAllowed() &&
                        !config->m_skip_wait_for_time_reference_server_connection &&
                        !adapter->waitReceivedMessage() && !AVProcessTerminated)
                {
                    AVLogFatal << "AVApplicationInit: could not establish connection to time reference server";
                }
                AVTimeReferenceAdapter::singleton().setExitOnTimeJump(params.isExitOnTimeJump());
            }
            else if(adapter_type == "SharedMemory")
            {
                if (deprecated_config != 0) //check consistency between old and new
                {
                    if(!deprecated_config->m_time_server.isEmpty() && deprecated_config->m_time_port != 0)
                    {
                        AVLogFatal << "AVApplicationInit: detected bot old and new Time reference configuration, which do not match.";
                    }
                }

                AVLogInfo << "AVApplicationInit: instantiating AVTimeReferenceAdapterSharedMemory";
                AVTimeReferenceAdapter::setSingleton(new AVTimeReferenceAdapterSharedMemory());
                AVTimeReferenceAdapter::singleton().setExitOnTimeJump(params.isExitOnTimeJump());
            }
            else if(adapter_type == "Local")
            {
                AVLogInfo << "AVApplicationInit: instantiating AVTimeReferenceAdapterLocal";
                AVTimeReferenceAdapter::setSingleton(new AVTimeReferenceAdapterLocal());
            }
            else if(adapter_type == "None")
            {
                AVLogInfo << "AVApplicationInit: not instantiating any AVTimeReferenceAdapter";
                // doing nothing
            }
            else
            {
                AVLogError << "AVApplicationInit: unknown time reference adapter: "<<config->m_type;
            }

            AVConfig2Global::singleton().keepOrDeleteConfigInstance(std::move(config));
        }
        else // neither old nor new config exists
        {
            // doing nothing
        }
    }

    // This must happen after time reference initialization because it depends on event loop.
    if (params.isConsoleEnabled())
    {
        AVConsole::initializeSingleton();
        AVConsole::singleton().setupDefaultInput();

        if(AVTimeReferenceAdapter::isSingletonInitialized())
            AVTimeReferenceAdapter::singleton().registerConsoleSlots();
    }

    AVSysProf::initializeSingleton();

    // parse AV_PLUGIN_PATH
    QStringList plugin_path_list = AVEnvironment::getPath("AV_PLUGIN_PATH");
    Q_FOREACH(QString path, plugin_path_list)
    {
        path = path.trimmed();
        if(!path.isEmpty())
            qApp->addLibraryPath(path);
    }

    // init exec/standby checker
    if(params.isExecStandbyCheckerEnabled())
    {
        AVExecStandbyChecker::initializeSingleton();
    }

    AVConfig2Global::callConfigSingletonInitializers();
    AVProcessWatchdog::createInstance(params.isAbortOnDeadlockEnabled());
    AVProcessWatchdog::singleton().registerThread();
}
#if defined (Q_CC_GNU)
#pragma GCC diagnostic pop
#endif

///////////////////////////////////////////////////////////////////////////////
// AVDaemonInitFunctions wrapper methods

void AVDaemonInit(unsigned int fl, const QString &psn, const QString &shm, bool logToStdout, bool logToFile,
                  bool enableConsole, int init_min_log_level)
{
    AVDaemonFunctions::AVDaemonInit(fl, psn, shm, logToStdout, logToFile, enableConsole, init_min_log_level);
}

///////////////////////////////////////////////////////////////////////////////

void AVDaemonDeinit()
{
    AVDaemonFunctions::AVDaemonDeinit();
}

///////////////////////////////////////////////////////////////////////////////

void AVDaemonProcessStateInit()
{
    AVDaemonFunctions::AVDaemonProcessStateInit();
}

///////////////////////////////////////////////////////////////////////////////

void AVDaemonProcessStateDeinit()
{
    AVDaemonFunctions::AVDaemonProcessStateDeinit();
}

///////////////////////////////////////////////////////////////////////////////

void AVDaemonUpdateHeartbeat()
{
    AVDaemonFunctions::AVDaemonUpdateHeartbeat();
}

///////////////////////////////////////////////////////////////////////////////

QString AVGetProcessName()
{
    return AVDaemonFunctions::AVGetProcessName();
}

///////////////////////////////////////////////////////////////////////////////

int getDSWSicStatus(const QString& procname)
{
    return AVDaemonFunctions::getDSWSicStatus(procname);
}


///////////////////////////////////////////////////////////////////////////////

bool AVDaemonFunctions::m_timereference_adapter_disabled = false;
using deinit_function_map = std::map<int, std::vector<AVDaemonFunctions::deinit_function>>;
Q_GLOBAL_STATIC(deinit_function_map, m_deinit_functions)

///////////////////////////////////////////////////////////////////////////////

#if defined (Q_CC_GNU)
//must be used for SIG_IGN definition
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif

void AVDaemonFunctions::AVDaemonInit(unsigned int fl, const QString &psn, const QString &shm,
                                     bool logToStdout, bool logToFile, bool enableConsole, int init_min_log_level)
{
    Q_UNUSED(fl);

    // We want to establish our signal handler as early as possible, but some applications (e.g. smct3) don't have their
    // qApp initialized before calling AVDaemonInit.
    // In that case, establishing the signal handler is done in AVDaemonProcessStateInit (slightly hackish...)
    if (qApp != nullptr)
    {
        AVSignalHandler::initializeSingleton();
    }

    set_terminate(avTerminateHandler);
    set_unexpected(avUnexpectedHandler);

    if (qgetenv("AV_USE_SYSTEM_PROXY_CONFIGURATION")=="0")
        QNetworkProxyFactory::setUseSystemConfiguration(false);

#if defined(Q_OS_WIN32)
    doWSAStartupForWindows();

    //Clear TZ environment variable as it does not work on Windows (QTBUG-64954)
    _putenv_s("TZ", "");
    tzset();
#endif

    AVASSERT(!AVConfigBase::process_uses_avconfig2);
    AVASSERT(AVLogger != 0 || !psn.isEmpty());

    if (!AVDeprecatedList::isSingletonInitialized()) AVDeprecatedList::initializeSingleton();
    AVDeprecatedList::singleton().startTimer();

    // perform per-thread initializations
    if (qApp != 0) AVThread::setMainThread();
    AVThread::fpinit();

    // sync iostream output with stdout output
    ios::sync_with_stdio();

    // append $APP_HOME/bin to the library search path
    if (qApp != 0) {
        QString home = AVEnvironment::getApplicationHome();
        if (!home.isNull()) {
            QString lp = home + "/bin";
            qApp->addLibraryPath(lp);
        }
    }

    // set the process name
    QString ipsn = AVEnvironment::getEnv("PROCESS_NAME", false);
    if (ipsn.isEmpty()) ipsn = psn;

    // init logger if not yet done
    if (AVLogger == 0 && !ipsn.isEmpty())
    {
        AVLog::setRootLogger(new AVLog(ipsn, logToStdout, init_min_log_level, logToFile, LOGGER_ROOT_ID));
        AVLogger->Write(LOG_INFO, ipsn.toUpper() + " starting up ...........");

        AVLogger->startLogPreamble(qApp ? qApp->arguments(): QStringList{});

        // create log factory after root logger to avoid creation of default root logger
        AVLogFactory::getInstance();
    }

    initializeRandomSeed(0);

    // set global process name in config
    AVEnvironment::setProcessName(ipsn);

    if (!shm.isEmpty()) {
        avshm = new QString(shm);
        AVASSERT(avshm != 0);
    }

    // turn off SIGFPE if allowed
    QString afp = AVEnvironment::getEnv("ALLOW_FP_EXCEPTIONS", false);
    if (afp.isNull()) {
        AVLogger->Write(LOG_INFO, "SIGFPE disabled");
        signal(SIGFPE, SIG_IGN);
    } else {
        AVLogger->Write(LOG_INFO, "SIGFPE enabled");
    }

    // set the std log flag
    if (AVLogger != 0) AVLogger->setStdOut(logToStdout);

    AVTimeReference::initializeSingleton();
    if (!m_timereference_adapter_disabled)
    {
        QSharedPointer<AVTimeReferenceConfigDeprecated> config(new AVTimeReferenceConfigDeprecated());
        if(!config->m_time_server.isEmpty() && config->m_time_port > 0)
        {
            AVTimeReferenceAdapterServer* adapter = new AVTimeReferenceAdapterServer(config->m_time_server, config->m_time_port);
            AVTimeReferenceAdapter::setSingleton(adapter);
            if ((!AVConfig2Global::isSingletonInitialized() || // required for processes with old config, see AC-731
                 AVConfig2Global::singleton().isApplicationStartupAllowed()) &&
                    !adapter->waitReceivedMessage() && !AVProcessTerminated)
            {
                AVLogFatal << "AVApplicationInit: could not establish connection to time reference server";
            }
            AVTimeReferenceAdapter::singleton().setExitOnTimeJump(true);
        }
        else
        {
            AVTimeReferenceAdapter::setSingleton(new AVTimeReferenceAdapterSharedMemory());
            AVTimeReferenceAdapter::singleton().setExitOnTimeJump(false);
        }
    }

    // This will happen if a time server is configured and the program is terminated via ctrl+c
    // before the connection is established.
    if (AVProcessTerminated)
    {
        exit(0);
    }

    // This must happen after time reference initialization because it depends on event loop.
    if (enableConsole && !AVConsole::isSingletonInitialized())
    {
        AVConsole::initializeSingleton();
        AVConsole::singleton().setupDefaultInput();

        if(AVTimeReferenceAdapter::isSingletonInitialized())
            AVTimeReferenceAdapter::singleton().registerConsoleSlots();
    }

    AVSysProf::initializeSingleton();

    //The following statement is a canary to detect wether AVConfig2 is used in a linked library.
    //If this leads to an FATAL stop linking in that libaray or transition to 2-phase initialization.
    AVConfig2Global::callConfigSingletonInitializers();
}

#if defined (Q_CC_GNU)
#pragma GCC diagnostic pop
#endif

///////////////////////////////////////////////////////////////////////////////

void AVDaemonFunctions::AVDaemonDeinit()
{
    AVASSERT(QCoreApplication::instance());
    // Ensure that pending deleteLater request will be handled, which may depend
    // on one of the singletons deleted below
    QCoreApplication::instance()->sendPostedEvents(nullptr, QEvent::DeferredDelete);

    if (AVDeprecatedList::isSingletonInitialized())
    {
        AVDeprecatedList::singleton().slotPrintEntries(false);
    }

    std::for_each(m_deinit_functions->crbegin(), m_deinit_functions->crend(),
    [](const deinit_function_map::value_type& value)
    {
        const auto & vector = value.second;
        std::for_each(vector.rbegin(), vector.rend(), [](const deinit_function& function)
        {
            function();
        });
    });

    // deinit process state
    AVDaemonProcessStateDeinit();

    // deinit logger
    if (AVLogger != 0) {
        AVLogger->Write(LOG_INFO, "process ended");
        // note that this also deletes the root logger, which was autoregistered in
        // AVLogFactory::AVLogFactory
        AVLogFactory::destroyInstance();
        AVLog::setRootLogger(0);
    }

    // delete process name and shm variables
    delete avshm;
    avshm = 0;

    // Ensure that any remaining deleteLater request will be handled eventually
    QCoreApplication::instance()->sendPostedEvents(nullptr, QEvent::DeferredDelete);

#if defined(Q_OS_WIN32)
    doWSACleanupForWindows();
#endif
}

///////////////////////////////////////////////////////////////////////////////

void AVDaemonFunctions::AVDaemonRegisterDeinitFunction(AVDaemonFunctions::deinit_function function, int rank)
{
    (*m_deinit_functions)[rank].push_back(function);
}

///////////////////////////////////////////////////////////////////////////////

void AVDaemonFunctions::AVDaemonProcessStateInit()
{
    // This is no longer necessary with the new init methods
    AVASSERT(!AVConfigBase::process_uses_avconfig2);

    const QString avpsn = AVEnvironment::getProcessName();

    // init the process state if not yet done
    if (AVProcState == 0 && !avpsn.isEmpty()) {
        if (avshm == 0 || avshm->isEmpty())
            AVProcState = AVProcessStateFactory::getInstance()->getNewProcessState(avpsn);
        else
            AVProcState = AVProcessStateFactory::getInstance()->getNewProcessState(avpsn, *avshm);
        AVASSERT(AVProcState != 0);
    }

    // see comment for proper code in AVDaemonInit
    if (!AVSignalHandler::isSingletonInitialized())
    {
        AVSignalHandler::initializeSingleton();
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVDaemonFunctions::AVDaemonProcessStateDeinit()
{
    delete AVProcState;
    AVProcState = 0;
    AVProcessStateFactory::destroyInstance();
}

///////////////////////////////////////////////////////////////////////////////

void AVDaemonFunctions::AVDaemonUpdateHeartbeat()
{
    // update the heartbeat timer
    if (AVProcState != 0) AVProcState->updateHeartbeat();
}

///////////////////////////////////////////////////////////////////////////////

QString AVDaemonFunctions::AVGetProcessName()
{
    return AVEnvironment::getProcessName();
}

/////////////////////////////////////////////////////////////////////////////

int AVDaemonFunctions::getDSWSicStatus(const QString& procname)
{
    AVASSERT(AVProcState != 0);
    AVProcessStateDesc* dswitch_process_state = AVProcState->getProcessStateDesc(procname);
    if (dswitch_process_state == 0) return -1;
    if (!dswitch_process_state->isValid())
    {
        delete dswitch_process_state;
        return -1;
    }

    AVCVariant exec;
    if (dswitch_process_state->getParameter(AV_SHM_DSWITCH_SERVER_STATE, exec))
    {
        delete dswitch_process_state;
        return exec.asInt();
    }
    delete dswitch_process_state;
    return -1;
}

///////////////////////////////////////////////////////////////////////////////

void AVDaemonFunctions::disableTimeReferenceAdapter()
{
    m_timereference_adapter_disabled = true;
}

///////////////////////////////////////////////////////////////////////////////

void AVDaemonFunctions::enableTimeReferenceAdapter()
{
    m_timereference_adapter_disabled = false;
}

/////////////////////////////////////////////////////////////////////////////

void avTerminateHandler()
{
    cerr << "Terminate was called!" << std::endl;
    cerr.flush();

    AVPrintStackTraceUsingGdb();

    abort();
}

/////////////////////////////////////////////////////////////////////////////

void avUnexpectedHandler()
{
    cerr << "An unexpected exception was thrown!" << std::endl;
    cerr.flush();

    AVPrintStackTraceUsingGdb();

    abort();
}

/////////////////////////////////////////////////////////////////////////////

void initializeRandomSeed(uint seed)
{
    if (seed)
    {
        AVLogInfo << "Random number generator seed (overridden by application): " << seed;
    } else
    {
        do
        {
            seed = AVSeed();
        } while (seed == 0);
        AVLogInfo << "Random number generator seed: " << seed;
    }
    AVSRandom(seed);
}

/////////////////////////////////////////////////////////////////////////////

// End of file

