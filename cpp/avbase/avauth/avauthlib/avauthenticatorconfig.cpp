#include "avauthenticatorconfig.h"

#include "avlog.h"

namespace av
{
namespace auth
{

//--------------------------------------------------------------------------------------------------

AuthenticatorConfig::AuthenticatorConfig(const QString& prefix, AVConfig2Container& container) :
    AVConfig2(prefix + ".authentication", container)
{
    registerParameter("keycloak_url", &m_keycloak_url,
                      "Keycloak URL (including protocol & port).");

    registerParameter("realm", &m_realm, "The keycloak realm to use.");
    registerParameter("client_id", &m_client_id, "The keycloak client ID.");

    registerParameter("login_body", &m_login_body, "HTML body of the login page.").setOptional("Login successful! Please close this window and switch back to the application.");

    registerParameter("http_timeout_s", &m_http_timeout_s, "Timeout for HTTP requests in seconds.").setOptional(10);
    registerParameter("login_timeout_s", &m_login_timeout_s, "Timeout for user performing login in browser in seconds.\nSet to -1 to disable.").setOptional(120);
    registerParameter("refresh_headstart_s", &m_refresh_headstart_s, "How many seconds before the token expires should refresh be done.\nSet to -1 to disable refresh.").setOptional(0);

    registerParameter("ignore_ssl_errors", &m_ignore_ssl_errors, "Ignore SSL errors.").setOptional(false);

    refreshParams();
}

//--------------------------------------------------------------------------------------------------

AuthenticatorConfig::AuthenticatorConfig(AVConfig2::UnitTestConstructorSelector s) :
    AVConfig2(s)
{
    m_http_timeout_s = 10;
    m_login_timeout_s = 120;
}


}  // namespace auth
}  // namespace av
