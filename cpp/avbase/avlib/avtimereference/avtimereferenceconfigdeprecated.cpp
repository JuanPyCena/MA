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
    \brief   AVTimeReferenceConfig implementation
 */

// Qt includes

// avlib includes
#include "avdeprecate.h"

// local includes
#include "avtimereferenceconfigdeprecated.h"

///////////////////////////////////////////////////////////////////////////////

AVTimeReferenceConfigDeprecated::AVTimeReferenceConfigDeprecated() : AVConfig2HybridConfig("Time reference", "timereference")
{
    m_show_help_per_default = false;

    QString help_version = "0=old TR version, 1=n e w TR version "
            "with milliseconds offset, 2=millisecond offset and correct speed handling";

    if (isNewConfigUsed())
    {
        registerParameter("tr_version", &m_tr_version, help_version).
                //setValidValues(QValueList<uint>() << 0u << 1u << 2u). // FIXXME: to be fixed by PTS#5697
                setOptional(2).
                setDeprecatedName("TRVersion");
        registerParameter("time_server", &m_time_server, "the timereference server to connect to. "
                          "leave this or the port empty to disable.").
                setOptional("");
        registerParameter("time_port",   &m_time_port,   "the port of the timereference server to connect to. "
                          "leave this or the server empty to disable.").
                setOptional(0);

        refreshParams();

    }
    else
    {
        ADDCE("TRVersion", &m_tr_version, 2, qPrintable(help_version));
        ADDCE("time_server", &m_time_server, "", "the time server address to connect to. "
              "leave this or the port empty to disable.");
        ADDCE("time_port",   &m_time_port,   0,  "the time server port to connect to. "
              "leave this or the server empty to disable.");

        standardInit(false);
    }

    if (m_tr_version < 2)
    {
        AVDEPRECATE(Old version of AVTimeReference);
    }
}

///////////////////////////////////////////////////////////////////////////////


// End of file
