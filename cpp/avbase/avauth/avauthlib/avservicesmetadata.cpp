#include "avservicesmetadata.h"

#include "avlog.h"

#include <QtCore/QEventLoop>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonValue>
#include <QtCore/QMetaEnum>
#include <QtNetwork/QNetworkReply>

#include <iostream>
#include <sstream>

namespace av
{
namespace auth
{
template <>
ServicesMetaData fromJson(QJsonObject const& jsonObj)
{
    ServicesMetaData smd;

    setMember("issuer", &ServicesMetaData::issuer, jsonObj, smd);
    setMember("authorization_endpoint", &ServicesMetaData::authorizationEndpoint, jsonObj, smd);
    setMember("token_endpoint", &ServicesMetaData::tokenEndpoint, jsonObj, smd);
    setMember("token_introspection_endpoint", &ServicesMetaData::tokenIntrospectionEndpoint, jsonObj, smd);
    setMember("end_session_endpoint", &ServicesMetaData::endSessionEndpoint, jsonObj, smd);
    setMember("jwks_uri", &ServicesMetaData::jwksUri, jsonObj, smd);
    setMember("grant_types_supported", &ServicesMetaData::grantTypesSupported, jsonObj, smd);
    setMember("response_types_supported", &ServicesMetaData::responseTypesSupported, jsonObj, smd);
    setMember("response_modes_supported", &ServicesMetaData::responseModesSupported, jsonObj, smd);
    setMember("registration_endpoint", &ServicesMetaData::registrationEndpoint, jsonObj, smd);
    setMember("token_endpoint_auth_methods_supported", &ServicesMetaData::tokenEndpointAuthMethodsSupported, jsonObj,
              smd);
    setMember("token_endpoint_auth_signing_alg_values_supported",
              &ServicesMetaData::tokenEndpointAuthSigningAlgValuesSupported, jsonObj, smd);
    setMember("scopes_supported", &ServicesMetaData::scopesSupported, jsonObj, smd);
    setMember("resource_registration_endpoint", &ServicesMetaData::resourceRegistrationEndpoint, jsonObj, smd);
    setMember("permission_endpoint", &ServicesMetaData::permissionEndpoint, jsonObj, smd);
    setMember("policy_endpoint", &ServicesMetaData::policyEndpoint, jsonObj, smd);
    setMember("introspection_endpoint", &ServicesMetaData::introspectionEndpoint, jsonObj, smd);

    return smd;
}

template <>
QJsonObject toJson(ServicesMetaData const& smd)
{
    QJsonObject jsonObj;

    jsonObj.insert("issuer", convert(smd.issuer, QJsonValue(QString())));
    jsonObj.insert("authorization_endpoint", convert(smd.authorizationEndpoint, QJsonValue(QString())));
    jsonObj.insert("token_endpoint", convert(smd.tokenEndpoint, QJsonValue(QString())));
    jsonObj.insert("token_introspection_endpoint", convert(smd.tokenIntrospectionEndpoint, QJsonValue(QString())));
    jsonObj.insert("end_session_endpoint", convert(smd.endSessionEndpoint, QJsonValue(QString())));
    jsonObj.insert("jwks_uri", convert(smd.jwksUri, QJsonValue(QString())));
    jsonObj.insert("grant_types_supported", convert(smd.grantTypesSupported, QJsonValue(QJsonArray())));
    jsonObj.insert("response_types_supported", convert(smd.responseTypesSupported, QJsonValue(QJsonArray())));
    jsonObj.insert("response_modes_supported", convert(smd.responseModesSupported, QJsonValue(QJsonArray())));
    jsonObj.insert("registration_endpoint", convert(smd.registrationEndpoint, QJsonValue(QString())));
    jsonObj.insert("token_endpoint_auth_methods_supported",
                   convert(smd.tokenEndpointAuthMethodsSupported, QJsonValue(QJsonArray())));
    jsonObj.insert("token_endpoint_auth_signing_alg_values_supported",
                   convert(smd.tokenEndpointAuthSigningAlgValuesSupported, QJsonValue(QJsonArray())));
    jsonObj.insert("scopes_supported", convert(smd.scopesSupported, QJsonValue(QJsonArray())));
    jsonObj.insert("resource_registration_endpoint", convert(smd.resourceRegistrationEndpoint, QJsonValue(QString())));
    jsonObj.insert("permission_endpoint", convert(smd.permissionEndpoint, QJsonValue(QString())));
    jsonObj.insert("policy_endpoint", convert(smd.policyEndpoint, QJsonValue(QString())));
    jsonObj.insert("introspection_endpoint", convert(smd.introspectionEndpoint, QJsonValue(QString())));
    return jsonObj;
}

}  // namespace auth
}  // namespace av
