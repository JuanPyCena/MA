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
    \brief   AVTimeReferenceConfigDeprecated header
 */

#ifndef AVTIMEREFERENCECONFIGDEPRECATED_INCLUDED
#define AVTIMEREFERENCECONFIGDEPRECATED_INCLUDED

// Qt includes
#include <QObject>

// avlib includes
#include "avlib_export.h"
#include "avmacros.h"

// local includes
#include "avconfig2.h"
#include "avconfig2classic.h"

// forward declarations

///////////////////////////////////////////////////////////////////////////////
//! Configuration of time reference
/*!
*/
class AVLIB_EXPORT AVTimeReferenceConfigDeprecated : public AVConfig2HybridConfig
{
public:
    AVTimeReferenceConfigDeprecated();

    void exitApp(int) override {}

    uint m_tr_version;
    QString m_time_server;
    uint m_time_port;
};

#endif // AVTIMEREFERENCECONFIGDEPRECATED_INCLUDED

// End of file
