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
    \author  Christian Muschick, c.muschick@avibit.com
    \brief
 */

#include "avexecstandbycheckerunittestdriver.h"

//-----------------------------------------------------------------------------

AVExecStandbyCheckerUnitTestDriver::AVExecStandbyCheckerUnitTestDriver()
    : m_current_state(AVExecStandbyChecker::Unknown)
{
}

//-----------------------------------------------------------------------------

void AVExecStandbyCheckerUnitTestDriver::switchToExec()
{
    if (m_current_state == AVExecStandbyChecker::Exec)
    {
        AVLogInfo << "AVExecStandbyCheckerUnitTestDriver::switchToExec: Already exec";
        return;
    }

    changeServerState(AVExecStandbyChecker::Exec);
}

//-----------------------------------------------------------------------------

void AVExecStandbyCheckerUnitTestDriver::switchToStandby()
{
    if (m_current_state == AVExecStandbyChecker::StandBy)
    {
        AVLogInfo << "AVExecStandbyCheckerUnitTestDriver::switchToExec: Already standby";
        return;
    }

    changeServerState(AVExecStandbyChecker::StandBy);
}

//-----------------------------------------------------------------------------

void AVExecStandbyCheckerUnitTestDriver::changeServerState(AVExecStandbyChecker::ServerState server_state)
{
    bool initial = (m_current_state == AVExecStandbyChecker::Unknown);
    AVProcessStatePoller::PollingEvent event_type = initial ? AVProcessStatePoller::Initial : AVProcessStatePoller::Changed;

    int value = static_cast<int>(server_state);
    emit signalShmVarChanged(event_type, AVCVariant(&value));
}

// End of file
