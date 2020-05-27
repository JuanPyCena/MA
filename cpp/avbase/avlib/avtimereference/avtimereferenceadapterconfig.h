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
    \brief   AVTimeReferenceAdapterConfig header
 */

#ifndef AVTIMEREFERENCEADAPTERCONFIG_INCLUDED
#define AVTIMEREFERENCEADAPTERCONFIG_INCLUDED

// local includes

// Qt includes
#include <QObject>

// avlib includes
#include "avlib_export.h"
#include "avconfig2.h"

// forward declarations


class AVLIB_EXPORT AVTimeReferenceAdapterConfigBase
{
public:
    explicit AVTimeReferenceAdapterConfigBase() {}
    virtual ~AVTimeReferenceAdapterConfigBase() {}

    QString m_type;
};

//-----------------------------------------------------------------------------
//! Generic class for AVTimeReferenceAdapter. Includes the type of adapter to be used
/*!
*/
class AVLIB_EXPORT AVTimeReferenceAdapterConfig : virtual public AVTimeReferenceAdapterConfigBase, public AVConfig2
{

public:
    explicit AVTimeReferenceAdapterConfig();
    ~AVTimeReferenceAdapterConfig() override;

    static const QString CONFIG_PREFIX;
};

#endif // AVTIMEREFERENCEADAPTERCONFIG_INCLUDED

// End of file
