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

#ifndef AVCONFIG2STORAGELOCATION_INCLUDED
#define AVCONFIG2STORAGELOCATION_INCLUDED

#include <QMap>
#include <QPair>
#include <QString>
#include <QStringList>
#include <QVector>

#include "avlib_export.h"
#include "avfromtostring.h"

///////////////////////////////////////////////////////////////////////////////
enum AVConfig2Format
{
    CF_CLASSIC,
    CF_CLASSIC_MAPPING,
    CF_XML,
    CF_CSTYLE,
    CF_LAST
};

///////////////////////////////////////////////////////////////////////////////
/**
 *  Stores the location from which an entity was read or where it is to be saved.
 *  Params with a null filename are ignored when saving (pure command line params, references, inherited parameters)
 */
class AVLIB_EXPORT AVConfig2StorageLocation
{
    friend class TstAVConfig2ExporterCstyle;
public:
    //! Maps fact name to possible values or'ed together.
    //! Note that the empty string also is a possible value; it matches if the fact was not specified at all.
    class AVLIB_EXPORT FactCondition
    {
    public:
        FactCondition(int level, const QString& conditions, const QStringList& values);
        FactCondition(int level = 0) : m_namespace_level(level) {}

        bool operator == (const FactCondition& other_condition) const;

        const QString& getFactName() const;
        void setFactName(const QString& fact_name);

        const QStringList& getValues() const;
        void addValue(const QString& fact_name);
        void setValues(const QStringList& values);

        //! Returns the namespace nesting level.
        int getLevel() const;

    private:
        int m_namespace_level;
        QString m_fact_name;
        QStringList m_possible_values;
    };

    //! The conjunction of multiple fact conditions. We don't use a map here because the nesting
    //! order should be preserved when saving.
    typedef QVector<FactCondition> FactConditionContainer;
    //! Maps fact name to actual fact value.
    typedef QMap<QString, QString> FactContainer;

    //! Order is according to priority.
    /**
     *  The source also implies the available metadata.
     *
     *  TODO CM replace logic dealing with loaded parameters to use this field instead of implicit assumptions
     *          (metadata completeness, etc). Also see LPS_OPTIONAL_DEFAULT.
     *          Always fill in known metadata if it is no longer a decision criterium.
     *
     *  TODO CM use order of enum (priority) to resolve loaded parameter conflicts, instead of explicitly writing
     *          code for that.
     */
    enum ParameterSource
    {
        //! "-" syntax on command line
        PS_CMDLINE_PARAM,
        //! "--" syntax on command line
        PS_CMDLINE_OVERRIDE,
        //! From file, with special dev override syntax
        PS_FILE_DEV_OVERRIDE,
        //! Ordinary parameter loaded from file
        PS_FILE_ORDINARY,
        //! See AVConfig2Container::resolveReferences.
        //! Note that this value is also used for parameters which are a result of an inherited reference (the reference itself has
        //! source PS_INHERITED_PARAMETER in this case).
        PS_RESOLVED_REFERENCE,
        //! Parameter copied when processing an inheritance directive
        PS_INHERITED_PARAMETER,
        //! Default value for pure cmdline param, specified at registration time.
        PS_DEFAULT_PURE,
        //! Default value for optional parameter, specified at registration time. Lowest priority.
        //! TODO CM handle defaults for other parameters identically? Let them be overwritten when they are actually
        //!         loaded? Adapt check for missing parameters accordingly.
        PS_DEFAULT_OPTIONAL,
        //! Default constructor. Currently also used for code-created subconfig template parameters.
        PS_UNKNOWN
    };

    AVConfig2StorageLocation();
    AVConfig2StorageLocation(ParameterSource source);
    //! See member docs for explanation of the parameters.
    AVConfig2StorageLocation(
            const QString& loc, const QString& name, AVConfig2Format format,
            const FactConditionContainer& facts = FactConditionContainer());

    bool operator==(const AVConfig2StorageLocation& other) const;

    //! Needed to use this as key for a map in exporter handling
    //!
    //! Note that this comparison deliberately doesn't compare the facts, so params with
    //! different facts still end up in the same file.
    bool operator<(const AVConfig2StorageLocation& other) const;

    //! Return the absolute path to the config file. Doesn't care whether this file really exists.
    //! If m_name is empty, returns QString::null.
    QString getFullFilePath() const;

    //! Return the absolute path to directory the config file resides in.
    //! Doesn't care whether this directory really exists.
    //! If m_name is empty, returns QString::null.
    QString getFullDirPath() const;

    //! Returns whether all fact conditions relevant for this storage location are met by the
    //! given facts.
    bool areFactsMet(const FactContainer& facts) const;
    //! TODO CM the same format is read by AVConfig2ImporterCstyle::readFacts, move reading and
    //! writing to the same location...
    //!
    //! \return A human readable string representation of the fact condition.
    static QString getFactConditionString(const FactConditionContainer& fact_condition);
    //! Convenience method. Returns the fact condition string for m_fact_conditions.
    QString getFactConditionString() const;

    //! \param extension The extension, including "."
    static AVConfig2Format getConfigFormatFromExtension(const QString& extension);
    static AVConfig2Format getConfigFormatFromString   (const QString& str);
    static QString         configFormatToString     (AVConfig2Format format);
    static QString         getConfigExtension       (AVConfig2Format format);

    //! The facts for which this parameter is relevant. Most times this will be empty ("global"
    //! parameters). Maps fact name to possible values (or'ed together). Also see \ref facts.
    const FactConditionContainer& getFactConditions() const;
    //! Helper method used for importer/exporter code.
    void pushCondition(const FactCondition& condition);
    //! Helper method used for importer/exporter code.
    void popCondition();

    //! Currently, the absolute path of the config base path; in the future this might be used to
    //! designate other locations, e.g. a config server.
    //! If this is empty, the application config directory is used as default when saving (see
    //! AVConfig2Saver::getTargetLocation()).
    //!
    //! Note that if a config file is loaded using an absolute path, this will be the directory containing the config
    //! file.
    QString m_location;
    //! The config file path relative to m_location. No extension for avconfig2 formats, including
    //! the extension for user configs (\see findUserConfig()).
    //!
    //! If this is QString::null, the parameter won't be saved. This is in addition to other checks performed in
    //! AVConfig2SimpleSaver::getOrCreateExporter().
    //!
    //! Note that if a config file is loaded using an absolute path, this will be the name of the file.
    QString m_name;
    //! CF_LAST for user configs (non avconfig formats, also see locateConfig()), the proper
    //! format otherwise.
    AVConfig2Format m_format;

    //! Any comment lines that are encountered prior to the entity. This exists so user comments can be preserved
    //! across "-save" calls.
    QStringList m_user_comments;

    //! Valid only if this parameter has a "parent parameter" via config inheritance.
    //! This does not mean that m_source has to be PS_INHERITED_PARAMETER; this field is valid even
    //! if the parameter is overridden (eg in the config file, via command line or dev overrides).
    //!
    //! This information is used to sync metadata to parent parameters.
    QString m_inherited_parameter_name;

    ParameterSource m_source;

private:
    //! The facts for which this parameter is relevant. Most times this will be empty ("global"
    //! parameters). Maps fact name to possible values (or'ed together). Also see \ref facts.
    FactConditionContainer m_fact_conditions;

    static const QString CONFIG_FORMAT_STRINGS[];
    static const QString CONFIG_EXTENSION_STRINGS[];
};

AVLIB_EXPORT QTextStream& operator <<(QTextStream& stream, const AVConfig2StorageLocation::FactCondition& condition);
template<> bool AVLIB_EXPORT AVFromString(const QString& str, AVConfig2StorageLocation::FactCondition& condition);

#endif // AVCONFIG2STORAGELOCATION_INCLUDED

// End of file
