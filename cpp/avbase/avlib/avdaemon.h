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

#if !defined(AVDAEMON_H_INCLUDED)
#define AVDAEMON_H_INCLUDED

// qt includes
#include <QString>
#include <QTimer>
#include <QObject>
#include <QDateTime>
#include <map>

// local includes
#include "avlib_export.h"
#include "avdatetime.h"
#include "avthread.h"
#include "avmisc.h"

// forward declarations
class AVProcessState;

///////////////////////////////////////////////////////////////////////////////
//! global process state pointer which is initialized by AVDaemonInit
//! when called with a process name parameter.

// TODO FLO check
AVLIB_EXPORT extern AVProcessState *AVProcState;
AVLIB_EXPORT extern bool AVProcessTerminated;      // global termination flag

///////////////////////////////////////////////////////////////////////////////
//! *DEPRECATED* See AVApplicationInitParams::disableEventLoop.
enum AVDaemonFlags { UseEventLoop      = 1,   //!< daemon uses an event loop
                     UseNoEventLoop    = 2    //!< does not use an event loop
                   };

///////////////////////////////////////////////////////////////////////////////
//!
//! This class encapsulates the various startup parameters used for AVApplicationPreInit and
//! AVApplicationInit. Apart from the arguments to the constructor, most programs should be fine
//! with the defaults. Setter methods are available only to deviate from the default behaviour.
//!
//! Used only with AVConfig2.
//!
class AVLIB_EXPORT AVApplicationInitParams
{
public:

    //! \param default_app_name     E.g. AVEnvironment::APP_AVCOMMON. Determines e.g. the set of
    //!                             environment variables used.
    //! \param default_process_name Usually the name of the executable. This will be the default returned by
    //!                             AVGetProcessName().
    AVApplicationInitParams(
            int argc, char **argv,
            const QString& default_app_name,
            const QString& default_process_name);

    void disableFileLogging  ();
    void disableStdoutLogging();
    void disableConsole      ();
    /**
     *  Call this method if the time reference for this process should never by modified internally or by an external source (shm, time server).
     *  Usually this is the case for utility programs etc which never run in simulation / replay setups.
     *  Note: this method must also be called if the process instantiates the time reference server adapter manually
     *  (as is the case for the AVTimeReferenceAdapterServerController).
     */
    void disableTimeReferenceAdapter();

    /*!
     * By default all processes exit, when a time jump is detected (in contrast to time shifts)
     * ATTENTION! This parameter has only influence when using the processconfig to define the time reference adapter
     */
    void disableExitOnTimeJump();

    //! See AVConfig2Container::loadCmdlineParams() for a detailed description.
    //! Use AVConfig2Container::getExtraCmdline() to retrieve the extra command line.
    //! Using this option makes the application responsible for checking the extra command line
    //! for valid syntax!
    void enableAllowExtraCmdLine();
    void setShmName          (const QString& name);
    //! *DEPRECATED* This method was formerly used to control behaviour of broken signal handling code, and is currently
    //!              without effect. See SWE-1255.
    void disableEventLoop    ();
    /**
     * This switch causes the AVExecStandbyChecker singleton to be instantiated. A valid configuration is then
     * mandatory.
     */
    void enableExecStandbyChecker();
    /**
     * If this is enabled, the process reports its replay state to the time reference server.
     * A process identifier and replay server port must then be configured (see AVTimeReferenceAdapterServerConfig).
     * To send replay state notifications, use the AVReplayStateNotifier obtained from the AVTimeReferenceAdapterServer.
     */
    void enableSendingReplayStateNotifications();
    /**
     * @brief setRandomSeed can be used to set the random seed for debugging purposes.
     * @param seed Must not be 0.
     */
    void setRandomSeed(uint seed);

    /**
     * @brief enableAbortOnDeadlock enables aborting on deadlock
     * If enabled, the process will be aborted when a deadlock is detected. For configuration and general design see:
     * https://confluence.avibit.com:2233/x/naUIAw
     */
    void enableAbortOnDeadlock();

    int     argc() const;
    char ** argv() const;
    const QString& getDefaultAppName    () const;
    const QString& getDefaultProcessName() const;
    bool isLogToFileEnabled()   const;
    bool islogToStdoutEnabled() const;
    bool isConsoleEnabled()     const;
    bool isTimeReferenceAdapterEnabled() const;
    bool isExitOnTimeJump() const;
    const QString& getShmName() const;
    int getFlags()              const;
    bool getAllowExtraCmdLine() const;
    bool isExecStandbyCheckerEnabled()   const;
    bool isReplayStateNotificationSendingEnabled() const;
    uint getRandomSeed() const;
    bool isAbortOnDeadlockEnabled() const;

private:

    int     m_argc;
    char ** m_argv;
    QString m_default_app_name;
    QString m_default_process_name;

    bool    m_log_to_file;
    bool    m_log_to_stdout;
    bool    m_enable_console;
    bool    m_enable_time_reference_adapter;
    bool    m_exit_on_time_jump;
    QString m_shm_name;
    //! *Deprecated* See AVApplicationInitParams::disableEventLoop.
    int     m_flags;
    // store cmd line params that have no '-<key>' or '--<key>'specified
    bool    m_allow_extra_cmdline;
    bool    m_exec_standby_checker_enabled; //!< if set, an according config has to be present, see AVExecStandbyChecker
    bool    m_enable_sending_replay_state_notifications;
    //! If 0, AVSeed is used to seed the random number generator.
    //! Otherwise this value overrides the used seed (can be used by the application to provide a fixed seed for debugging purposes).
    uint    m_random_seed;
    bool    m_enable_abort_on_deadlock = false;
};

///////////////////////////////////////////////////////////////////////////////
//! New initialization method if AVConfig2 is used, replaces AVDaemonInit/AVDaemonProcessStateInit.
//! It should be called first thing in the main routine (before QApplication is created) and is
//! designed to do all initialization for which the QApplication object is not yet required.
//!
//! It performs the following tasks:
//!
//! - Seeds the random number generator
//! - Initialize AVConfig2Global and set AVConfigBase::process_uses_avconfig2
//! - Initialize application name and process state
//! - Load command line arguments
//! - Initialize logger
//! - Load the initial config set (if specified on the command line)
//! - Initialize the AVConsole
//!
//! After calling this method, and after calling AVConfig2Global::refreshCoreParameters and creating
//! the QApplication object, call AVApplicationInit() (see e.g. vas_simulator.cpp).
AVLIB_EXPORT void AVApplicationPreInit(const AVApplicationInitParams& params);

///////////////////////////////////////////////////////////////////////////////
//! Formerly AVDaemonInit, this is the new init routine used with AVConfig2.
//! It must be called only after the QApplication object was created.
AVLIB_EXPORT void AVApplicationInit(const AVApplicationInitParams& params);

///////////////////////////////////////////////////////////////////////////////
/// \brief The AVDaemonFunctions class
///
class AVLIB_EXPORT AVDaemonFunctions
{
public:

    ///////////////////////////////////////////////////////////////////////////////
    //! Global initialization routine for all daemon programs. This routine
    //! performs system specific tasks to initialize background processes.
    /*!
    Don't call this method when using AVConfig2. Call AVApplicationPreInit and AVApplicationInit instead.

    For instance under windows, a special exception handler is installed. etc.
    Additionally this routine initializes the logger.
    \note if the routine detects that the logger is already initialized
          the logger is not re-initialized.
    \param fl *DEPRECATED* See AVApplicationInitParams::disableEventLoop.
    \param psn the desired process name. Also if psn is not specified, the
               logger is not initialized. The process name is always
               overwritten by the application dependent environment variable
               PROCESS_NAME (e.g. ASTOS_PROCESS_NAME).
               If psn is left empty the AVLogger must be created in advance
               to calling AVDaemonInit.
    \param shm shared memory name or "" to use the default.
    \param logToStdout true to send the log to stdout as well.
    \param logToFile  true to send the log to a file.
    \param enableConsole enable the network console
     */
    static void AVDaemonInit(unsigned int fl,
                             const QString &psn = "", const QString &shm = "",
                             bool logToStdout = false, bool logToFile = true,
                             bool enableConsole = true, int init_min_log_level = AVLog::LOG__INFO);

    ///////////////////////////////////////////////////////////////////////////////
    //! Global de-initialization routine for all daemon programs. This routine
    //! deletes the process state, all singletons and logger, if they exist.
    static void AVDaemonDeinit();

    ///////////////////////////////////////////////////////////////////////////////
    //! Default ranks for deinit funcitons
    //! \sa AVDaemonRegisterDeinitFunction

    enum DeinitFunctionRanks {
        BASE_RANK    = 0, //!< Rank for base deinit function/singletons which do not depend on others
        USER_RANK    = 5, //!< Default rank for user level deinit function/singletons
    };

    ///////////////////////////////////////////////////////////////////////////////
    ///! Registers a function that will be called within AVDaemonDeinit from higher rank
    ///! to lower rank and within a rank starting from more recently added.
    using deinit_function = void(*)(void);
    static void AVDaemonRegisterDeinitFunction(deinit_function function, int rank = DeinitFunctionRanks::USER_RANK);

    ///////////////////////////////////////////////////////////////////////////////
    //! Initialize the process state.
    /*!
    Don't call this method when using AVConfig2. Call AVApplicationPreInit and AVApplicationInit instead.

    This must be done in an application after the configuration is initialized
    because when a process state is initialized twice for a given process,
    the process variables are cleared. Invoking a process twice is usually
    done with the -help or -save options which would clear the process
    variables then.
     */
    static void AVDaemonProcessStateInit();

    ///////////////////////////////////////////////////////////////////////////////
    //! De-initialize the process state.
    /*! The global variable AVProcState is deleted and set to 0. This is also
    automatically done in \sa AVDaemonDeinit.
     */
    static void AVDaemonProcessStateDeinit();

    ///////////////////////////////////////////////////////////////////////////////
    //! Update the heartbeat timer.
    /*! Update the heartbeat timer. This is required in reqular intervals
    to signal that the process is still running.
     */
    static void AVDaemonUpdateHeartbeat();

    ///////////////////////////////////////////////////////////////////////////////
    //! Returns the process name
    //! TODO CM why does this method exist?
    /*! Returns the process name set in AVDaemonInit
     */
    static QString AVGetProcessName();

    /////////////////////////////////////////////////////////////////////////////
    //! DEPRECATED, use AVExecStandbyChecker instead.
    //! returns the SIC Status (EXEC, etc.) of the DSW, return -1 on error.
    /*! Server states (see avserverinterconnect.h) are:
     *  StandBy     = 0,
     *  Exec        = 1,
     *  Negotiate   = 2,
     *  PendingExec = 3
     */
    static int getDSWSicStatus(const QString& procname = "dswitch");

    static void disableTimeReferenceAdapter();
    static void enableTimeReferenceAdapter();

private:
    static bool m_timereference_adapter_disabled;
};

///////////////////////////////////////////////////////////////////////////////
// AVDaemonInitFunctions wrapper methods
AVLIB_EXPORT void AVDaemonInit(unsigned int fl, const QString &psn = "", const QString &shm = "",
                  bool logToStdout = false, bool logToFile = true,
                  bool enableConsole = true, int init_min_log_level = AVLog::LOG__INFO);

AVLIB_EXPORT void AVDaemonDeinit();

AVLIB_EXPORT void AVDaemonProcessStateInit();

AVLIB_EXPORT void AVDaemonProcessStateDeinit();

AVLIB_EXPORT void AVDaemonUpdateHeartbeat();

/**
 * @brief AVGetProcessName returns the name of the process provided in AVApplicationInitParams, or the value overridden
 *        by the <an>_PROCESS_NAME environment variable.
 *
 *  Also refer to the environment variable documentation in confluence.
 */
AVLIB_EXPORT QString AVGetProcessName();

//! DEPRECATED, use AVExecStandbyChecker instead.
AVLIB_EXPORT int getDSWSicStatus(const QString& procname = "dswitch");

///////////////////////////////////////////////////////////////////////////////
//! DEPRECATED, use AVExecStandbyChecker instead.
//! Checks if the DSW is EXEC or not.
/*!
 *  Checks if the DSW on this host is in EXEC mode by checking on the DSW SIC status (SHM).
 *
 *  \return True if the DSW on this host is in EXEC mode.
 */
inline bool isDSWExec()
{
    static AVDateTime is_dsw_exec_last_error_log_dt;

    int ret = getDSWSicStatus();
    if (ret < 0)
    {
        if (is_dsw_exec_last_error_log_dt.secsTo(AVDateTime::currentDateTimeUtc()) > 30000 ||
                !is_dsw_exec_last_error_log_dt.isValid())
        {
            AVLogError << "isDSWExec: got return value " << ret << " - "
                       << "omitting logging for 30s";
            is_dsw_exec_last_error_log_dt = AVDateTime::currentDateTimeUtc();
        }
    }

    //TODO: enum is defined inside AVServerInterConnect
    //return ret == AVServerInterConnect::Exec;
    return ret == 1;
}

//! An override for the default terminate() function of the STL
AVLIB_EXPORT void avTerminateHandler();

//! An override for the default unexpected() function of the STL
AVLIB_EXPORT void avUnexpectedHandler();

/**
 * @param seed Pass along 0 to use AVSeed (std::random_device), any other value overrides the seed (for debugging purposes)
 */
void initializeRandomSeed(uint seed);

/////////////////////////////////////////////////////////////////////////////

//! updates the avdaemon heartbeat in configurable regular intervals
class AVLIB_EXPORT AVDaemonHeartbeatUpdater : public QObject
{
    Q_OBJECT

public:
    //! Standard Constructor
    explicit AVDaemonHeartbeatUpdater(uint update_interval_ms = 100)
    {
        //----- setup heartbeat
        connect(&m_heartbeat_timer, SIGNAL(timeout()), this, SLOT(slotUpdateHeartbeat()));
        m_heartbeat_timer.start(update_interval_ms);
    }

    //! Destructor
    ~AVDaemonHeartbeatUpdater() override {}

protected slots:

    //! called by the heartbeat timer
    void slotUpdateHeartbeat() { AVDaemonUpdateHeartbeat(); }

protected:

    QTimer m_heartbeat_timer;

private:
    //! Hidden copy-constructor
    AVDaemonHeartbeatUpdater(const AVDaemonHeartbeatUpdater&);
    //! Hidden assignment operator
    const AVDaemonHeartbeatUpdater& operator = (const AVDaemonHeartbeatUpdater&);
};

/////////////////////////////////////////////////////////////////////////////

class AVLIB_EXPORT AVDaemonHeartbeatUpdaterThread : public AVThread
{
public:
    //! Standard Constructor
    explicit AVDaemonHeartbeatUpdaterThread(uint update_interval_ms) :
        AVThread("AVDaemonHeartbeatUpdaterThread"),
        m_update_interval_ms(update_interval_ms) {}

    //! Destructor
    ~AVDaemonHeartbeatUpdaterThread() override
    {
        AVLogger->Write(LOG_INFO, "AVDaemonUpdateHeartbeat: shutdown fin");
        stop();
    }

    void runThread() override
    {
        while (!checkForStop() && !AVProcessTerminated)
        {
            AVDaemonUpdateHeartbeat();
            sleepMs(m_update_interval_ms);
        }

        AVLogger->Write(LOG_INFO, "AVDaemonUpdateHeartbeat: finished");
    }

    void stopThread() override {}

protected:

    uint m_update_interval_ms;

private:
    //! Hidden copy-constructor
    AVDaemonHeartbeatUpdaterThread(const AVDaemonHeartbeatUpdaterThread&);
    //! Hidden assignment operator
    const AVDaemonHeartbeatUpdaterThread& operator = (const AVDaemonHeartbeatUpdaterThread&);
};

#endif

// End of file
