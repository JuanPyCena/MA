//-----------------------------------------------------------------------------
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2019
//
// Module:    AVLIB - AviBit Library
//
//-----------------------------------------------------------------------------

/*! \file
    \brief   AVProcessWatchdog header
 */

#pragma once

// system/Qt includes
#include <QtGlobal>
#include <mutex>

// common includes
#include "avsingleton.h"

// local includes
#include "avconfig2.h"
#include "avlib_export.h"

// forward declarations
class AVProcessWatchdogMonitor;

//-----------------------------------------------------------------------------

class AVLIB_EXPORT AVProcessWatchdogConfig : public AVConfig2
{
public:

    AVProcessWatchdogConfig();

    int m_deadlock_limit_msec;
    int m_info_limit_msec;
};

//-----------------------------------------------------------------------------
//! Class for monitoring threads.
class AVLIB_EXPORT AVProcessWatchdog : public AVExplicitSingleton<AVProcessWatchdog>
{
    //! friend declaration for function level test case
    friend class TstAVProcessWatchdog;
    friend class AVExplicitSingleton<AVProcessWatchdog>;

    static constexpr int DEINIT_RANK = AVDaemonFunctions::DeinitFunctionRanks::BASE_RANK;

    explicit AVProcessWatchdog(bool abort_on_deadlock);
    virtual ~AVProcessWatchdog();
public:

    //! start monitoring of the current thread
    void registerThread();

    //! Called to report that thread is still alive
    void reportAlive();

    void setAbortOnDeadlockEnabled(bool enabled);
    bool isAbortOnDeadlockEnabled();

private:
    QThread m_sentinel_thread;

    std::mutex m_map_mutex;
    std::map<QThread*, AVProcessWatchdogMonitor*> m_monitored_threads;

    bool m_abort_on_deadlock_enabled = false;
    AVProcessWatchdogConfig m_config;

    Q_DISABLE_COPY(AVProcessWatchdog)
};
