//-----------------------------------------------------------------------------
//
// Package:   AVCOMMON
// Copyright: AviBit data processing GmbH, 2001-2015
//
// Module:    AVLIB
//
//-----------------------------------------------------------------------------

/*! \file
    \author  Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   AVTimeReferenceAdapterConfig implementation
 */

// local includes
#include "avtimereferenceadapterconfig.h"

// Qt includes

// avlib includes

//-----------------------------------------------------------------------------

const QString AVTimeReferenceAdapterConfig::CONFIG_PREFIX = "processconfig.timereferenceadapter";

//-----------------------------------------------------------------------------

AVTimeReferenceAdapterConfig::AVTimeReferenceAdapterConfig() :
                 AVConfig2(CONFIG_PREFIX)
{
    setHelpGroup("timereferenceadapter");

    registerParameter("type", &m_type, "The type of adapter to be used. Use 'None' for operational setups.").setValidValues(
            QStringList() << "Server" << "SharedMemory" << "Local" << "None").setSuggestedValue("None");
    refreshParams();
}

//-----------------------------------------------------------------------------

AVTimeReferenceAdapterConfig::~AVTimeReferenceAdapterConfig()
{
}

// End of file
