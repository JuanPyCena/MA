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
*/


#include <limits>

// Qt includes
#include <QColor>
#include <QDateTime>
#include <QFont>
#include <QImage>
#include <QList>
#include <QMap>
#include <QHash>
#include <QPair>
#include <QPoint>
#include <QPolygon>
#include <QRect>
#include <QRegExp>
#include <QRegularExpression>
#include <QSize>
#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QVariant>
#include <QVector2D>
#include <QVector3D>
#include <QVector>
#include <QUuid>
#include <QBitArray>
#include <QTransform>

// local includes
#include "avdatetime.h"
#include "avfromtostring.h"
#include "avlog.h"
#include "avmisc.h"

// work around windows namespace pollution
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif



namespace avfromtostring
{

	// If touching any of those, also update SPECIAL_CHAR_REGEXP!
	const QChar DELIMITER_START = '[';
	const QChar DELIMITER_END = ']';
	const QChar QUOTE = '\"';
	const QChar ESCAPE = '\\';
	const QChar SEPARATOR = ';';
	const QChar MAPPING = ':';
    const QChar NEWLINE = '\n';
	//! Not in SPECIAL_CHAR_LIST as this is only used for QSize(s).
	const QChar CROSS = 'x';

	//! Strings are quoted if they contain any of the following characters.
	//! Not a QRegExp because that's not threadsafe (see Bug#5552)
	//! Keep in sync with the constants above!
    const QStringList SPECIAL_CHAR_LIST = QStringList {
        DELIMITER_START, DELIMITER_END,
        QUOTE, ESCAPE, SEPARATOR, MAPPING, NEWLINE
    };

	//! The format for parsing and writing QDateTime
	const QString QDATETIME_FORMAT = "yyyy-MM-ddThh:mm:ss.zzz";
	const QString QDATE_FORMAT = "yyyy-MM-dd";
	const QString QTIME_FORMAT = "hh:mm:ss.zzz";

	const QString EMPTY_STRING = QString(QUOTE) + QUOTE;

	//! If quoting a string, the quoting character itself must be escaped to allow unquoting later on.
	const QStringList ESCAPE_FROM =
		QStringList() << QString(QUOTE) << QString(ESCAPE);
	const QStringList ESCAPE_TO =
		QStringList() << QString(ESCAPE) + QUOTE << QString(ESCAPE) + ESCAPE;

	///////////////////////////////////////////////////////////////////////////////

bool nestedSplit(QStringList& list, const QString& separator, const QString& input_string,
                 bool strip_outmost_delimiters)
{
    list.clear();

    QString str = input_string;
    if (strip_outmost_delimiters)
    {
        if(!str.isNull())
        {
            str = str.trimmed();
        }
        if (!str.startsWith(QString(avfromtostring::DELIMITER_START)) ||
            !str.endsWith  (QString(avfromtostring::DELIMITER_END))) return false;
        str = str.mid(1, str.length()-2);
    }

    if (str.trimmed().isEmpty()) return true;

    bool quoted      = false; // whether we currently are in a quoted part of the input string
    bool prev_escape = false; // whether the previously read character was the escpae character
    uint nest_count  = 0;
    int start        = 0;     // The start position of the currently read list element
    int pos          = 0;

    while (str.length() > pos)
    {
        QChar cur_char = str[pos];
        if (quoted)
        {
            if        (cur_char == QUOTE)
            {
                if (!prev_escape) quoted = false;
                prev_escape = false;
            } else if (cur_char == ESCAPE)
            {
                prev_escape = !prev_escape;
            } else
            {
                prev_escape = false;
            }

        } else
        {
            if        (cur_char == DELIMITER_START)
            {
                ++nest_count;
            } else if (cur_char == DELIMITER_END)
            {
                if (nest_count == 0) return false;
                --nest_count;
            } else if (cur_char == QUOTE)
            {
                quoted = true;
            } else if (nest_count == 0 && QString(cur_char) == separator)
            {
                QString tmp = str.mid(start, pos-start).trimmed();
                if(tmp.isNull())
                {
                    tmp = "";
                }
                list << tmp;
                start = pos+1;
            }
        }
        ++pos;
    }

    // last character is separator, thus add an empty string
    // if str == separator there will be two empty strings
    if (!str.isEmpty() && QString(str[str.length() - 1]) == separator)
    {
        list << QString("");
    }

    if (pos > start)
    {
        QString tmp = str.mid(start, pos-start).trimmed();
        if(tmp.isNull())
        {
            tmp = "";
        }
        list << tmp;
    }

    return nest_count == 0 && !quoted;
}
} // namespace avfromtostring

///////////////////////////////////////////////////////////////////////////////

template<>
QString AVToString(const QString& arg, bool enable_escape)
{
    if (arg.isNull())  return "\\0";
    if (arg.isEmpty()) return avfromtostring::EMPTY_STRING;
    else if (enable_escape)
    {
        bool escape = arg.startsWith(" ") || arg.endsWith(" ");

        if (!escape)
        {
            for (QStringList::const_iterator it = avfromtostring::SPECIAL_CHAR_LIST.constBegin();
                    it != avfromtostring::SPECIAL_CHAR_LIST.constEnd(); ++it)
            {
                if (arg.contains(*it))
                {
                    escape = true;
                    break;
                }
            }
        }

        if (escape)
        {
            return avfromtostring::QUOTE +
                    AVEscape(arg, avfromtostring::ESCAPE_FROM, avfromtostring::ESCAPE_TO) +
                    avfromtostring::QUOTE;
        }
    }
    return arg;
}

///////////////////////////////////////////////////////////////////////////////

template<>
QString AVToString(const QString::Null& arg, bool enable_escape)
{
    Q_UNUSED(arg);
    QString s;
    return AVToString(s, enable_escape);
}

///////////////////////////////////////////////////////////////////////////////

template<>
bool AVFromString(const QString& str, QString& arg)
{
    if (str.isNull()) return false;
    if (str == "\\0")
    {
        arg = QString::null;
        return true;
    }

    arg = str.trimmed();

    // If the string was quoted, we need to unquote and unescape it.
    if (arg.startsWith(avfromtostring::QUOTE))
    {
        if (!arg.endsWith(avfromtostring::QUOTE)) return false;
        arg = arg.mid(1, arg.length()-2);
        arg = AVEscape(arg, avfromtostring::ESCAPE_TO, avfromtostring::ESCAPE_FROM);
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

template<> QString AVToString  (const QStringList& arg, bool enable_escape)
{
    return AVToString(static_cast<QList<QString> >(arg), enable_escape);
}

///////////////////////////////////////////////////////////////////////////////

template<> bool AVFromString(const QString& str, QStringList& arg)
{
    return AVFromString(str, *(static_cast<QList<QString>*>(&arg)));
}

///////////////////////////////////////////////////////////////////////////////

//! TODO CM use in AVConsole, document
template<>
QString AVToString(const bool& arg, bool enable_escape)
{
    Q_UNUSED(enable_escape);
    return arg ? "true" : "false";
}

///////////////////////////////////////////////////////////////////////////////

//! TODO CM use in AVConsole, document
template<>
bool AVFromString(const QString& str, bool& arg)
{
    QString lc = str.toLower();
    if      (lc == "1" || lc == "true"  || lc == "yes") arg = true;
    else if (lc == "0" || lc == "false" || lc == "no")  arg = false;
    else return false;

    return true;
}

///////////////////////////////////////////////////////////////////////////////

template<>
QString AVToString(const QPoint& arg, bool enable_escape)
{
    return AVPairToStringInternal(arg.x(), arg.y(), avfromtostring::SEPARATOR, enable_escape);
}

///////////////////////////////////////////////////////////////////////////////

template<>
bool AVFromString(const QString& str, QPoint& arg)
{
    return AVPairFromStringInternal(str, arg.rx(), arg.ry(),
                                    avfromtostring::SEPARATOR);
}

///////////////////////////////////////////////////////////////////////////////

template<>
QString AVToString(const QPointF& arg, bool enable_escape)
{
    return AVPairToStringInternal(arg.x(), arg.y(), avfromtostring::SEPARATOR, enable_escape);
}

///////////////////////////////////////////////////////////////////////////////

template<>
bool AVFromString(const QString& str, QPointF& arg)
{
    return AVPairFromStringInternal(str, arg.rx(), arg.ry(),
                                    avfromtostring::SEPARATOR);
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
static QString AVPointArrayToStringInternal(const T& arg, bool enable_escape)
{
    QStringList entries;
    for (int i = 0; i < arg.size(); ++i)
    {
        entries << AVToString(arg[i], enable_escape);
    }
    return avfromtostring::DELIMITER_START +
           entries.join(QString(avfromtostring::SEPARATOR) + " ") +
           avfromtostring::DELIMITER_END;
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
static bool AVPointArrayFromStringInternal(const QString& str, T& arg)
{
    QStringList entries;
    if (!avfromtostring::nestedSplit(entries, avfromtostring::SEPARATOR, str, true)) return false;

    arg.resize(entries.count());

    QStringList::const_iterator it  = entries.begin();
    QStringList::const_iterator end = entries.end();
    for (int index = 0; it != end; ++it, ++index)
    {
        QString value = (*it).trimmed();

        typename T::value_type val;

        if (!AVFromString(value, val)) return false;

        arg[index] = val;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

template<>
QString AVToString(const QPolygon& arg, bool enable_escape)
{
    return AVPointArrayToStringInternal(arg, enable_escape);
}

///////////////////////////////////////////////////////////////////////////////

template<>
bool AVFromString(const QString& str, QPolygon& arg)
{
    return AVPointArrayFromStringInternal(str, arg);
}

///////////////////////////////////////////////////////////////////////////////

template<>
QString AVToString(const QPolygonF& arg, bool enable_escape)
{
    return AVPointArrayToStringInternal(arg, enable_escape);
}

///////////////////////////////////////////////////////////////////////////////

template<>
bool AVFromString(const QString& str, QPolygonF& arg)
{
    return AVPointArrayFromStringInternal(str, arg);
}

///////////////////////////////////////////////////////////////////////////////

template<>
QString AVToString(const QSize& arg, bool enable_escape)
{
    return AVPairToStringInternal(arg.width(), arg.height(), avfromtostring::CROSS, enable_escape);
}

///////////////////////////////////////////////////////////////////////////////

template<>
bool AVFromString(const QString& str, QSize& arg)
{
    return AVPairFromStringInternal(str, arg.rwidth(), arg.rheight(), avfromtostring::CROSS);
}

///////////////////////////////////////////////////////////////////////////////

template<>
QString AVToString(const QSizeF& arg, bool enable_escape)
{
    return AVPairToStringInternal(arg.width(), arg.height(), avfromtostring::CROSS, enable_escape);
}

///////////////////////////////////////////////////////////////////////////////

template<>
bool AVFromString(const QString& str, QSizeF& arg)
{
    return AVPairFromStringInternal(str, arg.rwidth(), arg.rheight(), avfromtostring::CROSS);
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
static QString AVRectToStringInternal(const T& arg, bool enable_escape)
{
    QString result = avfromtostring::DELIMITER_START;
    result += AVToString(arg.topLeft(), enable_escape);
    result += QString(avfromtostring::SEPARATOR);
    result += " ";
    result += AVToString(arg.size(), enable_escape);
    result += avfromtostring::DELIMITER_END;
    return result;
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
static bool AVRectFromStringInternal(const QString& str, T& arg)
{
    QStringList entries;
    if (!avfromtostring::nestedSplit(entries, avfromtostring::SEPARATOR, str, true)) return false;

    if (entries.count() != 2) return false;

    QPoint topLeft;
    if (!AVFromString(entries.front(), topLeft)) return false;

    entries.pop_front();

    QSize size;
    if (!AVFromString(entries.front(), size)) return false;

    arg.setRect(topLeft.x(), topLeft.y(), size.width(), size.height());

    return true;
}

///////////////////////////////////////////////////////////////////////////////

template<>
QString AVToString(const QRect& arg, bool enable_escape)
{
    return AVRectToStringInternal(arg, enable_escape);
}

///////////////////////////////////////////////////////////////////////////////

template<>
bool AVFromString(const QString& str, QRect& arg)
{
    return AVRectFromStringInternal(str, arg);
}

///////////////////////////////////////////////////////////////////////////////

template<>
QString AVToString(const QRectF& arg, bool enable_escape)
{
    return AVRectToStringInternal(arg, enable_escape);
}

///////////////////////////////////////////////////////////////////////////////

template<>
bool AVFromString(const QString& str, QRectF& arg)
{
    return AVRectFromStringInternal(str, arg);
}

///////////////////////////////////////////////////////////////////////////////

template<>
QString AVToString(const QDateTime& arg, bool enable_escape)
{
    if (arg.isNull()) return AVToString(QString::null);
    return AVToString(arg.toString(avfromtostring::QDATETIME_FORMAT), enable_escape);
}

///////////////////////////////////////////////////////////////////////////////

template<>
bool AVFromString(const QString& str, QDateTime& arg)
{
    QString datetime_string;
    if(!AVFromString(str, datetime_string))
        return false;

    // either it was \0 or ""
    if (datetime_string.isEmpty())
    {
        arg = AVDateTime();
        return true;
    }

    arg = AVDateTime::fromString(datetime_string, avfromtostring::QDATETIME_FORMAT);

    if(!arg.isValid())
        arg = AVDateTime::fromString(datetime_string, "yyyy-M-dTh:m:s.z"); // fall back to single-digit parsing, see SWE-6656
    if(!arg.isValid())
        arg = AVDateTime::fromString(datetime_string, "yyyy-M-dTh:m:s"); // Note: Qt::ISODate makes troubles (multimatches)
    if(!arg.isValid())
        arg = AVDateTime::fromString(datetime_string, "yyyyMMddhhmmss.zzz");
    if(!arg.isValid())
        arg = AVDateTime::fromString(datetime_string, "yyyyMMddhhmmss");
    if(!arg.isValid())
        arg = AVDateTime::fromString(datetime_string, "yyyyMMddhhmm");

    return arg.isValid();
}

///////////////////////////////////////////////////////////////////////////////

template<>
QString AVToString(const QTime& arg, bool enable_escape)
{
    if (arg.isNull()) return AVToString(QString::null);
    return AVToString(arg.toString(avfromtostring::QTIME_FORMAT), enable_escape);
}

///////////////////////////////////////////////////////////////////////////////

template<>
bool AVFromString(const QString& str, QTime& arg)
{
    QString time_string;
    if(!AVFromString(str, time_string))
        return false;

    // either it was \0 or ""
    if (time_string.isEmpty())
    {
        arg = QTime();
        return true;
    }

    arg = QTime::fromString(time_string, avfromtostring::QTIME_FORMAT);

    if(!arg.isValid())
        arg = QTime::fromString(time_string, "h:m:s.z"); // fall back to single-digit parsing, see SWE-6656
    if(!arg.isValid())
        arg = QTime::fromString(time_string, "hhmmss.zzz");
    if(!arg.isValid())
        arg = QTime::fromString(time_string, "hhmmss");

    return arg.isValid();
}

///////////////////////////////////////////////////////////////////////////////

template<>
QString AVToString(const QDate& arg, bool enable_escape)
{
    if (arg.isNull()) return AVToString(QString::null);
    return AVToString(arg.toString(avfromtostring::QDATE_FORMAT), enable_escape);
}

///////////////////////////////////////////////////////////////////////////////

template<>
bool AVFromString(const QString& str, QDate& arg)
{
    QString date_string;
    if(!AVFromString(str, date_string))
        return false;

    // either it was \0 or ""
    if (date_string.isEmpty())
    {
        arg = QDate();
        return true;
    }

    arg = QDate::fromString(date_string, avfromtostring::QDATE_FORMAT);

    if(!arg.isValid())
        arg = QDate::fromString(date_string, "yyyy-M-d"); // fall back to single-digit parsing, see SWE-6656
    if(!arg.isValid())
        arg = QDate::fromString(date_string, "yyyyMMdd");

    return arg.isValid();
}

///////////////////////////////////////////////////////////////////////////////

template<> QString AVToString(const QRegExp& arg, bool enable_escape)
{
    return AVToString(arg.pattern(), enable_escape);
}

///////////////////////////////////////////////////////////////////////////////

template<> bool AVFromString(const QString& str, QRegExp& arg)
{
    QString pattern;
    if (!AVFromString(str, pattern)) return false;
    arg.setPattern(pattern);
    return arg.isValid();
}

///////////////////////////////////////////////////////////////////////////////

template<> QString AVToString(const QRegularExpression& arg, bool enable_escape)
{
    return AVToString(arg.pattern(), enable_escape);
}

///////////////////////////////////////////////////////////////////////////////

template<> bool AVFromString(const QString& str, QRegularExpression& arg)
{
    QString pattern;
    if (!AVFromString(str, pattern)) return false; // TODO: Hex values like \x0d are not supported here?!
    arg.setPattern(pattern);
    return arg.isValid();
}

///////////////////////////////////////////////////////////////////////////////

template<> QString AVToString(const QFont& arg, bool enable_escape)
{
    return AVToString(arg.toString(), enable_escape);
}

///////////////////////////////////////////////////////////////////////////////

template<> bool AVFromString(const QString& str, QFont& arg)
{
    QString name;
    if (!AVFromString(str, name)) return false;
    return arg.fromString(name);
}

///////////////////////////////////////////////////////////////////////////////

template<> QString AVToString(const QColor& arg, bool enable_escape)
{
    if (!arg.isValid())
    {
        // Use null string representation for invalid colors
        return AVToString(QString(), enable_escape);
    }

    // Build color string ourselves since QColor::name() does not respect the
    // alpha channel neither in Qt3 nor in Qt4/5. However, the alpha channel
    // is only printed for non-opaque colors and and only in the Qt4/5
    // implementation of AVToString since Qt3's QColor does not have an alpha
    // component and a fixed alpha value of 255 might be seen as clutter
    // depending on the context the string is used (e.g. parameter value in AVConfig2).
    QString str;
    str.sprintf("#%02x%02x%02x", arg.red(), arg.green(), arg.blue());

#if (QT_VERSION >= 0x040000)
    if (arg.alpha() != 0xff)
    {
        str += QString().sprintf("%02x", arg.alpha());
    }
#endif

    return AVToString(str, enable_escape);
}

///////////////////////////////////////////////////////////////////////////////

template<> bool AVFromString(const QString& str, QColor& arg)
{
    QString value_str;
    if(!AVFromString(str, value_str))
        return false;

    // Null string is used to represent invalid colors
    if (value_str.isNull())
    {
        arg = QColor();
        return true;
    }

    if (value_str.isEmpty() || value_str.at(0) != QChar('#')) return false;

    bool ok = false;
    uint value = value_str.mid(1).toUInt(&ok, 16);
    if (!ok) return false;

    // Capture #RRGGBB
    if (value_str.length() == 7u)
    {
        QRgb rgb = qRgb((value >> 16) & 0xff, (value >> 8) & 0xff, value & 0xff);
        arg.setRgb(rgb);
        return true;
    }

    // Capture #RRGGBBAA
    if (value_str.length() == 9u)
    {
        QRgb rgb = qRgba((value >> 24) & 0xff, (value >> 16) & 0xff, (value >> 8) & 0xff, value & 0xff);

#if (QT_VERSION < 0x040000)
        if (qAlpha(rgb) != 0xff)
        {
            AVLogError << "AVFromString: failed to parse color from string '"
                       << value_str << "' - Qt3's QColor does not support transparency.";
            return false;
        }

        arg.setRgb(rgb);
#else
        arg.setRgba(rgb);
#endif
        return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////

template<> QString AVToString(const QImage& img, bool enable_escape)
{
    return AVToString(QString("width=%1, height=%2, depth=%3")
            .arg(img.width()).arg(img.height())
            .arg(img.depth()), enable_escape);
}

///////////////////////////////////////////////////////////////////////////////

template<> bool AVFromString(const QString& str, QImage& arg)
{
    Q_UNUSED(str);
    Q_UNUSED(arg);
    AVLogFatal << "AVFromString: not possible for QImage";
    return false;
}

///////////////////////////////////////////////////////////////////////////////

template<> QString AVToString(const QVariant& value, bool enable_escape)
{
    QString result;
    switch (value.type())
    {
        case QVariant::Type::String:
        {
            const auto& str = value.toString();
            result = AVToString(str, enable_escape);
        }
            break;
        case QVariant::Type::StringList:
        {
            const auto& str_list = value.toStringList();
            result = AVToString(str_list, enable_escape);
        }
            break;
        case QVariant::Type::Rect:
        {
            const auto& rect = value.toRect();
            result = AVToString(rect, enable_escape);
        }
            break;
        case QVariant::Type::Size:
        {
            const auto& size = value.toSize();
            result = AVToString(size, enable_escape);
        }
            break;
        case QVariant::Type::Color:
        {
            const auto& color = value.value<QColor>();
            result = AVToString(color, enable_escape);
        }
            break;
        case QVariant::Type::Point:
        {
            const auto& point = value.toPoint();
            result = AVToString(point, enable_escape);
        }
            break;
        case QVariant::Type::Int:
        {
            const auto& num = value.toInt();
            result = AVToString(num, enable_escape);
        }
            break;
        case QVariant::Type::UInt:
        {
            const auto& num = value.toUInt();
            result = AVToString(num, enable_escape);
        }
            break;
        case QVariant::Type::Bool:
        {
            const auto& truth = value.toBool();
            result = AVToString(truth, enable_escape);
        }
            break;
        case QVariant::Type::Double:
        {
            const auto& num = value.toDouble();
            result = AVToString(num, enable_escape);
        }
            break;
        case QVariant::Type::Date:
        {
            const auto& date = value.toDate();
            result = AVToString(date, enable_escape);
        }
            break;
        case QVariant::Type::Time:
        {
            const auto& time = value.toTime();
            result = AVToString(time, enable_escape);
        }
            break;
        case QVariant::Type::DateTime:
        {
            const auto& date_time = value.toDateTime();
            result = AVToString(date_time, enable_escape);
        }
            break;
        case QVariant::Type::LongLong:
        {
            const auto& num = value.toLongLong();
            result = AVToString(num, enable_escape);
        }
            break;
        case QVariant::Type::ULongLong:
        {
            const auto& num = value.toULongLong();
            result = AVToString(num, enable_escape);
        }
            break;
        default:
            // using AVToString here probably is a bit paranoid...
            result = AVToString(QString("QVariant(") + value.typeName() + ")", enable_escape);
        // Additional Qt5 types not converted due to consistency reasons.
        case QVariant::Type::Char:
        case QVariant::Type::EasingCurve:
        case QVariant::Type::Uuid:
        case QVariant::Type::ModelIndex:
        case QVariant::Type::PersistentModelIndex:
        case QVariant::Type::Hash:
        case QVariant::Type::Icon:
        case QVariant::Type::LastType:
        case QVariant::Type::Line:
        case QVariant::Type::LineF:
        case QVariant::Type::Locale:
        case QVariant::Type::Matrix:
        case QVariant::Type::Transform:
        case QVariant::Type::Matrix4x4:
        case QVariant::Type::PointF:
        case QVariant::Type::Polygon:
        case QVariant::Type::PolygonF:
        case QVariant::Type::Quaternion:
        case QVariant::Type::RectF:
        case QVariant::Type::RegExp:
        case QVariant::Type::RegularExpression:
        case QVariant::Type::SizeF:
        case QVariant::Type::TextFormat:
        case QVariant::Type::TextLength:
        case QVariant::Type::Url:
        case QVariant::Type::Vector2D:
        case QVariant::Type::Vector3D:
        case QVariant::Type::Vector4D:
        case QVariant::Type::UserType:
            break;
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////

template<> bool AVFromString(const QString& str, QVariant& arg)
{
    Q_UNUSED(str);
    Q_UNUSED(arg);
    AVLogFatal << "AVFromString: not possible for QVariant";
    return false;
}

///////////////////////////////////////////////////////////////////////////////

template<> QString AVLIB_EXPORT AVToString(const QUuid& arg, bool enable_escape)
{
    Q_UNUSED(enable_escape);
    return arg.toString();
}

///////////////////////////////////////////////////////////////////////////////

template<> bool AVLIB_EXPORT    AVFromString(const QString& str, QUuid& arg)
{
    arg = QUuid(str);
    // null Quuid could be failure or correct, depending on input string.
    if (arg.isNull())
    {
        return arg.toString() == str;
    } else return true;
}

///////////////////////////////////////////////////////////////////////////////

template<> QString AVLIB_EXPORT AVToString(const QBitArray& arg, bool enable_escape)
{
    Q_UNUSED(enable_escape);

    QVector<bool> vector;
    vector.reserve(arg.count()); //avoids multiple allocations
    for(int i = 0; i < arg.count(); i++)
        vector.append(arg.testBit(i));

    return AVToString(vector);
}

///////////////////////////////////////////////////////////////////////////////

template<> bool AVLIB_EXPORT    AVFromString(const QString& str, QBitArray& arg)
{
    QVector<bool> vector;

    if(!AVFromString(str, vector))
        return false;

    arg = QBitArray(vector.size());
    for(auto iter = vector.constBegin(); iter != vector.constEnd(); iter++)
        arg.setBit(int(iter-vector.constBegin()), *iter);

    return true;
}

///////////////////////////////////////////////////////////////////////////////

template<> QString AVLIB_EXPORT AVToString(const QTransform& arg, bool enable_escape)
{
    Q_UNUSED(enable_escape);

    QVector<QVector<qreal>> vector {
        { arg.m11(), arg.m12(), arg.m13() },
        { arg.m21(), arg.m22(), arg.m23() },
        { arg.m31(), arg.m32(), arg.m33() }
    };

    return AVToString(vector);
}

///////////////////////////////////////////////////////////////////////////////

template<> bool AVLIB_EXPORT    AVFromString(const QString& str, QTransform& arg)
{
    QVector<QVector<qreal>> vector;

    if(!AVFromString(str, vector))
        return false;

    if(vector.count() != 3 ||
       !std::all_of(vector.constBegin(), vector.constEnd(), [](const QVector<qreal>& v){ return v.count() == 3; }))
        return false;

    arg = QTransform(
        vector.at(0).at(0), vector.at(0).at(1), vector.at(0).at(2),
        vector.at(1).at(0), vector.at(1).at(1), vector.at(1).at(2),
        vector.at(2).at(0), vector.at(2).at(1), vector.at(2).at(2)
    );

    return true;
}

///////////////////////////////////////////////////////////////////////////////

template<> bool AVFromString(const QString& str, quint8& arg)
{
    bool ok = false;
    uint arg_uint = str.toUInt(&ok);
    if( arg_uint > std::numeric_limits<quint8>::max())
        return false;
    arg = arg_uint;
    return ok;
}

///////////////////////////////////////////////////////////////////////////////

template<> bool AVFromString(const QString& str, qint8& arg)
{
    bool ok = false;
    int arg_int = str.toInt(&ok);
    if( arg_int > std::numeric_limits<qint8>::max() ||
        arg_int < std::numeric_limits<qint8>::min())
    {
        return false;
    }
    arg = arg_int;
    return ok;
}

///////////////////////////////////////////////////////////////////////////////

template<> bool AVFromString(const QString& str, quint16& arg)
{
    bool ok = false;
    uint arg_uint = str.toUInt(&ok);
    if( arg_uint > std::numeric_limits<quint16>::max())
        return false;
    arg = arg_uint;
    return ok;
}

///////////////////////////////////////////////////////////////////////////////

template<> bool AVFromString(const QString& str, qint16& arg)
{
    bool ok = false;
    int arg_int = str.toInt(&ok);
    if( arg_int > std::numeric_limits<qint16>::max() ||
        arg_int < std::numeric_limits<qint16>::min())
    {
        return false;
    }
    arg = arg_int;
    return ok;
}

///////////////////////////////////////////////////////////////////////////////

template<> bool AVFromString(const QString& str, uint& arg)
{
    bool ok = false;
    arg = str.toUInt(&ok);
    return ok;
}

///////////////////////////////////////////////////////////////////////////////

template<> bool AVFromString(const QString& str, int& arg)
{
    bool ok = false;
    arg = str.toInt(&ok);
    return ok;
}

///////////////////////////////////////////////////////////////////////////////

template<>
bool AVFromString(const QString& str, quint64& arg)
{
    bool ok = false;
    arg =  str.toULongLong(&ok);
    return ok;
}

///////////////////////////////////////////////////////////////////////////////

template<>
bool AVFromString(const QString& str, qint64& arg)
{
    bool ok = false;
    arg =  str.toLongLong(&ok);
    return ok;
}

///////////////////////////////////////////////////////////////////////////////

template<> bool AVFromString(const QString& str, float& arg)
{
    bool ok = false;
    arg = str.toFloat(&ok);
    return ok;
}

///////////////////////////////////////////////////////////////////////////////

template<> bool AVFromString(const QString& str, double& arg)
{
    bool ok = false;
    arg = str.toDouble(&ok);
    return ok;
}

///////////////////////////////////////////////////////////////////////////////

template<> bool AVFromString(const QString& str, long& arg)
{
    bool ok = false;
    arg = str.toLong(&ok);
    return ok;
}

///////////////////////////////////////////////////////////////////////////////

template<> bool AVFromString(const QString& str, unsigned long& arg)
{
    bool ok = false;
    arg = str.toULong(&ok);
    return ok;
}

///////////////////////////////////////////////////////////////////////////////
// ------------- Qt version specific implementation goes below this line ---------------------
///////////////////////////////////////////////////////////////////////////////

template<>
QString AVToString(const QVector2D& arg, bool enable_escape)
{
    QString result = avfromtostring::DELIMITER_START;
    result += AVToString(arg.x(), enable_escape);
    result += QString(avfromtostring::SEPARATOR);
    result += " ";
    result += AVToString(arg.y(), enable_escape);
    result += avfromtostring::DELIMITER_END;
    return result;
}

///////////////////////////////////////////////////////////////////////////////

template<>
bool AVFromString(const QString& str, QVector2D& arg)
{
    QStringList entries;
    if (!avfromtostring::nestedSplit(entries, avfromtostring::SEPARATOR, str, true)) return false;

    if (entries.count() != 2) return false;

    double x = 0;
    if (!AVFromString(entries.front(), x)) return false;

    entries.pop_front();

    double y = 0;
    if (!AVFromString(entries.front(), y)) return false;

    arg.setX(x);
    arg.setY(y);

    return true;
}

///////////////////////////////////////////////////////////////////////////////

template<>
QString AVToString(const QVector3D& arg, bool enable_escape)
{
    QString result = avfromtostring::DELIMITER_START;
    result += AVToString(arg.x(), enable_escape);
    result += QString(avfromtostring::SEPARATOR);
    result += " ";
    result += AVToString(arg.y(), enable_escape);
    result += QString(avfromtostring::SEPARATOR);
    result += " ";
    result += AVToString(arg.z(), enable_escape);
    result += avfromtostring::DELIMITER_END;
    return result;
}

///////////////////////////////////////////////////////////////////////////////

template<>
bool AVFromString(const QString& str, QVector3D& arg)
{
    QStringList entries;
    if (!avfromtostring::nestedSplit(entries, avfromtostring::SEPARATOR, str, true)) return false;

    if (entries.count() != 3) return false;

    double x = 0;
    if (!AVFromString(entries.front(), x)) return false;

    entries.pop_front();

    double y = 0;
    if (!AVFromString(entries.front(), y)) return false;

    entries.pop_front();

    double z = 0;
    if (!AVFromString(entries.front(), z)) return false;

    arg.setX(x);
    arg.setY(y);
    arg.setZ(z);

    return true;
}

// End of file
