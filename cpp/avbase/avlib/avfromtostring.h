///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2010
//
// Module:     AVLIB - AviBit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Christian Muschick, c.muschick@avibit.com
    \author  QT4-PORT: Christian Muschick, c.muschick@avibit.com
    \brief   AviBit-wide standardized toString / fromString methods for some common data types
             You can augment this anytime by implementing additional specialized template functions
             for your data types.
             See docs/misc/specs/tofromstring_design.doc.
*/

#if !defined(AVFROMTOSTRING_H_INCLUDED)
#define AVFROMTOSTRING_H_INCLUDED

#include <QList>
#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QMetaEnum>

#include "avlib_export.h"

#include "avenumhelpers.h"

// forward declarations
class QColor;
class QDateTime;
class QFont;
class QImage;
class QPoint;
class QPolygon;
class QRect;
class QRegExp;
class QRegularExpression;
class QSize;
class QVariant;
class QVector2D;
class QVector3D;
class QBitArray;
class QTransform;

template<class Key, class T>
class QMap;
template<class Key, class T>
class QHash;

template<class T1, class T2>
struct QPair;

template<class T>
class QVector;


// TODO CM refactor avtostring.h/cpp

namespace avfromtostring
{

// If touching any of those, also update SPECIAL_CHAR_REGEXP!
// Also keep this in sync with the python implementation!
AVLIB_EXPORT extern const QChar DELIMITER_START;
AVLIB_EXPORT extern const QChar DELIMITER_END;
AVLIB_EXPORT extern const QChar QUOTE;
AVLIB_EXPORT extern const QChar ESCAPE;
AVLIB_EXPORT extern const QChar SEPARATOR;
AVLIB_EXPORT extern const QChar MAPPING;
AVLIB_EXPORT extern const QChar NEWLINE;
//! Not in SPECIAL_CHAR_LIST as this is only used for QSize(s).
AVLIB_EXPORT extern const QChar CROSS;

//! Strings are quoted if they contain any of the following characters.
//! Not a QRegExp because that's not threadsafe (see Bug#5552)
//! Keep in sync with the constants above!
//! Also keep this in sync with the python implementation!
AVLIB_EXPORT extern const QStringList SPECIAL_CHAR_LIST;

//! The format for parsing and writing QDateTime
AVLIB_EXPORT extern const QString QDATETIME_FORMAT;
AVLIB_EXPORT extern const QString QDATE_FORMAT;
AVLIB_EXPORT extern const QString QTIME_FORMAT;

AVLIB_EXPORT extern const QString EMPTY_STRING;

//! If quoting a string, the quoting character itself must be escaped to allow unquoting later on.
AVLIB_EXPORT extern const QStringList ESCAPE_FROM;
AVLIB_EXPORT extern const QStringList ESCAPE_TO;

//! This method splits the input string along separator tokens, but ignoring separators in nested
//! delimited or escaped regions. The outmost enclosing delimiters are optionally stripped.
//! Note that leading and trailing whitespace is removed from the elements in the result list.
//!
//! Passing an empty string will return an empty list. Passing a single separator token will result
//! in two empty strings in the list.
//!
//! Keep this method implementation in sync with python.
//!
//! \param list      This will contain the split elements of the input string.
//! \param separator The separator along which to split, usually avfromtostring::SEPARATOR
//! \param str       The string to process.
//! \param strip_outmost_delimiters Whether to remove the outmost delimiter pair before processing
//!                                 the input string. If this is true, leading and trailing
//!                                 whitespace is removed from the input string as well.
//! \return True if input was OK, false if not (e.g. mismatched delimiters, or no outmost
//!         delimiters although they should be stripped).
AVLIB_EXPORT bool nestedSplit(QStringList& list, const QString& separator, const QString& str,
                 bool strip_outmost_delimiters);

//! This the most generic AVToString implementation, which can handle all the fundamental data
//! types which can be handled by QTextStream.
template <typename T>
typename std::enable_if<!is_qenum<T>::value ,QString>::type
AVToStringInternal(const T& arg)
{
    static_assert( !std::is_same<typename std::decay<T>::type, QTextStreamFunction>::value, "Do not use stream manipulation functions" );
    static_assert( !std::is_same<typename std::decay<T>::type, QTextStreamManipulator>::value, "Do not use stream manipulation functions" );
    QString ret;
    QTextStream out_stream(&ret, QIODevice::WriteOnly);
    out_stream.setRealNumberPrecision(12);
    out_stream << arg;
    return ret;
}

template <typename T>
typename std::enable_if<is_qenum<T>::value , QString>::type
AVToStringInternal(const T& arg)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<T>();
    if (auto value = metaEnum.valueToKey(static_cast<int>(arg)))
        return QString::fromLatin1(value);
    return QString::null;

}

QString AVToStringInternal(...) = delete; //Fallback default printing for this type

template<typename T>
typename std::enable_if<is_qenum<T>::value, bool>::type AVFromStringInternal(const QString& str, T& arg)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<T>();
    bool ok = false;
    auto value = metaEnum.keyToValue(str.toLatin1().constData(), &ok);
    if (ok)
        arg = static_cast<T>(value);
    return ok;
}

//! The default implementation is deleted for fromString conversion because it might be
//! accidentially used for number types, which then would lack proper range and conversion checking.
bool AVFromStringInternal(...) = delete; //Fallback for default

} // namespace avfromtostring

///////////////////////////////////////////////////////////////////////////////


template<typename T>
QString AVToString(const T& arg, bool enable_escape = false)
{
    Q_UNUSED(enable_escape)
    return avfromtostring::AVToStringInternal(arg);
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
bool AVFromString(const QString& str, T& arg)
{
    QString temp;
    // call of AVFromString<QString> beforehand allows enums to be quoted in config files
    return AVFromString(str, temp) && avfromtostring::AVFromStringInternal(temp, arg);
}

///////////////////////////////////////////////////////////////////////////////

template<typename T1, typename T2>
QString AVPairToStringInternal(const T1& first, const T2& second,
                               const QChar& separator,
                               bool enable_escape)
{
    return avfromtostring::DELIMITER_START + AVToString(first, enable_escape) +
            separator + " " + AVToString(second, enable_escape) + avfromtostring::DELIMITER_END;
}

///////////////////////////////////////////////////////////////////////////////

template<typename T1, typename T2>
bool AVPairFromStringInternal(const QString& str, T1& first, T2& second, const QChar& separator)
{
    QStringList entries;
    if (!avfromtostring::nestedSplit(entries, separator, str, true)) return false;

    if (entries.size() != 2) return false;

    if (!AVFromString(entries[0], first) || !AVFromString(entries[1], second)) return false;

    return true;
}

///////////////////////////////////////////////////////////////////////////////

template<typename T1, typename T2>
QString AVToString(const QPair<T1, T2>& arg, bool enable_escape = false)
{
    Q_UNUSED(enable_escape);
    return AVPairToStringInternal(arg.first, arg.second, avfromtostring::SEPARATOR, true);
}

///////////////////////////////////////////////////////////////////////////////

template<typename T1, typename T2>
bool AVFromString(const QString& str, QPair<T1, T2>& arg)
{
    return AVPairFromStringInternal(str, arg.first, arg.second, avfromtostring::SEPARATOR);
}

///////////////////////////////////////////////////////////////////////////////

template<typename T1, typename T2>
QString AVToString(const QMap<T1, T2>& arg, bool enable_escape = false)
{
    Q_UNUSED(enable_escape);
    QStringList entries;
    for (typename QMap<T1, T2>::const_iterator it = arg.begin();
         it != arg.end();
         ++it)
    {
        entries << AVToString(it.key(), true) + " " +
            avfromtostring::MAPPING + " " + AVToString(it.value(), true);
    }
    return avfromtostring::DELIMITER_START +
           entries.join(QString(avfromtostring::SEPARATOR) + " ") +
           avfromtostring::DELIMITER_END;
}

///////////////////////////////////////////////////////////////////////////////

template<typename T1, typename T2>
QString AVToString(const QHash<T1, T2>& arg, bool enable_escape = false)
{
    // We want consistently sorted output (config files, debug output...)
    // -> use QMap implementation
    QMap<T1, T2> map;
    for(auto iter = arg.constBegin(); iter != arg.constEnd(); iter++)
        map.insert(iter.key(), iter.value());
    return AVToString(map, enable_escape);
}

///////////////////////////////////////////////////////////////////////////////

template<typename T1, typename T2>
bool AVFromString(const QString& str, QMap<T1, T2>& arg)
{
    arg.clear();

    QStringList entries;
    if (!avfromtostring::nestedSplit(entries, avfromtostring::SEPARATOR, str, true)) return false;
    for (int e=0; e<entries.size(); ++e)
    {
        QStringList key_value;
        if (!avfromtostring::nestedSplit(key_value, avfromtostring::MAPPING, entries[e], false))
            return false;
        if (key_value.size() != 2) return false;

        QString key   = key_value[0];
        QString value = key_value[1];

        T1 key_val;
        T2 val_val;

        if (!AVFromString(key,   key_val)) return false;
        if (!AVFromString(value, val_val)) return false;
        if (arg.contains(key_val))         return false;

        arg[key_val] = val_val;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

template<typename T1, typename T2>
bool AVFromString(const QString& str, QHash<T1, T2>& arg)
{
    arg.clear();

    QStringList entries;
    if (!avfromtostring::nestedSplit(entries, avfromtostring::SEPARATOR, str, true)) return false;
    for (int e=0; e<entries.size(); ++e)
    {
        QStringList key_value;
        if (!avfromtostring::nestedSplit(key_value, avfromtostring::MAPPING, entries[e], false))
            return false;
        if (key_value.size() != 2) return false;

        QString key   = key_value[0];
        QString value = key_value[1];

        T1 key_val;
        T2 val_val;

        if (!AVFromString(key,   key_val)) return false;
        if (!AVFromString(value, val_val)) return false;
        if (arg.contains(key_val))         return false;

        arg[key_val] = val_val;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
QString AVToString(
        const QVector<T>& arg, bool enable_escape = false)
{
    Q_UNUSED(enable_escape);
    QStringList entries;
    for (int i=0; i<arg.size(); ++i)
    {
        entries << AVToString(arg[i], true);
    }
    return avfromtostring::DELIMITER_START +
           entries.join(QString(avfromtostring::SEPARATOR) + " ") +
           avfromtostring::DELIMITER_END;
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
bool AVFromString(const QString& str, QVector<T>& arg)
{
    arg.clear();
    QStringList entries;
    if (!avfromtostring::nestedSplit(entries, avfromtostring::SEPARATOR, str, true)) return false;
    for (int e=0; e<entries.size(); ++e)
    {
        QString value = entries[e].trimmed();

        T val;

        if (!AVFromString(value, val)) return false;

        arg.push_back(val);
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
QString AVToString(const QList<T>& arg, bool enable_escape = false)
{
    Q_UNUSED(enable_escape);
    QStringList entries;
    for (typename QList<T>::const_iterator it = arg.begin();
         it != arg.end();
         ++it)
    {
        entries << AVToString(*it, true);
    }
    return avfromtostring::DELIMITER_START +
           entries.join(QString(avfromtostring::SEPARATOR) + " ") +
           avfromtostring::DELIMITER_END;
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
bool AVFromString(const QString& str, QList<T>& arg)
{
    arg.clear();
    QStringList entries;
    if (!avfromtostring::nestedSplit(entries, avfromtostring::SEPARATOR, str, true)) return false;
    for (int e=0; e<entries.size(); ++e)
    {
        QString value = entries[e].trimmed();

        T val;

        if (!AVFromString(value, val)) return false;

        arg.push_back(val);
    }

    return true;
}

//! The result has representations for empty string and null string. If arg contains any special
//! characters, the whole string is quoted, and quoting and escape characters are escaped.
template<> QString AVLIB_EXPORT AVToString(const QString& arg, bool enable_escape);
//! Work around the special way the null string is implemented as struct in Qt5. See AVFromToStringTest::testString.
template<> QString AVLIB_EXPORT AVToString(const QString::Null& arg, bool enable_escape);
template<> bool AVLIB_EXPORT    AVFromString(const QString& str, QString& arg);

template<> QString AVLIB_EXPORT AVToString(const QStringList& arg, bool enable_escape);
template<> bool AVLIB_EXPORT    AVFromString(const QString& str, QStringList& arg);

template<> QString AVLIB_EXPORT AVToString(const bool& arg, bool enable_escape);
template<> bool AVLIB_EXPORT    AVFromString(const QString& str, bool& arg);

template<> QString AVLIB_EXPORT AVToString(const QPoint& arg, bool enable_escape);
template<> bool AVLIB_EXPORT    AVFromString(const QString& str, QPoint& arg);

template<> QString AVLIB_EXPORT AVToString(const QPointF& arg, bool enable_escape);
template<> bool AVLIB_EXPORT    AVFromString(const QString& str, QPointF& arg);

template<> QString AVLIB_EXPORT AVToString(const QPolygon& arg, bool enable_escape);
template<> bool AVLIB_EXPORT    AVFromString(const QString& str, QPolygon& arg);

template<> QString AVLIB_EXPORT AVToString(const QPolygonF& arg, bool enable_escape);
template<> bool AVLIB_EXPORT    AVFromString(const QString& str, QPolygonF& arg);

template<> QString AVLIB_EXPORT AVToString(const QSize& arg, bool enable_escape);
template<> bool AVLIB_EXPORT    AVFromString(const QString& str, QSize& arg);

template<> QString AVLIB_EXPORT AVToString(const QSizeF& arg, bool enable_escape);
template<> bool AVLIB_EXPORT    AVFromString(const QString& str, QSizeF& arg);

template<> QString AVLIB_EXPORT AVToString(const QRect& arg, bool enable_escape);
template<> bool AVLIB_EXPORT    AVFromString(const QString& str, QRect& arg);

template<> QString AVLIB_EXPORT AVToString(const QRectF& arg, bool enable_escape);
template<> bool AVLIB_EXPORT    AVFromString(const QString& str, QRectF& arg);

template<> QString AVLIB_EXPORT AVToString(const QDateTime& arg, bool enable_escape);
template<> bool AVLIB_EXPORT    AVFromString(const QString& str, QDateTime& arg);

template<> QString AVLIB_EXPORT AVToString(const QTime& arg, bool enable_escape);
template<> bool AVLIB_EXPORT    AVFromString(const QString& str, QTime& arg);

template<> QString AVLIB_EXPORT AVToString(const QDate& arg, bool enable_escape);
template<> bool AVLIB_EXPORT    AVFromString(const QString& str, QDate& arg);

template<> QString AVLIB_EXPORT AVToString(const QRegExp& arg, bool enable_escape);
template<> bool AVLIB_EXPORT    AVFromString(const QString& str, QRegExp& arg);

template<> QString AVLIB_EXPORT AVToString(const QRegularExpression& arg, bool enable_escape);
template<> bool AVLIB_EXPORT    AVFromString(const QString& str, QRegularExpression& arg);

template<> QString AVLIB_EXPORT AVToString(const QFont& arg, bool enable_escape);
template<> bool AVLIB_EXPORT    AVFromString(const QString& str, QFont& arg);

template<> QString AVLIB_EXPORT AVToString(const QColor& arg, bool enable_escape);
template<> bool AVLIB_EXPORT    AVFromString(const QString& str, QColor& arg);

template<> QString AVLIB_EXPORT AVToString(const QImage& arg, bool enable_escape);
template<> bool AVLIB_EXPORT    AVFromString(const QString& str, QImage& arg);

template<> QString AVLIB_EXPORT AVToString(const QVariant& arg, bool enable_escape);
template<> bool AVLIB_EXPORT    AVFromString(const QString& str, QVariant& arg);

template<> QString AVLIB_EXPORT AVToString(const QUuid& arg, bool enable_escape);
template<> bool AVLIB_EXPORT    AVFromString(const QString& str, QUuid& arg);

template<> QString AVLIB_EXPORT AVToString(const QBitArray& arg, bool enable_escape);
template<> bool AVLIB_EXPORT    AVFromString(const QString& str, QBitArray& arg);

template<> QString AVLIB_EXPORT AVToString(const QTransform& arg, bool enable_escape);
template<> bool AVLIB_EXPORT    AVFromString(const QString& str, QTransform& arg);

// The following fromString specializations do proper range and conversion checking.
// Under Qt3 this is neccessary for all primitive data types as the text stream does not provide
// error handling capabilities.
// For symmetry reasons, all of these methods exist in Qt4 as well.
// toString conversion for the types below happens via default textstream implementation.
template<> bool AVLIB_EXPORT    AVFromString(const QString& str, quint8& arg);
template<> bool AVLIB_EXPORT    AVFromString(const QString& str, qint8& arg);
template<> bool AVLIB_EXPORT    AVFromString(const QString& str, quint16& arg);
template<> bool AVLIB_EXPORT    AVFromString(const QString& str, qint16& arg);
template<> bool AVLIB_EXPORT    AVFromString(const QString& str, uint& arg);
template<> bool AVLIB_EXPORT    AVFromString(const QString& str, int& arg);
template<> bool AVLIB_EXPORT    AVFromString(const QString& str, quint64& arg);
template<> bool AVLIB_EXPORT    AVFromString(const QString& str, qint64& arg);
template<> bool AVLIB_EXPORT    AVFromString(const QString& str, float& arg);
template<> bool AVLIB_EXPORT    AVFromString(const QString& str, double& arg);
template<> bool AVLIB_EXPORT    AVFromString(const QString& str, long& arg);
template<> bool AVLIB_EXPORT    AVFromString(const QString& str, unsigned long& arg);

///////////////////////////////////////////////////////////////////////////////
// ------------- Qt version specific implementation goes below this line ---------------------
///////////////////////////////////////////////////////////////////////////////

template<typename T>
QString AVToString(const QSet<T>& arg, bool enable_escape = false)
{
    auto list = arg.toList();
    std::sort(list.begin(), list.end());
    return AVToString(list, enable_escape);
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
bool AVFromString(const QString& str, QSet<T>& arg)
{
    arg.clear();
    QStringList entries;
    if (!avfromtostring::nestedSplit(entries, avfromtostring::SEPARATOR, str, true)) return false;
    for (int e=0; e<entries.size(); ++e)
    {
        QString value = entries[e].trimmed();

        T val;

        if (!AVFromString(value, val)) return false;

        arg.insert(val);
    }

    return true;
}

template<> QString AVLIB_EXPORT AVToString(const QVector2D& arg, bool enable_escape);
template<> bool AVLIB_EXPORT    AVFromString(const QString& str, QVector2D& arg);

template<> QString AVLIB_EXPORT AVToString(const QVector3D& arg, bool enable_escape);
template<> bool AVLIB_EXPORT    AVFromString(const QString& str, QVector3D& arg);

#endif

// End of file
