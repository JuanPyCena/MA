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
    \brief   Classes to handle config parameters which can be freely partitioned among config files.
 */

#include "avconfig2storagelocation.h"

#include "avlog.h"

///////////////////////////////////////////////////////////////////////////////

const QString AVConfig2StorageLocation::CONFIG_FORMAT_STRINGS[] =
    { "classic", "classic_mapping", "xml", "cstyle" };
const QString AVConfig2StorageLocation::CONFIG_EXTENSION_STRINGS[] =
    { ".cfg", ".config", ".xf2", ".cc" };

///////////////////////////////////////////////////////////////////////////////

AVConfig2StorageLocation::AVConfig2StorageLocation() :
    m_format(CF_LAST),
    m_source(PS_UNKNOWN)
{
    AV_STATIC_ASSERT(sizeof(CONFIG_FORMAT_STRINGS) / sizeof(CONFIG_FORMAT_STRINGS[0]) == CF_LAST);
    AV_STATIC_ASSERT(sizeof(CONFIG_EXTENSION_STRINGS) / sizeof(CONFIG_EXTENSION_STRINGS[0]) ==
            CF_LAST);
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2StorageLocation::AVConfig2StorageLocation(ParameterSource source) :
    m_format(CF_LAST),
    m_source(source)
{
    AV_STATIC_ASSERT(sizeof(CONFIG_FORMAT_STRINGS) / sizeof(CONFIG_FORMAT_STRINGS[0]) == CF_LAST);
    AV_STATIC_ASSERT(sizeof(CONFIG_EXTENSION_STRINGS) / sizeof(CONFIG_EXTENSION_STRINGS[0]) ==
            CF_LAST);
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2StorageLocation::AVConfig2StorageLocation(
        const QString& loc, const QString& name, AVConfig2Format format,
        const FactConditionContainer& facts) :
        m_location(loc),
        m_name(name),
        m_format(format),
        m_source(PS_FILE_ORDINARY),
        m_fact_conditions(facts)
{
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2StorageLocation::operator==(const AVConfig2StorageLocation& other) const
{
    return m_location        == other.m_location &&
           m_name            == other.m_name     &&
           m_format          == other.m_format   &&
           m_fact_conditions == other.getFactConditions();
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2StorageLocation::operator<(const AVConfig2StorageLocation& other) const
{
    if (m_location != other.m_location) return m_location < other.m_location;
    if (m_name     != other.m_name    ) return m_name     < other.m_name;
    else                                return m_format   < other.m_format;
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2StorageLocation::getFullFilePath() const
{
    if (m_name.isEmpty()) return QString::null;

    QString full_path = m_name;
    AVASSERT(QDir::isRelativePath(full_path));

    if (m_format != CF_LAST) full_path += getConfigExtension(m_format);

    full_path = m_location + "/" + full_path;

    return full_path;
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2StorageLocation::getFullDirPath() const
{
    if (m_name.isEmpty()) return QString();

    QFileInfo info(getFullFilePath());
    return info.absolutePath();
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2StorageLocation::areFactsMet(const FactContainer& facts) const
{
    // This is the conjunction of several conditions, so if any condition fails, the facts are not met.
    for (FactConditionContainer::const_iterator cond_it = m_fact_conditions.begin();
         cond_it != m_fact_conditions.end();
         ++cond_it)
    {
        QString     cur_fact_name        = cond_it->getFactName();
        // Facts or'ed together. If any of them matches, this term is satisfied.
        QStringList cur_fact_disjunction = cond_it->getValues();

        FactContainer::const_iterator fact_it = facts.find(cur_fact_name);

        QString cur_fact_value;
        // The empty string is not allowed as fact value and signifies "not specified".
        if (fact_it == facts.end()) cur_fact_value = "";
        else                        cur_fact_value = fact_it.value();

        // Bail if the actual fact doesn't show up in our conditions
        if (!cur_fact_disjunction.contains(cur_fact_value)) return false;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2StorageLocation::getFactConditionString(
        const FactConditionContainer& fact_condition)
{
    QStringList ret;
    for (FactConditionContainer::const_iterator conjunction_it = fact_condition.begin();
            conjunction_it != fact_condition.end(); ++conjunction_it)
    {
        QStringList disjunction;
        const FactCondition& cur_condition = *conjunction_it;
        for (int i=0; i<cur_condition.getValues().size(); ++i)
        {
            QString fact  = cur_condition.getFactName();
            QString value = cur_condition.getValues()[i];
            if (value.isEmpty()) value = "\"\"";
            disjunction.push_back(fact + "==" + value);
        }

        QString disjunction_term = disjunction.join(" || ");
        if (fact_condition.count() > 1 && disjunction.count() > 1)
        {
            disjunction_term = "(" + disjunction_term + ")";
        }
        ret.push_back(disjunction_term);
    }

    return ret.join(" && ");
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2StorageLocation::getFactConditionString() const
{
    return getFactConditionString(m_fact_conditions);
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2Format AVConfig2StorageLocation::getConfigFormatFromExtension(const QString& extension)
{
    AVASSERT(extension.startsWith("."));
    for (uint ret=0; ret<CF_LAST; ++ret)
    {
        if (CONFIG_EXTENSION_STRINGS[ret] == extension) return static_cast<AVConfig2Format>(ret);
    }
    AVLogError << "AVConfig2Container::getConfigFormatFromExtension: unknown extension " << extension;
    return CF_LAST;
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2Format AVConfig2StorageLocation::getConfigFormatFromString(const QString& str)
{
    for (uint ret=0; ret<CF_LAST; ++ret)
    {
        if (CONFIG_FORMAT_STRINGS[ret] == str) return static_cast<AVConfig2Format>(ret);
    }
    AVLogError << "AVConfig2Container::configFormatFromString: invalid format " << str;
    return CF_LAST;
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2StorageLocation::configFormatToString(AVConfig2Format format)
{
    AVASSERT(format < CF_LAST);
    return CONFIG_FORMAT_STRINGS[format];
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2StorageLocation::getConfigExtension(AVConfig2Format format)
{
    AVASSERT(format < CF_LAST);
    return CONFIG_EXTENSION_STRINGS[format];
}

const AVConfig2StorageLocation::FactConditionContainer& AVConfig2StorageLocation::getFactConditions() const
{
    return m_fact_conditions;
}

void AVConfig2StorageLocation::pushCondition(const AVConfig2StorageLocation::FactCondition &condition)
{
    m_fact_conditions.push_back(condition);
}

void AVConfig2StorageLocation::popCondition()
{
    m_fact_conditions.pop_back();
}

AVConfig2StorageLocation::FactCondition::FactCondition(int level, const QString &conditions, const QStringList &values)
    : m_namespace_level(level),
      m_fact_name(conditions),
      m_possible_values(values)
{}

bool AVConfig2StorageLocation::FactCondition::operator ==(const AVConfig2StorageLocation::FactCondition &other_condition) const
{
    return (m_namespace_level == other_condition.getLevel() && m_fact_name == other_condition.getFactName() && m_possible_values == other_condition.getValues());
}

const QString& AVConfig2StorageLocation::FactCondition::getFactName() const
{
    return m_fact_name;
}

void AVConfig2StorageLocation::FactCondition::setFactName(const QString &fact_name)
{
    m_fact_name = fact_name;
}

const QStringList& AVConfig2StorageLocation::FactCondition::getValues() const
{
    return m_possible_values;
}

void AVConfig2StorageLocation::FactCondition::addValue(const QString &fact_name)
{
    m_possible_values << fact_name;
}

void AVConfig2StorageLocation::FactCondition::setValues(const QStringList &values)
{
    m_possible_values = values;
}

int AVConfig2StorageLocation::FactCondition::getLevel() const
{
    return m_namespace_level;
}

QTextStream &operator <<(QTextStream &stream, const AVConfig2StorageLocation::FactCondition &condition)
{
    static const QString pattern("condition: %1, values: %2, nesting: %3");

    stream << pattern.arg(condition.getFactName())
                     .arg(condition.getValues().join(", "))
                     .arg(condition.getLevel());
    return stream;
}

template<> bool AVFromString(const QString &str, AVConfig2StorageLocation::FactCondition &fact_condition)
{
    QString condition = fact_condition.getFactName();
    QStringList vals = fact_condition.getValues();

    AVPairFromStringInternal(str, fact_condition, vals, avfromtostring::SEPARATOR);

    fact_condition.setFactName(condition);
    fact_condition.setValues(vals);

    return true;
}

// End of file
