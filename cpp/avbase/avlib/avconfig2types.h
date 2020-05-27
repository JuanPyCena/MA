///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2011
//
// Module:     AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Christian Muschick, c.muschick@avibit.com
    \author  QT4-PORT: Christian Muschick, c.muschick@avibit.com
    \brief   Defines types known to the config system
 */

#if !defined AVCONFIG2TYPES_H_INCLUDED
#define AVCONFIG2TYPES_H_INCLUDED

#include <QString>
#include <QMap>
#include <QHash>
#include <QRegularExpression>
#include <QVector2D>
#include <QVector3D>
#include <QFont>
#include <QPolygon>
#include <QColor>
#include <QPointF>
#include <QBitArray>
#include <QTransform>

#include "avlib_export.h"
#include "avfromtostring.h"
#include "avpath.h"
#include "avmatrix.h"
#include "avdatetime.h"

///////////////////////////////////////////////////////////////////////////////
/**
 *  This is helper datatype to handle e.g. "-help" or "-save".
 *  It is used like a boolean, but is set to true even if read from an empty string.
 *  This makes it possible to specify all of "-help", "-help true", "-help 0" etc.
 */
class AVLIB_EXPORT CmdlineSwitch
{
public:
    CmdlineSwitch() : m_value(false) {}
    //parasoft suppress item CODSTA-CPP-04
    CmdlineSwitch(bool b) : m_value(b) {}
    //parasoft on
    //parasoft suppress item CODSTA-CPP-05
    operator bool() const { return m_value; }
    //parasoft on
    void enable() { m_value = true; }
private:
    bool m_value;
};
// TODO CM move those to .cpp file
template<> inline QString AVToString(
        const CmdlineSwitch& arg, bool enable_escape)
{
    return AVToString(static_cast<bool>(arg), enable_escape);
}

template<> inline bool AVFromString(const QString& str, CmdlineSwitch& arg)
{
    if (str.isEmpty())
    {
        arg.enable();
        return true;
    } else
    {
        bool b;
        if (!AVFromString(str, b)) return false;
        if (b) arg.enable();
        return true;
    }
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  This class can be used like an integer, but the text representation is hexadecimal.
 */
class AVLIB_EXPORT HexInt
{
public:
    HexInt() : m_value(0) {}
    //parasoft suppress item CODSTA-CPP-04
    HexInt(int i) : m_value(i) {}
    //parasoft on
    //parasoft suppress item CODSTA-CPP-05
    operator int() const { return m_value; }
    //parasoft on
private:
    int m_value;
};

template<> inline QString AVToString(const HexInt& arg, bool enable_escape)
{
    Q_UNUSED(enable_escape);
    QString ret = QString("0x%1").arg(arg, 0, 16);
    return ret;
}

template<> inline bool AVFromString(const QString& str, HexInt& arg)
{
    int i;
    if (!AVFromString(str, i)) return false;
    arg = i;
    return true;
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  Template-based conversion of types to their string representation. All types known
 *  to the config system must specialize the getTypeName method.
 */
class AVLIB_EXPORT AVConfig2Types
{
 public:

    //! This method is used to obtain a string name for the arbitrary data types known to the config
    //! system. For technical reasons (some specializations require two template parameters),
    //! it instantiates an object and delegates the work to a helper method (is there a more elegant
    //! solution?)
    //!
    //! \return QString::null for types not known to the config system, a name for the type
    //!         otherwise.
    template<typename T>
    static QString getTypeName()
    {
        T dummy;
        return getTypeName(dummy);
    }

    //! This method exists because of PTS#4918. For primitive values which are not initialized by
    //! their constructor, an initial value can be specified in a template specialization.
    //!
    //! The initial value is directly assigned at parameter
    //! registration time. Note that this value is completely meaningless for the client
    //! application and should never be accessed if the program works correctly.
    template<typename T>
    static T getInitValue()
    {
        return T();
    }

 private:

    //! Helper method to check for nested unknown types. See SWE-4447.
    static QString checkUnknown(const QStringList& args)
    {
        if (args.contains(QString::null)) return QString::null;
        return args.join("");
    }

    //! This method needs to be specialized for all types known to the config system.
    //! Note that it returns QString::null for types not known to the config system, always check
    //! for this case when using this method.
    template<typename T>
    static QString getTypeName(const T& dummy)
    {
        Q_UNUSED(dummy);
        static_assert(std::is_enum<T>::value, "Type is not supported by AVConfig2");
        return std::is_enum<T>::value ? QStringLiteral("QString") : QString::null;
    }

    //! Handle arbitrary QVectors
    template<typename T>
    static QString getTypeName(const QVector<T>& dummy)
    {
        Q_UNUSED(dummy);
        T t;
        return checkUnknown(QStringList() << "QVector<" << getTypeName(t) << " >");
    }

    //! Handle arbitrary QLists
    template<typename T>
    static QString getTypeName(const QList<T>& dummy)
    {
        Q_UNUSED(dummy);
        T t;
        return checkUnknown(QStringList() << "QList<" << getTypeName(t) << " >");
    }

    //! Handle arbitrary QSets
    template<typename T>
    static QString getTypeName(const QSet<T>& dummy)
    {
        Q_UNUSED(dummy);
        T t;
        return checkUnknown(QStringList() << "QSet<" << getTypeName(t) << " >");
    }

    //! Handle arbitrary QMaps
    template<typename T1, typename T2>
    static QString getTypeName(const QMap<T1, T2>& dummy)
    {
        Q_UNUSED(dummy);
        T1 t1;
        T2 t2;
        return checkUnknown(QStringList() << "QMap<" << getTypeName(t1) << ", " << getTypeName(t2) << " >");
    }

    //! Handle arbitrary QHashs
    template<typename T1, typename T2>
    static QString getTypeName(const QHash<T1, T2>& dummy)
    {
        Q_UNUSED(dummy);
        T1 t1;
        T2 t2;
        return checkUnknown(QStringList() << "QHash<" << getTypeName(t1) << ", " << getTypeName(t2) << " >");
    }

    //! Handle arbitrary QPairs
    template<typename T1, typename T2>
    static QString getTypeName(const QPair<T1, T2>& dummy)
    {
        Q_UNUSED(dummy);
        T1 t1;
        T2 t2;
        return checkUnknown(QStringList() << "QPair<" << getTypeName(t1) << ", " << getTypeName(t2) << " >");
    }

    //! Handle arbitrary AVMatrices
    template<typename T>
    static QString getTypeName(const AVMatrix<T>& dummy)
    {
        Q_UNUSED(dummy);
        T t;
        return checkUnknown(QStringList() << "AVMatrix<" << getTypeName(t) << " >");
    }
};

//! Handle AVDateTime
//! Note that we use the name QDateTime here, because we want to shield the configuration from Q<->AV technicalities
template<> inline QString AVConfig2Types::\
getTypeName(const AVDateTime& dummy)
{ Q_UNUSED(dummy); return "QDateTime"; }

// Handle simple data types
#define AVCONFIG2_REGISTER_TYPE(T)        \
template<> inline QString AVConfig2Types::\
getTypeName(const T& dummy) { Q_UNUSED(dummy); return #T; }

// Types which are not initialized by their constructor should use this macro to achieve
// proper initialization in the config class constructors.
#define AVCONFIG2_REGISTER_PRIMITIVE_TYPE(T, INIT_VALUE) \
AVCONFIG2_REGISTER_TYPE(T); \
template<> inline T AVConfig2Types::getInitValue() { return INIT_VALUE; }

AVCONFIG2_REGISTER_PRIMITIVE_TYPE(bool,   false);
AVCONFIG2_REGISTER_PRIMITIVE_TYPE(uint,   0);
AVCONFIG2_REGISTER_PRIMITIVE_TYPE(quint8, 0);
AVCONFIG2_REGISTER_PRIMITIVE_TYPE(qint8,  0);
AVCONFIG2_REGISTER_PRIMITIVE_TYPE(quint16,0);
AVCONFIG2_REGISTER_PRIMITIVE_TYPE(qint16, 0);
AVCONFIG2_REGISTER_PRIMITIVE_TYPE(int,    0);
AVCONFIG2_REGISTER_PRIMITIVE_TYPE(float,  0.0f);
AVCONFIG2_REGISTER_PRIMITIVE_TYPE(double, 0);

AVCONFIG2_REGISTER_TYPE(QString);
AVCONFIG2_REGISTER_TYPE(QStringList);
AVCONFIG2_REGISTER_TYPE(QRegExp); // TODO: replace QRegExp usages with QRegularExpression
AVCONFIG2_REGISTER_TYPE(QRegularExpression);
AVCONFIG2_REGISTER_TYPE(CmdlineSwitch);
AVCONFIG2_REGISTER_TYPE(HexInt);
AVCONFIG2_REGISTER_TYPE(AVEnvString);
AVCONFIG2_REGISTER_TYPE(AVPath);
AVCONFIG2_REGISTER_TYPE(QVector2D);
AVCONFIG2_REGISTER_TYPE(QVector3D);
AVCONFIG2_REGISTER_TYPE(QSize);
AVCONFIG2_REGISTER_TYPE(QFont);
AVCONFIG2_REGISTER_TYPE(QPolygon);
AVCONFIG2_REGISTER_TYPE(QPolygonF);
AVCONFIG2_REGISTER_TYPE(QColor);
AVCONFIG2_REGISTER_TYPE(QRect);
AVCONFIG2_REGISTER_TYPE(QPoint);
AVCONFIG2_REGISTER_TYPE(QPointF);
AVCONFIG2_REGISTER_TYPE(QDateTime);
AVCONFIG2_REGISTER_TYPE(QBitArray);
AVCONFIG2_REGISTER_TYPE(QTransform);

#endif

// End of file
