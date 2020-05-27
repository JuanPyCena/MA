///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
// QT4-Port:  avconfig2cstyle.cpp
//
// Module:    AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Christian Muschick, c.muschick@avibit.com
    \brief   Classes to handle config params stored in C-style format
 */

#if !defined AVCONFIG2CSTYLE_H_INCLUDED
#define AVCONFIG2CSTYLE_H_INCLUDED

#include "avlib_export.h"
#include "avconfig2.h"

///////////////////////////////////////////////////////////////////////////////
/**
 *
 */
class AVLIB_EXPORT AVConfig2ImporterCstyle : public AVConfig2ImporterBase
{
public:
    explicit AVConfig2ImporterCstyle(const AVConfig2StorageLocation& location);

    //! used to load the config directly from the given string instead of using a file
    explicit AVConfig2ImporterCstyle(const QString& source_string);

    ~AVConfig2ImporterCstyle() override;
    /*!
     * \brief import client should be set before import. \sa AVConfig2ImporterBase::setClient
     */
    void import() override;

    enum VERSION
    {
        V_UNKNOWN = 0,
        V_1       = 1, //!< Initial version
        V_2,           //!< Replaced "section" with "namespace"
        V_3,           //!< Introduced subconfig templates
        V_CURRENT = V_3
    };

private:

    void readVersion();
    void readSearchPaths();
    void readIncludes();
    void readMappings();
    void readDefines();
    void readOverrides();
    //! Reads in a section or single parameter, including all leading and trailing fact statements.
    void readSectionOrFactOrParam(const QString& parent_section);
    //! Reads in a complete section (SECTION_TOKEN to SECTION_END_TOKEN).
    //! Does not handle leading or trailing fact statements.
    void readSection(const QString& parent_section_name, bool is_subconfig_template);
    //! TODO CM This method reads the format which is output by
    //! AVConfig2StorageLocation::getFactString(); move reading and writing to the same location...
    //!
    //! This method can be called at any time while parsing, and will consume any #if and #endif statements,
    //! modifying the condition stack in m_location.m_fact_conditions accordingly.
    void readFacts(int namespace_level = 0);

    void readParam(const QString& section, bool is_subconfig_template);

    //! Reads subconfig template
    //! \return Name of the subconfig template
    QString readSubconfigTemplate();
    //! Reads subconfig template section or single parameter
    void readSubconfigTemplateSectionOrParam(const QString& parent_section);

    QString readHelp();
    void readOptionalMetadata(AVConfig2Metadata& metadata);

    QString readQuoted(bool optional);

    //! Prints a fatal error containing the line number and currently handled config file.
    void fatalError(const QString& message) const;

    //! Returns the next token from the current line, and cuts it away from the current line
    //! (together with any whitespace).
    //!
    //! \param expected_token If empty, the token is optional and nothing will happen if there are
    //!                       no more tokens. If non-empty, will be used to build a descriptive
    //!                       error message.
    //! \param exact_match    If true, the next token must be exactly expected_token.
    //! \return               The next token from m_cur_line.
    QString nextToken(QString expected_token, bool exact_match=false);
    //! Helper method.
    //! Returns the next token from the given string, and removes it from the string.
    //!
    //! \param line         The line to cut the token from. Must not be QString::null.
    //! \param strip_spaces Whether to strip leading spaces from the remaining line.
    //! \return             The next token, or an empty string if there is no token left.
    static QString nextTokenFromLine(QString& line, bool strip_spaces = true);

    //! Return the next line which is going to be returned by readLine(). Calling this method
    //! multiple times without intermediate calls to readLine() will always return the same value.
    //! Advances m_cur_line_number!
    //! \param strip_spaces Whether to remove leading&trailing whitespace. Most of the time we want
    //!                     it, but for multiline parameter values we don't want to break the
    //!                     formatting.
    QString peekLine(bool strip_spaces = true);
    //! Reads the next interesting line and advances m_cur_line_number accordingly.
    //! m_cur_line will contain the read line or QString::null if EOF is reached.
    //! \param strip_spaces Whether to remove leading&trailing whitespace. Most of the time we want
    //!                     it, but for multiline parameter values we don't want to break the
    //!                     formatting. Note that for technical reasons, it is fatal if the line
    //!                     already was peeked() and strip_spaces is false.
    void readLine(bool strip_spaces = true);

    //! Starting with the contents of m_cur_line, read a value (which potentially spans multiple lines).
    //! Values must be terminated with a ";"
    QString readValue();

    QTextStream m_instream;
    QFile m_file;

    QString m_peeked_line;
    uint m_cur_line_number;
    //! The line currently being processed (missing everything which already was consumed by
    //! nextToken()).
    QString m_cur_line;

    VERSION m_cur_version;

    //! for import from string
    QString m_source_string;

    //! The current namespace nesting depth. Required to track this on fact conditions so they are exported at the correct level again.
    int m_current_namespace_level;
};

///////////////////////////////////////////////////////////////////////////////
/**
 *
 */
class AVLIB_EXPORT AVConfig2ExporterCstyle : public AVConfig2ExporterBase
{
public:

    friend class TstAVConfig2ExporterCstyle;
    friend class AVConfig2Test;

    AVConfig2ExporterCstyle();
    ~AVConfig2ExporterCstyle() override {}

    void addParameter(const QString& value, const AVConfig2Metadata& metadata,
                      const AVConfig2StorageLocation& location) override;
    void addReference(const QString& ref, const AVConfig2Metadata& metadata,
                      const AVConfig2StorageLocation& location) override;
    void addSearchPath(const QString& path, const AVConfig2StorageLocation& location) override;
    void addInclude(const QString& include, const AVConfig2StorageLocation& location) override;
    void addConfigMapping(const QString& map_from, const QString& mapped_dir, const QString& mapped_name,
                          const AVConfig2StorageLocation& location) override;

    void addOverride(const QString& name, const QString& value, const AVConfig2StorageLocation& location) override;

    void addDefine(const QString& name, const QString& value, const AVConfig2StorageLocation& location) override;

    /**
     * This method immediately creates a new section if it doesn't exist yet.
     * Doing so allows saving out empty inherited sections.
     *
     * The exporter never sees inherited parameters if they are not overridden, see AVConfig2StorageLocation::m_name.
     */
    void addInheritedSection(const QString& parent, const QString& section,
                             const AVConfig2StorageLocation& location) override;

    void addSubconfigTemplateParameter(const QString& value, const AVConfig2Metadata& metadata,
                                       const AVConfig2StorageLocation& location) override;

    QString doExport() override;
    void    reset() override;

    //! TODO CM correctly handling facts here would require a bit of refactoring (make traversal recursive for
    //! simplicity?)
    bool removeParameter(const QString&                                          full_name,
                         const AVConfig2StorageLocation::FactConditionContainer& facts) override;

    //! Set pinned version. Check AVConfig2ImporterCstyle::VERSION for available versions.
    //! If version is lower than V_2 or UNKNOWN, then the code asserts.
    //! If version is V_2, then subconfig templates are not going to be saved.
    void setPinnedVersion(int version);

private:

    enum {
        INDENT = 4,
        //! Best formatting for width 80 text consoles; emacs reserves rightmost column.
        MAX_HELP_LENGTH = 79
    };

    //! TODO CM eliminate this type...
	class AVLIB_EXPORT ParamOrReference
    {
    public:
        ParamOrReference();
        ParamOrReference(
                const QString& value, const AVConfig2Metadata& metadata, bool is_ref,
                const AVConfig2StorageLocation& location);

        QString m_value;
        bool m_is_ref;
        AVConfig2Metadata m_metadata;
        AVConfig2StorageLocation m_location;
    };

    //! Used to build the internal represenation before exporting.
	class AVLIB_EXPORT ParameterSection
    {
    public:
        ParameterSection() {}

        //! Maps name to subsection
        typedef QVector<QPair<QString, QSharedPointer<ParameterSection> > > SubsectionContainer;
        SubsectionContainer m_subsection;

        //! Maps name to param
        typedef QVector<QPair<QString, ParamOrReference> > ParamContainer;
        ParamContainer m_param;

    private:
        AVDISABLECOPY(ParameterSection);
    };

    //! Used to build the internal represenation before exporting.
    //! Represents parameters depending on fact conditions.
	class AVLIB_EXPORT ConditionalSection
    {
    public:
        ConditionalSection() {}

        typedef QVector<QPair<AVConfig2StorageLocation::FactCondition,
                     QSharedPointer<ConditionalSection> > > SubConditionContainer;
        SubConditionContainer m_sub_conditions;

        //! The top-level section will never contain parameters, as every parameter must reside
        //! within a named section.
        ParameterSection m_section;

    private:
        AVDISABLECOPY(ConditionalSection);
    };

    class AVLIB_EXPORT SubconfigTemplateParameterSection
    {
    public:
        SubconfigTemplateParameterSection() {}

        //! Maps name to subsection
        typedef QVector<QPair<QString, QSharedPointer<SubconfigTemplateParameterSection> > > SubsectionContainer;
        SubsectionContainer m_subsection;

        //! Maps name to param
        typedef QVector<QPair<QString, LoadedSubconfigTemplateParameter>> ParamContainer;
        ParamContainer m_param;

    private:
        AVDISABLECOPY(SubconfigTemplateParameterSection);
    };

    class AVLIB_EXPORT SubconfigTemplateSection
    {
    public:
        SubconfigTemplateSection(const QString& name) : m_name(name) {}

        QString m_name;

        SubconfigTemplateParameterSection m_section;
    private:
        AVDISABLECOPY(SubconfigTemplateSection);
    };

    void addParamOrReference(const QString& value, const AVConfig2Metadata& metadata,
                             const AVConfig2StorageLocation& location, bool is_ref);

    //! If the passed conditions are empty, directly returns parent. If not, creates/finds the
    //! conditional subsection in parent for the first condition and recurses.
    //!
    //! Note that this way, nesting order is preserved.
    ConditionalSection& getOrCreateConditionalSection(
            AVConfig2StorageLocation::FactConditionContainer conditions,
            ConditionalSection& parent);

    //! If name contains ".", recurses into the created/found subsection of parent, else creates/
    //! finds the subsection in parent and returns it.
    ParameterSection& getOrCreateSection(const QString& name, ParameterSection& parent);

    SubconfigTemplateParameterSection& getOrCreateSubconfigTemplateParameterSection(
            const QString& name, SubconfigTemplateParameterSection& parent);

    QSharedPointer<SubconfigTemplateSection> getOrCreateSubconfigTemplateSection(const QString& name);

    void writeVersion();
    void writeSearchPaths();
    void writeIncludes();
    void writeMappings();
    void writeDefines();
    void writeOverrides();
    void writeSubconfigTemplates();
    void writeSubconfigTemplateParameterSection(const SubconfigTemplateParameterSection& section);
    void writeSubconfigTemplateParam(const QString&, const LoadedSubconfigTemplateParameter& param);
    void writeConditionalSection(const ConditionalSection& section);
    void writeSection(const ParameterSection& section);
    //! \param name We don't want the full name from the metadata.
    //! \param param The parameter or reference to write.
    void writeParam(const QString& name, const ParamOrReference& param);
    void writeMetadata(const AVConfig2Metadata& metadata);
    //! Avoids writing double empty lines.
    void writeLine(const QString& line = "");

    //! Export #if end #endif as required to fulfil the given facts for the next exported element.
    //! Modifies m_fact_condition_stack accordingly.
    //!
    //! Also exports user comments.
    void establishPreconditions(const AVConfig2StorageLocation& location);

    //! Converts a fact condition to the string representation in the config file.
    QString factConditionToString(const AVConfig2StorageLocation::FactCondition& condition) const;

    //! The top-level section contains parameter sections without conditions, and nested sections
    //! with fact conditions.
    ConditionalSection m_top_section;

    typedef QVector<LoadedInclude> IncludeContainer;
    IncludeContainer m_includes;

    typedef QVector<LoadedSearchPath> SearchPathContainer;
    SearchPathContainer m_search_paths;

    typedef QVector<LoadedOverride> OverrideContainer;
    OverrideContainer m_overrides;

    typedef QVector<LoadedDefine> DefineContainer;
    DefineContainer m_defines;

    typedef QVector<LoadedMapping> MappingContainer;
    MappingContainer m_mappings;

    typedef QVector<LoadedInheritance> InheritanceContainer;
    InheritanceContainer m_inheritances;

    typedef QVector<QSharedPointer<SubconfigTemplateSection>> SubconfigTemplateSectionContainer;
    SubconfigTemplateSectionContainer m_subconfig_template_sections;

    //! The currently activated facts.
    AVConfig2StorageLocation::FactConditionContainer m_fact_condition_stack;
    //! The currently exported sections. Required to obtain the full section name for accessing m_inheritances.
    QStringList m_section_name_stack;

    //! If this is != 0, export with a comment prefix ("//").
    uint m_export_commented;
    //! State of current export
    uint m_cur_indent;
    //! The complete exported string is built in here.
    QString m_cur_export;

    //! Used to avoid double empty lines
    bool m_empty_line_written;

    int m_pinned_version;
};

#endif

// End of file
