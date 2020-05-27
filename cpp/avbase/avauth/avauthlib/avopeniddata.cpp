#include "avopeniddata.h"

#include "avlog.h"

#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonValue>
#include <QtCore/QMetaEnum>

#include <iostream>
#include <sstream>

namespace av
{
namespace auth
{
template <>
KeyData fromJson(QJsonObject const& jsonObj)
{
    KeyData keyData;
    setMember("kid", &KeyData::kid, jsonObj, keyData);
    setMember("kty", &KeyData::kty, jsonObj, keyData);
    setMember("alg", &KeyData::alg, jsonObj, keyData);
    setMember("use", &KeyData::use, jsonObj, keyData);
    setMember("n", &KeyData::n, jsonObj, keyData);
    setMember("e", &KeyData::e, jsonObj, keyData);

    return keyData;
}

template <>
QJsonObject toJson(KeyData const& keyData)
{
    QJsonObject jsonObj;

    jsonObj.insert("kid", convert(keyData.kid, QJsonValue(QString())));
    jsonObj.insert("kty", convert(keyData.kty, QJsonValue(QString())));
    jsonObj.insert("alg", convert(keyData.alg, QJsonValue(QString())));
    jsonObj.insert("use", convert(keyData.use, QJsonValue(QString())));
    jsonObj.insert("n", convert(keyData.n, QJsonValue(QString())));
    jsonObj.insert("e", convert(keyData.e, QJsonValue(QString())));

    return jsonObj;
}

std::tuple<QNetworkReply::NetworkError, KeyData> retrieveKeyData(QUrl const& url)
{
    std::tuple<QNetworkReply::NetworkError, QByteArray> urlReaderOutcome {QNetworkReply::OperationNotImplementedError, {} }; // = AVUrlReader().read(url);
    QNetworkReply::NetworkError networkStatus    = std::get<0>(urlReaderOutcome);
    if (networkStatus != QNetworkReply::NoError)
    {
        QMetaEnum metaEnum = QMetaEnum::fromType<QNetworkReply::NetworkError>();
        AVLogInfo << "Error while reading Certification Key Data: " << metaEnum.valueToKey(networkStatus)
                  << " from: " << url.toDisplayString();
        return std::make_tuple(networkStatus, KeyData{});
    }

    auto jsonDoc = QJsonDocument::fromJson(std::get<1>(urlReaderOutcome));

    auto jsonObj = jsonDoc.object();
    auto iter    = jsonObj.find("keys");

    if (iter == jsonObj.end())
    {
        // fix this - see SWE-6816
        AVLogInfo << "Invalid Certification Key Data received";
        return std::make_tuple(QNetworkReply::NoError, KeyData{});
    }
    auto keys = *iter;

    // TODO: don't rely on first key, or at laest log meaningful error if expectation is not met
    // see SWE-6820

    KeyData keyData;
    if (keys.isArray())
    {
        QJsonArray keysArray = keys.toArray();
        if (!keysArray.empty())
        {
            QJsonValue item = keysArray.first();
            if (item.isObject())
            {
                keyData = fromJson<KeyData>(item.toObject());
            }
            // else log error
        }
        // else log error
    }
    else
    {
        keyData = fromJson<KeyData>(keys.toObject());
    }
    return std::make_tuple(QNetworkReply::NoError, keyData);
}

}  // namespace auth
}  // namespace av
