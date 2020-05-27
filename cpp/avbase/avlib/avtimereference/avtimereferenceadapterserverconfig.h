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
    \brief   AVTimeReferenceAdapterServerConfig header
 */

#ifndef AVTIMEREFERENCEADAPTERSERVERCONFIG_INCLUDED
#define AVTIMEREFERENCEADAPTERSERVERCONFIG_INCLUDED

// Qt includes
#include <QObject>

// avlib includes
#include "avlib_export.h"
#include "avmacros.h"
#include "avconfig2.h"

// local includes

// forward declarations


class AVLIB_EXPORT AVTimeReferenceAdapterServerConfigBase
{
public:
    AVTimeReferenceAdapterServerConfigBase() :
        m_time_server_distribution_port(0),
        m_time_server_replay_notification_port(0),
        m_skip_wait_for_time_reference_server_connection(false)
        {}
    virtual ~AVTimeReferenceAdapterServerConfigBase() {}

    QString m_time_server_hostname;
    uint m_time_server_distribution_port;

    QString m_time_server_replay_notification_process_identifier;
    uint m_time_server_replay_notification_port;

    bool m_skip_wait_for_time_reference_server_connection;
};


///////////////////////////////////////////////////////////////////////////////
//! Configuration class for AVTimereferenceAdapterServer
/*!
*/
class AVLIB_EXPORT AVTimeReferenceAdapterServerConfig : virtual public AVTimeReferenceAdapterServerConfigBase, public AVConfig2
{
public:
    explicit AVTimeReferenceAdapterServerConfig(bool with_replay_notification = false);
    ~AVTimeReferenceAdapterServerConfig() override;

private:
    static const QString CONFIG_PREFIX;

};

#endif // AVTIMEREFERENCEADAPTERSERVERCONFIG_INCLUDED

// End of file
