#pragma once

#include "avauthlib_export.h"
#include <avjsonutils.h>

#include <QtCore/QByteArray>
#include <QtCore/QJsonObject>
#include <QtCore/QUrl>
#include <QtNetwork/QNetworkReply>

#include <functional>
#include <string>
#include <vector>

namespace av
{
namespace auth
{
using StringVector = std::vector<QString>;

//! See https://tools.ietf.org/html/rfc7517
struct AVAUTHLIB_EXPORT KeyData
{
    QString    kid;
    QString    kty;
    QString    alg;
    QString    use;
    QByteArray n;
    QString    e;
};  // struct KeyData

extern std::tuple<QNetworkReply::NetworkError, KeyData> AVAUTHLIB_EXPORT retrieveKeyData(QUrl const& url);

template <>
KeyData AVAUTHLIB_EXPORT fromJson(QJsonObject const& jsonObj);
template <>
QJsonObject AVAUTHLIB_EXPORT toJson(KeyData const& keyData);

}  // namespace auth
}  // namespace av
