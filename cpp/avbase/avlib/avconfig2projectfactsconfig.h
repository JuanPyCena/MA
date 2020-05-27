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
             See https://confluence.avibit.com:2233/display/AVC/avconfig2 for more extensive documentation.
 */

#if !defined AVCONFIG2_PROJECT_CONFIG_H_INCLUDED
#define AVCONFIG2_PROJECT_CONFIG_H_INCLUDED

#include "avlib_export.h"
#include "avconfig2.h"

///////////////////////////////////////////////////////////////////////////////
/**
 * @brief The AVConfig2FactConfig class describes a single fact, its possible values and project usage.
 */
class AVLIB_EXPORT AVConfig2FactConfig : public AVConfig2
{
public:
    AVConfig2FactConfig(const QString& prefix, AVConfig2Container& container);

    //! Ensures that facts have a non-empty description.
    QString postRefresh() override;

    QString getFactName() const;
    QStringList getAllowedValues() const;

    void addAllowedValue(const QString& value);

private:
    QString m_description;
    QStringList m_allowed_values;
};

///////////////////////////////////////////////////////////////////////////////
/**
 * @brief The AVConfig2ProjectFactsConfig class holds project-specific fact information.
 *
 * This information is optional, but if a project config has the corresponding namespace "project_facts",
 * a list of all used facts and their possible values, together with an explanation is mandatory
 */
class AVLIB_EXPORT AVConfig2ProjectFactsConfig : public AVConfig2
{
public:

    static const QString FACTS_CONFIG_NAMESPACE;

    AVConfig2ProjectFactsConfig(AVConfig2Container& container);

    /**
     * @brief createFactsConfig is a factory method for the project facts configuration object.
     *
     * Depending on whether the project facts configuration namespace is present in the given container,
     * returns a new'ed config object or nullptr.
     *
     * It is in the responsibility of the caller to call refreshParams on the returned object and ensure
     * that the config actually could be loaded successfully.
     *
     * @param container The container to check for fact configuration parameters.
     * @return          A new`ed object or nullptr. Caller takes ownership.
     */
    static AVConfig2ProjectFactsConfig* createFactsConfig(AVConfig2Container& container);

    /**
     * @brief checkAndUpdateFact checks for a given fact that its value and name are described in the project config.
     *
     * Updates the project config if the fact is not described, so it can be saved out later.
     *
     * @return Empty string if successful, error message otherwise.
     */
    QString checkAndUpdateFact(QString name, QString value);
    //! Same as checkAndUpdateFact, for fact conditions.
    QString checkAndUpdateCondition(const AVConfig2StorageLocation &location);

private:

    /**
     * @brief checkAndUpdateHelper does the actual work of checking and updating facts.
     *
     * @param name                 The name of the fact.
     * @param values               Either the value of the fact during the program invocation, or the list of or'd values in a condition.
     * @param allowed_values [out] If empty, the fact is completely unknown in the project configuration. The list of configured allowed
     *                             values otherwise. This only works because every fact must have at least one valid value.
     *
     * @return True if the project config is OK, false otherwise.
     */
    bool checkAndUpdateHelper(const QString& name, const QStringList& values, QStringList& allowed_values);

    AVConfig2Map<AVConfig2FactConfig> m_facts;
};

#endif

// End of file
