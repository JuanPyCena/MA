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
    \brief   AVExecStandbyCheckerConfig implementation
 */

// Qt includes

// avlib includes

// local includes
#include "avexecstandbycheckerconfig.h"

///////////////////////////////////////////////////////////////////////////////

const QString AVExecStandbyCheckerConfig::CONFIG_PREFIX = "processconfig.avexecstandbychecker";

///////////////////////////////////////////////////////////////////////////////

AVExecStandbyCheckerConfig::AVExecStandbyCheckerConfig()  :
    AVConfig2(CONFIG_PREFIX)
{
    setHelpGroup("avexecstandbychecker");

    registerParameter("polling_process_name", &m_polling_process_name,
                      "The process that should be polled for exec/standby, disabled if empty")
                      .setCmdlineSwitch("exec_standby_polling_process_name")
                      .setSuggestedValue(""); // this is intentionally empty so processes in test setups without
                                              // processtate.cc will not suddenly wait for exec status in SHM...
                                              // Would be "dswitch" otherwise.

    registerParameter("polling_parameter_name",
                      &m_polling_parameter_name,
                      "The parameter name determining the process state")
                      .setCmdlineSwitch("exec_standby_polling_parameter_name")
                      .setSuggestedValue("serverState");

    registerParameter("polling_interval_msecs",
                      &m_polling_interval_msecs,
                      "The polling for exec/standby interval in msecs")
                      .setCmdlineSwitch("exec_standby_polling_interval_msecs")
                      .setSuggestedValue(1000).setMin(10);

    refreshParams();
}

///////////////////////////////////////////////////////////////////////////////

AVExecStandbyCheckerConfig::~AVExecStandbyCheckerConfig()
{
}

// End of file
