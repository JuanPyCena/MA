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
    \brief   Classes to handle old config params with the AVConfig2 implementation.
 */

#if !defined AVCONFIG2CLASSIC_H_INCLUDED
#define AVCONFIG2CLASSIC_H_INCLUDED

#include <qvector.h>

#include "avlib_export.h"
#include "avconfig2.h"
#include "avconfig.h" // TODO CM QTP-10

///////////////////////////////////////////////////////////////////////////////
/**
 *  TODO CM docs
 */
class AVLIB_EXPORT AVConfig2ImporterClassic : public AVConfig2ImporterBase
{
public:
    //! TODO CM fix comment below
    //! \param config_name The name of the config file, without the "cfg" extension, located in
    //!                    $APP_CONFIG. This will be used as parameter name prefix ("config_name.")
    explicit AVConfig2ImporterClassic(const AVConfig2StorageLocation& location);
    ~AVConfig2ImporterClassic() override;

    void import() override;

    static const QString DEFAULT_PARASET_NAME;

private:

    std::unique_ptr<AVConfig2RestrictionBase> buildRestriction(const AVConfigEntry& entry) const;
    //! \return The AVConfig2Container type name for the given "old" variant type.
    //!         Will do a fatal if the type is unknown to the config system.
    QString variantTypeToString(AVCVariant::Type type) const;

    //! Every loaded parameter is prefixed with the name of the config file for tranistion purposes.
    //! This prefix can differ from the config name if config file mappings are used...
    QString m_param_name_prefix;
};

///////////////////////////////////////////////////////////////////////////////
/**
 *  TODO CM docs
 */
class AVLIB_EXPORT AVConfig2ImporterClassicMapping : public AVConfig2ImporterBase
{
public:

    explicit AVConfig2ImporterClassicMapping(const AVConfig2StorageLocation& location);
    ~AVConfig2ImporterClassicMapping() override;

    //! This duplicates functionality from AVConfigBase::parseConfigFileMappings.
    void import() override;
};

///////////////////////////////////////////////////////////////////////////////
/**
 *  This class is responsible for producing output in the old config format.
 */
class AVLIB_EXPORT AVConfig2ExporterClassic : public AVConfig2ExporterBase
{
public:

    explicit AVConfig2ExporterClassic(const QString& name) : m_config_name(name) {}
    ~AVConfig2ExporterClassic() override {}

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
    void addInheritedSection(const QString&, const QString&, const AVConfig2StorageLocation&) override {
    }  // TODO CM review
    void addSubconfigTemplateParameter(const QString& value, const AVConfig2Metadata& metadata,
                                       const AVConfig2StorageLocation& location) override;

    QString doExport() override;
    void    reset() override;
    bool    removeParameter(const QString&                                          full_name,
                            const AVConfig2StorageLocation::FactConditionContainer& facts) override;

private:

    struct ParamOrReference
    {
        ParamOrReference() : m_is_reference(false) {}
        ParamOrReference(
                const QString& value, const AVConfig2Metadata& metadata, bool is_reference);
        QString m_value;
        AVConfig2Metadata m_metadata;
        bool m_is_reference;
    };
    //! Stores value and metadata
    typedef QVector<ParamOrReference> ParamList;
    //! Maps section name to parameter list for this section.
    //! Vector of pairs instead of map so order is preserved.
    typedef QVector<QPair<QString,  ParamList> > ParamContainer;

    void add(const QString& value, const AVConfig2Metadata& metadata, bool ref);

    QString toString(const QString& value, const AVConfig2Metadata& metadata, bool is_ref) const;
    //! \return The string representation of the given parameter in the classic config format.
    template<typename T> QString toString(
            const QString& value, const AVConfig2Metadata& metadata, bool is_ref,
            T *min = 0, T *max = 0) const;
    template<typename T> QString toStringWithRestriction(
            const QString& value, const AVConfig2Metadata& metadata, bool is_ref) const;

    //! In the classic format, all params must reside within a section. Also, params within the default section
    //! implicitly are put in a section which is named after the config file. When saving back, try not to change the
    //! existing config files if possible -> take into account file name and number of subsections.
    //!
    //! \param full_name The full parameter name with all leading sections
    //! \param section   Will be set to the section to store the param in.
    //! \param name      Will be set to the remaining param name.
    void splitSectionAndName(const QString& full_name, QString& section, QString& name);

    AVConfig2ExporterClassic::ParamList& getOrCreateParamList(const QString& section);

    //! The name of the config as in AVConfig2StorageLocation::m_name.
    QString m_config_name;
    ParamContainer m_param;
};


///////////////////////////////////////////////////////////////////////////////
/**
 *  This class is responsible for producing output in the old config mapping format.
 *  This is achieved by using the AVConfigEntry::write method.
 */
class AVLIB_EXPORT AVConfig2ExporterClassicMapping : public AVConfig2ExporterBase
{
public:
    AVConfig2ExporterClassicMapping();
    ~AVConfig2ExporterClassicMapping() override;

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
    void addInheritedSection(const QString&, const QString&, const AVConfig2StorageLocation&) override {
    }  // TODO CM review
    void addSubconfigTemplateParameter(const QString& value, const AVConfig2Metadata& metadata,
                                       const AVConfig2StorageLocation& location) override;

    QString doExport() override;
    void    reset() override;

private:
    uint m_longest_from;

    typedef QMap<QString, LoadedMapping> MappingContainer;
    MappingContainer m_mappings;

    typedef std::set<QString> IncludeContainer;
    IncludeContainer m_includes;
};

///////////////////////////////////////////////////////////////////////////////
/**
 *  This class facilitates writing config classes used by both AVConfig and AVConfig2 processes.
 */
class AVLIB_EXPORT AVConfig2HybridConfig : public AVConfig2, public AVConfigBase
{
public:
    //! \param container Only relevant if the subclass wants to have control over the container
    //!                  used. If this is null, either the global container singleton or m_dummy_container is used,
    //!                  depending on AVConfigBase::process_uses_avconfig2.
    AVConfig2HybridConfig(const QString& help_group, const QString& prefix,
                          AVConfig2Container *container = 0);

    //! Use this method to determine whether to use old or new registration calls for this particular config.
    /**
     *  Don't use AVConfigBase::process_uses_avconfig2 to avoid problems with configs loaded via a
     *  AVConfig2Container - this is possible even if the process itself does not use the new config.
     */
    bool isNewConfigUsed() const;

protected:

    //! Do nothing. \see LastConfig.
    void exitApp(int) override {}

    //! Used in classic AVConfig processes.
    static AVConfig2Container m_dummy_container;

};

///////////////////////////////////////////////////////////////////////////////

template<typename T>
QString AVConfig2ExporterClassic::toString(
        const QString& value, const AVConfig2Metadata& metadata, bool is_ref, T *min, T *max) const
{
    AVASSERT(metadata.m_type == AVConfig2Types::getTypeName<T>());

    T val;
    T default_val = AVConfig2Types::getInitValue<T>(); // satisfy clazy
    bool default_ok = !metadata.m_default_value.isNull() &&
                      AVFromString(metadata.m_default_value, default_val);

    AVConfigEntry entry(
            qPrintable(metadata.m_name), &val, default_val, qPrintable(metadata.getHelp()),
            qPrintable(metadata.m_command_line_switch));

    if (!default_ok) entry.invalidateDefault();
    if (is_ref)
    {
        entry.reference = value;
    } else
    {
        // This must happen after creating the entry, because there the default value is adopted.
        if (!AVFromString(value, val))
        {
            AVLogFatal << "AVConfig2ExporterClassic::toString: Failed to convert " << value
                       << " to " << metadata.m_type << " for " << metadata.m_name;
        }
    }

    if (min != 0)
    {
        AVCVariant min_variant(min);
        entry.setMin(min_variant);
    }
    if (max != 0)
    {
        AVCVariant max_variant(max);
        entry.setMax(max_variant);
    }

    QString ret;
    QTextStream stream(&ret, QIODevice::WriteOnly);
    entry.write(stream);

    return ret;
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
QString AVConfig2ExporterClassic::toStringWithRestriction(
        const QString& value, const AVConfig2Metadata& metadata, bool is_ref) const
{
    T min, max;
    T *minp = 0;
    T *maxp = 0;

    const AVConfig2RangeRestriction<T> *range_restriction =
            dynamic_cast<const AVConfig2RangeRestriction<T>*>(metadata.m_restriction.get());
    if (range_restriction != 0)
    {
        if (range_restriction->getMin(min)) minp = &min;
        if (range_restriction->getMax(max)) maxp = &max;
    } else if (metadata.m_restriction != 0)
    {
        AVLogger->Write(
            LOG_FATAL, "AVConfig2BuilderClassic::toString: "
            "%s has a restriction which cannot be represented with the old config format.",
            qPrintable(metadata.m_name));
    }

    return toString(value, metadata, is_ref, minp, maxp);
}


#endif

// End of file
