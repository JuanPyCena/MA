#include "avtokendata.h"
#include "avlog.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QString>

#include <iostream>
#include <sstream>

namespace av
{
namespace auth
{
template <>
AuthenticationResponse fromJson(QJsonObject const& jsonObj)
{
    AuthenticationResponse tokenData;
    setMember("access_token", &AuthenticationResponse::access_token, jsonObj, tokenData);
    setMember("refresh_token", &AuthenticationResponse::refresh_token, jsonObj, tokenData);
    setMember("expires_in", &AuthenticationResponse::expires_in, jsonObj, tokenData);
    setMember("refresh_expires_in", &AuthenticationResponse::refresh_expires_in, jsonObj, tokenData);
    setMember("token_type", &AuthenticationResponse::token_type, jsonObj, tokenData);
    setMember("id_token", &AuthenticationResponse::id_token, jsonObj, tokenData);
    setMember("not_before_policy", &AuthenticationResponse::not_before_policy, jsonObj, tokenData);
    setMember("session_state", &AuthenticationResponse::session_state, jsonObj, tokenData);
    setMember("scope", &AuthenticationResponse::scope, jsonObj, tokenData);

    return tokenData;
}

template <>
QJsonObject toJson(AuthenticationResponse const& tokenData)
{
    QJsonObject jsonObj;

    jsonObj.insert("access_token", convert(tokenData.access_token, QJsonValue(QString())));
    jsonObj.insert("refresh_token", convert(tokenData.refresh_token, QJsonValue(QString())));
    jsonObj.insert("expires_in", convert(tokenData.expires_in, QJsonValue(QString())));
    jsonObj.insert("refresh_expires_in", convert(tokenData.refresh_expires_in, QJsonValue(QString())));
    jsonObj.insert("token_type", convert(tokenData.token_type, QJsonValue(QString())));
    jsonObj.insert("id_token", convert(tokenData.id_token, QJsonValue(QString())));
    jsonObj.insert("not_before_policy", convert(tokenData.not_before_policy, QJsonValue(QString())));
    jsonObj.insert("session_state", convert(tokenData.session_state, QJsonValue(QString())));
    jsonObj.insert("scope", convert(tokenData.scope, QJsonValue(QString())));

    return jsonObj;
}

template <>
TokenHeader fromJson(QJsonObject const& jsonObj)
{
    TokenHeader tokenHeader;
    setMember("alg", &TokenHeader::algorithm, jsonObj, tokenHeader);
    setMember("typ", &TokenHeader::type, jsonObj, tokenHeader);
    setMember("kid", &TokenHeader::key_id, jsonObj, tokenHeader);

    return tokenHeader;
}

template <>
QJsonObject toJson(TokenHeader const& tokenHeader)
{
    QJsonObject jsonObj;

    jsonObj.insert("alg", convert(tokenHeader.algorithm, QJsonValue(QString())));
    jsonObj.insert("typ", convert(tokenHeader.type, QJsonValue(QString())));
    jsonObj.insert("kid", convert(tokenHeader.key_id, QJsonValue(QString())));

    return jsonObj;
}

template <>
TokenPayload fromJson(QJsonObject const& jsonObj)
{
    TokenPayload tokenPayload;

    setMember("jti", &TokenPayload::jti, jsonObj, tokenPayload);
    setMember("exp", &TokenPayload::exp, jsonObj, tokenPayload);
    setMember("nbf", &TokenPayload::nbf, jsonObj, tokenPayload);
    setMember("iat", &TokenPayload::iat, jsonObj, tokenPayload);
    setMember("iss", &TokenPayload::iss, jsonObj, tokenPayload);
    setMember("aud", &TokenPayload::aud, jsonObj, tokenPayload);
    setMember("sub", &TokenPayload::sub, jsonObj, tokenPayload);
    setMember("typ", &TokenPayload::typ, jsonObj, tokenPayload);
    setMember("azp", &TokenPayload::azp, jsonObj, tokenPayload);
    setMember("auth_time", &TokenPayload::auth_time, jsonObj, tokenPayload);
    setMember("session_state", &TokenPayload::session_state, jsonObj, tokenPayload);
    setMember("acr", &TokenPayload::acr, jsonObj, tokenPayload);
    setMember("scope", &TokenPayload::scope, jsonObj, tokenPayload);
    setMember("email_verified", &TokenPayload::email_verified, jsonObj, tokenPayload);
    setMember("name", &TokenPayload::name, jsonObj, tokenPayload);
    setMember("preferred_username", &TokenPayload::preferred_username, jsonObj, tokenPayload);
    setMember("given_name", &TokenPayload::given_name, jsonObj, tokenPayload);
    setMember("family_name", &TokenPayload::family_name, jsonObj, tokenPayload);
    setMember("email", &TokenPayload::email, jsonObj, tokenPayload);

    // pass along client ID for role parsing, or store it differently
    // see SWE-6820

    // Have to process roles specially as it is in a nested structure, e.g.:
    // "resource_access": {
    //   "sam": {
    //     "roles": [
    //       "allow_docking",
    //       "allow_undocking"
    //     ]
    //   }
    // },
    tokenPayload.roles_map.clear();
    auto resource_access_iter = jsonObj.constFind("resource_access");
    if (resource_access_iter != jsonObj.constEnd() && resource_access_iter.value().isObject())
    {
        auto rsrcAccObj = resource_access_iter->toObject();  // the resource_access object
        for (auto clients_iter = rsrcAccObj.constBegin(); clients_iter != rsrcAccObj.constEnd(); clients_iter++)
        {
            QString client_id = clients_iter.key();
            auto subObj = clients_iter->toObject();  // e.g. the sam object
            auto roles_iter        = subObj.constFind("roles");

            if (roles_iter != subObj.constEnd() && roles_iter->isArray())
            {
                auto rolesArr = roles_iter->toArray();

                auto& roles = tokenPayload.roles_map[client_id];

                for (auto const& rit : qAsConst(rolesArr))
                {
                    if (rit.isString())
                    {
                        roles.insert(rit.toString());
                    }
                }  // roles element loop
            }      // roles array
        }          // subobject
    }              // resource_access object

    return tokenPayload;
}

template <>
QJsonObject toJson(TokenPayload const& tokenPayload)
{
    QJsonObject jsonObj;

    jsonObj.insert("jti", convert(tokenPayload.jti, QJsonValue(QString())));
    jsonObj.insert("exp", convert(tokenPayload.exp, QJsonValue(QString())));
    jsonObj.insert("nbf", convert(tokenPayload.nbf, QJsonValue(QString())));
    jsonObj.insert("iat", convert(tokenPayload.iat, QJsonValue(QString())));
    jsonObj.insert("iss", convert(tokenPayload.iss, QJsonValue(QString())));
    jsonObj.insert("aud", convert(tokenPayload.aud, QJsonValue(QString())));
    jsonObj.insert("sub", convert(tokenPayload.sub, QJsonValue(QString())));
    jsonObj.insert("typ", convert(tokenPayload.typ, QJsonValue(QString())));
    jsonObj.insert("azp", convert(tokenPayload.azp, QJsonValue(QString())));
    jsonObj.insert("auth_time", convert(tokenPayload.auth_time, QJsonValue(QString())));
    jsonObj.insert("session_state", convert(tokenPayload.session_state, QJsonValue(QString())));
    jsonObj.insert("acr", convert(tokenPayload.acr, QJsonValue(QString())));
    jsonObj.insert("scope", convert(tokenPayload.scope, QJsonValue(QString())));
    jsonObj.insert("email_verified", convert(tokenPayload.email_verified, QJsonValue(QString())));
    jsonObj.insert("name", convert(tokenPayload.name, QJsonValue(QString())));
    jsonObj.insert("preferred_username", convert(tokenPayload.preferred_username, QJsonValue(QString())));
    jsonObj.insert("given_name", convert(tokenPayload.given_name, QJsonValue(QString())));
    jsonObj.insert("family_name", convert(tokenPayload.family_name, QJsonValue(QString())));
    jsonObj.insert("email", convert(tokenPayload.email, QJsonValue(QString())));

    // Construct the resource_access structure (from the bottom up)

    QJsonObject rsrcAccObj;
    for (auto& roles_entry: tokenPayload.roles_map.toStdMap())
    {
        QJsonArray rolesArr;
        for (auto const& rit : roles_entry.second)
        {
            rolesArr.push_back(rit);
        }
        QJsonObject azpObj;
        azpObj.insert("roles", rolesArr);

        rsrcAccObj.insert(roles_entry.first, azpObj);
    }

    jsonObj.insert("resource_access", rsrcAccObj);

    return jsonObj;
}

boost::optional<RawTokenData> RawTokenData::parse(const QString &token_string)
{
    RawTokenData ret;

    ret.m_token_string = token_string;

    const auto split_content = token_string.splitRef('.');
    if (split_content.count() != 3)
    {
        AVLogError << "RawTokenData::parse: token malformed.";
        return boost::none;
    }

    ret.m_raw_header  = split_content[0];
    ret.m_raw_payload = split_content[1];
    ret.m_signature   = split_content[2];

    ret.m_decoded_header  = QByteArray::fromBase64(ret.m_raw_header.toUtf8());
    ret.m_decoded_payload = QByteArray::fromBase64(ret.m_raw_payload.toUtf8());

    return ret;
}

TokenHeader RawTokenData::getHeader() const
{
    QJsonDocument header_doc  = QJsonDocument::fromJson(m_decoded_header);
    return fromJson<TokenHeader> (header_doc.object());
}

}  // namespace auth
}  // namespace av
