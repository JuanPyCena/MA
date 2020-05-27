#include "avjsonutils.h"

#include <QtCore/QJsonArray>
#include <QtCore/QJsonValue>

#include <sstream>

namespace av
{
namespace auth
{
// Function template specializations for the types supported by QJsonValue
template <>
bool convert(QJsonValue const& v, bool&& defaultValue)
{
    return v.isBool() ? v.toBool() : defaultValue;
}

template <>
int convert(QJsonValue const& v, int&& defaultValue)
{
    // Note: QJsonValue doesn't support int directly, stores it as double
    return v.isDouble() ? static_cast<int>(v.toDouble()) : defaultValue;
}

template <>
qint64 convert(QJsonValue const& v, qint64&& defaultValue)
{
    // Note: QJsonValue doesn't support int directly, stores it as double
    return v.isDouble() ? static_cast<qint64>(v.toDouble()) : defaultValue;
}

template <>
double convert(QJsonValue const& v, double&& defaultValue)
{
    return v.isDouble() ? v.toDouble() : defaultValue;
}

template <>
QString convert(QJsonValue const& v, QString&& defaultValue)
{
    return v.isString() ? v.toString() : defaultValue;
}

template <>
QByteArray convert(QJsonValue const& v, QByteArray&& defaultValue)
{
    return v.isString() ? QByteArray(v.toString().toUtf8()) : defaultValue;
}

template <>
StringVector convert(QJsonValue const& v, StringVector&& defaultValue)
{
    StringVector result;

    if (v.isArray())
    {
        auto values = v.toArray();
        result.reserve(values.size());
        for (auto const& value : values)
        {
            if (value.isString())
            {
                result.push_back(value.toString());
            }
        }
    }
    else if (v.isString())
    {
        result.push_back(v.toString());
    }
    else
    {
        result = std::move(defaultValue);
    }
    return result;
}

template <>
QJsonValue convert(bool const& v, QJsonValue&& j)
{
    (void)j;
    return QJsonValue(v);
}

template <>
QJsonValue convert(int const& v, QJsonValue&& j)
{
    (void)j;
    return QJsonValue(v);
}

template <>
QJsonValue convert(qint64 const& v, QJsonValue&& j)
{
    (void)j;
    return QJsonValue(v);
}

template <>
QJsonValue convert(double const& v, QJsonValue&& j)
{
    (void)j;
    return QJsonValue(v);
}

template <>
QJsonValue convert(QString const& v, QJsonValue&& j)
{
    (void)j;
    return QJsonValue(v);
}

template <>
QJsonValue convert(QByteArray const& v, QJsonValue&& j)
{
    (void)j;
    return QJsonValue(QString(v));
}

template <>
QJsonValue convert(StringVector const& v, QJsonValue&& j)
{
    (void)j;
    QJsonArray array;
    for (auto const& s : v)
    {
        array.push_back(QJsonValue(s));
    }

    return array;
}

}  // namespace auth
}  // namespace av
