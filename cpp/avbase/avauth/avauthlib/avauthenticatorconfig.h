#pragma once

#include "avauthlib_export.h"

#include <QString>

#include "avconfig2.h"

namespace av
{
namespace auth
{

//--------------------------------------------------------------------------------------------------
class AVAUTHLIB_EXPORT AuthenticatorConfig : public AVConfig2
{
public:
    /**
     * @brief AuthenticatorConfig
     * @param prefix    The config prefix below which the "authentication" namespace of this class goes.
     * @param container
     */
    explicit AuthenticatorConfig(const QString& prefix, AVConfig2Container& container = AVConfig2Global::singleton());
    explicit AuthenticatorConfig(AVConfig2::UnitTestConstructorSelector s);

    QString m_keycloak_url;
    QString m_realm;
    QString m_client_id;

    QString m_login_body;

    int m_http_timeout_s = 0;
    int m_login_timeout_s = 0;
    int m_refresh_headstart_s = 0;
    bool m_ignore_ssl_errors = false;
};

}  // namespace auth
}  // namespace av
