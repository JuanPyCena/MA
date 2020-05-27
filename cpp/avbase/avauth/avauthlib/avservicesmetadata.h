#pragma once

#include "avauthlib_export.h"
#include <avjsonutils.h>

#include <string>
#include <tuple>
#include <vector>

namespace av
{
namespace auth
{
using StringVector = std::vector<QString>;

/**
 * @brief The ServicesMetaData struct encapsulates the reply to an authorization service request,
 *        see https://www.keycloak.org/docs/6.0/authorization_services/#_service_authorization_api
 *        or https://tools.ietf.org/html/rfc8414.
 */
struct AVAUTHLIB_EXPORT ServicesMetaData
{
    QString      issuer;
    QString      authorizationEndpoint;
    QString      tokenEndpoint; /**< Used to exchange token from code */
    QString      tokenIntrospectionEndpoint;
    QString      endSessionEndpoint;
    QString      jwksUri; /**< Used for validating token signature. */
    StringVector grantTypesSupported;
    StringVector responseTypesSupported;
    StringVector responseModesSupported;
    QString      registrationEndpoint;
    StringVector tokenEndpointAuthMethodsSupported;
    StringVector tokenEndpointAuthSigningAlgValuesSupported;
    StringVector scopesSupported;
    QString      resourceRegistrationEndpoint;
    QString      permissionEndpoint;
    QString      policyEndpoint;
    QString      introspectionEndpoint;

};  // struct ServicesMetaData

template <>
ServicesMetaData AVAUTHLIB_EXPORT fromJson(QJsonObject const& jsonObj);
template <>
QJsonObject AVAUTHLIB_EXPORT toJson(ServicesMetaData const& smd);

}  // namespace auth
}  // namespace av
