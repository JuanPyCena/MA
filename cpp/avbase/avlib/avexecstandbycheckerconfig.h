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
    \brief   AVExecStandbyCheckerConfig header
 */

#ifndef AVEXECSTANDBYCHECKERCONFIG_INCLUDED
#define AVEXECSTANDBYCHECKERCONFIG_INCLUDED

// Qt includes
#include <QObject>

// avlib includes
#include "avlib_export.h"
#include "avmacros.h"

// local includes
#include "avconfig2.h"

// forward declarations

///////////////////////////////////////////////////////////////////////////////
//! Config for the AVExecStandbyChecker
/*!
 *  Note that the processconfig.cc file is loaded in avdaemon.cpp, and is also used e.g. for time reference configuration.
 */
class AVLIB_EXPORT AVExecStandbyCheckerConfig : public AVConfig2
{
public:
    static const QString CONFIG_PREFIX;

public:
    AVExecStandbyCheckerConfig();
    ~AVExecStandbyCheckerConfig() override;

public:

    QString m_polling_process_name;
    QString m_polling_parameter_name;
    uint    m_polling_interval_msecs;
};

#endif // AVEXECSTANDBYCHECKERCONFIG_INCLUDED

// End of file
