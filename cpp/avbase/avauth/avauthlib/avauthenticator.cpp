#include "avauthenticator.h"
#include "avopeniddata.h"
#include "avtokendata.h"

#include "avlog.h"

#include <QtCore/QCryptographicHash>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QMessageAuthenticationCode>
#include <QtCore/QRandomGenerator>
#include <QtCore/QState>
#include <QtCore/QTimer>
#include <QtCore/QUrl>
#include <QtCore/QUrlQuery>
#include <QtGui/QDesktopServices>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QTcpServer>

#include <iostream>
#include <sstream>
#include <string>

namespace av
{
namespace auth
{

const QString SCOPE                         = QStringLiteral("openid");
const QString CODE_CHALLENGE_METHOD         = QStringLiteral("S256");
const QString RESPONSE_TYPE                 = QStringLiteral("code");
const QString GRANT_TYPE_AUTHORIZATION_CODE = QStringLiteral("authorization_code");
const QString GRANT_TYPE_REFRESH_TOKEN      = QStringLiteral("refresh_token");
const int     CODE_VERIFIER_BYTE_COUNT      = 32;

Authenticator::Authenticator(std::unique_ptr<AuthenticatorConfig> config, QObject *parent) :
    QObject(parent),
    m_request_state(generateStateString(this)),
    m_open_browser_callback(QDesktopServices::openUrl),
    m_config(std::move(config))
{
    qRegisterMetaType<State>();
    qRegisterMetaType<ErrorCode>();
    if (!m_config->m_keycloak_url.isEmpty())
    {
        m_nam = new QNetworkAccessManager(this);
    }
    m_login_timer.setInterval(std::chrono::seconds {m_config->m_login_timeout_s});
    m_login_timer.setSingleShot(true);
    connect(&m_login_timer, &QTimer::timeout, this, [this] {
        handleError(ErrorCode::TimedOut);
        if (m_redirect_app_server)
        {
            m_redirect_app_server->close();
        }
    });
    m_refresh_timer.setSingleShot(true);
    connect(&m_refresh_timer, &QTimer::timeout, this, &Authenticator::refreshToken);
}

//--------------------------------------------------------------------------------------------------

Authenticator::Authenticator(QNetworkAccessManager *nam, std::function<bool (QUrl)> open_browser_callback, std::unique_ptr<AuthenticatorConfig> config) :
    Authenticator(config?std::move(config):std::make_unique<AuthenticatorConfig>(AuthenticatorConfig::UNIT_TEST_CONSTRUCTOR))

{
    AVASSERT(nam);
    m_nam = nam;
    m_open_browser_callback = open_browser_callback;

}

//--------------------------------------------------------------------------------------------------

Authenticator::~Authenticator() = default;

//--------------------------------------------------------------------------------------------------

const IdToken &Authenticator::getIdToken() const
{
    return m_id_token;
}

//--------------------------------------------------------------------------------------------------

const QString &Authenticator::getAccessToken() const
{
    return m_access_token;
}

//--------------------------------------------------------------------------------------------------

Authenticator::State Authenticator::getState() const
{
    return m_state;
}

//--------------------------------------------------------------------------------------------------

void Authenticator::reset_internal_state()
{
    m_state = State::Uninitialized;
    m_errorcode = ErrorCode::NoError;

    m_server_port = -1;
    m_code_verifier = generateCodeVerifier();
    m_code.clear();

    if (m_login_timer.isActive())
    {
        AVLogWarning << AV_FUNC_PREAMBLE << "Login in process. Starting new one.";
        m_login_timer.stop();
    }
    m_refresh_timer.stop();
}

//--------------------------------------------------------------------------------------------------

void Authenticator::authenticate()
{
    AVLOG_ENTER_METHOD();

    reset_internal_state();

    if (m_reply)
    {
        m_reply->deleteLater();
    }

    // 1. Retrieve Service information
    requestServices();

}
//--------------------------------------------------------------------------------------------------

void Authenticator::logout()
{
    AVLOG_ENTER_METHOD();

    QNetworkRequest logout_request(QUrl(m_service_metadata.endSessionEndpoint));
    logout_request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QUrlQuery logout_query;
    logout_query.addQueryItem("client_id", m_config->m_client_id);
    logout_query.addQueryItem("refresh_token", m_refresh_token);
    QNetworkReply* logout_reply= m_nam->post(logout_request, logout_query.query().toLocal8Bit());
    if (m_config->m_ignore_ssl_errors)
    {
        logout_reply->ignoreSslErrors();
    }
    QObject::connect(logout_reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), logout_reply, [reply= logout_reply] (QNetworkReply::NetworkError error) {

        AVLogError << AV_FUNC_PREAMBLE << "Network error for "<< reply->request().url().toString() << ": " << error << ": " << reply->errorString();
    });

    QObject::connect(logout_reply, &QNetworkReply::finished, logout_reply, [ reply = logout_reply]
    {
        QByteArray reply_data(reply->readAll());
        reply->deleteLater();
        if (reply->error() !=  QNetworkReply::NetworkError::NoError)
        {
            AVLogError << AV_FUNC_PREAMBLE << "Content: " << reply_data;
            return;
        }

        AVLogInfo << AV_FUNC_PREAMBLE << "Logout successful " << reply_data;
    });


    reset_internal_state();
    emit stateChanged(m_state = State::Uninitialized);
}

//--------------------------------------------------------------------------------------------------

void Authenticator::requestServices()
{
    AVLOG_ENTER_METHOD();

    emit stateChanged(m_state = State::ReadingServices);

    AVLogInfo << "NAM: " << (qintptr)m_nam;
    AVASSERT(m_nam);

    m_reply = m_nam->get(QNetworkRequest(QUrl(m_config->m_keycloak_url + "/auth/realms/" + m_config->m_realm + "/.well-known/uma2-configuration")));
    if (m_config->m_ignore_ssl_errors)
    {
        m_reply->ignoreSslErrors();
    }

    QTimer::singleShot(std::chrono::seconds {m_config->m_http_timeout_s}, m_reply, &QNetworkReply::abort);

    QObject::connect(m_reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), this, [this, reply= m_reply] (QNetworkReply::NetworkError error) {

        AVLogError << AV_FUNC_PREAMBLE << "Network error for "<< reply->request().url().toString()<<": " << error << ": " << reply->errorString();
        handleError(ErrorCode::CouldNotReadServiceInfo);
    });

    connect(m_reply, &QNetworkReply::finished, this, [this, reply = m_reply]
    {
        QByteArray replyData(reply->readAll());
        reply->deleteLater();
        if (reply->error() !=  QNetworkReply::NetworkError::NoError)
        {
            AVLogError << AV_FUNC_PREAMBLE << "Content: " << replyData;
            return;
        }
        QJsonDocument jsonDoc = QJsonDocument::fromJson(replyData);

        if (jsonDoc.isNull())
        {
            AVLogError << AV_FUNC_PREAMBLE << "Could not parse JSON: " << replyData;
            handleError(ErrorCode::CouldNotReadServiceInfo);
            return;
        }

        m_service_metadata = fromJson<ServicesMetaData>(jsonDoc.object());


        AVLogDebug << AV_FUNC_PREAMBLE << "service metadata:\n" << QJsonDocument(toJson<ServicesMetaData>(m_service_metadata)).toJson(QJsonDocument::Indented);

        emit stateChanged(m_state = State::ServicesRead);

        // 2. Open external web browser for user to enter credentials
        initiateLogin();
    });
}

//--------------------------------------------------------------------------------------------------

void Authenticator::initiateLogin()
{
    AVLOG_ENTER_METHOD();

    if (!m_redirect_app_server)
    {
        m_redirect_app_server.reset(new QTcpServer(this));
        // 3. Wait for the redirect data from the Redirector app
        connect(m_redirect_app_server.get(), &QTcpServer::newConnection, this, &Authenticator::onNewConnection);
    }
    // Listen on all interfaces and auto assign port
    // We have to start listening here already because we need to pass the
    // server port as part of the redirectUri
    if (!m_redirect_app_server->isListening())
    {
        if (!m_redirect_app_server->listen(QHostAddress::LocalHost))
        {
            handleError(ErrorCode::ServerError);
            return;
        }
    }
    m_server_port = m_redirect_app_server->serverPort();

    AVLogDebug << AV_FUNC_PREAMBLE << "Authentication Server using port: " << m_server_port << '\n';
    AVLogDebug << AV_FUNC_PREAMBLE << "Code Verifier [length=" << m_code_verifier.size() << "]: " << m_code_verifier;

    QString     url;
    QTextStream oss(&url);
    oss << m_service_metadata.authorizationEndpoint << '?' << "client_id=" << m_config->m_client_id
        << "&redirect_uri=" << buildRedirectUri()
        << "&scope=" << SCOPE
        << "&response_type=" << RESPONSE_TYPE
        << "&state=" << m_request_state
           // The following parameters are for the PKCE process - step 1
           // see RFC 7636
        << "&code_challenge=" << createCodeChallenge(m_code_verifier)
        << "&code_challenge_method=" << CODE_CHALLENGE_METHOD;

    AVLogInfo << "Opening URL in default browser: " << url << '\n';
    m_login_timer.start();
    if (!m_open_browser_callback(QUrl(url)))
    {
        m_login_timer.stop();
        handleError(ErrorCode::InvalidBrowser);
    }
}

//--------------------------------------------------------------------------------------------------

void Authenticator::onNewConnection()
{
    AVLOG_ENTER_METHOD();
    while (m_redirect_app_server->hasPendingConnections())
    {
        QTcpSocket *socket = m_redirect_app_server->nextPendingConnection();
        AVLogDebug << AV_FUNC_PREAMBLE << "Browser connected.";
        auto readyReadHandler = [socket, this]
        {

            QByteArray data = socket->readLine();

            AVLogDebug << AV_FUNC_PREAMBLE << "Get Request: " << data;

            const QByteArray prefix = "GET /?";
            const QByteArray postfix = " HTTP/1.1\r\n";
            if (!data.startsWith(prefix) || !data.endsWith(postfix))
            {
                AVLogError << AV_FUNC_PREAMBLE << "Invalid request: " << data;
                const QByteArray html = QByteArrayLiteral("<html><head><title>Error</title></head><body>"
                "Invalid request."
                "</body></html>");
                const QByteArray htmlSize = QString::number(html.size()).toUtf8();
                const QByteArray replyMessage = QByteArrayLiteral("HTTP/1.0 403 Forbidden \r\n"
                                                                  "Content-Type: text/html; "
                                                                  "charset=\"utf-8\"\r\n"
                                                                  "Connection: close\r\n"
                                                                  "Content-Length: ") + htmlSize +
                        QByteArrayLiteral("\r\n\r\n") +
                        html;
                socket->write(replyMessage);
            }
            else
            {
                QString query_str = data.mid(prefix.length(), data.length() - prefix.length() - postfix.length());
                AVLogDebug << AV_FUNC_INFO << "Query string: " << query_str;
                QUrlQuery query(query_str);
                m_code = query.queryItemValue("code");
                AVLogInfo << "Authenticator::waitForRedirectData(): Received code: [" << m_code << "]";

                const QByteArray html = QByteArrayLiteral("<html><head><title>Success</title></head><body onLoad=\"window.close()\">") +
                                m_config->m_login_body.toUtf8() +
                                QByteArrayLiteral("</body></html>");
                const QByteArray htmlSize = QString::number(html.size()).toUtf8();
                const QByteArray replyMessage = QByteArrayLiteral("HTTP/1.0 200 OK \r\n"
                                                                  "Content-Type: text/html; "
                                                                  "charset=\"utf-8\"\r\n"
                                                                  "Connection: close\r\n"
                                                                  "Content-Length: ") + htmlSize +
                        QByteArrayLiteral("\r\n\r\n") +
                        html;
                socket->write(replyMessage);

            }


            socket->flush();
            socket->disconnectFromHost();
            socket->deleteLater();
            if (!m_code.isEmpty())
            {
                AVLogInfo << AV_FUNC_PREAMBLE << "Got code. Closing HTTP server.";
                m_login_timer.stop();
                m_redirect_app_server->close();

                // 4. Using the code retrieved from the Redirection, request authentication
                //    data returned as a raw string buffer
                requestAuthenticationData();
                return;
            }
        };
        connect(socket, &QIODevice::readyRead, this, readyReadHandler);
        QTimer::singleShot(std::chrono::seconds {m_config->m_http_timeout_s}, socket, [socket] {
            AVLogError << "Timed out or error while waiting for socket\n";
            socket->deleteLater();
        });
        if (!m_login_timer.isActive())
        {
            AVLogError << "Internal error. Timer not active";
        }

        connect(&m_login_timer, &QTimer::timeout, socket, &QObject::deleteLater);
    }
}

//--------------------------------------------------------------------------------------------------

void Authenticator::refreshToken()
{
    AVLOG_ENTER_METHOD();



    requestToken({{"grant_type", GRANT_TYPE_REFRESH_TOKEN},
                  {"client_id", m_config->m_client_id},
                  {"refresh_token", m_refresh_token},
                  // must match previously sent value, see RFC
                  {"redirect_uri", buildRedirectUri()}});


}

//--------------------------------------------------------------------------------------------------

void Authenticator::requestAuthenticationData()
{
    AVLOG_ENTER_METHOD();

    emit stateChanged(m_state = State::RequestingAuthenticationData);
    if (m_code.isEmpty())
    {
        handleError(ErrorCode::InvalidCode);
        return;
    }

    // Access the previously obtained token endpoint, see RFC 6749, chapter 4.1.3. for details


    requestToken({{"grant_type", GRANT_TYPE_AUTHORIZATION_CODE},
                  {"client_id", m_config->m_client_id},
                  // must match previously sent value, see RFC
                  {"redirect_uri", buildRedirectUri()},
                  {"code", m_code},
                  {"scope", SCOPE},
                  // PKCE process - step 2 - send original, unhashed, code_verifier
                  // see RFC 7636
                  {"code_verifier", m_code_verifier},
                  {"state", m_request_state}});
}

//--------------------------------------------------------------------------------------------------

void Authenticator::requestToken(std::vector<std::pair<QString, QString>> query_parameters)
{

    QUrlQuery query;
    for (auto key_value: query_parameters)
    {
        query.addQueryItem(key_value.first, key_value.second);
    }
    QUrl            serviceUrl(m_service_metadata.tokenEndpoint);
    QNetworkRequest request(serviceUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    auto query_string = query.toString(QUrl::FullyEncoded).toUtf8();
    AVLogDebug << AV_FUNC_PREAMBLE << "Query string: " <<  query_string;
    m_reply = m_nam->post(request, query_string);
    if (m_config->m_ignore_ssl_errors)
    {
        m_reply->ignoreSslErrors();
    }
    QTimer::singleShot(std::chrono::seconds {m_config->m_http_timeout_s}, m_reply, &QNetworkReply::abort);

    if (!AVLogger->isLevelSuppressed(AVLog::LOG__DEBUG))
    {
        QObject::connect(m_reply, &QNetworkReply::readyRead, this,
                         []() { AVLogInfo << "Authenticating: NetworkReply::readyRead()"; });
        QObject::connect(m_reply, &QNetworkReply::downloadProgress, this, [](qint64 bytesReceived, qint64 bytesTotal) {
            AVLogInfo << "Authenticating: NetworkReply::downloadProgress(" << bytesReceived << ", " << bytesTotal << ")";
        });
        QObject::connect(m_reply, &QNetworkReply::uploadProgress, this, [](qint64 bytesSent, qint64 bytesTotal) {
            AVLogInfo << "Authenticating: NetworkReply::uploadProgress(" << bytesSent << ", " << bytesTotal << ")";
        });

        QObject::connect(m_reply, &QNetworkReply::encrypted, this,
                         []() { AVLogInfo << "Authenticating: NetworkReply::encrypted()"; });
        QObject::connect(m_reply, &QNetworkReply::metaDataChanged, this,
                         []() { AVLogInfo << "Authenticating: NetworkReply::metaDataChanged()"; });
        QObject::connect(m_reply, &QNetworkReply::redirected, this, [](QUrl const &url) {
            AVLogInfo << "Authenticating: NetworkReply::redirected(" << url.toDisplayString() << ")";
        });
    }

    QObject::connect(m_reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), this,
                     [this, reply = m_reply](QNetworkReply::NetworkError error) {
                         AVLogError << AV_FUNC_PREAMBLE << "Network error: " << error << ": " << reply->errorString();
                         handleError(ErrorCode::ServerError);
                     });

    QObject::connect(m_reply, &QNetworkReply::finished, this, [this, reply = m_reply]() {
        QByteArray replyData(reply->readAll());
        m_reply->deleteLater();
        if (reply->error() !=  QNetworkReply::NetworkError::NoError)
        {
            AVLogError << AV_FUNC_PREAMBLE << "Error Content: " << replyData;
            return;
        }

        QJsonDocument json = QJsonDocument::fromJson(replyData);
        if (json.isNull())
        {
            AVLogError << AV_FUNC_PREAMBLE << "Could not parse JSON: " << replyData;
            handleError(ErrorCode::InvalidCode);
            return;
        }

        AVLogDebug << AV_FUNC_PREAMBLE << "Authenticating: Received data:\n" << json.toJson(QJsonDocument::Indented);

        auto token_data = fromJson<AuthenticationResponse>(json.object());
        handleAuthenticationResponse(token_data);
    });
}

void Authenticator::handleAuthenticationResponse(AuthenticationResponse auth_response)
{
    AVLOG_ENTER_METHOD();

    // parse ID token authentication response
    boost::optional<RawTokenData> raw_id_token = RawTokenData::parse(auth_response.id_token);
    if (!raw_id_token)
    {
        AVLogError << "ID Token malformed.";
        handleError(ErrorCode::InvalidIdToken);
        return;
    }

    if (!AVLogger->isLevelSuppressed(AVLog::LOG__DEBUG))
    {
        QJsonDocument header_doc  = QJsonDocument::fromJson(raw_id_token->m_decoded_header);
        QJsonDocument payload_doc = QJsonDocument::fromJson(raw_id_token->m_decoded_payload);
        AVLogInfo << "Decoded ID token header: " << header_doc.toJson(QJsonDocument::Indented);
        AVLogInfo << "Decoded ID token payload: " << payload_doc.toJson(QJsonDocument::Indented);
        AVLogInfo << "ID token signature: " << raw_id_token->m_signature;

        // informational only: print info about access token
        boost::optional<RawTokenData> raw_access_token = RawTokenData::parse(auth_response.access_token);
        if (raw_access_token)
        {
            header_doc  = QJsonDocument::fromJson(raw_access_token->m_decoded_header);
            payload_doc = QJsonDocument::fromJson(raw_access_token->m_decoded_payload);
            AVLogInfo << "Decoded access token header: " << header_doc.toJson(QJsonDocument::Indented);
            AVLogInfo << "Decoded access token payload: " << payload_doc.toJson(QJsonDocument::Indented);
            AVLogInfo << "access token signature: " << raw_access_token->m_signature;
        }

        // informational only: print info about refresh token
        boost::optional<RawTokenData> raw_refresh_token = RawTokenData::parse(auth_response.refresh_token);
        if (raw_refresh_token)
        {
            header_doc  = QJsonDocument::fromJson(raw_refresh_token->m_decoded_header);
            payload_doc = QJsonDocument::fromJson(raw_refresh_token->m_decoded_payload);
            AVLogInfo << "Decoded refresh token header: " << header_doc.toJson(QJsonDocument::Indented);
            AVLogInfo << "Decoded refresh token payload: " << payload_doc.toJson(QJsonDocument::Indented);
            AVLogInfo << "refresh token signature: " << raw_refresh_token->m_signature;
        }
    }

    // 5. Validate the authentication data (TODO)
    //auto validateOutcome = validateToken(*raw_id_token);
    m_access_token                = auth_response.access_token;
    m_refresh_token               = auth_response.refresh_token;

    TokenPayload id_token_payload = fromJson<TokenPayload>(QJsonDocument::fromJson(raw_id_token->m_decoded_payload).object());
    m_id_token.preferred_username = id_token_payload.preferred_username;
    m_id_token.given_name         = id_token_payload.given_name;
    m_id_token.family_name        = id_token_payload.family_name;
    m_id_token.email              = id_token_payload.email;
    auto it = id_token_payload.roles_map.find(m_config->m_client_id);
    if (it != id_token_payload.roles_map.end())
    {
        m_id_token.roles          = *it;
    }
    else
    {
        AVLogError << AV_FUNC_PREAMBLE << "Could not find roles for: " << m_config->m_client_id;
        handleError(ErrorCode::NoRolesForClientId);
        return;
    }

    // 6. Everything succeeded! can continue with the application
    emit stateChanged(m_state = State::Authenticated);
    emit authenticated();

    if (m_config->m_refresh_headstart_s < 0)
    {
        AVLogDebug << AV_FUNC_PREAMBLE << "Refresh is disabled.";
        return;
    }
    std::chrono::seconds  refresh_interval {id_token_payload.exp - id_token_payload.iat - m_config->m_refresh_headstart_s};
    if (refresh_interval.count() < 0 )
    {
        AVLogError << AV_FUNC_INFO << "Invalid refresh interval computed: " << refresh_interval.count() << " second";
        handleError(ErrorCode::InvalidRefreshTimeOut);
        return;
    }
    m_refresh_timer.start(refresh_interval);
    AVLogDebug << AV_FUNC_PREAMBLE << "Will refresh tokens in " << refresh_interval.count() << " seconds";



}

//--------------------------------------------------------------------------------------------------

void Authenticator::validateToken(const RawTokenData& token)
{
    AVLOG_ENTER_METHOD();

    Q_UNUSED(token);
    AVLogError << "Authenticator::validateToken: not yet implemented!";
    //return Outcome<>(ErrorCode::NoError, nullptr);
    return;
#if false
    emit stateChanged(m_state = State::ValidatingAuthenticationData);

    // ----------------------------------------------------------------------
    // Retrieve Certificates from the ServiceMetaData::jwksUri and check that
    // the public key ID there matches the Token Header key ID
    // ----------------------------------------------------------------------
    auto key_result = retrieveKeyData(QUrl(m_service_metadata.jwksUri));
    if (std::get<0>(key_result) != QNetworkReply::NoError)
    {
        auto      networkError = std::get<0>(key_result);
        QMetaEnum metaEnum     = QMetaEnum::fromType<QNetworkReply::NetworkError>();
        AVLogError << "Verification: Network Error: [" << metaEnum.valueToKey(networkError) << "]";
        return Outcome<>(ErrorCode::ServerError, nullptr);
    }
    KeyData const &key_data = std::get<1>(key_result);

    AVLogDebug << AV_FUNC_PREAMBLE << "got key data\n" << QJsonDocument(toJson(key_data)).toJson(QJsonDocument::Indented);

    TokenHeader token_header = token.getHeader();

    if (token_header.key_id != key_data.kid)
    {
        AVLogError << "Header KeyID [" << token_header.key_id << "] and Public KeyID [" << key_data.kid
                   << "] do not match";
        return Outcome<>(ErrorCode::VerificationFailed, nullptr);
    }
    AVLogInfo << "KeyData.kid and TokenHeader.kid match: [" << token_header.key_id << ']';

    // Verify the signature is correct by generating a hash from header+"."+payload
    // using the KeyData.n field as a public key, and then comparing it to the
    // signature part of the raw accessToken.
    bool alg_ok      = false;
    auto hash_method = stringToAlgo(token_header.algorithm, &alg_ok);
    if (!alg_ok)
    {
        AVLogError << "Invalid/unknown algorithm specification [" << token_header.algorithm
                   << "] in access token header";
        return Outcome<>(ErrorCode::InvalidIdToken, nullptr);
    }

    QByteArray pre_hash = (token.m_raw_header + '.' + token.m_raw_payload).toUtf8();

    auto test_hash =
        QMessageAuthenticationCode::hash(QByteArray(pre_hash), key_data.n, QCryptographicHash::Sha256).toHex();
    AVLogInfo << "Verification Hash[Sha256] = " << test_hash;
    test_hash = QMessageAuthenticationCode::hash(QByteArray(pre_hash), key_data.n, QCryptographicHash::Sha3_256).toHex();
    AVLogInfo << "Verification Hash[Sha3_256] = " << test_hash;

    test_hash = QMessageAuthenticationCode::hash(QByteArray(pre_hash), key_data.n, QCryptographicHash::Sha384).toHex();
    AVLogInfo << "Verification Hash[Sha384] = " << test_hash;
    test_hash = QMessageAuthenticationCode::hash(QByteArray(pre_hash), key_data.n, QCryptographicHash::Sha3_384).toHex();
    AVLogInfo << "Verification Hash[Sha3_384] = " << test_hash;

    test_hash = QMessageAuthenticationCode::hash(QByteArray(pre_hash), key_data.n, QCryptographicHash::Sha512).toHex();
    AVLogInfo << "Verification Hash[Sha512] = " << test_hash;
    test_hash = QMessageAuthenticationCode::hash(QByteArray(pre_hash), key_data.n, QCryptographicHash::Sha3_512).toHex();
    AVLogInfo << "Verification Hash[Sha3_512] = " << test_hash;

    auto verification_hash = QMessageAuthenticationCode::hash(QByteArray(pre_hash), key_data.n, hash_method).toHex();
    AVLogInfo << "Verification hash: " << verification_hash;

    if (verification_hash != token.m_signature)
    {
        AVLogError << "Access token signature did not match hash.";
        return Outcome<>(ErrorCode::InvalidIdToken, nullptr);
    }

    AVLogInfo << "Access token Signature and Verification hash match. Token is Valid!";

    return Outcome<>(ErrorCode::NoError, nullptr);
}

//--------------------------------------------------------------------------------------------------

QCryptographicHash::Algorithm Authenticator::stringToAlgo(const QString &sAlg, bool *pOK)
{
    QCryptographicHash::Algorithm alg = QCryptographicHash::Sha256;

    bool ok = true;
    if (sAlg == "RS256")
    {
        alg = QCryptographicHash::Algorithm::Sha3_256;
    }
    else
    {
        ok = false;
    }

    if (pOK)
    {
        *pOK = ok;
    }
    return alg;
#endif
}
//--------------------------------------------------------------------------------------------------

void Authenticator::handleError(Authenticator::ErrorCode error_code)
{
    m_errorcode = error_code;
    emit stateChanged(m_state = State::Invalid);
    emit error(error_code);
}

//--------------------------------------------------------------------------------------------------

QString Authenticator::buildRedirectUri() const
{
    return "http://127.0.0.1:" + QString::number(m_server_port);
}

//--------------------------------------------------------------------------------------------------

QString Authenticator::generateStateString(void *ptr)
{
    QByteArray id = ::AVToString(ptr).toUtf8();
    QByteArray hash = QCryptographicHash::hash(id, QCryptographicHash::Sha256);
    return hash.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
}

//--------------------------------------------------------------------------------------------------

QString Authenticator::generateCodeVerifier()
{
    uint32_t challenge[CODE_VERIFIER_BYTE_COUNT / sizeof (uint32_t)];
    static_assert (CODE_VERIFIER_BYTE_COUNT % sizeof (uint32_t) == 0 , "CODE_VERIFIER_BYTE_COUNT must be divisible by 4");

    QRandomGenerator::global()->fillRange(challenge);

    char* begin = reinterpret_cast<char*>(std::begin(challenge));
    char* end = reinterpret_cast<char*>(std::end(challenge));

    return QByteArray::fromRawData(begin, (end-begin)).toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
}

//--------------------------------------------------------------------------------------------------

QString Authenticator::createCodeChallenge(const QString &codeVerifier)
{
    auto hash = QCryptographicHash::hash(codeVerifier.toUtf8(), QCryptographicHash::Sha256);

    return hash.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
}


}  // namespace auth
}  // namespace av
