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

#include <memory>
#include "avconfig2builtinparams.h"
#include "avconfig2exporterfactory.h"
#include "avconfig2saver.h"

#include "avenvironment.h"

const QString AVConfig2Saver::NO_SUGGESTION_REF_VALUE = "???";

///////////////////////////////////////////////////////////////////////////////

AVConfig2SimpleSaver::AVConfig2SimpleSaver() :
        m_save_order(SO_APPLICATION),
        m_unknown_param_policy(UPP_DISCARD),
        m_format(CF_CSTYLE),
        m_save_dir(QString::null),
        m_exporter_factory(nullptr)
{

}

///////////////////////////////////////////////////////////////////////////////

AVConfig2SimpleSaver::~AVConfig2SimpleSaver()
{

}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2SimpleSaver::setSaveOrder(SaveOrder order)
{
    m_save_order = order;
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2SimpleSaver::SaveOrder AVConfig2SimpleSaver::getSaveOrder() const
{
    return m_save_order;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2SimpleSaver::setUnknownParameterPolicy(UnknownParameterPolicy policy)
{
    m_unknown_param_policy = policy;
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2SimpleSaver::UnknownParameterPolicy AVConfig2SimpleSaver::getUnknownParameterPolicy() const
{
    return m_unknown_param_policy;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2SimpleSaver::setConfigFormat(AVConfig2Format format)
{
    m_format = format;
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2Format AVConfig2SimpleSaver::getConfigFormat() const
{
    return m_format;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2SimpleSaver::setSaveDir(const QString& save_dir)
{
    m_save_dir = save_dir;
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2SimpleSaver::getSaveDir() const
{
    return m_save_dir;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2SimpleSaver::setFilenameSaveFilter(const QRegularExpression &filename_filter)
{
    m_filename_filter = filename_filter;
}

///////////////////////////////////////////////////////////////////////////////

const QRegularExpression &AVConfig2SimpleSaver::getFilenameSaveFilter() const
{
    return m_filename_filter;
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2SimpleSaver::isLocationFiltered(const AVConfig2StorageLocation &location) const
{
    return isLocationFilteredHelper(location, m_filename_filter);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2SimpleSaver::setParameterSaveFilter(const QRegularExpression &parameter_filter)
{
    m_parameter_filter = parameter_filter;
}

///////////////////////////////////////////////////////////////////////////////

const QRegularExpression &AVConfig2SimpleSaver::getParameterSaveFilter() const
{
    return m_parameter_filter;
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2SimpleSaver::isParameterFiltered(const QString &parameter_name) const
{
    return isParameterFilteredHelper(parameter_name, m_parameter_filter);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2SimpleSaver::setExporterFactory(const AVConfig2ExporterFactory& factory)
{
    m_exporter_factory = &factory;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2SimpleSaver::save(const AVConfig2LoadedData& data) const
{
    ExporterContainer exporters;

    if (m_save_order == SO_LOADED && m_unknown_param_policy == UPP_DISCARD)
    {
        AVLogFatal << "unsupported combination of modes.";
    }

    // Make sure that all config files originally loaded are saved out again, even if there no
    // longer are any contents in that file.
    if (m_unknown_param_policy == UPP_DISCARD)
    {
        for (const AVConfig2StorageLocation& location: data.getLoadedLocations())
        {
            getOrCreateExporter(location, "", exporters);
        }
    }

    for (uint e=0; e < data.getElementTotalCount(); ++e)
    {
        const AVConfig2ImporterClient::LoadedElement& cur_element = data.getElementByTotalIndex(e);
        saveElement(cur_element, exporters);
    }

    doSave(exporters);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2SimpleSaver::saveParam(
        const QString& value, const AVConfig2Metadata& metadata,
        const AVConfig2StorageLocation& location, bool is_ref,
        ExporterContainer &exporters) const
{
    AVConfig2ExporterBase *exporter = getOrCreateExporter(location, metadata.m_name, exporters);
    if (exporter)
    {
        if (is_ref) exporter->addReference(value, metadata, location);
        else        exporter->addParameter(value, metadata, location);
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2SimpleSaver::saveElement(const AVConfig2ImporterClient::LoadedElement &element, ExporterContainer &exporters) const
{
    // the parameter name used for filtering (currently supported for parameters and references only)
    QString parameter_name;
    if (dynamic_cast<const AVConfig2ImporterClient::LoadedParameter*>(&element) != nullptr ||
        dynamic_cast<const AVConfig2ImporterClient::LoadedReference*>(&element) != nullptr)
    {
        parameter_name = element.getName();
    }

    AVConfig2ExporterBase *exporter = getOrCreateExporter(element.getLocation(), parameter_name, exporters);
    if (exporter == nullptr) return;
    element.addToImporterClient(*exporter);
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2ExporterBase *AVConfig2SimpleSaver::getOrCreateExporter(
        const AVConfig2StorageLocation& location, const QString &parameter_name,
        const QStringList& removed_params, ExporterContainer &exporters) const
{
    // Ignore pure command line params and params loaded from references when saving.
    if (location.m_name.isNull()) return 0;
    // Ignore parameters which were created by resolving references
    if (location.m_source == AVConfig2StorageLocation::PS_RESOLVED_REFERENCE) return 0;
    // Ignore inherited parameters
    if (!location.m_inherited_parameter_name.isEmpty()) return 0;

    // skip exporting to filtered files.
    if (isLocationFiltered(location)) return 0;
    // skip exporting filtered parameters.
    if (isParameterFiltered(parameter_name)) return 0;

    // First determine where the parameter is to be saved.
    AVConfig2StorageLocation target_location = getTargetLocation(location);

    // Check whether we already have an exporter...
    ExporterContainer::iterator it = exporters.find(target_location);
    if (it != exporters.end()) return it.value().data();

    // If not, preload data if neccessary
    AVConfig2ExporterBase *ret = nullptr;
    if (m_exporter_factory != nullptr)
    {
        ret = m_exporter_factory->createExporter(target_location.m_format, target_location.m_name);
    }
    else
    {
        ret = AVConfig2ExporterFactory().createExporter(target_location.m_format, target_location.m_name);
    }
    ret->setRemovedParamNames(removed_params);

    QSharedPointer<AVConfig2ExporterBase> exporter_pointer(ret);
    exporters[target_location] = exporter_pointer;
    AVASSERT(exporters.find(target_location) != exporters.end());

    if (m_save_order == SO_LOADED) ret->setOrderPolicy(AVConfig2ExporterBase::OP_REPLACE);
    else                           ret->setOrderPolicy(AVConfig2ExporterBase::OP_REMOVE_AND_APPEND);

    if (m_unknown_param_policy == UPP_KEEP)
    {
        QString full_source_path = target_location.getFullFilePath();
        QFileInfo fi(full_source_path);
        if (fi.exists())
        {
            AVLogInfo << "AVConfig2Saver::saveParam: preloading config from "
                    << full_source_path;
            AVASSERT(AVConfig2ImporterBase::import(ret, target_location));
        } else
        {
            AVLogInfo << "AVConfig2Saver::saveParam: "
                    << full_source_path << " doesn't exist yet, creating new config file.";
        }
    }

    return ret;
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2ExporterBase *AVConfig2SimpleSaver::getOrCreateExporter(
        const AVConfig2StorageLocation& location, const QString &parameter_name, ExporterContainer &exporters) const
{
    return getOrCreateExporter(location, parameter_name, QStringList(), exporters);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2SimpleSaver::doSave(ExporterContainer &exporters) const
{
    for (ExporterContainer::iterator it = exporters.begin();
         it != exporters.end();
         ++it)
    {
        AVConfig2Format        cur_format        = it.key().m_format;
        AVConfig2ExporterBase& cur_exporter      = *(it.value());
        QString                cur_exported_data = cur_exporter.doExport();

        QString complete_target_name = it.key().getFullFilePath();
        QFileInfo fi(complete_target_name);
        QDir dir(fi.absolutePath());
        if (!dir.exists())
        {
            AVLogInfo << "AVConfig2Saver::doSave: " << dir.path()
                      << " doesn't exist yet. Creating it...";
            AVASSERT(makeDirectories(dir.path()));
        }
        QFile f(complete_target_name);
        if (!f.open(QIODevice::WriteOnly))
        {
            AVLogFatal << "AVConfig2Saver::doSave: Failed to open "
                       << complete_target_name
                       << " for writing.";
        } else
        {
            AVLogInfo << "AVConfig2Saver::doSave::save: Saving to "
                      << complete_target_name
                      << " in format "
                      << AVConfig2StorageLocation::configFormatToString(cur_format);
        }

        f.write(cur_exported_data.toUtf8());
    }
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2SimpleSaver::isLocationFilteredHelper(const AVConfig2StorageLocation &location, const QRegularExpression &filter)
{
    if (filter.pattern().isEmpty()) return false;
    return !filter.match(location.m_name).hasMatch();
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2SimpleSaver::isParameterFilteredHelper(const QString &parameter_name, const QRegularExpression &filter)
{
    if (parameter_name.isEmpty()) return false;
    if (filter.pattern().isEmpty()) return false;
    return !filter.match(parameter_name).hasMatch();
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2StorageLocation AVConfig2SimpleSaver::getTargetLocation(
        const AVConfig2StorageLocation& location) const
{
    // TODO CM add handling for multiple config directories to the code below...
    AVASSERT(!location.m_name.isEmpty());

    AVConfig2StorageLocation ret;

    if      (!m_save_dir.isNull())           ret.m_location = m_save_dir;
    else if (!location.m_location.isEmpty()) ret.m_location = location.m_location;
    else                                     ret.m_location = AVEnvironment::getApplicationConfig();

    ret.m_name   = location.m_name;
    if (m_format == CF_LAST) ret.m_format = location.m_format;
    else                     ret.m_format = m_format;

    AVASSERT(ret.m_format != CF_LAST);

    return ret;
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2Saver::AVConfig2Saver() :
    m_save_mode(SM_ONLY_REGISTERED),
    m_save_value_mode(SVM_ADOPT_PROCESS_VALUES),
    m_force_optionals(false)
{
    setUnknownParameterPolicy(UPP_KEEP);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Saver::setSaveMode(SaveMode mode)
{
    m_save_mode = mode;
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2Saver::SaveMode AVConfig2Saver::getSaveMode() const
{
    return m_save_mode;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Saver::setSaveValueMode(AVConfig2Saver::SaveValueMode mode)
{
    m_save_value_mode = mode;
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2Saver::SaveValueMode AVConfig2Saver::getSaveValueMode() const
{
    return m_save_value_mode;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Saver::setForceOptionals(bool force)
{
    m_force_optionals = force;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Saver::save(const AVConfig2Container& config) const
{
    ExporterContainer exporters;

    if (getSaveOrder() == SO_LOADED && getUnknownParameterPolicy() == UPP_DISCARD)
    {
        AVLogFatal << "unsupported combination of modes.";
    }
    if (m_save_value_mode == SVM_ADOPT_PROCESS_VALUES && m_save_mode != SM_ONLY_REGISTERED)
    {
        AVLogFatal << "Saving process values is currently supported only with save mode SM_ONLY_REGISTERED";
    }

    // Preload all originally loaded config files and discard removed parameters
    if (getUnknownParameterPolicy() == UPP_KEEP && !config.m_removed_param_names.empty())
    {
        for (const AVConfig2StorageLocation& location: config.getLoadedData().getLoadedLocations())
        {
            getOrCreateExporter(location, "", config.m_removed_param_names, exporters);
        }
    }

    if (m_save_mode == SM_ONLY_REGISTERED)
    {
        saveRegisteredParameters(config, exporters);
        saveSubconfigTemplateParameters(config, exporters);
    }

    // Now save remaining stuff
    for (uint e=0; e<config.getLoadedData().getElementTotalCount(); ++e)
    {
        const AVConfig2ImporterClient::LoadedElement& cur_element =
                config.getLoadedData().getElementByTotalIndex(e);

        if (m_save_mode == SM_ONLY_REGISTERED)
        {
            if (dynamic_cast<const AVConfig2ImporterClient::LoadedParameter*>(&cur_element) ||
                dynamic_cast<const AVConfig2ImporterClient::LoadedReference*>(&cur_element) ||
                dynamic_cast<const AVConfig2ImporterClient::LoadedSubconfigTemplateParameter*>(&cur_element))
            {
                continue;
            }
        }

        saveElement(cur_element, exporters);
    }

    // Do the actual saving to file
    doSave(exporters);
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2StorageLocation AVConfig2Saver::findStorageLocationForMissingParameter(
        const QString& name, const AVConfig2Container& config, const QRegularExpression &filename_filter, const QRegularExpression &parameter_filter)
{
    QString prefix = name;

    // TODO this code is slow, improve algorithm (don't repeatedly iterate over all parameters).

    for (;;)
    {
        // Reduce prefix length by 1. If that's not possible, save the param to process config file
        // in format cstyle.
        int dot_index = prefix.lastIndexOf(".");
        if (dot_index == -1)
        {
            AVConfig2StorageLocation result(AVConfig2StorageLocation::PS_FILE_ORDINARY);

            // Use first part of parameter name as config file name. Make sure to use any given mapping (see AC-650).
            result.m_name = config.getMappedConfigFile(name.section('.', 0, 0));

            // Make sure that we don't introduce ambiguities e.g. by creating a cc file next to a cfg file...
            AVConfig2StorageLocation existing_location = config.locateConfig(result.m_name, true);
            if (!existing_location.m_name.isNull())
            {
                AVASSERT(existing_location.m_name == result.m_name);
                result = existing_location;
            }

            // User specified save format overrides ambiguity logic above, default for non-avconfig2 processes is
            // cstyle.
            AVConfig2Format user_specified_save_format = CF_CSTYLE;
            // This should also work with non-avconfig2 processes -> check for existence of singleton.
            if (AVConfig2Global::isSingletonInitialized())
            {
                user_specified_save_format = AVConfig2Global::singleton().getSaveFormat();
            }
            if (user_specified_save_format != CF_LAST) result.m_format = user_specified_save_format;

            // No format specified and no existing config file - default to cstyle format
            if (result.m_format == CF_LAST) result.m_format = CF_CSTYLE;

            if (!isLocationFilteredHelper(result, filename_filter) &&
                !isParameterFilteredHelper(name, parameter_filter))
            {
                AVLogWarning << "Could not determine where to put " << name
                             << " - Saving it to "
                             << result.m_name
                             << " in format "
                             << AVConfig2StorageLocation::configFormatToString(result.m_format);
            }

            return result;
        }
        prefix = prefix.left(dot_index);

        // Now search for any parameter with identical prefix. Prefer "shorter" parameters, to avoid parameters ending up in
        // specialized configuration files (such as fdp2 interfaces).
        // TODO CM references probably should be checked here as well...
        AVConfig2StorageLocation best_result;
        int best_result_depth = -1;
        for (uint p=0; p<config.getLoadedData().getElementCount<AVConfig2ImporterClient::LoadedParameter>(); ++p)
        {
            const AVConfig2ImporterClient::LoadedParameter& cur_param =
                    config.getLoadedData().getElementByIndex<AVConfig2ImporterClient::LoadedParameter>(p);

            if (!cur_param.getName().startsWith(prefix + ".")) continue;
            // This check is necessary because pure command line params are added to the loaded
            // params as well, and should be ignored here.
            if (cur_param.getLocation().m_format == CF_LAST) continue;

            int cur_depth = cur_param.getName().split(".").count();
            if (best_result_depth != -1 && cur_depth >= best_result_depth)
            {
                continue;
            }

            best_result_depth = cur_depth;
            best_result       = cur_param.getLocation();
        }

        if (best_result_depth != -1)
        {
            // avoid having to clear incorrect values (user comments, inherited parameter, source etc) and just adopt the values we actually need.
            AVConfig2StorageLocation result(best_result.m_location, best_result.m_name, best_result.m_format, best_result.getFactConditions());
            result.m_format = best_result.m_format;

            if (!isLocationFilteredHelper(result, filename_filter) &&
                !isParameterFilteredHelper(name, parameter_filter))
            {
                AVLogInfo << "Saving missing parameter " << name
                          << " to " << result.getFullFilePath()
                          << " in format "
                          << AVConfig2StorageLocation::configFormatToString(result.m_format);
            }

            return result;
        }
    }
    // should never come here.
    AVASSERT(false);
    return AVConfig2StorageLocation();
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Saver::saveRegisteredParameters(const AVConfig2Container& config, ExporterContainer &exporters) const
{
    // Versions must be saved explicitly because there is no registration for them...
    saveVersions(config, exporters);

    // First iterate over configs objects...
    for (AVConfig2Container::RegisteredParameterContainer::const_iterator it =
            config.m_registered_param.begin();
         it != config.m_registered_param.end();
         ++it)
    {
        const AVConfig2Container::ConfigWithRegisteredParameters& pair = *it;
        const AVConfig2Container::RegisteredParameterList& cur_parameter_list = pair.second;

        // Now iterate over parameter lists themselves...
        for (AVConfig2Container::RegisteredParameterList::const_iterator it2 =
                cur_parameter_list.begin(); it2 != cur_parameter_list.end(); ++it2)
        {
            const AVConfig2Container::RegisteredParameterBase& cur_param_info = **it2;
            // non-const because we want to adopt user comment from loaded metadata
            AVConfig2Metadata        cur_metadata = cur_param_info.getMetadata();
            const QString&           cur_param_name = cur_metadata.m_name;

            if(cur_param_info.isPureCmdlineOption()) continue;
            if (!cur_param_info.isSavingEnabled()) continue;
            if (isParameterFiltered(cur_param_info.getMetadata().m_name)) continue;

            QString deprecated_name_to_remove;
            AVConfig2StorageLocation::FactConditionContainer deprecated_conditions;

            const AVConfig2ImporterClient::LoadedReference * existing_ref =
                findLoadedEntity<AVConfig2ImporterClient::LoadedReference>(deprecated_name_to_remove, deprecated_conditions,
                                 config.getLoadedData(), cur_param_name, cur_param_info.getDeprecatedNames());

            QString value;
            AVConfig2StorageLocation target_location;
            bool ref = false;

            // A parameter can never be ref and value at the same time. However, resolved references
            // are added to m_loaded_param. By handling references first and not proceeding to the
            // loaded params if the param exists as reference, we avoid overwriting the reference
            // with the loaded value.

            if (existing_ref != nullptr)
            {
                // This parameter is a reference. Fix up metadata in config file.
                value                       = existing_ref->m_referenced_param;
                target_location             = existing_ref->getLocation();
                cur_metadata.m_user_comment = existing_ref->m_metadata.m_user_comment;
                ref                         = true;
            } else
            {
                const AVConfig2ImporterClient::LoadedParameter * cur_loaded =
                    findLoadedEntity<AVConfig2ImporterClient::LoadedParameter>(deprecated_name_to_remove, deprecated_conditions,
                                     config.getLoadedData(), cur_param_name, cur_param_info.getDeprecatedNames());

                std::tie(value, ref) = determineSaveValue(cur_param_info, cur_loaded);

                switch (determineSaveAction(value, cur_param_info, cur_loaded))
                {
                case SaveAction::Skip:
                    continue;
                case SaveAction::FindLocationForMissing:
                    target_location = findStorageLocationForMissingParameter(
                                cur_param_name, config, getFilenameSaveFilter(), getParameterSaveFilter());
                    break;
                case SaveAction::UseLoadedLocation:
                    target_location             = cur_loaded->getLocation();
                    // The saved metadata always is the registered metadata -> adopt any user comment
                    cur_metadata.m_user_comment = cur_loaded->getMetadata().m_user_comment;
                    break;
                }

                // Don't allow saving of syntactically invalid parameters
                if (!ref)
                {
                    if (!cur_param_info.isSyntacticallyValid(value))
                    {
                        AVLogError << "AVConfig2Saver::saveRegisteredParameters: failed attempt to save parameter "
                                   << cur_param_name << " with invalid syntax (value is "
                                   << value << ").";
                        value = NO_SUGGESTION_REF_VALUE;
                        ref = true;
                    }
                }

                // Check whether any restriction is fulfilled. If not, log the error but keep the value.
                if (!ref)
                {
                    QString error_msg;
                    const AVConfig2RestrictionBase *cur_restriction = cur_metadata.m_restriction.get();
                    if (cur_restriction != 0)
                    {
                        error_msg = cur_restriction->errorMessage(cur_metadata.m_name, value);
                    }
                    if (error_msg != QString::null)
                    {
                        AVLogError << "AVConfig2Saver::saveRegisteredParameters: attempt to save parameter "
                                   << cur_param_name
                                   << " with mismatching restriction: "
                                   << error_msg;
                    }
                }
            }

            // avoid deprecated parameter handling below if we are not going to save anyway. See SWE-4838.
            AVConfig2ExporterBase *exporter = getOrCreateExporter(target_location, cur_metadata.m_name,
                                                                  config.m_removed_param_names, exporters);
            if (exporter == nullptr) continue;

            if (!deprecated_name_to_remove.isEmpty())
            {
                if (exporter->removeParameter(deprecated_name_to_remove, deprecated_conditions))
                {
                    AVLogInfo << "replaced deprecated parameter " << deprecated_name_to_remove
                              << " with " << cur_metadata.m_name
                              << " in " << target_location.getFullFilePath();
                }
            }

            // Note that saving always happens with the registered metadata, not the loaded one.
            saveParam(value, cur_metadata, target_location, ref, exporters);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Saver::saveSubconfigTemplateParameters(const AVConfig2Container& config, ExporterContainer &exporters) const
{
    using TemplateParameter = AVConfig2ImporterClient::LoadedSubconfigTemplateParameter;
    for (uint i = 0; i < config.m_loaded_data.getElementCount<TemplateParameter>(); ++i)
    {
        const TemplateParameter& subconfig_template_param = config.m_loaded_data.getElementByIndex<TemplateParameter>(i);
        const AVConfig2StorageLocation& location = subconfig_template_param.getLocation();
        const QString& value = subconfig_template_param.getValue();
        const AVConfig2Metadata& metadata = subconfig_template_param.getMetadata();
        if (location.m_source != AVConfig2StorageLocation::PS_UNKNOWN)
        {
            AVConfig2ExporterBase *exporter = getOrCreateExporter(location, QString(), exporters);
            if (exporter != nullptr)
            {
                exporter->addSubconfigTemplateParameter(value, metadata, location);
            }
        } else
        {
            const AVConfig2StorageLocation new_location = findStorageLocationForMissingParameter(
                        subconfig_template_param.getName(), config, getFilenameSaveFilter(), getParameterSaveFilter());
            AVConfig2ExporterBase *exporter = getOrCreateExporter(new_location, QString(), exporters);
            if (exporter != nullptr)
            {
                exporter->addSubconfigTemplateParameter(value, metadata, new_location);
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Saver::saveVersions(const AVConfig2Container& config, ExporterContainer &exporters) const
{
    for (AVConfig2Container::ConfigObjectContainer::const_iterator it =
            config.m_config_object.begin(); it != config.m_config_object.end(); ++it)
    {
        const AVConfig2& cur_config = **it;
        if (cur_config.getVersion() == AVConfig2::NO_VERSION) continue;

        QString param_name = cur_config.getPrefix() + ".avconfig2_class_version";

        const AVConfig2ImporterClient::LoadedParameter* cur_param =
                config.getLoadedData().getElementByName<AVConfig2ImporterClient::LoadedParameter>(param_name);
        AVASSERT(cur_param != nullptr);

        saveParam(cur_param->getValue(), cur_param->getMetadata(), cur_param->getLocation(), false, exporters);
    }
}

///////////////////////////////////////////////////////////////////////////////

std::tuple<QString, bool> AVConfig2Saver::determineSaveValue(
        const AVConfig2Container::RegisteredParameterBase &registered,
        const AVConfig2ImporterClient::LoadedParameter *loaded) const
{
    QString value;
    bool ref = false;

    if (m_save_value_mode == SVM_ADOPT_PROCESS_VALUES)
    {
        if (loaded == nullptr)
        {
            // Loaded value is not available. Save member variable value.
            value = registered.toString();
        } else if (loaded->getOriginalValue() != loaded->getValue())
        {
            // Value was overridden. No known use case exists for saving overriden values, so even when adopting
            // process values, save back loaded value in this case.
            // TODO add mode SVM_ADOPT_PROCESS_AND_OVERRIDDEN_VALUES?
            value = loaded->getOriginalValue();
        } else if (registered.areParamStringsEqual(loaded->getOriginalValue(), registered.toString()))
        {
            // Loaded and member variable params match, save loaded value to preserve formatting.
            value = loaded->getOriginalValue();
        } else
        {
            // save value in member variable otherwise.
            value = registered.toString();
        }
    } else if (loaded == nullptr)
    {
        // This is a missing parameter which is to be saved.
        // If there is a loaded or suggested value, use it.
        value = registered.getMetadata().m_default_value;
        if (value.isNull())
        {
            // Otherwise, check if there is a default reference.
            value = registered.getMetadata().m_suggested_reference;
            ref = true;
        }
        if (value.isNull())
        {
            // As a last resort, save it as reference to "???".
            value = NO_SUGGESTION_REF_VALUE;
            ref = true;
        }
    } else
    {
        // Save back existing loaded value
        value = loaded->getOriginalValue();
    }

    return std::make_tuple(value, ref);
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2Saver::SaveAction AVConfig2Saver::determineSaveAction(
        const QString& value,
        const AVConfig2Container::RegisteredParameterBase &registered,
        const AVConfig2ImporterClient::LoadedParameter *loaded) const
{
    if (loaded == nullptr || loaded->getLocation().m_name.isEmpty())
    {
        if (loaded != nullptr)
        {
            // The parameter is present in loaded parameters, but did not come from a file (and no storage
            // location was set).
            //
            // We want to save this parameter to the file in the following cases:
            // - It is an optional parameter which was overwritten on the command line
            // - It is an optional parameter which was changed in the application, and we want to adopt
            //   the process values
            // - "-save" was used with switch "force_optionals"
            bool overridden_optional = registered.isOptional() &&
                (loaded->getLocation().m_source == AVConfig2StorageLocation::PS_CMDLINE_PARAM ||
                 loaded->getLocation().m_source == AVConfig2StorageLocation::PS_CMDLINE_OVERRIDE);

            bool optional_changed_by_application =
                    registered.isOptional() &&
                    m_save_value_mode == SVM_ADOPT_PROCESS_VALUES &&
                    value != registered.getMetadata().m_default_value;

            bool forced_optional = registered.isOptional() && m_force_optionals;

            if (!overridden_optional && !forced_optional && !optional_changed_by_application)
            {
                return SaveAction::Skip;
            }
        }

        return SaveAction::FindLocationForMissing;
    } else
    {
        // Skipping optional params with matching metadata and matching the default value makes it possible
        // to prune them by writing to a different file.
        // Only skip if the "force_optionals" switch is not used.
        // Ignore metadata mismatch if the metadata wasn't even set (this is the case for parameters for
        // which AVConfig2Container::setStorageLocation() was called)
        if (!m_force_optionals &&
            registered.isOptional() &&
            value == registered.getMetadata().m_default_value &&
            (loaded->getMetadata().isIncomplete() ||
             loaded->getMetadata().isEquivalent(registered.getMetadata(), true)))
        {
            return SaveAction::Skip;
        }

        return SaveAction::UseLoadedLocation;
    }
}

// End of file
