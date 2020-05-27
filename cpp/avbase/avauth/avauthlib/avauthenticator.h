#pragma once

#include "avauthlib_export.h"
#include <avidtoken.h>
#include <avservicesmetadata.h>
#include <avauthenticatorconfig.h>

#include <QtCore/QObject>
#include <QtNetwork/QNetworkAccessManager>

class QTcpServer;
class AuthenticatorTest;

namespace av
{
namespace auth
{
using AccessToken = QString;
struct AuthenticationResponse;
struct RawTokenData;

//--------------------------------------------------------------------------------------------------
/**
 * @brief The main interface to a user application for authenticating a user.
 * The several configuration parameters can be configured by passing AuthenticatorConfig.
 *
 * The login process is initiated by calling the Authenticator::authenticate method. All
 * background task like HTTP requests and waiting for browser interaction are done event driven asynchronously.
 * Therefore, an event loop needs to be active till either Authenticator::error or Authenticator::authenticated
 * signal is emitted.
 *
 * The id token and access token are not to be stored by the application, instead they should always be
 * retrieved via the their getters from the Authenticator instance, because it might change on refresh.
 *
 * Refreshing the token can be done either automatically by configuring "refresh_interval_s" config option or
 * by manually calling the Authenticator::refreshToken() method.
 *
 * When an error occurs no further action is taken by Authenticator. Particularly after a failed refresh
 * a new authentication process has to be started explicitly by calling the Authenticator::authenticate method.
 *
 * This class is reentrant and must only be accessed from one thread.
 *
 *
 * @code
 * av::auth::Authenticator authenticator(config);
 * QObject::connect(&authenticator, &adbsg::Authenticator::authenticated, [](av::auth::IdToken id_token){
 *   //use id_token
 * });
 * QObject::connect(&authenticator, &adbsg::Authenticator::error, [](av::auth::Authenticator::Error error_code){
 *   //handle error
 * });
 *
 * authenticator.authenticate();
 * // resume event processing
 *
 * qApp->exec();
 *
 * @endcode
 */
class AVAUTHLIB_EXPORT Authenticator : public QObject
{
    Q_OBJECT

    friend class ::AuthenticatorTest;

    //UnitTest construtor
    Authenticator(QNetworkAccessManager* nam, std::function<bool(QUrl)> open_browser_callback, std::unique_ptr<AuthenticatorConfig> config = {});

public:
    Authenticator(std::unique_ptr<AuthenticatorConfig> config, QObject *parent = nullptr);

    ~Authenticator() override;

    /**
     * @brief getIdToken
     * The ID token can be used for retrieving properties of the user.
     * The ID token must not be stored anywhere and always obtained via this function, because it is subject
     * to change when expired or refreshed.
     * @return The current id token.
     */
    IdToken const &getIdToken() const;
    /**
     * @brief getAccessToken
     * The access token can be used for authenticating at other services.
     * The access token must not be stored anywhere and always obtained via this function, because it is subject
     * to change when expired or refreshed.
     * @return The current access token
     */
    AccessToken const &getAccessToken() const;

    enum class State
    {
        Uninitialized,
        Invalid,
        ReadingServices,
        ServicesRead,
        WaitingForRedirect,
        RequestingAuthenticationData,
        ValidatingAuthenticationData,
        Authenticated,
        Expired
    };
    Q_ENUM(State)

    enum class ErrorCode
    {
        NoError,
        CouldNotReadServiceInfo,
        InvalidBrowser,
        ServerError,
        TimedOut,
        MaxTriesReached,
        InvalidCode,
        InvalidIdToken,
        NoRolesForClientId,
        VerificationFailed,
        TokenExpired,
        InvalidRefreshTimeOut,
    };
    Q_ENUM(ErrorCode)

    /**
     * @brief getState
     * Represents the current state of the Authenticator. Initially state is Invalid.
     * @return current state of the Authenticator
     */
    State getState() const;

    /**
     * @name Authentication Process
     * @{
     */
signals:

    /**
     * @brief stateChanged
     * This signal is emitted when the state of the Authenticator has been changed
     */
    void stateChanged(av::auth::Authenticator::State);

    /**
     * @brief authenticated
     * This signal is emitted when the authentication was obtained successfully. This signal is
     * emitted after the corresponding stateChanged(Authenticated) signal.
     */
    void authenticated();

    /**
     * @brief error
     * This signal is emitted when the authentication was not possible and an error occurred.
     * This signal is emitted after the corresponding stateChanged(Invalid) signal.
     */
    void error(av::auth::Authenticator::ErrorCode);

public:

    /**
     * @brief authenticate Starts a new authentication process
     * This starts a new authentication process asynchronously. At the end either the authenticated or
     * the error signal is emitted.
     *
     */
    void authenticate();

    /**
     * @brief logout Clears local authentication data.
     * This doesn't perform an actual logout request, it only clears local authentication data.
     * After this the state is Uninitialized.
     */
    void logout();

    /**
     * @} */

private slots:
    void onNewConnection();

private:

    /**
     * @brief refreshToken refreshes the authentication token
     * This starts an asynchronous request to refresh the tokens. At the end either the authenticated or
     * the error signal is emitted.
     */
    void refreshToken();

    /**
     * @name Authentication process
     * The following methods roughly follow the steps sequentially
     * of the authentication process.
     * @{
     */

    /**
     * @brief Request service information from a standard URL based on client
     *        provided information.
     */
    void requestServices();

    /**
     * @brief Starts the login procedure using external web browser
     *
     * Using the information from the previously retrieved ServiceMetaData, an
     * external web browser is opened for the user to enter their credentials.
     *
     * @return NoError if successful or InvalidBrowser if browser failed.
     */
    void initiateLogin();

    /**
     * @brief Using the received code, requests authentication data.
     */
    void requestAuthenticationData();

    void handleAuthenticationResponse(AuthenticationResponse auth_response);

    /**
     * @brief Checks the validity of the received authentication data.
     */
    void validateToken(const RawTokenData &token);

    /** @} */


    void requestToken(std::vector<std::pair<QString, QString> > query_parameter);

    void handleError(ErrorCode error_code);

    QString buildRedirectUri() const;

    // Generates a string representing the current application and its instance.
    // For now just make a string from the (this) pointer.
    static QString generateStateString(void *ptr);
    static QString generateCodeVerifier();
    static QString createCodeChallenge(QString const &codeVerifier);
    static QCryptographicHash::Algorithm stringToAlgo(QString const &sAlg, bool *pOK);

    IdToken m_id_token;
    QString m_access_token;

    State m_state = State::Uninitialized;

    // Values generated internally for verification
    QString m_request_state;
    QString m_code_verifier; /**< Used for PKCE - Proof Key for Code Exchange */

    // Values received from the Authentication server
    QString m_code;

    std::unique_ptr<QTcpServer> m_redirect_app_server;
    int                        m_server_port = -1;
    ServicesMetaData           m_service_metadata;

    std::function<bool(QUrl)>  m_open_browser_callback;

    QNetworkAccessManager*     m_nam = nullptr;
    QTimer                     m_login_timer { this };

    QTimer                     m_refresh_timer { this };
    QString                    m_refresh_token;

    ErrorCode                  m_errorcode;

    QPointer<QNetworkReply>    m_reply;

    std::unique_ptr<AuthenticatorConfig> m_config;

    void reset_internal_state();
};  // class Authenticator

}  // namespace auth
}  // namespace av
