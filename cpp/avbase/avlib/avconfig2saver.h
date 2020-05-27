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

#ifndef AVCONFIG2SAVER_INCLUDED
#define AVCONFIG2SAVER_INCLUDED

#include "avlib_export.h"
#include "avconfig2.h"
#include "avconfig2loadeddata.h"

class AVConfig2ExporterFactory;

///////////////////////////////////////////////////////////////////////////////
/**
 *  To be used by processes which directly operate on config files, without config classes which register parameters.
 *  Examples: AviConf, config_tool.
 */
class AVLIB_EXPORT AVConfig2SimpleSaver
{
public:

    //! Whether to keep the parameter order from the existing config file, or
    //! to use the order from application (the default). If there is no existing config file, this will have no
    //! effect (application order is used).
    enum SaveOrder
    {
        SO_LOADED,
        SO_APPLICATION // default
    };

    //! If UPP_KEEP is used (the default for AVConfig2Saver), the target config files are preloaded,
    //! and then updated with the parameters to be saved (=unknown parameters are kept).
    //! If UPP_DISCARD is used (the default for AVConfig2SimpleSaver), the preloading step is skipped and the
    //! config files are overwritten with the new content.
    enum UnknownParameterPolicy
    {
        UPP_KEEP,    // default for AVConfig2Saver
        UPP_DISCARD  // default for AVConfig2SimpleSaver
    };

    //! A default saver saves in config file order, style cformat, and to the directory the parameter was loaded from.
    AVConfig2SimpleSaver();
    virtual ~AVConfig2SimpleSaver();

    void setSaveOrder(SaveOrder order);
    SaveOrder getSaveOrder() const;

    void setUnknownParameterPolicy(UnknownParameterPolicy policy);
    UnknownParameterPolicy getUnknownParameterPolicy() const;

    void setConfigFormat(AVConfig2Format format);
    AVConfig2Format getConfigFormat() const;

    //! \param save_dir QString::null to use the location the param was loaded from.
    void setSaveDir(const QString& save_dir);
    QString getSaveDir() const;

    /**
     * @brief setSaveFilter restricts the elements which are actually saved to files matching this filter.
     *
     * @param filename_filter Matched against AVConfig2StorageLocation::m_name, see documentation there.
     *                        Note that this is applied after the heuristics for new parameters determined the file
     *                        an element should go to, see findStorageLocationForMissingParameter().
     */
    void setFilenameSaveFilter(const QRegularExpression& filename_filter);
    const QRegularExpression& getFilenameSaveFilter() const;
    //! \return True if the parameter should *not* be saved, false otherwise.
    bool isLocationFiltered(const AVConfig2StorageLocation& location) const;

    //! Same as setFilenameSaveFilter, only for full parameter names.
    void setParameterSaveFilter(const QRegularExpression& parameter_filter);
    const QRegularExpression& getParameterSaveFilter() const;
    //! \return True if the parameter should *not* be saved, false otherwise. An empty name is never filtered.
    bool isParameterFiltered(const QString& parameter_name) const;

    //! Set exporter factory for this saver. It will be used to create exporters.
    //! If there is no factory set, then saver uses default AVConfig2ExporterFactory().
    void setExporterFactory(const AVConfig2ExporterFactory& factory);

    void save(const AVConfig2LoadedData &data) const;

protected:

    //! Contains an exporter for each required format and target. See getOrCreateExporter() for details.
    typedef QMap<AVConfig2StorageLocation, QSharedPointer<AVConfig2ExporterBase> > ExporterContainer;

    //! TODO CM split up ref and param
    //!
    //! Save a single parameter in the correct format to the correct target.
    //! Saving is done using an exporter from the given exporter container (which is created if necessary).
    //! Exporters save their content in a later step.
    void saveParam(const QString& value, const AVConfig2Metadata& metadata,
                   const AVConfig2StorageLocation& location, bool is_ref,
                   ExporterContainer &exporters) const;

    void saveElement(const AVConfig2ImporterClient::LoadedElement& element, ExporterContainer &exporters) const;

    /**
     * @brief getOrCreateExporter creates an exporter for the given location, preloads it (if neccessary,
     *                            depends on m_unknown_param_policy) and stores it in the given exporter container.
     *
     * Filtering is implemented here, so if the element is filtered, nullptr is returned.
     *
     * @param location       The location where to save. Checked against m_filename_filter.
     * @param parameter_name Empty if the element to save is not a parameter or if filtering based on parameter name should be disabled;
     *                       the parameter name otherwise. Checked against m_parameter_filter.
     * @param removed_params The list of full parameter names that were removed. These parameters shoudn't be saved to
     *                       the config file.
     * @param exporters      The list of exporters built during the ongoing save() call.
     * @return               nullptr if the element is not to be saved, a pointer to a (potentially newly created)
     *                       element in exporters otherwise.
     */
    AVConfig2ExporterBase *getOrCreateExporter(
            const AVConfig2StorageLocation& location, const QString& parameter_name,
            const QStringList& removed_params, ExporterContainer& exporters) const;

    //! Convenience method when exporter doesn't have the list of removed parameters.
    //! For the descriptions of parameters, check the overloaded method.
    AVConfig2ExporterBase *getOrCreateExporter(
            const AVConfig2StorageLocation& location, const QString& parameter_name, ExporterContainer& exporters) const;

    //! Uses the given exporters (which now hold all parameters to be saved) to actually
    //! save the config.
    //!
    //! TODO CM return value + error handling
    void doSave(ExporterContainer &exporters) const;

    static bool isLocationFilteredHelper(const AVConfig2StorageLocation& location, const QRegularExpression &filter);
    static bool isParameterFilteredHelper(const QString& parameter_name, const QRegularExpression& filter);

private:

    //! Determine where a given entity should be saved.
    AVConfig2StorageLocation getTargetLocation(const AVConfig2StorageLocation& location) const;

    SaveOrder m_save_order;
    UnknownParameterPolicy m_unknown_param_policy;

    //! CF_LAST is used to save to the same format the parameter was loaded from.
    AVConfig2Format m_format;
    QString m_save_dir;

    //! If non-empty, only files matching the regexp are saved. See AVConfig2StorageLocation::m_name.
    QRegularExpression m_filename_filter;
    //! Same as m_filename_filter, but for parameter names.
    QRegularExpression m_parameter_filter;

    const AVConfig2ExporterFactory* m_exporter_factory;
};

///////////////////////////////////////////////////////////////////////////////
/**
 *  This class does the job of saving back the config to the (perhaps existing) config files
 *  when -save is called. In contrast to AVConfig2SimpleSaver, it is designed to be used in conjunction with
 *  config classes which actually register parameters.
 */
class AVLIB_EXPORT AVConfig2Saver : public AVConfig2SimpleSaver
{
public:

    //! Determines whether only the registered parameters are saved back to the file (this is the default
    //! behaviour and used when handling the commandline option "-save"), or whether all loaded parameters are saved
    //! (used e.g. by the AviConf tool).
    //!
    //! If the mode is "SM_ONLY_REGISTERED", the actual values of registered parameters in the AVConfig2
    //! object can be taken into account when saving (see SaveValueMode).
    //! If mode is "SM_ALL_LOADED", the value of any registered parameter is ignored (so this does not work with
    //! SVM_ADOPT_PROCESS_VALUES)
    //!
    //! Note that regardless of this parameter, the UnknownParameterPolicy determines whether configuration files
    //! are preloaded before saving (thus keeping parameters which are not explicitly saved out).
    enum SaveMode
    {
        SM_ONLY_REGISTERED, // default
        SM_ALL_LOADED
    };

    //! Controls whether the values of registered parameters are actually saved out
    //! (SVM_ADOPT_PROCESS_VALUES, useful for tools which are config-aware), or
    //! whether they are ignored (SVM_KEEP_LOADED_VALUES, happens with ordinary "-save")
    //! Only relevant in save mode SM_ONLY_REGISTERED.
    //!
    //! Note that this also controls how missing parameters are dealt with: in SVM_ADOPT_PROCESS_VALUES,
    //! it is assumed that the code already filled parameters with sensible values, and they are
    //! saved as they are. In SVM_KEEP_LOADED_VALUES, suggested values are saved if they exist, otherwise
    //! dead references are stored to indicate that a parameter is missing.
    enum SaveValueMode
    {
        SVM_KEEP_LOADED_VALUES,  // used for ordinary "-save"
        SVM_ADOPT_PROCESS_VALUES // default, for tools
    };

    static const QString NO_SUGGESTION_REF_VALUE;

    explicit AVConfig2Saver();

    //! Note that for technical reasons, SM_ALL_LOADED currently works only with SaveValueMode SVM_KEEP_LOADED_VALUES.
    void setSaveMode(SaveMode mode);
    SaveMode getSaveMode() const;

    void setSaveValueMode(SaveValueMode mode);
    SaveValueMode getSaveValueMode() const;

    //! Should optionals which have the default value be saved?
    void setForceOptionals(bool force);

    //! Save the config.
    //!
    //! Note that we are able to append to a file which already contains arbitrary parameters. This
    //! happens by "preloading" the existing file (see getOrCreateExporter()), then adding our
    //! parameters, and finally saving the merged state.
    //!
    //! Also note that this doesn't save *all* parameters loaded by the process, but only those
    //! which are actually registered. This makes it possible to extract only the parameters used by
    //! a particular process.
    void save(const AVConfig2Container& config) const;

    /**
     * @brief findStorageLocationForMissingParameter takes a guess where to put a parameter which was not
     *                                               present in the loaded config and should be saved now.
     *
     *  Tries to find some other parameter with a prefix identical to the missing parameter, preferring parameters
     *  with a low prefix length (favors "generic" files such as fdp2.cc over "specialized" files such as
     *  fdp2_interfaces.cc)
     *  If it finds any such parameter, returns its format and location. If it fails, the common prefix is
     *  iteratively reduced in length by 1.
     *
     *  If no match can be found at all, the parameter is saved to a file corresponding to the
     *  first part of the param name. If such a file already exists, its format is used, the cstyle
     *  format otherwise.
     *
     *  This is static because it is also used from AVConfig2Container::handleVersionTranslation.
     *
     * @param name             The full parameter name.
     * @param config           The container to search for "sibling" parameters
     * @param filename_filter  Only used for logging. If the resulting storage location is filtered, don't log anything.
     * @param parameter_filter Only used for logging. If the parameter is filtered, don't log anything.
     *
     * @return                The location where to save the parameter.
     */
    static AVConfig2StorageLocation findStorageLocationForMissingParameter(
            const QString& name, const AVConfig2Container& config,
            const QRegularExpression& filename_filter,
            const QRegularExpression& parameter_filter);

private:

    //! Helper method which tries to find a parameter or reference, also with any of the deprecated names registered for
    //! the parameter.
    //!
    //! \param deprecated_name  [out] The deprecated name under which the param or reference was found, or "".
    //! \param deprecated_facts [out] The fact conditions for the found parameter or reference, or an empty container.
    //! \param data                   The loaded avconfig2 data.
    //! \param name                   The ordinary name of the param or references.
    //! \param deprecated_names       The list of deprecated names to check.
    //! \return                       A pointer to the loaded element, or nullptr.
    template<typename T>
    const T * findLoadedEntity(
        QString& deprecated_name, AVConfig2StorageLocation::FactConditionContainer& deprecated_facts,
        const AVConfig2LoadedData& data, const QString& name, const QStringList& deprecated_names) const;

    //! Implements saving for SaveMode SM_ONLY_REGISTERED.
    //! Iterates over all registered parameters and does the right thing dependent on whether it's a reference, an
    //! unknown parameter or an ordinary parameter.
    void saveRegisteredParameters(const AVConfig2Container& config, ExporterContainer &exporters) const;

    //! Implements saving for SaveMode SM_ONLY_REGISTERED.
    //! Saves loaded subconfig template parameters if config format supports it.
    void saveSubconfigTemplateParameters(const AVConfig2Container& config, ExporterContainer &exporters) const;

    //! Saves the versions currently in use by all config instances to a special variable
    //! within the config's namespace.
    void saveVersions(const AVConfig2Container& config, ExporterContainer &exporters) const;

    /**
     * @brief determineSaveValue is a helper method for saving registered parameters.
     *
     * Determines which value to actually save out, based on various criteria (e.g. save value mode, attempt to preserve
     * parameter formatting if the value did not changed).
     *
     * @return The value to save, and whether to save a reference.
     */
    std::tuple<QString, bool> determineSaveValue(
            const AVConfig2Container::RegisteredParameterBase& registered,
            const AVConfig2ImporterClient::LoadedParameter *loaded) const;

    enum class SaveAction
    {
        Skip,
        FindLocationForMissing,
        UseLoadedLocation
    };
    /**
     * @brief determineSaveAction is a helper method for saving registered parameters.
     *
     * Determines whether a parameter should be saved to the location where it was loaded from, whether to
     * use a heuristic to determine its save location (i.e. a parameter which was not previously loaded from a file
     * but should be saved out), or whether to skip saving a parameter (e.g. an optional parameter which still has
     * its default value).
     *
     * @param value      The parameter value which would be saved.
     * @param registered The parameter registration.
     * @param loaded     The loaded parameter (if any). This does not imply that the parameter was loaded from a file,
     *                   see AVConfig2StorageLocation::ParameterSource.
     */
    SaveAction determineSaveAction(
            const QString& value,
            const AVConfig2Container::RegisteredParameterBase& registered,
            const AVConfig2ImporterClient::LoadedParameter *loaded) const;

    SaveMode m_save_mode;
    SaveValueMode m_save_value_mode;

    bool m_force_optionals;
};

///////////////////////////////////////////////////////////////////////////////

template<typename T>
const T *AVConfig2Saver::findLoadedEntity(QString& deprecated_name, AVConfig2StorageLocation::FactConditionContainer& deprecated_facts,
    const AVConfig2LoadedData &data, const QString& name, const QStringList& deprecated_names) const
{
    deprecated_name = "";
    deprecated_facts = AVConfig2StorageLocation::FactConditionContainer();

    const T*ret = data.getElementByName<T>(name);
    if (ret != nullptr) return ret;

    if (deprecated_names.isEmpty()) return 0;

    // Find a deprecated name which is actually used...
    for (QStringList::const_iterator dep_it = deprecated_names.begin();
         dep_it != deprecated_names.end();  ++dep_it)
    {
        ret = data.getElementByName<T>(*dep_it);
        if (ret != nullptr)
        {
            deprecated_name = *dep_it;
            deprecated_facts = ret->getLocation().getFactConditions();
            break;
        }
    }

    return ret;
}

#endif // AVCONFIG2SAVER_INCLUDED

// End of file
