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

#ifndef AVEXECSTANDBYCHECKER_UNITTEST_DRIVER_INCLUDED
#define AVEXECSTANDBYCHECKER_UNITTEST_DRIVER_INCLUDED

#include "avexecstandbychecker.h"

//-----------------------------------------------------------------------------
//! Test driver for the AVExecStandbyChecker to be used in unit tests
//!
class AVLIB_EXPORT AVExecStandbyCheckerUnitTestDriver : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(AVExecStandbyCheckerUnitTestDriver)

public:

    explicit AVExecStandbyCheckerUnitTestDriver();
    virtual ~AVExecStandbyCheckerUnitTestDriver() = default;

    void switchToExec();
    void switchToStandby();

signals:
    void signalShmVarChanged(AVProcessStatePoller::PollingEvent event_type, const AVCVariant& new_value);

private:
    void changeServerState(AVExecStandbyChecker::ServerState server_state);

    AVExecStandbyChecker::ServerState m_current_state;
};

#endif // AVEXECSTANDBYCHECKER_UNITTEST_DRIVER_INCLUDED

// End of file
