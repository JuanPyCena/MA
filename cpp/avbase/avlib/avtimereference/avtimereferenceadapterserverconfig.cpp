///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON
// Copyright: AviBit data processing GmbH, 2001-2014
// QT-Version: QT5
//
// Module:    AVLIB
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   AVTimeReferenceAdapterServerConfig implementation
 */

// Qt includes

// avlib includes

// local includes
#include "avtimereferenceadapterserverconfig.h"

///////////////////////////////////////////////////////////////////////////////

const QString AVTimeReferenceAdapterServerConfig::CONFIG_PREFIX = "processconfig.timereferenceadapter.server";

///////////////////////////////////////////////////////////////////////////////

AVTimeReferenceAdapterServerConfig::AVTimeReferenceAdapterServerConfig(bool with_replay_notification) :
         AVConfig2(CONFIG_PREFIX)
{
    setHelpGroup("timereferenceadapterserver");

    registerParameter("time_server_hostname", &m_time_server_hostname, "host name of the time reference server, "
            "leave empty for no connection");

    registerParameter("time_server_distribution_port", &m_time_server_distribution_port, "distribution port of the time reference server");

    if(with_replay_notification)
    {
        registerParameter("time_server_replay_notification_process_identifier",
                          &m_time_server_replay_notification_process_identifier,
                          "Identifies this process at the time reference server for submitting replay state");
        registerParameter("time_server_replay_notification_port", &m_time_server_replay_notification_port,
                          "port for notifying replay state at time reference server");
    }

    registerParameter("ignore_timeserver", &m_skip_wait_for_time_reference_server_connection,
                      "Skip waiting for the connection to the timereference server", false)
            .setPureCmdlineOption(false);

    refreshParams();
}

///////////////////////////////////////////////////////////////////////////////

AVTimeReferenceAdapterServerConfig::~AVTimeReferenceAdapterServerConfig()
{
}

// End of file
