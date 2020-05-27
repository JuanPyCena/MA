#pragma once

#include "avauthlib_export.h"
#include <avconvert.h>

#include <QtCore/QJsonObject>

#include <string>
#include <vector>

class QByteArray;
class QJsonObject;
class QJsonValue;
class QString;

namespace av
{
namespace auth
{
using StringVector = std::vector<QString>;

/**
 * @name Convert
 * @brief Explicit template function instantiation declarations for types
 *        supported by QJsonValue
 * @{
 */

template <>
bool AVAUTHLIB_EXPORT convert(QJsonValue const &, bool &&);
template <>
int AVAUTHLIB_EXPORT convert(QJsonValue const &, int &&);
template <>
qint64 AVAUTHLIB_EXPORT convert(QJsonValue const &, qint64 &&);
template <>
double AVAUTHLIB_EXPORT convert(QJsonValue const &, double &&);
template <>
QString AVAUTHLIB_EXPORT convert(QJsonValue const &, QString &&);
template <>
QByteArray AVAUTHLIB_EXPORT convert(QJsonValue const &, QByteArray &&);
template <>
StringVector AVAUTHLIB_EXPORT convert(QJsonValue const &, StringVector &&);

template <>
QJsonValue AVAUTHLIB_EXPORT convert(bool const &, QJsonValue &&);
template <>
QJsonValue AVAUTHLIB_EXPORT convert(int const &, QJsonValue &&);
template <>
QJsonValue AVAUTHLIB_EXPORT convert(qint64 const &, QJsonValue &&);
template <>
QJsonValue AVAUTHLIB_EXPORT convert(double const &, QJsonValue &&);
template <>
QJsonValue AVAUTHLIB_EXPORT convert(QString const &, QJsonValue &&);
template <>
QJsonValue AVAUTHLIB_EXPORT convert(QByteArray const &, QJsonValue &&);
template <>
QJsonValue AVAUTHLIB_EXPORT convert(StringVector const &, QJsonValue &&);

/** @} */

/**
 * @name Json convert
 * @brief
 * @{
 */
template <typename ToType>
ToType fromJson(QJsonObject const &jsonObj);

template <typename FromType>
QJsonObject toJson(FromType const &fromData);

/** @} */

/**
 * @brief Set the value of a structure member based on a QJsonObject key.
 * @tparam StructType The structure type holding the member.
 * @tparam MemberType The type of the member of the structure.
 * @param [in] key The name of the key to loop up in the QJsonObject.
 * @param [in] member Pointer to the member of targt structure type.
 * @param [in] jsonObj The QJson object to extract values from.
 * @param [in] obj The object with the member to be set.
 * @param [in] defaultValue The default value of the member should conversion fail.
 * @return true if successfully set member, false otherwise
 */
template <typename StructType, typename MemberType>
static bool setMember(QString const &key, MemberType StructType::*member, QJsonObject const &jsonObj, StructType &obj,
                      MemberType &&defaultValue = MemberType{})
{
    auto iter = jsonObj.constFind(key);
    if (iter != jsonObj.constEnd())
    {
        QJsonValue ref = *iter;
        obj.*member    = convert<MemberType>(ref, std::forward<MemberType>(defaultValue));
        return true;
    }
    return false;
}

}  // namespace auth
}  // namespace av
