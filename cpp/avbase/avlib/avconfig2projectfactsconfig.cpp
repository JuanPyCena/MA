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


#include "avconfig2projectfactsconfig.h"

///////////////////////////////////////////////////////////////////////////////

AVConfig2FactConfig::AVConfig2FactConfig(const QString &prefix, AVConfig2Container &container) :
    AVConfig2(prefix, container)
{
    registerParameter("description", &m_description, "Description of the fact in the project context.");
    registerParameter("allowed_values", &m_allowed_values, "A comprehensive list of values this fact can have in this project. If empty all values are valid.");
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2FactConfig::postRefresh()
{
    if (m_description.isEmpty())
    {
        return "No description given for fact " + getConfigMapKey();
    }

    return "";
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2FactConfig::getFactName() const
{
    return AVConfig2::getConfigMapKey();
}

///////////////////////////////////////////////////////////////////////////////

QStringList AVConfig2FactConfig::getAllowedValues() const
{
    return m_allowed_values;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2FactConfig::addAllowedValue(const QString &value)
{
    m_allowed_values.append(value);
}

///////////////////////////////////////////////////////////////////////////////

const QString AVConfig2ProjectFactsConfig::FACTS_CONFIG_NAMESPACE = "project_facts";

///////////////////////////////////////////////////////////////////////////////

AVConfig2ProjectFactsConfig::AVConfig2ProjectFactsConfig(AVConfig2Container &container) :
    AVConfig2("", container)
{
    registerSubconfig(FACTS_CONFIG_NAMESPACE, &m_facts);
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2ProjectFactsConfig *AVConfig2ProjectFactsConfig::createFactsConfig(AVConfig2Container &container)
{
    if (!container.containsLoadedParameterWithPrefix(FACTS_CONFIG_NAMESPACE + ".")) return nullptr;

    AVConfig2ProjectFactsConfig *ret = new AVConfig2ProjectFactsConfig(container);

    return ret;
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2ProjectFactsConfig::checkAndUpdateFact(QString name, QString value)
{
    QStringList allowed_values;
    if (checkAndUpdateHelper(name, QStringList() << value, allowed_values)) return "";

    return "The fact \"" + name + "\" has value \"" + value + "\", which is not described in the project configuration. Allowed values are: "
            + AVToString(allowed_values);
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2ProjectFactsConfig::checkAndUpdateCondition(const AVConfig2StorageLocation &location)
{
    QStringList allowed_values;
    for (const AVConfig2StorageLocation::FactCondition& cur_condition : location.getFactConditions())
    {
        const QString& name       = cur_condition.getFactName();
        const QStringList& values = cur_condition.getValues();

        if (checkAndUpdateHelper(name, values, allowed_values)) continue;

        return "The fact \"" + name + "\" used in " + location.getFullFilePath() + " is checked for values " + AVToString(values)
                + " which are not described in the project configuration. Allowed values are: " + AVToString(allowed_values);
    }
    return "";
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2ProjectFactsConfig::checkAndUpdateHelper(const QString &name, const QStringList &values, QStringList &allowed_values)
{
    allowed_values.clear();
    AVConfig2Map<AVConfig2FactConfig>::iterator it = m_facts.find(name);
    if (it == m_facts.end())
    {
        AVConfig2FactConfig * new_fact_config = new AVConfig2FactConfig(m_prefix + FACTS_CONFIG_NAMESPACE + "." + name, getContainer());
        for (const QString& v : values) new_fact_config->addAllowedValue(v);
        m_facts.insert(name, QSharedPointer<AVConfig2FactConfig>(new_fact_config));
        return false;
    }

    AVConfig2FactConfig& cur_fact_config = *it.value();

    bool ret = true;
    allowed_values = cur_fact_config.getAllowedValues();
    if(allowed_values.isEmpty())
    {
        return true;
    }
    for (const QString& value : values)
    {
        if (!allowed_values.contains(value))
        {
            cur_fact_config.addAllowedValue(value);
            ret = false;
        }
    }
    return ret;
}

// End of file
