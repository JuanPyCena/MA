///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2013
// QT-Version: QT5
//
// Module:    AVLIB - AviBit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Ralf Gamillscheg, r.gamillscheg@avibit.com
    \author  QT5-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   AVExecStandbyChecker implementation
 */

#include <memory>

// Qt includes
#include <QTimer>

// avlib includes

// local includes
#include "avexecstandbychecker.h"
#include "avexecstandbycheckerconfig.h"
#include "avexecstandbycheckerunittestdriver.h"

///////////////////////////////////////////////////////////////////////////////

AVExecStandbyChecker::AVExecStandbyChecker() :
    m_unit_test_driver(nullptr),
    m_current_state(Unknown)
{
    std::unique_ptr<AVExecStandbyCheckerConfig> config(new (LOG_HERE) AVExecStandbyCheckerConfig());

    if(!config->m_polling_process_name.isEmpty() && !config->m_polling_parameter_name.isEmpty())
    {
        AVASSERT(config->m_polling_interval_msecs > 0);

        AVLogDebug << "AVExecStandbyChecker::AVExecStandbyChecker: process name: "
                <<config->m_polling_process_name<<", parameter name: "
                <<config->m_polling_parameter_name<<", polling interval:"
                <<config->m_polling_interval_msecs;

        m_process_state_poller = std::make_unique<AVProcessStatePoller>(
                config->m_polling_process_name,
                config->m_polling_parameter_name,
                config->m_polling_interval_msecs);

        AVDIRECTCONNECT(m_process_state_poller.get(),
                  SIGNAL(signalShmVarChanged(AVProcessStatePoller::PollingEvent,
                                             const AVCVariant&)),
                  this, SLOT(slotInternalShmVarChanged(AVProcessStatePoller::PollingEvent,
                                                       const AVCVariant&)));
        m_process_state_poller->start();

        AVLogInfo << "AVExecStandbyChecker::AVExecStandbyChecker: start polling for exec/standby";

    }
    else
    {
        AVLogInfo << "AVExecStandbyChecker::AVExecStandbyChecker: "
                "No valid exec/standby checker config found, starting as EXEC";
        m_current_state = Exec;

        // we also have to emit the "aboutToSwitchToExec" signal to inform
        // the AVTwinConnection2, which initializes the debugging container (if
        // so configured)
        QTimer::singleShot(0, this, SIGNAL(signalAboutToSwitchToExec()));
        QTimer::singleShot(0, this, SIGNAL(signalSwitchedToExec()));
    }

    AVConfig2Global::singleton().keepOrDeleteConfigInstance(std::move(config));
}

///////////////////////////////////////////////////////////////////////////////

AVExecStandbyChecker::AVExecStandbyChecker(const QObject& driver) :
    m_unit_test_driver{&driver},
    m_process_state_poller{nullptr},
    m_current_state(Unknown)
{
    AVLogInfo << "AVExecStandbyChecker::AVExecStandbyChecker(const QObject&): "
                 "Test driven exec/standy checker, starting in Unknown";
    AVDIRECTCONNECT(m_unit_test_driver,
              SIGNAL(signalShmVarChanged(AVProcessStatePoller::PollingEvent,
                                         const AVCVariant&)),
              this, SLOT(slotInternalShmVarChanged(AVProcessStatePoller::PollingEvent,
                                                   const AVCVariant&)));
}

///////////////////////////////////////////////////////////////////////////////

AVExecStandbyChecker::~AVExecStandbyChecker()
{
}

///////////////////////////////////////////////////////////////////////////////

AVExecStandbyChecker& AVExecStandbyChecker::initializeSingleton()
{
    return setSingleton(new AVExecStandbyChecker());
}

///////////////////////////////////////////////////////////////////////////////

AVExecStandbyChecker& AVExecStandbyChecker::initializeSingleton(
        const QObject *driver)
{
    return setSingleton(new AVExecStandbyChecker(*driver));
}

///////////////////////////////////////////////////////////////////////////////

AVExecStandbyChecker &AVExecStandbyChecker::initializeSingleton(const AVExecStandbyCheckerUnitTestDriver &driver)
{
    return setSingleton(new (LOG_HERE) AVExecStandbyChecker(driver));
}

///////////////////////////////////////////////////////////////////////////////

bool AVExecStandbyChecker::isExec() const
{
    return m_current_state == Exec;
}

///////////////////////////////////////////////////////////////////////////////

bool AVExecStandbyChecker::isStandby() const
{
    return m_current_state == StandBy;
}

///////////////////////////////////////////////////////////////////////////////

void AVExecStandbyChecker::slotInternalShmVarChanged(AVProcessStatePoller::PollingEvent type,
                                                  const AVCVariant& new_value)
{
    AVLogDebug << "AVExecStandbyChecker::slotInternalShmVarChanged: polling event "<<type
            <<", new_value: "<<new_value.toString();

    if(type == AVProcessStatePoller::Initial || type == AVProcessStatePoller::Changed)
    {
        if(new_value.type() != AVCVariant::Undef)
        {
            ServerState new_server_state = static_cast<ServerState>(new_value.asIntConst());
            if(new_server_state == m_current_state) return;

            bool was_exec = (m_current_state == Exec);
            m_current_state = new_server_state;

            AVLogInfo << "AVExecStandbyChecker::slotInternalShmVarChanged: SWITCHING TO "
                      << enumToString(new_server_state).toUpper();

            if(new_server_state == Exec)
            {
                emit signalAboutToSwitchToExec();
                emit signalSwitchedToExec();
                return;
            } else
            {
                // Always report if we are no longer exec, but suppress reporting the initial transient state
                // transitions (e.g. twin con 2 immediately tries to connect to exec when it receives this signal)
                if(was_exec || new_server_state == StandBy)
                {
                    emit signalAboutToSwitchToNotExec();
                    emit signalSwitchedToNotExec();
                }
                return;
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////


// End of file
