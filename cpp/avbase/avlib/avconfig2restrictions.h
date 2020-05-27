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
    \brief   Classes to handle restrictions on config parameters.
 */

#if !defined AVCONFIG2RESTRICTIONS_H_INCLUDED
#define AVCONFIG2RESTRICTIONS_H_INCLUDED

#include "avlib_export.h"
#include "avconfig2types.h"
#include "avlog.h"

// Qt includes
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QStringBuilder>

///////////////////////////////////////////////////////////////////////////////
/**
 *   TODO CM doc
 */
class AVLIB_EXPORT AVConfig2RestrictionBase
{
public:
    AVConfig2RestrictionBase() {}
    virtual ~AVConfig2RestrictionBase() {}

    virtual bool operator==(const AVConfig2RestrictionBase& other) const = 0;
    bool operator!=(const AVConfig2RestrictionBase& other) const { return !operator==(other); }

    virtual AVConfig2RestrictionBase *clone()                      const = 0;

    //! \param param The name of the parameter. This is required only to build a meaningful
    //!              error message in case of failure.
    //! \param value The parameter value to check against the restriction.
    //! \return QString::null if everything is ok, a descriptive error message otherwise.
    virtual QString errorMessage(const QString& param, QString value) const = 0;
    virtual QString toString() const = 0;

    //! \param str The restriction in string representation.
    //! \return true if everything is ok.
    virtual bool fromString(const QString &str) = 0;

protected:

    static const QString VALUE_LIST_RESTRICTION_PREFIX;
    static const QString CONTAINER_VALUE_RESTRICTION_PREFIX;
    static const QString RANGE_RESTRICTION_PREFIX;
    static const QString REGEX_RESTRICTION_PREFIX;

    AVConfig2RestrictionBase(const AVConfig2RestrictionBase&) {}
	AVConfig2RestrictionBase& operator=(const AVConfig2RestrictionBase&) { return *this; } // TODO FLO Check
};

///////////////////////////////////////////////////////////////////////////////

//! This is just a helper class to forbid passing incorrectly typed restrictions to the
//! RegisteredParameter::setRestriction method.
template<typename T>
class AVConfig2TypedRestriction : public AVConfig2RestrictionBase
{
};

///////////////////////////////////////////////////////////////////////////////

//! This class is used to model restrictions for loaded parameters where type information is not
//! available. It simply stores the restriction string (so metadata comparison can be done with
//! registered parameters).
//! Also see AVConfig2Metadata::isEqual(), which compares the string value for restrictions for this
//! reason.
class AVLIB_EXPORT AVConfig2UntypedRestriction : public AVConfig2RestrictionBase
{
public:
    explicit AVConfig2UntypedRestriction(const QString& restriction_string);
    ~AVConfig2UntypedRestriction() override;

    bool                      operator==(const AVConfig2RestrictionBase &other) const override;
    AVConfig2RestrictionBase *clone() const override;

    template<typename T>
    AVConfig2TypedRestriction<T> *toTypedRestriction() const;

    //! It is fatal to call this method.
    QString errorMessage(const QString &param, QString value) const override;
    QString toString() const override;
    bool    fromString(const QString &str) override;

private:
    QString m_restriction_string;
};

///////////////////////////////////////////////////////////////////////////////

template <typename T>
class AVConfig2ValueListRestriction : public AVConfig2TypedRestriction<T>
{
public:
    explicit AVConfig2ValueListRestriction(const QList<T>& valid_values = QList<T>()) :
        m_valid_values(valid_values) {}

    ~AVConfig2ValueListRestriction() override {}

    bool                           operator==(const AVConfig2RestrictionBase &other) const override;
    AVConfig2ValueListRestriction *clone() const override;

    QString errorMessage(const QString &name, QString value) const override;
    QString toString() const override;
    bool    fromString(const QString &str) override;

    void getValidValues(QList<T> &valid_values) const;

private:
    QList<T> m_valid_values;
};

///////////////////////////////////////////////////////////////////////////////

//! Allows to define which values a container ought to contain
/*! When you have a container you can use this class to define
    which values are valid for that container. That means you have
    to define every valid value. The order of these values is not
    taken into consideration.
 */
template <typename T>
class AVConfig2ContainerValueRestriction : public AVConfig2TypedRestriction<T>
{
public:
    explicit AVConfig2ContainerValueRestriction(
            const QList<typename T::value_type>& valid_values) :
        m_valid_values(valid_values) {}

    ~AVConfig2ContainerValueRestriction() override {}

    bool                                operator==(const AVConfig2RestrictionBase &other) const override;
    AVConfig2ContainerValueRestriction *clone() const override;

    QString errorMessage(const QString &name, QString value) const override;
    QString toString() const override;
    bool    fromString(const QString &str) override;

private:
    QList<typename T::value_type> m_valid_values;
};

///////////////////////////////////////////////////////////////////////////////

template<typename T>
class AVConfig2RangeRestriction : public AVConfig2TypedRestriction<T>
{
public:
    AVConfig2RangeRestriction() : m_min(AVConfig2Types::getInitValue<T>()),
                                  m_max(AVConfig2Types::getInitValue<T>()),
                                  m_use_min(false), m_use_max(false) {}
    AVConfig2RangeRestriction(T min, T max) :
        m_min(min), m_max(max), m_use_min(true), m_use_max(true) {}
    ~AVConfig2RangeRestriction() override {}

    bool                       operator==(const AVConfig2RestrictionBase &other) const override;
    AVConfig2RangeRestriction *clone() const override;

    QString errorMessage(const QString &name, QString value) const override;
    QString toString() const override;
    bool    fromString(const QString &str) override;

    void setMin(T min) {m_min = min; m_use_min = true;}
    void setMax(T max) {m_max = max; m_use_max = true;}

    bool getMin(T& min) const { min = m_min; return m_use_min; }
    bool getMax(T& max) const { max = m_max; return m_use_max; }

private:
    T m_min;
    T m_max;
    bool m_use_min;
    bool m_use_max;
};

///////////////////////////////////////////////////////////////////////////////

template <typename T>
class AVConfig2RegexRestriction : public AVConfig2TypedRestriction<T>
{
public:
    explicit AVConfig2RegexRestriction() {}

    virtual ~AVConfig2RegexRestriction() {}

    virtual bool operator==(const AVConfig2RestrictionBase& other)   const;
    virtual AVConfig2RegexRestriction *clone()                       const;

    virtual QString errorMessage(const QString& name, QString value) const;
    virtual QString toString() const;
    virtual bool fromString(const QString &str);

    void setRegexPattern(const QString& regex_pattern);
    const QString &getRegexPattern() const;

private:
    QString m_regex_pattern;
    QRegularExpression m_regex;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
AVConfig2TypedRestriction<T> *AVConfig2UntypedRestriction::toTypedRestriction() const
{
    AVConfig2TypedRestriction<T> *typed_restriction = 0;
    if (m_restriction_string.startsWith(VALUE_LIST_RESTRICTION_PREFIX))
    {
        typed_restriction = new AVConfig2ValueListRestriction<T>();
        AVASSERT(typed_restriction != 0);
    }
    /*
    else if (m_restriction_string.startsWith(CONTAINER_VALUE_RESTRICTION_PREFIX))
    {
        typed_restriction = new AVConfig2ContainerValueRestriction<T>();
        AVASSERT(typed_restriction != 0);
    }
    */
    else if (m_restriction_string.startsWith(RANGE_RESTRICTION_PREFIX))
    {
        typed_restriction = new AVConfig2RangeRestriction<T>();
        AVASSERT(typed_restriction != 0);
    }
    else if (m_restriction_string.startsWith(REGEX_RESTRICTION_PREFIX))
    {
        typed_restriction = new AVConfig2RegexRestriction<T>();
        AVASSERT(typed_restriction != 0);
    }
    else
    {
        AVLogError << "Failed to convert unsupported restriction type";
        return 0;
    }
    if (!typed_restriction->fromString(m_restriction_string))
    {
        AVLogError << "Failed to convert string to typed expression";
        delete typed_restriction;
        return 0;
    }
    return typed_restriction;
}

///////////////////////////////////////////////////////////////////////////////

template <typename T>
bool AVConfig2ValueListRestriction<T>::operator==(const AVConfig2RestrictionBase& other) const
{
    const AVConfig2ValueListRestriction<T> *other_typed =
            dynamic_cast<const AVConfig2ValueListRestriction<T> *>(&other);
    if (other_typed == 0) return false;

    return m_valid_values == other_typed->m_valid_values;
}

///////////////////////////////////////////////////////////////////////////////

template <typename T>
AVConfig2ValueListRestriction<T> *AVConfig2ValueListRestriction<T>::clone() const
{
    return new (LOG_HERE) AVConfig2ValueListRestriction(*this);
}

///////////////////////////////////////////////////////////////////////////////

template <typename T>
QString AVConfig2ValueListRestriction<T>::errorMessage(const QString& name, QString value) const
{
    T typed_value;
    AVFromString(value, typed_value);

    if (m_valid_values.contains(typed_value)) return QString::null;
    else
    {
        QString ret;
        QTextStream out_stream(&ret, QIODevice::WriteOnly);
        out_stream << "Invalid value specified for \"" << name << "\" : " << AVToString(value)
                << ".\nValid values are: ";
        for (int i=0; i<m_valid_values.size(); ++i)
        {
            out_stream << AVToString(m_valid_values[i]);
            if (i == m_valid_values.size()-1) out_stream << ".";
            else out_stream << ", ";
        }
        return ret;
    }
}

///////////////////////////////////////////////////////////////////////////////

template <typename T>
QString AVConfig2ValueListRestriction<T>::toString() const
{
    QString ret = this->VALUE_LIST_RESTRICTION_PREFIX + ": ";
    for (int i=0; i<m_valid_values.size(); ++i)
    {
        ret += AVToString(m_valid_values[i]);
        if (i != m_valid_values.size()-1) ret += ", ";
    }
    return ret;
}

///////////////////////////////////////////////////////////////////////////////

template <typename T>
bool AVConfig2ValueListRestriction<T>::fromString(const QString &str)
{
    QString str_without_prefix = str.mid(this->VALUE_LIST_RESTRICTION_PREFIX.length() + 2);
    QStringList entries;
    if (!avfromtostring::nestedSplit(entries, ",", str_without_prefix, false)) return false;
    m_valid_values.clear();
    for (QStringList::const_iterator entry_iter = entries.begin();
         entry_iter != entries.end(); ++entry_iter)
    {
        T value;
        if (!AVFromString(*entry_iter, value))
            return false;
        m_valid_values.append(value);
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

template <typename T>
void AVConfig2ValueListRestriction<T>::getValidValues(QList<T> &valid_values) const
{
    valid_values.clear();
    valid_values.append(m_valid_values);
}

///////////////////////////////////////////////////////////////////////////////

template <typename T>
bool AVConfig2ContainerValueRestriction<T>::operator==(const AVConfig2RestrictionBase& other) const
{
    const AVConfig2ContainerValueRestriction<T> *other_typed =
            dynamic_cast<const AVConfig2ContainerValueRestriction<T> *>(&other);
    if (other_typed == 0) return false;

    return m_valid_values == other_typed->m_valid_values;
}

///////////////////////////////////////////////////////////////////////////////

template <typename T>
AVConfig2ContainerValueRestriction<T> *AVConfig2ContainerValueRestriction<T>::clone() const
{
    return new (LOG_HERE) AVConfig2ContainerValueRestriction(*this);
}

///////////////////////////////////////////////////////////////////////////////

template <typename T>
QString AVConfig2ContainerValueRestriction<T>::errorMessage(
        const QString& name, QString value) const
{
    T typed_value;
    AVFromString(value, typed_value);

    bool ok = true;

    typename T::const_iterator it;
    typename T::const_iterator itEnd = typed_value.constEnd();
    for (it = typed_value.constBegin(); it != itEnd; ++it)
    {
        if (!m_valid_values.contains(*it))
        {
            ok = false;
            break;
        }
    }

    if (ok) return QString::null;
    else
    {
        QString ret;
        QTextStream out_stream(&ret, QIODevice::WriteOnly);
        out_stream << "Invalid value specified for \"" << name << "\" : " << value
                << ".\nValid values are: ";
        for (int i=0; i<m_valid_values.size(); ++i)
        {
            out_stream << AVToString(m_valid_values[i]);
            if (i == m_valid_values.size()-1) out_stream << ".";
            else out_stream << ", ";
        }
        return ret;
    }
}

///////////////////////////////////////////////////////////////////////////////

template <typename T>
QString AVConfig2ContainerValueRestriction<T>::toString() const
{
    QString ret = this->CONTAINER_VALUE_RESTRICTION_PREFIX + ": ";
    for (int i=0; i<m_valid_values.size(); ++i)
    {
        ret += AVToString(m_valid_values[i]);
        if (i != m_valid_values.size()-1) ret += ", ";
    }
    return ret;
}

///////////////////////////////////////////////////////////////////////////////

template <typename T>
bool AVConfig2ContainerValueRestriction<T>::fromString(const QString &str)
{
    QString str_without_prefix = str.mid(this->CONTAINER_VALUE_RESTRICTION_PREFIX.length() + 2);
    QStringList entries;
    if (!avfromtostring::nestedSplit(entries, ",", str_without_prefix, false)) return false;
    m_valid_values.clear();
    for (QStringList::const_iterator entry_iter = entries.begin();
         entry_iter != entries.end(); ++entry_iter)
    {
        typename T::value_type value;
        if (!AVFromString(*entry_iter, value))
            return false;
        m_valid_values.append(value);
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

template <typename T>
bool AVConfig2RangeRestriction<T>::operator==(const AVConfig2RestrictionBase& other) const
{
    const AVConfig2RangeRestriction<T> *other_typed =
            dynamic_cast<const AVConfig2RangeRestriction<T> *>(&other);
    if (other_typed == 0) return false;

    return m_use_max == other_typed->m_use_max &&
           m_use_min == other_typed->m_use_min &&
           m_max     == other_typed->m_max &&
           m_min     == other_typed->m_min;
}

///////////////////////////////////////////////////////////////////////////////

template <typename T>
AVConfig2RangeRestriction<T> *AVConfig2RangeRestriction<T>::clone() const
{
    return new (LOG_HERE) AVConfig2RangeRestriction(*this);
}

///////////////////////////////////////////////////////////////////////////////

template <typename T>
QString AVConfig2RangeRestriction<T>::errorMessage(const QString& name, QString value) const
{
    AVASSERT(m_use_min || m_use_max);

    T typed_value;
    AVFromString(value, typed_value);

    QString ret;
    QTextStream out_stream(&ret, QIODevice::WriteOnly);
    if (m_use_min && typed_value < m_min)
    {
        out_stream << "Value \"" << value << "\" is smaller than minimum allowed value \""
                   << m_min << "\" for parameter \"" << name << "\".";
    } else if (m_use_max && typed_value > m_max)
    {
        out_stream << "Value \"" << value << "\" is larger than maximum allowed value \""
                   << m_max << "\" for parameter \"" << name << "\".";
    }

    return ret;
}

///////////////////////////////////////////////////////////////////////////////

template <typename T>
QString AVConfig2RangeRestriction<T>::toString() const
{
    QString ret = this->RANGE_RESTRICTION_PREFIX + ": ";
    if (m_use_min)
    {
        ret += "from " + AVToString(m_min);
        if (m_use_max) ret += " ";
    }
    if (m_use_max) ret += "to " + AVToString(m_max);
    return ret;
}

///////////////////////////////////////////////////////////////////////////////

template <typename T>
bool AVConfig2RangeRestriction<T>::fromString(const QString &str)
{
    QString str_without_prefix = str.mid(this->RANGE_RESTRICTION_PREFIX.length() + 2);
    QStringList entries;
    if (!avfromtostring::nestedSplit(entries, " ", str_without_prefix, false)) return false;
    bool ok = false;
    if (entries.size() == 4)
    {
        if (entries[0] != "from" || entries[2] != "to")
            return false;
        if (!AVFromString(entries[1], m_min))
            return false;
        if (!AVFromString(entries[3], m_max))
            return false;
        m_use_min = m_use_max = true;
        ok = true;
    }
    else if (entries.size() == 2)
    {
        if (entries[0] == "from")
        {
            if (!AVFromString(entries[1], m_min))
                return false;
            m_use_min = true;
            m_use_max = false;
            ok = true;
        }
        else if (entries[0] == "to")
        {
            if (!AVFromString(entries[1], m_max))
                return false;
            m_use_min = false;
            m_use_max = true;
            ok = true;
        }
    }
    return ok;
}

///////////////////////////////////////////////////////////////////////////////

template <typename T>
bool AVConfig2RegexRestriction<T>::operator==(const AVConfig2RestrictionBase& other) const
{
    const AVConfig2RegexRestriction<T> *other_typed =
            dynamic_cast<const AVConfig2RegexRestriction<T> *>(&other);
    if (other_typed == 0) return false;

    return m_regex_pattern == other_typed->m_regex_pattern;
}

///////////////////////////////////////////////////////////////////////////////

template <typename T>
AVConfig2RegexRestriction<T> *AVConfig2RegexRestriction<T>::clone() const
{
    return new (LOG_HERE) AVConfig2RegexRestriction(*this);
}

///////////////////////////////////////////////////////////////////////////////

template <typename T>
QString AVConfig2RegexRestriction<T>::errorMessage(const QString& name, QString value) const
{
    QRegularExpressionMatch match = m_regex.match(value);
    if(match.hasMatch()) return QString::null;

    return QStringLiteral("Value '") % value % QStringLiteral("' for parameter '") % name %
           QStringLiteral("' does not match specified regular expression '") %
           m_regex_pattern % QStringLiteral("'!");
}

///////////////////////////////////////////////////////////////////////////////

template <typename T>
QString AVConfig2RegexRestriction<T>::toString() const
{
    return AVConfig2RestrictionBase::REGEX_RESTRICTION_PREFIX % QStringLiteral(": ") % m_regex_pattern;
}

///////////////////////////////////////////////////////////////////////////////

template <typename T>
bool AVConfig2RegexRestriction<T>::fromString(const QString &str)
{
    m_regex_pattern = str.mid(this->REGEX_RESTRICTION_PREFIX.length() + 2);
    m_regex.setPattern(m_regex_pattern);
    return m_regex.isValid();
}

///////////////////////////////////////////////////////////////////////////////

template <typename T>
void AVConfig2RegexRestriction<T>::setRegexPattern(const QString& regex_pattern)
{
    m_regex_pattern = regex_pattern;
    m_regex.setPattern(m_regex_pattern);

    if(!m_regex.isValid())
        AVLogFatal << QStringLiteral("AVConfig2RegexRestriction<T>::setRegexPattern: invalid regex pattern '") << m_regex_pattern
                   << QStringLiteral("' specified!");
}

///////////////////////////////////////////////////////////////////////////////

template <typename T>
const QString &AVConfig2RegexRestriction<T>::getRegexPattern() const
{
    return m_regex_pattern;
}

///////////////////////////////////////////////////////////////////////////////

template<>
QString AVLIB_EXPORT AVToString(const AVConfig2RestrictionBase& arg, bool enable_escape);

#endif

// End of file
