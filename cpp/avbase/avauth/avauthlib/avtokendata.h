#pragma once

#include "avauthlib_export.h"
#include <avauthlib/avjsonutils.h>

#include <QByteArray>
#include <QMap>
#include <QSet>

#include <boost/optional.hpp>

#include <functional>
#include <vector>

namespace av
{
namespace auth
{
using StringUSet    = QSet<QString>;
using StringUSetMap = QMap<QString, StringUSet>;
using StringVector  = std::vector<QString>;

//--------------------------------------------------------------------------------------------------
/**
 * @brief The AuthenticationResponse struct encapsulates the result of the authentication endpoint.
 *
 * See https://tools.ietf.org/html/rfc6749.
 */
struct AVAUTHLIB_EXPORT AuthenticationResponse
{
    QString      access_token;
    int          expires_in;
    QString      id_token;
    int          not_before_policy;
    int          refresh_expires_in;
    QString      refresh_token;
    StringVector scope;
    QString      session_state;
    QString      token_type;
};

//--------------------------------------------------------------------------------------------------
struct AVAUTHLIB_EXPORT TokenHeader
{
    QString algorithm; /**< alg */
    QString type;      /**< typ */
    QString key_id;    /**< kid */
};

//--------------------------------------------------------------------------------------------------
/**
 * See
 * - https://tools.ietf.org/html/rfc7519
 * - https://openid.net/specs/openid-connect-core-1_0.html
 *
 * This is the internal structure to handle tokens, also see IdToken for the public subset.
 */
struct AVAUTHLIB_EXPORT TokenPayload
{
    QString    jti;
    qint64     exp;
    QString    nbf;
    qint64     iat;
    QString    iss;
    QString    aud;
    QString    sub;
    QString    typ;
    QString    azp;
    qint64     auth_time;
    QString    session_state;
    QString    acr;
    StringUSetMap   roles_map;
    QString    scope;
    bool       email_verified;
    QString    name;
    QString    preferred_username;
    QString    given_name;
    QString    family_name;
    QString    email;
};

//--------------------------------------------------------------------------------------------------
struct AVAUTHLIB_EXPORT RawTokenData
{
    static boost::optional<RawTokenData> parse(const QString& token_string);

    TokenHeader getHeader() const;

    QString    m_token_string;
    QStringRef m_raw_header;
    QStringRef m_raw_payload;
    QStringRef m_signature;

    QByteArray m_decoded_header;
    QByteArray m_decoded_payload;
};

//--------------------------------------------------------------------------------------------------

template <>
AuthenticationResponse AVAUTHLIB_EXPORT fromJson(QJsonObject const& jsonObj);
template <>
QJsonObject AVAUTHLIB_EXPORT toJson(AuthenticationResponse const& tokenData);

template <>
TokenHeader AVAUTHLIB_EXPORT fromJson(QJsonObject const& jsonObj);
template <>
QJsonObject AVAUTHLIB_EXPORT toJson(TokenHeader const& tokenData);

template <>
QJsonObject AVAUTHLIB_EXPORT toJson(TokenPayload const& tokenData);
template <>
TokenPayload AVAUTHLIB_EXPORT fromJson(QJsonObject const& jsonObj);

}  // namespace auth
}  // namespace av
