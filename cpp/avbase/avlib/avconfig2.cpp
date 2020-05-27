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


#include "avconfig2.h"
#include "avpackageinfo.h"

#include <memory>
#include <iostream>
#include <set>

// QT includes
#include <QtGlobal>
#include <QDir>
#include <QRegularExpression>

// local includes
#include "avconfig.h" // TODO CM used for application name and config, fix this...
#include "avconfig2cstyle.h"
#include "avconfig2builtinparams.h"
#include "avconfig2projectfactsconfig.h"
#include "avdeprecate.h"
#include "avlog.h"
#include "avthread.h"
#include "avdaemon.h"

///////////////////////////////////////////////////////////////////////////////

const QChar DOT_CHAR('.');

const char* AVConfig2Container::AVCONFIG2_NO_STRICT_CHECKING          = "AVCONFIG2_NO_STRICT_CHECKING";
const char* AVConfig2Container::AVCONFIG2_NO_STRICT_CHECKING_AUTOSAVE = "AUTOSAVE";

///////////////////////////////////////////////////////////////////////////////

AVConfig2Container::AVConfig2Container() :
    m_loaded_data(true),
    m_save_required(false),
    m_checking_mode(CheckingMode::CM_LENIENT)
{
    QString no_strict_env = AVEnvironment::getEnv(AVCONFIG2_NO_STRICT_CHECKING, false, AVEnvironment::NoPrefix);
    if (!no_strict_env.isEmpty())
    {
        if (no_strict_env == "1")
        {
            m_checking_mode = CheckingMode::CM_LENIENT;
        } else if (no_strict_env == "0")
        {
            m_checking_mode = CheckingMode::CM_STRICT;
        } else if (no_strict_env == AVCONFIG2_NO_STRICT_CHECKING_AUTOSAVE)
        {
            m_checking_mode = CheckingMode::CM_AUTOSAVE;
        } else
        {
            // no logger yet...
            std::cout << "** ERROR: Unrecognized value for " << AVCONFIG2_NO_STRICT_CHECKING
                      << ": " << qPrintable(no_strict_env) << " (valid values are 0, 1, "
                      << AVCONFIG2_NO_STRICT_CHECKING_AUTOSAVE << ")\n";
            exit (-1);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2Container::~AVConfig2Container()
{
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Container::parseCmdlineParams(
        int argc, const char* const *argv, bool allow_extra_cmdline)
{
    QStringList args;
    for (int i=1; i<argc; ++i) // omit program name -> start with 1
    {
        args += argv[i];
    }
    parseCmdlineParams(args, allow_extra_cmdline);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Container::parseCmdlineParams(const QStringList& args, bool allow_extra_cmdline)
{
    // Attention: Cannot do logging here, as this happens before logger initialization.
    AVASSERTNOLOGGER(m_loaded_data.getElementCount<LoadedParameter>() == 0);
    AVASSERTNOLOGGER(m_registered_param.isEmpty());

    bool parsing_extra_cmdline = false;
    bool error = false;
    for (int i=0; i<args.count(); ++i)
    {
        CmdLineParamContainer *container = nullptr;
        QString param = args[i];

        bool doubledash_syntax = false;
        if (parsing_extra_cmdline)
        {
            m_extra_cmdline.append(param);
            continue;
        } else if (param == "--")
        {
            if (allow_extra_cmdline)
            {
                // This signifies the start of the extra command line - ignore "--" and switch mode
                parsing_extra_cmdline = true;
                continue;
            } else error = true;
        } else if (param.startsWith("--"))
        {
            container = &m_loaded_cmdline_overrides;
            param = param.right(param.length()-2);
            doubledash_syntax = true;
        } else if (param.startsWith("-") && param.length() > 1)
        {
            // Just ignore the qmljsdebugger cmd line option. It doesn't fit into our syntax and
            // is consumed by the qApp.
            if (param.startsWith("-qmljsdebugger="))
            {
                std::cout << "AVConfig2: Ignoring Qt command line param " << qPrintable(param);
                continue;
            }
            container = &m_loaded_cmdline_param;
            param = param.right(param.length()-1);
        } else
        {
            // Parse error - continue only if extra command line params are allowed.
            if (allow_extra_cmdline)
            {
                m_extra_cmdline.append(param);
                parsing_extra_cmdline = true;
                continue;
            } else error = true;
        }

        if (error)
        {
            std::cout << "** ERROR: Failed to parse command line argument \"" << qPrintable(param) << "\".\n Aborting.\n";
            exit (-1);
        }

        AVASSERTNOLOGGER(container != 0 && !param.isEmpty());

        // Retrieve value from command line.
        // If the current parameter starts with "--" the next parameter is always considered a value.
        // Otherwise, if the next parameter starts with "-", there is no value on the command line (e.g. when
        // using -help), but now we internally have an empty string instead of a null string (also see CmdlineSwitch
        // type).
        QString value = "";
        if (doubledash_syntax && i+1 == args.count())
        {
            std::cout << "** ERROR: Value is mandatory with '--' syntax - failed to parse command line. Aborting.\n";
            exit (-1);
        }
        if (doubledash_syntax || (i+1<args.count() && (args[i+1].isEmpty() || args[i+1][0] != '-')))
        {
            ++i;
            value = args[i];
        }

        CmdLineParamContainer::iterator existing_param_it = container->find(param);
        if (existing_param_it != container->end())
        {
            const QString& existing_value = existing_param_it.value();
            if (value != existing_value)
            {
                std::cout << "** ERROR: \"-" << qPrintable(param)
                          << "\" was specified twice on the command line with different values ("
                          << qPrintable(AVToString(value)) << " vs " << qPrintable(AVToString(existing_value)) << "). Aborting.\n";
                exit (-1);
            }
        } else
        {
            (*container)[param] = value;
        }
    }

    // Immediately adopt "--" overrides into m_loaded_param (because we know their name)
    AVASSERT(m_loaded_data.getElementCount<LoadedParameter>() == 0);
    for (CmdLineParamContainer::const_iterator it = m_loaded_cmdline_overrides.begin();
         it != m_loaded_cmdline_overrides.end(); ++it)
    {
        QString cur_name  = it.key();
        QString cur_value = it.value();
        // Note that this specifies incomplete metadata. Also see AVConfig2Container::checkNewLoadedParam().
        m_loaded_data.addElement(LoadedParameter(
            cur_value, AVConfig2Metadata(cur_name), AVConfig2StorageLocation(AVConfig2StorageLocation::PS_CMDLINE_OVERRIDE)));
    }
}

///////////////////////////////////////////////////////////////////////////////

QStringList AVConfig2Container::getExtraCmdline() const
{
    return m_extra_cmdline;
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2Container::loadConfig(const QString& config, bool optional)
{
    return loadConfigInternal(config, optional, EmptyQStringList);
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2Container::loadConfigAbsolute(const QString& filename, bool optional)
{
    AVASSERT(!QDir::isRelativePath(filename));

    AVConfig2StorageLocation location = locateConfig(filename, true);
    if (location.m_name.isNull())
    {
        if (!optional) m_missing_configs.push_back(filename);
        return false;
    }

    AVASSERT(AVConfig2ImporterBase::import(this, location));
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2Container::loadConfigTree(const QString& directory)
{
    QDir dir(directory);
    dir.setFilter(QDir::Files | QDir::NoSymLinks);
    dir.setSorting(QDir::Name);
    dir.makeAbsolute();

    // fetch names of supported config files
    QStringList all_filenames = dir.entryList(), config_filenames;
    for (QStringList::iterator filename_iter = all_filenames.begin();
            filename_iter != all_filenames.end(); ++filename_iter)
    {
        QString filename = dir.absolutePath() + "/" + *filename_iter;
        for (uint f = 0; f < CF_LAST; ++f)
        {
            AVConfig2Format cur_format = static_cast<AVConfig2Format>(f);
            QString cur_extension = AVConfig2StorageLocation::getConfigExtension(cur_format);
            if (filename.endsWith(cur_extension))
            {
                config_filenames.push_back(filename);
                break;
            }
        }
    }

    // load config files
    for (QStringList::iterator filename_iter = config_filenames.begin();
            filename_iter != config_filenames.end(); ++filename_iter)
    {
        if (!loadConfigAbsolute(*filename_iter))
            return false;

    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2Container::RefreshResult AVConfig2Container::refreshAllParams()
{
    RefreshResult ret;

    // Refresh all configurations, regardless of previous success. See SWE-5537.
    m_unrefreshed = m_config_object;

    // Note that creating subconfigs potentially triggers loading additional configuration files, and resolveReferences
    // will be repeatedly called in refreshParams().
    // Still, call it first independently from any registered configuration to achieve more consistent behaviour.
    resolveReferences(ret);

    // m_unrefreshed will be updated in the process (configs will be removed by refreshing, and possibly are added
    // in subconfig constructors) - just pop configs until we are done.
    auto handle_unrefreshed = [&]()
    {
        while (!m_unrefreshed.isEmpty())
        {
            AVConfig2* cur_unrefreshed = *m_unrefreshed.begin();

            // Refreshing subconfigs will cause configs to be created and destroyed.
            //
            // For newly created subconfigs, refresh is immediately called, see AVConfig2Container::refreshParams.
            // Because of deleted subconfigs, we cannot rely on the pointers in m_unrefreshed still being valid.
            // Using QPointer here would be better but is not done for consistency with QT3.
            if (!m_registered_param.containsConfig(cur_unrefreshed) &&
                 m_registered_subconfig.find(cur_unrefreshed) == m_registered_subconfig.end())
            {
                m_unrefreshed.remove(cur_unrefreshed);
            } else
            {
                if (!refreshParams(cur_unrefreshed, &ret))
                {
                    m_unrefreshed.remove(cur_unrefreshed);
                }
            }
        }
    };
    handle_unrefreshed();

    // TODO CM enable
//    ret.m_loaded_deprecated_names = m_loaded_deprecated_names;

    // Calling postRefresh() possibly creates additional config objects (which then are tracked as unrefreshed).
    // Iterate until nothing remains.
    QSet<AVConfig2*> pending_post_refresh = m_config_object;
    while (!pending_post_refresh.isEmpty())
    {
        if (!ret.refreshSuccessful()) return ret;

        // postRefresh is only called if (currently) no refresh is pending.
        AVASSERT(m_unrefreshed.isEmpty());
        for (AVConfig2* cur_config: pending_post_refresh)
        {
            QString post_refresh_result = cur_config->postRefresh();
            if (!post_refresh_result.isEmpty())
            {
                ret.m_errors << cur_config->getPrefix() + " configuration: " + post_refresh_result;
            }
        }

        pending_post_refresh = m_unrefreshed;
        handle_unrefreshed();
    }

    return ret;
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2Container::findUserConfig(const QString& config, const QString& extension)
{
    AVASSERT(extension.isEmpty() || extension.startsWith("."));

    // Empty configs are a noop.
    if (config.isEmpty()) return config;

    QString key = config;
    if (key.endsWith(extension)) key = key.left(key.length() - extension.length());
    AVConfig2StorageLocation location = locateConfig(getAndFlagMappedConfigFile(key) + extension, false);
    if (location.m_name.isEmpty())
    {
        QString ret = key + extension;
        // Don't make assumptions about how this method is used - only log debug
        AVLogDebug << "AVConfig2Container::findUserConfig: " << ret
                   << " could not be found.";
        return ret;
    }
    return location.getFullFilePath();
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2StorageLocation AVConfig2Container::findConfig(const QString &config)
{
    return resolveConfig(config, {});
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2Container::addFacts(const AVConfig2StorageLocation::FactContainer& facts)
{
    for (AVConfig2StorageLocation::FactContainer::const_iterator it = facts.begin();
            it != facts.end(); ++it)
    {
        const QString& cur_fact_name  = it.key();
        const QString& cur_fact_value = it.value();

        if (m_facts.contains(cur_fact_name))
        {
            return cur_fact_name + " was specified more than once.";
        } if (cur_fact_value.isEmpty() || cur_fact_name.isEmpty())
        {
            // This is not allowed because the empty string signifies "fact not specified", see
            // AVConfig2StorageLocation::areFactsMet
            return "Empty fact name or value specified (" + cur_fact_name + ":" + cur_fact_value + ")";
        } else m_facts[cur_fact_name] = cur_fact_value;
    }

    return QString::null;
}

///////////////////////////////////////////////////////////////////////////////

const AVConfig2StorageLocation::FactContainer& AVConfig2Container::getFacts() const
{
    return m_facts;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Container::dumpConfig() const
{
    QString dump;
    QTextStream dump_stream(&dump, QIODevice::WriteOnly);

    bool printed_include = false;
    for (uint m=0; m<m_loaded_data.getElementCount<LoadedInclude>(); ++m)
    {
        const LoadedInclude& cur_include = m_loaded_data.getElementByIndex<LoadedInclude>(m);
        AVConfig2StorageLocation location = cur_include.getLocation();

        if (!printed_include)
        {
            dump_stream << "-------------------- Includes --------------------\n";
            printed_include = true;
        }
        dumpConfigHelper(dump_stream, cur_include.getName(), "", location);
    }
    if (printed_include) dump_stream << "\n";

    bool printed_mapping = false;
    for (uint m=0; m<m_loaded_data.getElementCount<LoadedMapping>(); ++m)
    {
        const LoadedMapping& cur_mapping = m_loaded_data.getElementByIndex<LoadedMapping>(m);
        QString from = cur_mapping.m_from;
        QString to   = cur_mapping.getFullMapping();
        AVConfig2StorageLocation location = cur_mapping.getLocation();

        if (from == to) continue;

        if (!printed_mapping)
        {
            dump_stream << "-------------------- Config mappings --------------------\n";
            printed_mapping = true;
        }
        dumpConfigHelper(dump_stream, from, to, location);
    }
    if (printed_mapping) dump_stream << "\n";

    dump_stream << "-------------------- Configuration loaded at startup --------------------\n";
    for (uint p=0; p<m_loaded_data.getElementCount<AVConfig2ImporterClient::LoadedParameter>(); ++p)
    {
        const LoadedParameter& cur_loaded = m_loaded_data.getElementByIndex<LoadedParameter>(p);
        const QString&         cur_param  = cur_loaded.getName();
        AVConfig2StorageLocation location = cur_loaded.getLocation();

        // Check whether parameter has been overridden
        // We need the registered metadata and not the loaded one (e.g. pure command line params)
        QString cmdline_switch;
        const RegisteredParameterBase *reg = getRegisteredParameterInfo(cur_param).second;
        if (reg != 0) cmdline_switch = reg->getCommandlineSwitch();

        QString override_text;
        switch (location.m_source)
        {
        case AVConfig2StorageLocation::PS_CMDLINE_PARAM:
            override_text = "overwritten via cmdline switch";
            break;
        case AVConfig2StorageLocation::PS_CMDLINE_OVERRIDE:
            override_text = "overridden on cmdline";
            break;
        case AVConfig2StorageLocation::PS_FILE_DEV_OVERRIDE:
            override_text = "overridden in dev config";
            break;
        case AVConfig2StorageLocation::PS_FILE_ORDINARY:
            // do nothing
            break;
        case AVConfig2StorageLocation::PS_RESOLVED_REFERENCE:
            override_text = "reference";
            break;
        case AVConfig2StorageLocation::PS_INHERITED_PARAMETER:
            override_text = "inherited from " + location.m_inherited_parameter_name;
            break;
        case AVConfig2StorageLocation::PS_DEFAULT_PURE:
            override_text = "default for pure command line option";
            break;
        case AVConfig2StorageLocation::PS_DEFAULT_OPTIONAL:
            override_text = "default for optional param";
            break;
        case AVConfig2StorageLocation::PS_UNKNOWN:
            override_text = "unknown source!?";
            break;
        }

        // Indicate whether the loaded parameter corresponds to a registered one - see AC-622.
        // Tracking references would be more effort and is left for the future...
        QString prefix = reg != 0 ? " " : "~";

        // Use AVToString for proper null string representation
        dumpConfigHelper(dump_stream, prefix + cur_param, AVToString(cur_loaded.getValue()), location,
                         override_text);
    }

    std::cout << qPrintable(dump);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Container::dumpConfigHelper(
        QTextStream& stream, const QString& text1, const QString& text2,
        const AVConfig2StorageLocation& location, QString override_text) const
{
    const uint COLUMN_WIDTH_1 = 70;
    const uint COLUMN_WIDTH_2 = 50;

    stream.setFieldAlignment(QTextStream::AlignLeft);

    stream << qSetFieldWidth(COLUMN_WIDTH_1) << qSetPadChar(' ') << text1;
    stream << qSetFieldWidth(COLUMN_WIDTH_2) << qSetPadChar(' ') << (" : " + text2);
    stream << qSetFieldWidth(0);

    if (location.m_name.isNull())
    {
        stream << override_text;
    } else
    {
        AVPath path(location.getFullFilePath());
        QString location_info = " (" + path.pathRelativeToAppEnv();
        if (!location.getFactConditions().isEmpty())
        {
            location_info += " " + AVToString(location.getFactConditions());
        }
        if (!override_text.isNull()) location_info += ", " + override_text;
        location_info += ")";

        stream << location_info;
    }
    stream.setFieldAlignment(QTextStream::AlignRight);

    stream << "\n";
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Container::printBuildVersion() const
{
    std::cout << "Package:  " << qPrintable(AVPackageInfo::getName()) << std::endl
              << "Version:  " << qPrintable(AVPackageInfo::getVersion()) << std::endl
              << "Revision: " << qPrintable(AVPackageInfo::getRevision()) << std::endl
              << "Build:    " << qPrintable(AVPackageInfo::getBuildKey()) << std::endl;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Container::addParameter(const QString& value, const AVConfig2Metadata& metadata,
                                      const AVConfig2StorageLocation& location)
{
    if (!location.areFactsMet(m_facts)) return;

//    std::cout << "AVConfig2Container::addParameter " << qPrintable(metadata.m_name) << ": " << qPrintable(value) << "\n";

    const QString& name = metadata.m_name;
    if (!checkNewLoadedParam(name, location)) return;

    // (a)  If a command line switch was given for the parameter (which can be determined only now that we know the parameter's metadata),
    //      to avoid issues when subconfigs reference other subconfig parameters:
    // (a1) If the parameter already is present (override, inherited value, default for optionals...),
    //      remember original value from file, then override it (command line switch '-' syntax has topmost priority)
    //      The overriding value is not of any interest in this case.
    // (a2) If no override is present, remember the original value and override it with the command line switch
    //
    // (b1) If the parameter already was inherited, and value and metadata match, ignore the newly loaded parameter (thus avoiding
    //      a save of this redundant information if a different target file is given)
    // (b2) If the parameter was inherited, but the value mismatches, overwrite the inherited parameter (metadata mismatch is fatal)
    //
    // (c)  Completely overwrite any optional default value.
    //
    // (d)  If the parameter was already overridden, use the value from the override, but fix up the metadata and original value.
    //
    // (e)  If no parameter was yet loaded, simply load it.

    LoadedParameter * loaded_parameter = m_loaded_data.getElementByName<LoadedParameter>(name);
    CmdLineParamContainer::const_iterator cmdline_it = m_loaded_cmdline_param.find(metadata.m_command_line_switch);

    if (cmdline_it != m_loaded_cmdline_param.end())
    {
        // (a)
        if (loaded_parameter != nullptr)
        {
            // (a1)
            *loaded_parameter = LoadedParameter(value, metadata, location);
        } else
        {
            // (a2)
            loaded_parameter = m_loaded_data.addElement(LoadedParameter(value, metadata, location));
        }

        loaded_parameter->setOverrideValue(cmdline_it.value(), AVConfig2StorageLocation::PS_CMDLINE_PARAM);

    } else if (loaded_parameter != 0)
    {
        switch (loaded_parameter->getLocation().m_source)
        {
        case AVConfig2StorageLocation::PS_INHERITED_PARAMETER:
        {
            if (value == loaded_parameter->getValue() && metadata.isEquivalent(loaded_parameter->getMetadata(), true))
            {
                // (b1)
                return;
            }
            // (b2)
            if (!metadata.isEquivalent(loaded_parameter->getMetadata(), false))
            {
                AVLogFatal << "Metadata mismatch on overridden inherited parameter " << loaded_parameter->getName() << " -> " << name;
            }

            // completely overwrite inherited parameter
            *loaded_parameter = LoadedParameter(value, metadata, location);
            break;
        }
        case AVConfig2StorageLocation::PS_DEFAULT_OPTIONAL:
        {
            // (c)
            *loaded_parameter = (LoadedParameter(value, metadata, location));
            break;
        }
        case AVConfig2StorageLocation::PS_CMDLINE_OVERRIDE:
        case AVConfig2StorageLocation::PS_FILE_DEV_OVERRIDE:
        {
            // (d)
            QString                                   loaded_value  = loaded_parameter->getValue();
            AVConfig2StorageLocation::ParameterSource loaded_source = loaded_parameter->getLocation().m_source;

            *loaded_parameter = LoadedParameter(value, metadata, location);
            loaded_parameter->setOverrideValue(loaded_value, loaded_source);
            break;
        }
        case AVConfig2StorageLocation::PS_CMDLINE_PARAM: // covered by (a) for first occurence, other occurences are duplicates
        case AVConfig2StorageLocation::PS_FILE_ORDINARY: // covered by duplicate check
        case AVConfig2StorageLocation::PS_DEFAULT_PURE:  // covered by duplicate check
        case AVConfig2StorageLocation::PS_RESOLVED_REFERENCE: // covered by duplicate check
        case AVConfig2StorageLocation::PS_UNKNOWN:
        default:
            AVLogFatal << "AVConfig2Container::addParameter: Parameter already exists with unexpected source "
                       << loaded_parameter->getLocation().m_source;
        }
    } else
    {
        // (e)
        loaded_parameter = m_loaded_data.addElement(LoadedParameter(value, metadata, location));

        // skip the rest of function. In case (e), new loaded parameter is already at the back
        return;
    }

    // ensure that overrides, inherited values etc. don't influence the parameter order -> move the parameter to the end
    // of the list.
    AVASSERT(loaded_parameter != nullptr);
    loaded_parameter = m_loaded_data.moveToBack(loaded_parameter);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Container::addReference(const QString& ref, const AVConfig2Metadata& metadata,
                                      const AVConfig2StorageLocation& location)
{
    if (!location.areFactsMet(m_facts)) return;

//    std::cout << "AVConfig2Container::addReference " << qPrintable(metadata.m_name) << " -> " << qPrintable(ref) << "\n";

    const QString& name = metadata.m_name;
    if (!checkNewLoadedParam(name, location)) return;

    m_loaded_data.addElement<LoadedReference>(LoadedReference(ref, metadata, location));

    CmdLineParamContainer::const_iterator cmdline_it = m_loaded_cmdline_param.find(metadata.m_command_line_switch);

    if (cmdline_it != m_loaded_cmdline_param.end())
    {
        // If we find a command line param, use it right away. Overwrite any loaded param from override in config file here.
        AVConfig2StorageLocation cmdline_location(AVConfig2StorageLocation::PS_CMDLINE_PARAM);
        m_loaded_data.addElement(LoadedParameter(cmdline_it.value(), metadata, cmdline_location));
    } else
    {
        LoadedParameter * existing_param = m_loaded_data.getElementByName<LoadedParameter>(name);
        if (existing_param != nullptr)
        {
            switch (existing_param->getLocation().m_source)
            {
            case AVConfig2StorageLocation::PS_DEFAULT_OPTIONAL:
            case AVConfig2StorageLocation::PS_INHERITED_PARAMETER:
                // *Attention* Existing parameters of these types need to be overwritten when resolving the reference.
                // Keep in sync with the code in AVConfig2Container::resolveReferences!
                break;
            case AVConfig2StorageLocation::PS_CMDLINE_OVERRIDE:
            case AVConfig2StorageLocation::PS_FILE_DEV_OVERRIDE:
                // Loaded value based on overrides or default for optionals -> retain it, but fill in loaded metadata (it is required for
                // metadata plausi check in AVConfig2Container::resolveReferences)
                // Override values are not overwritten in resolveReferences, but optionals are.
                existing_param->updateMetadata(metadata);
                break;

            case AVConfig2StorageLocation::PS_DEFAULT_PURE: // TODO CM probably should be covered earlier with more specific output
            case AVConfig2StorageLocation::PS_CMDLINE_PARAM: // covered by branch above
            case AVConfig2StorageLocation::PS_FILE_ORDINARY: // covered by duplicate check
            case AVConfig2StorageLocation::PS_RESOLVED_REFERENCE: // covered by duplicate check
            case AVConfig2StorageLocation::PS_UNKNOWN:
            default:
                AVLogFatal << "AVConfig2Container::addReference: unexpected existing parameter "
                           << metadata.m_name << "(" << existing_param->getLocation().m_source << ")";
            }

        }
    }

    // We want to make sure references are valid, independently on override values, so always add this.
    // Overriden values are not overwritten, see AVConfig2Container::resolveReferences.
    m_unresolved_references[name] = LoadedReference(ref, metadata, location);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Container::addSearchPath(
        const QString& path, const AVConfig2StorageLocation& location)
{
    if (!location.areFactsMet(m_facts)) return;
    m_loaded_data.addElement(LoadedSearchPath(path, location));
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Container::addInclude(const QString& include,
                                    const AVConfig2StorageLocation& location)
{
    if (!location.areFactsMet(m_facts)) return;

    const LoadedInclude * existing_include = m_loaded_data.getElementByName<LoadedInclude>(include);
    if (existing_include != nullptr)
    {
        // TODO CM make non-fatal and report together with other errors?
        AVLogFatal << include << " was included twice!";
    }

    // Allow specifying the include relative to the current file's location so dev configs can include other
    // dev configs which reside in the same directory.
    if (loadConfigInternal(include, false, QStringList() << location.m_location))
    {
        m_loaded_data.addElement(LoadedInclude(include, location));
    } else
    {
        AVLogError << "AVConfig2Container::addInclude: failed to load config " << include << " included in "
                   << location.getFullFilePath();
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Container::addConfigMapping(
    const QString& map_from, const QString& mapped_dir,
    const QString& mapped_name,
    const AVConfig2StorageLocation& location)
{
    if (!location.areFactsMet(m_facts)) return;

    LoadedMapping new_mapping(map_from, mapped_dir, mapped_name, location);

    const LoadedMapping * existing_mapping = m_loaded_data.getElementByName<LoadedMapping>(new_mapping.getName());
    if (existing_mapping != nullptr)
    {
        AVLogWarning << existing_mapping->getName() << " already maps to "
                     << existing_mapping->getFullMapping()
                     << ", new mapping is "
                     << new_mapping.getFullMapping();
    }
    m_loaded_data.addElement(new_mapping);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Container::addOverride(const QString& name, const QString& value,
                                     const AVConfig2StorageLocation& location)
{
    if (!location.areFactsMet(m_facts)) return;

    if (m_loaded_data.getElementByName<LoadedReference>(name) != nullptr)
    {
        // Overrides are immediately adopted when loading a param / reference, so the order is important.
        // We don't allow params/references before overrides within a single config file, but if the configuration
        // is split among multiple files, this might still occur.
        // The policy that overrides occur only in dev configs should avoid this error altogether.
        AVLogFatal << "Dev Override unexpectedly encountered after reference ("
                   << name << ")";
    }
    if (m_loaded_data.getElementByName<LoadedOverride>(name) != nullptr)
    {
        AVLogFatal << "AVConfig2Container::addOverride: "
                   << name << " was overridden multiple times.";
    }

    m_loaded_data.addElement(LoadedOverride(name, value, location));

    LoadedParameter * existing_loaded = m_loaded_data.getElementByName<LoadedParameter>(name);
    if (existing_loaded != nullptr)
    {
        switch (existing_loaded->getLocation().m_source)
        {
        case AVConfig2StorageLocation::PS_CMDLINE_PARAM:
        case AVConfig2StorageLocation::PS_CMDLINE_OVERRIDE:
            // nothing to do, those have higher priority than dev overrides in a file.
            return;

        case AVConfig2StorageLocation::PS_DEFAULT_OPTIONAL:
            // simply overwrite defaults for optionals
            *existing_loaded = LoadedParameter(value, AVConfig2Metadata(name), AVConfig2StorageLocation(AVConfig2StorageLocation::PS_FILE_DEV_OVERRIDE));
            break;

        case AVConfig2StorageLocation::PS_FILE_ORDINARY:
        case AVConfig2StorageLocation::PS_RESOLVED_REFERENCE:
        case AVConfig2StorageLocation::PS_INHERITED_PARAMETER:
            // see notes above for LoadedReference check
            AVLogFatal << "AVConfig2Container::addOverride: override unexpectedly encountered after parameter ("
                       << name << ")";
            return;
        case AVConfig2StorageLocation::PS_DEFAULT_PURE:
            AVLogFatal << "AVConfig2Container::addOverride: override for pure command line param "
                       << name;
            break;
        case AVConfig2StorageLocation::PS_FILE_DEV_OVERRIDE: // covered by LoadedOverride check above
        case AVConfig2StorageLocation::PS_UNKNOWN:
        default:
            AVLogFatal << "AVConfig2Container::addOverride: unexpected existing parameter with source "
                       << existing_loaded->getLocation().m_source;
        }
    } else
    {
        // Specify incomplete metadata, this is filled in when the parameter is actually loaded.
        m_loaded_data.addElement(
                    LoadedParameter(value, AVConfig2Metadata(name), AVConfig2StorageLocation(AVConfig2StorageLocation::PS_FILE_DEV_OVERRIDE)));
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Container::addDefine(const QString& name, const QString& value,
                                   const AVConfig2StorageLocation& location)
{
    if (!location.areFactsMet(m_facts)) return;

    if (m_facts.contains(name))
    {
        // TODO CM make non-fatal and report together with other errors?
        AVLogFatal << name << " was defined multiple times";
    }
    m_facts[name] = value;
    m_loaded_data.addElement(LoadedDefine(name, value, location));
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Container::addInheritedSection(
        const QString &parent, const QString &section, const AVConfig2StorageLocation& location)
{
    if (!location.areFactsMet(m_facts)) return;

    if (m_loaded_data.getElementByName<LoadedInheritance>(section))
    {
        AVLogFatal << "AVConfig2Container::addInheritedSection: " << section << " already inherits from a parent section.";
    }
    m_loaded_data.addElement(LoadedInheritance(parent, section, location));

    // Iterate over all parameters and references from parent section, copy them to the target section
    // We can't manipulate the containers while we are iterating, so remember the elements to copy and do it afterwards.
    typedef QVector<LoadedParameter> CopyParamContainer;
    typedef QVector<LoadedReference> CopyRefContainer;
    CopyParamContainer params_to_copy;
    CopyRefContainer   refs_to_copy;

    uint total_count =
            m_loaded_data.getElementCount<LoadedParameter>() +
            m_loaded_data.getElementCount<LoadedReference>();

    bool parent_section_exists = false; // don't rely on any parameter actually being copied, see SWE-4318.
    for (uint total_index=0; total_index<total_count; ++total_index)
    {
        const LoadedParameter *cur_param = nullptr;
        const LoadedReference *cur_ref   = nullptr;
        QString cur_name;
        const AVConfig2Metadata * cur_metadata = 0;

        if (total_index < m_loaded_data.getElementCount<LoadedParameter>())
        {
            cur_param    = &m_loaded_data.getElementByIndex<LoadedParameter>(total_index);
            cur_name     = cur_param->getName();
            cur_metadata = &cur_param->getMetadata();
        } else
        {
            cur_ref      = &m_loaded_data.getElementByIndex<LoadedReference>(total_index - m_loaded_data.getElementCount<LoadedParameter>());
            cur_name     = cur_ref->getName();
            cur_metadata = &cur_ref->m_metadata;
        }
        if (!cur_name.startsWith(parent + ".")) continue;
        parent_section_exists = true;

        QString new_name = section + cur_name.mid(parent.length());

        // Check for any existing parameter, don't overwrite command line switches and dev overrides with the inherited value.
        // Nevertheless, store the information that there is a parent parameter.
        //
        // It is fatal if an ordinary file parameter already is present - it would mean that the section which now inherits was
        // already defined earlier, which is forbidden.
        LoadedParameter * existing_parameter = m_loaded_data.getElementByName<LoadedParameter>(new_name);
        LoadedReference * existing_reference = m_loaded_data.getElementByName<LoadedReference>(new_name);

        // fatal for now, will likely change in the future (SWE-3221 - support parameter references in other contexts)
        if (existing_reference != nullptr)
        {
            AVLogFatal << "namespace inheritance error (" << section << " : " << parent
                       << "):\ncannot copy to " << new_name << " because a reference of this name already exists.";
        }

        if (existing_parameter != nullptr)
        {
            if (existing_parameter ->getLocation().m_source == AVConfig2StorageLocation::PS_CMDLINE_PARAM    ||
                existing_parameter ->getLocation().m_source == AVConfig2StorageLocation::PS_CMDLINE_OVERRIDE ||
                existing_parameter ->getLocation().m_source == AVConfig2StorageLocation::PS_FILE_DEV_OVERRIDE)
            {
                // provide override with the complete metadata
                existing_parameter->updateMetadata(*cur_metadata);
                // and let it "point" to the inherited parameter
                existing_parameter->getLocationForModification().m_inherited_parameter_name = cur_name;
                // don't copy the parameter
                continue;
            } else
            {
                AVLogFatal << "namespace inheritance error (" << section << " : " << parent
                           << "):\ncannot copy to " << new_name << " because it already exists.";
            }
        }

        AVConfig2StorageLocation copied_param_location(location);
        copied_param_location.m_source = AVConfig2StorageLocation::PS_INHERITED_PARAMETER;
        copied_param_location.m_inherited_parameter_name = cur_name;
        AVConfig2Metadata inherited_metadata(cur_param ? cur_param->getMetadata() : cur_ref->m_metadata);
        inherited_metadata.m_name = new_name;

        if (cur_ref != nullptr)
        {
            LoadedReference inherited_reference(cur_ref->m_referenced_param, inherited_metadata, copied_param_location);
            refs_to_copy.push_back(inherited_reference);
        } else
        {
            LoadedParameter inherited_parameter(cur_param->getValue(), inherited_metadata, copied_param_location);
            params_to_copy.push_back(inherited_parameter);
        }
    }

    if (!parent_section_exists)
    {
        AVLogFatal << "AVConfig2Container::addInheritedSection: parent section \"" << parent << "\" does not exist!";
    }

    for (CopyParamContainer::const_iterator it = params_to_copy.begin();
         it != params_to_copy.end(); ++it)
    {
        const LoadedParameter& cur_param = *it;
        m_loaded_data.addElement(cur_param);
    }
    for (CopyRefContainer::const_iterator it = refs_to_copy.begin();
         it != refs_to_copy.end(); ++it)
    {
        const LoadedReference& cur_ref = *it;
        m_loaded_data.addElement(cur_ref);
        m_unresolved_references[cur_ref.getName()] = cur_ref;
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Container::addSubconfigTemplateParameter(const QString& value, const AVConfig2Metadata& metadata,
                                                       const AVConfig2StorageLocation& location)
{
    m_loaded_data.addElement(LoadedSubconfigTemplateParameter(value, metadata, location));
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Container::setStorageLocation(const AVConfig2StorageLocation& location, QRegExp regexp)
{
    for (ConfigObjectContainer::const_iterator config_object_it = m_config_object.begin();
            config_object_it != m_config_object.end(); ++config_object_it)
    {
        const AVConfig2& cur_config = **config_object_it;

        AVASSERT(m_registered_param.containsConfig(&cur_config));
        const RegisteredParameterList& cur_registered_list = m_registered_param.getParameterList(&cur_config);

        for (RegisteredParameterList::const_iterator registered_param_it = cur_registered_list.begin();
                registered_param_it != cur_registered_list.end(); ++registered_param_it)
        {
            const RegisteredParameterBase& cur_param    = *(*(registered_param_it));
            const AVConfig2Metadata&       cur_metadata = cur_param.getMetadata();
            const QString&                 cur_name     = cur_metadata.m_name;

            if (!regexp.exactMatch(cur_name)) continue;
            if (!cur_param.isSavingEnabled()) continue;

            // Don't simply replace loaded param because we don't want to break any formatting and we
            // want to preserve user comments
            LoadedParameter * existing_loaded = m_loaded_data.getElementByName<LoadedParameter>(cur_name);
            if (existing_loaded != nullptr)
            {
                existing_loaded->setLocation(location.m_location, location.m_name, location.m_format);
            } else
            {
                AVConfig2StorageLocation file_location(location);
                file_location.m_source = AVConfig2StorageLocation::PS_FILE_ORDINARY;
                m_loaded_data.addElement(LoadedParameter(cur_param.toString(), cur_metadata, file_location));
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2Container::containsLoadedParameterWithPrefix(const QString& prefix) const
{
    for(uint p=0; p< m_loaded_data.getElementCount<LoadedParameter>(); ++p)
    {
        const LoadedParameter& cur_loaded = m_loaded_data.getElementByIndex<LoadedParameter>(p);
        // Ignore parameters which are only present because of overrides - see AC-606.
        if(cur_loaded.getName().startsWith(prefix) && !cur_loaded.getMetadata().isIncomplete())
        {
            return true;
        }
    }

    for(uint p=0; p< m_loaded_data.getElementCount<LoadedReference>(); ++p)
    {
        const LoadedReference& cur_loaded = m_loaded_data.getElementByIndex<LoadedReference>(p);
        if (cur_loaded.getName().startsWith(prefix))
        {
            return true;
        }
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2Container::replaceParameterValue(const QString &param_name, const QString &new_value)
{
    LoadedParameter *param = m_loaded_data.getElementByName<LoadedParameter>(param_name);
    if (!param) return false;
    param->setValue(new_value);
    param->setOriginalValue(new_value);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

const AVConfig2ImporterClient::LoadedParameter *AVConfig2Container::getLoadedParameter(const QString& name) const
{
    return m_loaded_data.getElementByName<LoadedParameter>(name);
}

///////////////////////////////////////////////////////////////////////////////

const AVConfig2Container::LoadedParameterContainer AVConfig2Container::getLoadedParameters() const
{
    AVDEPRECATE(AVConfig2Container::getLoadedParameters);

    LoadedParameterContainer ret;

    for (uint p=0; p<m_loaded_data.getElementCount<AVConfig2ImporterClient::LoadedParameter>(); ++p)
    {
        const LoadedParameter& cur_loaded = m_loaded_data.getElementByIndex<AVConfig2ImporterClient::LoadedParameter>(p);
        ret[cur_loaded.getName()] = cur_loaded;
    }

    return ret;
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2Container::LoadedParameterContainer AVConfig2Container::getLoadedParameters(const QRegularExpression& reg_exp) const
{
    LoadedParameterContainer container;

    for (uint p=0; p<m_loaded_data.getElementCount<AVConfig2ImporterClient::LoadedParameter>(); ++p)
    {
        const LoadedParameter& cur_loaded = m_loaded_data.getElementByIndex<AVConfig2ImporterClient::LoadedParameter>(p);

        if (reg_exp.match(cur_loaded.getName()).hasMatch())
        {
            container.insert(cur_loaded.getName(), cur_loaded);
        }
    }
    return container;
}

///////////////////////////////////////////////////////////////////////////////

const AVConfig2LoadedData& AVConfig2Container::getLoadedData() const
{
    return m_loaded_data;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Container::assignLoadedMetadataTo(AVConfig2Container& target) const
{
    AVASSERT(target.m_loaded_data.isEmpty());

    target.m_facts = m_facts;

    for (uint i=0; i<m_loaded_data.getElementTotalCount(); ++i)
    {
        const LoadedElement& cur_element = m_loaded_data.getElementByTotalIndex(i);

        const LoadedDefine* loaded_define = dynamic_cast<const LoadedDefine*>(&cur_element);
        if (loaded_define != nullptr)
        {
            // Avoid "fact XYZ was defined multiple times" errors by removing the fact before re-adding it.
            // This allows directly reusing the code triggered by addToImporterClient.
            target.m_facts.remove(loaded_define->getName());
            cur_element.addToImporterClient(target);
        } else if (dynamic_cast<const LoadedMapping*>   (&cur_element) != nullptr ||
                   dynamic_cast<const LoadedSearchPath*>(&cur_element) != nullptr ||
                   dynamic_cast<const LoadedOverride*>  (&cur_element) != nullptr)
        {
            cur_element.addToImporterClient(target);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Container::assignLoadedParamsTo(AVConfig2Container& target) const
{
    AVASSERT(target.m_loaded_data.isEmpty());

    for (uint i=0; i<m_loaded_data.getElementTotalCount(); ++i)
    {
        const LoadedElement& cur_element = m_loaded_data.getElementByTotalIndex(i);
        cur_element.addToImporterClient(target);
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Container::assignDataTo(AVConfig2Container &target) const
{
    target.m_loaded_data              = m_loaded_data;
    target.m_unresolved_references    = m_unresolved_references;
    target.m_loaded_cmdline_param     = m_loaded_cmdline_param;
    target.m_loaded_cmdline_overrides = m_loaded_cmdline_overrides;
    target.m_loaded_duplicates        = m_loaded_duplicates;
    target.m_missing_configs          = m_missing_configs;
    target.m_loaded_deprecated_names  = m_loaded_deprecated_names;
    target.m_facts                    = m_facts;
    target.m_extra_cmdline            = m_extra_cmdline;
    target.m_checking_mode            = m_checking_mode;
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2Container::isConfigRefreshed(const AVConfig2 *config) const
{
    // yes this is ugly...
    // No better solution as AVSet is a value container.
    AVASSERT(m_config_object.contains(const_cast<AVConfig2*>(config)));
    return !m_unrefreshed.contains(const_cast<AVConfig2*>(config));
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Container::loadFromCstyleString(const QString &config_string)
{
    AVConfig2ImporterCstyle string_importer(config_string);
    string_importer.setClient(this);
    string_importer.import();
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Container::setCheckingMode(AVConfig2Container::CheckingMode mode)
{
    m_checking_mode = mode;
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2Container::CheckingMode AVConfig2Container::getCheckingMode() const
{
    return m_checking_mode;
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2Container::RefreshResult::RefreshResult()
{
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2Container::RefreshResult::refreshSuccessful() const
{
    return m_missing_parameters.isEmpty() && m_loaded_deprecated_names.isEmpty() && m_metadata_mismatch.isEmpty() &&
           m_loaded_duplicates.isEmpty()  && m_cmdline_errors.isEmpty() && m_errors.isEmpty() &&
           m_missing_configs.isEmpty() &&
           m_unresolved_references.isEmpty();
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2Container::RefreshResult::getErrorMessage() const
{
    if (refreshSuccessful()) return QString::null;

    QString ret = "Errors occured while loading the configuration "
        "(consider -save to fix this):\n";
    ret += QString().fill('v', 80);

    ret += errorHelper("The following parameters are not present in the loaded configuration:", m_missing_parameters);

    ret += errorHelper("The following parameters have mismatching metadata in the loaded configuration "
            "(see log output above):", m_metadata_mismatch);

    ret += errorHelper("The following parameters are present more than once in the loaded configuration:",
                       m_loaded_duplicates);

    ret += errorHelper("The following configs could not be found:", m_missing_configs);

    ret += errorHelper("The following references could not be resolved:", m_unresolved_references.toList());

    ret += errorHelper("The following parameters still use their deprecated names:", m_loaded_deprecated_names);

    // Probably never used as command line errors are handled earlier...
    ret += errorHelper("The following errors occured while handling the command line:", m_cmdline_errors);

    ret += errorHelper("The following general errors occured:", m_errors);

    ret += QString().fill('^', 80) + "\n";

    return ret;
}

///////////////////////////////////////////////////////////////////////////////

QStringList AVConfig2Container::RegisteredParameterBase::getDeprecatedNames() const
{
    QSet<QString> ret;
    if (m_deprecated_prefix.isEmpty() && m_deprecated_names.isEmpty()) return ret.toList();

    QString full_name = m_metadata.m_name;
    int dot = full_name.lastIndexOf('.');
    if (dot == -1)
    {
        AVLogFatal << "Expected at least one \".\" in parameter name " + full_name;
    }
    QString prefix = full_name.left(dot);
    QString name   = full_name.mid(dot+1);

    for (const QString& deprecated_name : m_deprecated_names)
    {
        ret.insert(m_deprecated_prefix + "." + deprecated_name);
        ret.insert(prefix + "." + deprecated_name);
    }
    ret.insert(m_deprecated_prefix + "." + name);

    ret.remove(full_name);

    return ret.toList();
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Container::RegisteredParameterBase::disableSaving()
{
    m_disable_saving = true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2Container::RegisteredParameterBase::isSavingEnabled() const
{
    return !m_disable_saving;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Container::RegisteredParameterBase::setSuccessfullyLoaded()
{
    m_was_successfully_loaded = true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2Container::RegisteredParameterBase::wasSuccessfullyLoaded() const
{
    return m_was_successfully_loaded;
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2Container::refreshParams(AVConfig2 *config, RefreshResult *result)
{
    AVASSERT(config != 0);

    RefreshResult local_result;
    if (result == 0)
    {
        result = &local_result;
    }

    m_save_required |= handleVersionTranslation(config, result->m_errors);

    // Will overwrite if repeatedly called, but this doesn't really matter...
    result->m_loaded_duplicates = m_loaded_duplicates;
    result->m_missing_configs   = m_missing_configs;

    if (m_registered_subconfig.contains(config))
    {
        QString prefix_and_dot;

        // Iterate over registered subconfigs.
        RegisteredSubconfigList registered_subconfig_list = m_registered_subconfig[config];
        for (auto subconfig_element : registered_subconfig_list)
        {
            RegisteredSubconfigBase& cur_subconfig = *subconfig_element;

            // First, find the list of subconfigs to create. Find all params and references with the given
            // prefix, and determine the different sections which exist as element following the prefix.
            // Pay attention to preserve order - don't actually use a set, and iterate over all elements in their correct order. See SWE-5465.
            QVector<QString> subconfig_name_set;
            {
                prefix_and_dot = cur_subconfig.getPrefix() + DOT_CHAR;
                const int prefix_length(cur_subconfig.getPrefix().length());

                const uint total_element_count(m_loaded_data.getElementTotalCount());
                for (uint e=0; e<total_element_count; ++e)
                {
                    const AVConfig2ImporterClient::LoadedElement& cur_element = m_loaded_data.getElementByTotalIndex(e);
                    const QString& cur_param_name = cur_element.getName();
                    if (cur_param_name.startsWith(prefix_and_dot)
                        && (cur_element.isLoadedParameter() || cur_element.isLoadedReference()))
                    {
                        const QStringRef remaining         = cur_param_name.midRef(prefix_length);
                        const QVector<QStringRef> splitted = remaining.split(DOT_CHAR);

                        // remaining could be any combination of strings between dots
                        // so use QVector.value() instead of QVector.at(), avoiding invalid index error

                        const QString subconfig_name       = splitted.value(1).toString();
                        const QStringRef param_name        = splitted.value(2);

                        if (param_name.isEmpty())
                        {
                            result->m_errors += "While refreshing " + config->getPrefix() + ": Subconfig \""
                                    + subconfig_name + "\" with prefix \"" + cur_subconfig.getPrefix() +
                                    "\" has no parameters! Error in config structure?";
                            continue;
                        }

                        if (!subconfig_name_set.contains(subconfig_name))
                        {
                            subconfig_name_set.push_back(subconfig_name);
                        }
                    }
                }
            }

            // Now, create a subconfig object for each set element
            for (const QString& name : subconfig_name_set)
            {
                refreshParams(cur_subconfig.getOrCreateSubconfig(name), result);
            }
        }
    }

    // Creating subconfigs may trigger loading additional configuration files -> resolve references (again).
    // Note that it currently isn't supported to reference a parameter contained in a different
    // subconfig (as it might not yet be loaded, leading to a complaint about an unresolved
    // reference).
    resolveReferences(*result);

    if (m_registered_param.containsConfig(config))
    {
        const AVConfig2Container::RegisteredParameterList& param_list = m_registered_param.getParameterList(config);
        for (auto param_element : param_list)
        {
            RegisteredParameterBase& cur_param_info = *param_element;
            const AVConfig2Metadata& cur_metadata   = cur_param_info.getMetadata();
            const QString            cur_param_name = cur_metadata.m_name;

            LoadedParameter * existing_loaded = m_loaded_data.getElementByName<LoadedParameter>(cur_param_name);
            QString value;
            bool deprecated = false;
            // If all we have to go with is the default for optional parameters, remember it in this variable. This is required
            // to properly remove the default when handling deprecated parameters (also see SWE-2570)
            LoadedParameter * default_optional = nullptr;
            if (existing_loaded && existing_loaded->getLocation().m_source == AVConfig2StorageLocation::PS_DEFAULT_OPTIONAL)
            {
                default_optional = existing_loaded;
                AVASSERT(cur_metadata.m_optional);
            }

            // Flag whether to check that the loaded metadata is consistent. Don't do it for
            // (a) pure cmdline params
            // (b) optional parameters which were not present in the config file.
            // (c) params which have a suggested values and were not present in the config file.
            // (d) Parameters loaded from classic style config files (because some metadata cannot
            //     be expressed in the old format)

            // (a)
            bool check_metadata = !cur_param_info.isPureCmdlineOption();

            // First try alternative (deprecated) names
            if (existing_loaded == nullptr || default_optional != nullptr)
            {
                deprecated = true;
                QStringList deprecated_names = cur_param_info.getDeprecatedNames();
                for (const QString& cur_deprecated_name : deprecated_names)
                {
                    AVASSERT(cur_deprecated_name != cur_param_name);
                    LoadedParameter * loaded_deprecated = m_loaded_data.getElementByName<LoadedParameter>(cur_deprecated_name);
                    if (loaded_deprecated == nullptr) continue;
                    existing_loaded = loaded_deprecated;
                    // remove the default for the optional parameter, because it is no longer required and breaks things down
                    // the road, see e.g. SWE-2570
                    if (default_optional != nullptr)
                    {
                        AVASSERT(m_loaded_data.removeElementByName<LoadedParameter>(default_optional->getName()));
                    }
                    break;
                }
            }

            // If a non-optional parameter is specified on the command line, treat it like a missing parameter. See SWE-3946.
            if (existing_loaded == nullptr || (existing_loaded->getMetadata().isIncomplete() && !cur_param_info.isOptional()))
            {
                // A loaded parameter is created for optionals at registration time, so it cannot be missing now.
                AVASSERT(!cur_param_info.isOptional());

                if (cur_metadata.m_default_value.isNull())
                {
                    // no default to go with - we can just report the parameter as missing.
                    result->m_missing_parameters << cur_param_name;
                    continue;
                } else
                {
                    // Ok, we have a default - use it if strict checking is disabled.
                    // (but instead of the default, use the value from the command line if it was specified)
                    value = existing_loaded ? existing_loaded->getValue() : cur_metadata.m_default_value;

                    if (m_checking_mode == CheckingMode::CM_LENIENT)
                    {
                        AVLogWarning << "Parameter " << cur_param_name
                                   << " was registered by the program but is not "
                                   << "present in the loaded configuration. Using suggested value ("
                                   << value << ").";
                        AVLogWarning << "Consider calling -save.";
                        // (c)
                        check_metadata = false;
                    } else if (m_checking_mode == CheckingMode::CM_AUTOSAVE)
                    {
                        m_save_required = true;
                        check_metadata = false;
                    } else
                    {
                        result->m_missing_parameters << cur_param_name;
                        continue;
                    }
                }
            } else
            {
                if (deprecated) m_loaded_deprecated_names << existing_loaded->getName();
                value = existing_loaded->getValue();

                if (existing_loaded->getLocation().m_format == CF_CLASSIC)
                {
                    // (d)
                    check_metadata = false;
                }
                if (cur_param_info.isOptional() && existing_loaded->getMetadata().isIncomplete())
                {
                    // (b)
                    check_metadata = false;
                }
            }

            // Try to actually load the parameter
            if (!cur_param_info.fromString(value))
            {
                result->m_errors << QString("Failed to parse param \"%1\" from \"%2\" "
                        "(type is %3).").arg(cur_param_name).arg(value).arg(cur_metadata.m_type);
                // Note that syntactically invalid values are not saved, see AVConfig2Saver::saveRegisteredParameters.
                continue;
            } else
            {
                cur_param_info.setSuccessfullyLoaded();
            }

            // Now that we know that the param could be loaded,
            // check restrictions...
            const AVConfig2RestrictionBase *cur_restriction =
                    cur_param_info.getMetadata().m_restriction.get();
            if (cur_restriction)
            {
                QString err = cur_restriction->errorMessage(cur_param_name, value);
                if (!err.isNull())
                {
                    result->m_errors << err;
                    continue;
                }
            }

            if (check_metadata)
            {
                AVASSERT(existing_loaded != nullptr);
                const AVConfig2Metadata& loaded_metadata = existing_loaded->getMetadata();
                if (!loaded_metadata.isEquivalent(cur_metadata, true))
                {
                    if (AVLogger != 0)
                    {
                        AVLogInfo  << "Config metadata mismatch encountered\n"
                                   << " ------------------- loaded ------------------- "
                                   << AVToString(loaded_metadata)
                                   << "\n ----------------- registered ---------------- "
                                   << AVToString(cur_metadata)
                                   << "\n ------------------------------------------";
                    }
                    if (m_checking_mode == CheckingMode::CM_LENIENT)
                    {
                        AVLogWarning << "Config metadata mismatch encountered and ignored.";
                    } else if (m_checking_mode == CheckingMode::CM_AUTOSAVE)
                    {
                        m_save_required = true;
                    } else
                    {
                        result->m_metadata_mismatch << cur_param_name;
                    }
                    continue;
                }
            }
        }
    }

    if (result->refreshSuccessful())
    {
        // only remove from unrefreshed if refresh was successful to ensure
        // that error still is reported in refreshAll() even if individual refresh was called beforehand.
        m_unrefreshed.remove(config);
        return true;
    } else return false;
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2Container::getAndFlagMappedConfigFile(const QString& config_name)
{
    LoadedMapping * existing_mapping = m_loaded_data.getElementByName<LoadedMapping>(config_name);
    if (existing_mapping == nullptr)
    {
        // Store identity mapping with source QString::null to avoid saving it back.
        m_loaded_data.addElement(LoadedMapping(
            config_name, QString::null, QString::null,
            AVConfig2StorageLocation(QString::null, QString::null, CF_LAST)));
        return config_name;
    } else
    {
        return existing_mapping->getFullMapping();
    }
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2Container::getMappedConfigFile(const QString& config_name) const
{
    const LoadedMapping * existing_mapping = m_loaded_data.getElementByName<LoadedMapping>(config_name);
    if (existing_mapping == nullptr)
    {
        return config_name;
    } else
    {
        return existing_mapping->getFullMapping();
    }
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2Container::loadConfigInternal(
        const QString& config, bool optional, const QStringList& extra_search_paths)
{
    AVConfig2StorageLocation location = resolveConfig(config, extra_search_paths);
    if (location.m_name.isEmpty())
    {
        if (!optional) m_missing_configs.push_back(config);
        return false;
    }

    AVASSERT(AVConfig2ImporterBase::import(this, location));

    AVLogInfo << "AVConfig2Container::loadConfig: loaded " << config << " from " << location.getFullFilePath();

    return true;
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2StorageLocation AVConfig2Container::resolveConfig(
        const QString &config, const QStringList &extra_search_paths)
{
    AVConfig2StorageLocation ret;

    // make sure this is a relative config without extension
    if (!QDir::isRelativePath(config))
    {
        AVLogError << "AVConfig2Container::resolveConfig: " << config << " is not a relative path!";
        return ret;
    }
    for (uint f=0; f<CF_LAST; ++f)
    {
        AVConfig2Format cur_format = static_cast<AVConfig2Format>(f);
        if (config.endsWith(AVConfig2StorageLocation::getConfigExtension(cur_format)))
        {
            AVLogError << "AVConfig2Container::resolveConfig: " << config << " must be given without extension!";
            return ret;
        }
    }

    if (config.isEmpty())
    {
        AVLogError << "AVConfig2Container::resolveConfig: Empty config name specified.";
        return ret;
    }

    QString mapped_config = getAndFlagMappedConfigFile(config);

    if (mapped_config != config)
    {
        AVLogDebug << "AVConfig2Container::resolveConfig: " << config << " mapped to " << mapped_config;
    }

    return locateConfig(mapped_config, true, extra_search_paths);
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2StorageLocation AVConfig2Container::locateConfig(
        const QString& name, bool avibit_config, const QStringList& extra_search_paths) const
{
    QString expanded_name = AVEnvString(name).expandEnvString();
    AVASSERT(!expanded_name.isEmpty());

    // Build the set of paths which will be searched for the config
    typedef QSet<QString> SearchPathContainer;
    SearchPathContainer search_paths;
    QString searched_name;

    bool relative_path = QDir::isRelativePath(expanded_name);
    if (relative_path)
    {
        // If a relative path is specified, search the application config dir and all additionally
        // loaded search paths.
        QString app_config = AVEnvironment::getApplicationConfig();
        app_config = QDir(app_config).absolutePath(); // get rid of "../" ambiguities
        QFileInfo app_config_info(app_config);
        if (!app_config_info.isReadable() || !app_config_info.isDir())
        {
            // don't print a warning here as not all products have a product config (eg astos/config has been deleted)
            AVLogInfo << "AVConfig2Container::locateConfig: application config directory " << app_config
                      << " is not readable.";
        } else
        {
            search_paths.insert(app_config);
        }

        for (uint p=0; p< m_loaded_data.getElementCount<LoadedSearchPath>(); ++p)
        {
            const LoadedSearchPath& cur_path = m_loaded_data.getElementByIndex<LoadedSearchPath>(p);
            search_paths.insert(cur_path.expandPath());
        }

        // Also add any additional search paths for this specific call.
        for (int i=0; i<extra_search_paths.size(); ++i)
        {
            search_paths.insert(AVEnvString(extra_search_paths[i]).expandEnvString());
        }

        searched_name = expanded_name;
    } else
    {
        QFileInfo fi(expanded_name);
        // If an absolute path is specified, just search that path.
        search_paths.insert(fi.absolutePath());
        searched_name = fi.fileName();
    }

    // Now search all paths, looking for configs of all types if this is an AviBit config.
    AVLogDebug << "AVConfig2Container::locateConfig: search paths:\n" << search_paths.toList().join("\n");

    // Keep track of found configs for building error message if required.
    QStringList found_configs;
    AVConfig2StorageLocation ret;
    for (const QString& location : search_paths)
    {
        AVConfig2StorageLocation test_location;
        test_location.m_name = searched_name;
        test_location.m_location = location;

        // Do plausibility check on search path to avoid later headaches
        QFileInfo location_info(test_location.m_location);
        if (relative_path && (!location_info.isReadable() || !location_info.isDir()))
        {
            AVLogFatal << "AVConfig2Container::locateConfig: " << test_location.m_location
                       << " is not a readable directory.";
        }

        if (!avibit_config)
        {
            test_location.m_format = CF_LAST;

            QFileInfo full_path_info(test_location.getFullFilePath());
            if (full_path_info.exists())
            {
                ret = test_location;
                found_configs << ret.getFullFilePath();
            }
        } else
        {
            bool extension_given = false;
            for (uint f=0; f<CF_LAST; ++f)
            {
                AVConfig2Format cur_format = static_cast<AVConfig2Format>(f);
                // First check whether a valid extension is given
                const QString& cur_extension = AVConfig2StorageLocation::getConfigExtension(cur_format);
                if (!test_location.m_name.endsWith(cur_extension)) continue;

                test_location.m_format = static_cast<AVConfig2Format>(f);
                // Strip the extension
                test_location.m_name = test_location.m_name.left(
                        test_location.m_name.length() - cur_extension.length());
                extension_given = true;

                QFileInfo full_path_info(test_location.getFullFilePath());
                if (full_path_info.exists())
                {
                    ret = test_location;
                    found_configs << ret.getFullFilePath();
                }
                break;
            }

            if (!extension_given)
            {
                for (uint f=0; f<CF_LAST; ++f)
                {
                    test_location.m_format = static_cast<AVConfig2Format>(f);

                    QFileInfo full_path_info(test_location.getFullFilePath());
                    if (full_path_info.exists())
                    {
                        ret = test_location;
                        found_configs << ret.getFullFilePath();
                    }
                }
            }
        }
    }

    if (found_configs.size() > 1)
    {
        AVLogFatal << "AVConfig2Container::locateConfig: config file \""
                   << expanded_name
                   << "\" is ambiguous; it was found in the following locations:\n"
                   << found_configs.join("\n");
    }

    if (ret.m_name.isEmpty())
    {
        AVASSERT(found_configs.size() == 0);
        // Don't assume how this method is used - just log debug
        AVLogDebug << "AVConfig2Container::locateConfig: Failed to find configuration \""
                   << expanded_name << "\"";
    } else
    {
        AVASSERT(found_configs.size() == 1);
        AVLogDebug << "AVConfig2Container::locateConfig: got it: "
                   << ret.getFullFilePath();
    }
    return ret;
}

///////////////////////////////////////////////////////////////////////////////

QPair<const AVConfig2*, const AVConfig2Container::RegisteredParameterBase*> AVConfig2Container::getRegisteredParameterInfo(
        const QString& param_name) const
{
    if (m_registered_param_names.find(param_name) == m_registered_param_names.end())
    {
        return qMakePair(static_cast<const AVConfig2*>(nullptr), static_cast<const RegisteredParameterBase*>(nullptr));
    }

    // Iterate over all params from all configs
    for (RegisteredParameterContainer::const_iterator it = m_registered_param.begin();
         it != m_registered_param.end();
         ++it)
    {
        const AVConfig2Container::RegisteredParameterList& cur_parameter_list = it->second;

        for (RegisteredParameterList::const_iterator it2 = cur_parameter_list.begin();
             it2 != cur_parameter_list.end();
             ++it2)
        {
            const RegisteredParameterBase& param = **it2;
            if (param.getMetadata().m_name == param_name) return qMakePair(it->first, &param);
        }
    }
    AVLogFatal << "AVConfig2Container::getRegisteredParameterInfo: unexpected internal inconsistency";
    return qMakePair(static_cast<const AVConfig2*>(nullptr), static_cast<const RegisteredParameterBase*>(nullptr));
}

///////////////////////////////////////////////////////////////////////////////

QPair<const AVConfig2*, const AVConfig2Container::RegisteredSubconfigBase*> AVConfig2Container::getRegisteredSubconfigInfo(
        const QString& subconfig_name) const
{
    if (m_registered_subconfig_names.find(subconfig_name) == m_registered_subconfig_names.end())
    {
        return {};
    }

    // Iterate over all params from all configs
    for (RegisteredSubconfigContainer::const_iterator it = m_registered_subconfig.begin();
         it != m_registered_subconfig.end();
         ++it)
    {
        const AVConfig2Container::RegisteredSubconfigList& cur_subconfig_list = it.value();

        for (RegisteredSubconfigList::const_iterator it2 = cur_subconfig_list.begin();
             it2 != cur_subconfig_list.end();
             ++it2)
        {
            const RegisteredSubconfigBase& subconfig = **it2;
            if (subconfig.getPrefix() == subconfig_name) return qMakePair(it.key(), &subconfig);
        }
    }
    AVLogFatal << "AVConfig2Container::getRegisteredSubconfigInfo: unexpected internal inconsistency";
    return {};
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Container::checkCmdlineParams(QStringList& errors) const
{
    QMap<QString, QString> switch_to_param_name_map;

    for (RegisteredParameterContainer::const_iterator it = m_registered_param.begin();
         it != m_registered_param.end();
         ++it)
    {
        const AVConfig2Container::RegisteredParameterList& cur_parameter_list = it->second;

        for (RegisteredParameterList::const_iterator it2 = cur_parameter_list.begin();
             it2 != cur_parameter_list.end();
             ++it2)
        {
            const RegisteredParameterBase& cur_param_info = **it2;
            const AVConfig2Metadata& cur_metadata         = cur_param_info.getMetadata();
            const QString& cur_name                       = cur_metadata.m_name;
            const QString& cur_cmdline_switch             = cur_param_info.getCommandlineSwitch();

            // Ignore params which can't be set via command line
            if (cur_cmdline_switch.isNull()) continue;

            // Build map
            if (switch_to_param_name_map.contains(cur_cmdline_switch))
            {
                errors << QString("Command line switch \"%1\" is used multiple times for "
                        "different registered parameters (%2 and %3).").
                        arg(cur_cmdline_switch).
                        arg(switch_to_param_name_map[cur_cmdline_switch]).
                        arg(cur_name);
            } else
            {
                switch_to_param_name_map[cur_cmdline_switch] = cur_name;
            }
        }
    }

    // Now, check for unknown command line switches
    for (CmdLineParamContainer::const_iterator cmdline_switch_it =
            m_loaded_cmdline_param.begin();
         cmdline_switch_it != m_loaded_cmdline_param.end();
         ++cmdline_switch_it)
    {
        const QString& cur_cmdline_switch = cmdline_switch_it.key();

        if (!switch_to_param_name_map.contains(cur_cmdline_switch))
        {
            errors << QString("Command line switch \"%1\" is unknown.").arg(cur_cmdline_switch);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Container::addConfigObject(AVConfig2 *config)
{
    AVASSERTNOLOGGER(!m_config_object.contains(config));
    m_config_object.insert(config);
    m_registered_param.addConfig(config);
    m_unrefreshed.insert(config);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Container::resolveReferences(RefreshResult& result)
{
//    std::cout << "AVConfig2Container::resolveReferences\n";
//    std::cout << "unresolved: " << m_unresolved_references.count() << "\n";

    // To resolve references with arbitrary nesting, iterate until no references are left or none
    // of the remaining references could be resolved.
    bool resolved_any_reference;
    do
    {
        resolved_any_reference = false;

        LoadedReferenceContainer::iterator reference_it = m_unresolved_references.begin();
        while (reference_it != m_unresolved_references.end())
        {
            const QString&         cur_param_name = reference_it.key();
            const LoadedReference& cur_reference  = reference_it.value();

//            std::cout << "handling " << qPrintable(cur_param_name) << "\n";

            // to unify reference handling, treat every reference as $(XYZ)
            // check metadata only for "plain" references
            bool check_metadata = false;
            QString cur_reference_string = cur_reference.m_referenced_param;
            if (!cur_reference_string.startsWith("["))
            {
                cur_reference_string = "[$(" + cur_reference_string + ")]";
                check_metadata = true;
            }

            QStringList list;
            avfromtostring::nestedSplit(list, ";", cur_reference_string, true);

            QString resolved_param;
            bool resolved_all_parts = true;
            for (QString l:list)
            {
                if (l.startsWith("$"))
                {
                    if (!l.startsWith("$(") || !l.endsWith(")"))
                    {
                        result.m_errors.push_back("Failed to parse " + cur_param_name + " reference part " + l);
                        resolved_all_parts = false;
                        break;
                    }
                    // strip "$()"
                    QString referenced_param_name = l.mid(2, l.length()-3);

                    // If this reference refers another unresolved reference, wait until it is resolved,
                    // even if a loaded parameter already is available. Also see SWE-6220.
                    //
                    // Note that this relies on the fact that a reference for referenced_param_name
                    // cannot be loaded later, or is irrelevant (it will not change the value we compute now)
                    //
                    // This currently is the case:
                    // PS_DEFAULT_OPTIONAL and PS_INHERITED_PARAMETER are created at the same time as loading the
                    // reference, PS_CMDLINE_OVERRIDE and PS_FILE_DEV_OVERRIDE have a higher priority.
                    // All other types are not allowed when reading a reference, see AVConfig2Container::addReference.
                    if (m_unresolved_references.contains(referenced_param_name))
                    {
                        resolved_all_parts = false;
                        break;
                    }

                    // See whether we can find the loaded param
                    LoadedParameter * existing_loaded = m_loaded_data.getElementByName<LoadedParameter>(referenced_param_name);
                    // Not yet .. hope that it will become available by resolving remaining references
                    if (existing_loaded == nullptr)
                    {
                        resolved_all_parts = false;
                        break;
                    }

                    if (check_metadata)
                    {
//                        std::cout << "metadata check\n";
                        if (!existing_loaded->getMetadata().isEquivalent(cur_reference.m_metadata, false))
                        {
//                            std::cout << "failed\n";
                            // As the reference is not resolved, this can happen multiple times if refreshParams is repeatedly
                            // called for this config
                            // TODO CM make errors a set? What about other members of result?
                            QString error_string = QString("\"%1\" references \"%2\", which has mismatching metadata.").
                                    arg(cur_param_name).arg(cur_reference.m_referenced_param);
                            if (!result.m_errors.contains(error_string)) result.m_errors << error_string;
                            resolved_all_parts = false;
                            break;
                        }
                    }

                    // If a string variable is part of the built value, unquote it. See SWE-4315.
                    QString cur_reference_part = existing_loaded->getValue();
                    if (existing_loaded->getMetadata().m_type == AVConfig2Types::getTypeName<QString>())
                    {
                        if (!AVFromString(cur_reference_part, cur_reference_part))
                        {
                            result.m_errors.push_back("Failed to parse " + cur_param_name + " reference part from \""
                                                      + cur_reference_part + "\"");
                            resolved_all_parts = false;
                            break;
                        }
                    }
                    resolved_param += cur_reference_part;
                } else
                {

                    QString fromstr;
                    if (!AVFromString(l, fromstr))
                    {
                        result.m_errors.push_back("Failed to parse " + cur_param_name + " reference part " + l);
                        resolved_all_parts = false;
                        break;
                    }
                    resolved_param += fromstr;
                }
            }

            // Everything's ok, resolve the reference and move it from m_unresolved_references to
            // m_loaded_param
            if (resolved_all_parts)
            {
                resolved_any_reference = true;

                AVConfig2StorageLocation resolved_location(cur_reference.getLocation());
                resolved_location.m_source = AVConfig2StorageLocation::PS_RESOLVED_REFERENCE;

                // Don't overwrite any value obtained via override mechanisms...
                AVConfig2ImporterClient::LoadedParameter * existing_loaded_param =
                        m_loaded_data.getElementByName<LoadedParameter>(cur_param_name);
                if (existing_loaded_param == nullptr)
                {
                    // No harm is done by using the same location as the reference, because
                    // AVConfig2Saver::save takes care not to overwrite references with loaded values.
                    m_loaded_data.addElement(LoadedParameter(
                                                 resolved_param, cur_reference.m_metadata,
                                                 resolved_location));
                } else if (existing_loaded_param->getLocation().m_source == AVConfig2StorageLocation::PS_DEFAULT_OPTIONAL ||
                           existing_loaded_param->getLocation().m_source == AVConfig2StorageLocation::PS_INHERITED_PARAMETER)
                {
//                    std::cout << "param existed, overwriting\n";
                    // simply overwrite defaults for optionals and inherited parameters
                    *existing_loaded_param = LoadedParameter(resolved_param, cur_reference.m_metadata, resolved_location);
                }

//                std::cout << qPrintable(cur_param_name) << " now has value "<< qPrintable(resolved_param) << "\n";

                m_unresolved_references.erase(reference_it++);
            } else ++reference_it;
        }
    } while (!m_unresolved_references.isEmpty() && resolved_any_reference);

    // Construct meaningful error message about unresolved references
    if (!m_unresolved_references.isEmpty())
    {
        QStringList unresolved_references;
        for (LoadedReferenceContainer::const_iterator it = m_unresolved_references.begin();
             it != m_unresolved_references.end();
             ++it)
        {
            result.m_unresolved_references.insert(
                    QString("%1 -> %2").arg(it.key()).arg(it.value().m_referenced_param));
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Container::configDeleted(void *c)
{
    // We can't dynamic cast as the object already is destroyed (slightly hackish...)
    AVConfig2 *config = static_cast<AVConfig2*>(c);

    // This can only happen when configs are copied around (which should only be the case for
    // unit tests)
    if (!m_config_object.contains(config)) return;

    const bool should_remove = config->shouldRemoveRegisteredParamOnDestruction();

    if (m_registered_param.containsConfig(config))
    {
        for (QSharedPointer<RegisteredParameterBase> p : m_registered_param.getParameterList(config))
        {
            if (should_remove && !m_removed_param_names.contains(p->getMetadata().m_name))
            {
                m_removed_param_names.append(p->getMetadata().m_name);
            }
            RegisteredParameterByNameContainer::const_iterator it =
                    m_registered_param_names.find(p->getMetadata().m_name);
            AVASSERT(it != m_registered_param_names.end());
            m_registered_param_names.erase(it);
        }
    }

    m_registered_param.removeConfig(config);

    if (m_registered_subconfig.contains(config))
    {
        for (QSharedPointer<RegisteredSubconfigBase> s : m_registered_subconfig.value(config))
        {
            RegisteredSubconfigByNameContainer::const_iterator it =
                    m_registered_subconfig_names.find(s->getPrefix());
            AVASSERT(it != m_registered_subconfig_names.end());
            m_registered_subconfig_names.erase(it);
        }
    }
    m_registered_subconfig.remove(config);
    m_config_object.remove(config);
    // We could also remove the config fom m_unrefreshed, but this is not sufficient and is thus
    // not done. AVConfig2Container::refreshAllParams checks whether the pointer in m_unrefreshed
    // is still valid by checking whether there still are registered parameters.
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2Container::checkNewLoadedParam(const QString& name, const AVConfig2StorageLocation& location)
{
    // First, check that a parameter is not registered both with a value and as reference
    const LoadedParameter * already_loaded = m_loaded_data.getElementByName<LoadedParameter>(name);
    const LoadedReference * existing_ref   = m_loaded_data.getElementByName<LoadedReference>(name);

    // m_loaded_param can already contain the entry via overrides, inheritance or as default for optionals.
    // This is OK, either the metadata is supplemented and the override value is used (for cmdline overrides
    // or dev file overrides), or the value is overwritten (for inherited params and default values for optionals).
    bool duplicate =
        (existing_ref != 0 &&
            existing_ref->getLocation().m_source != AVConfig2StorageLocation::PS_INHERITED_PARAMETER)
            ||
        (already_loaded != 0 &&
            (already_loaded->getLocation().m_source != AVConfig2StorageLocation::PS_CMDLINE_PARAM &&
             already_loaded->getLocation().m_source != AVConfig2StorageLocation::PS_CMDLINE_OVERRIDE &&
             already_loaded->getLocation().m_source != AVConfig2StorageLocation::PS_FILE_DEV_OVERRIDE &&
             already_loaded->getLocation().m_source != AVConfig2StorageLocation::PS_INHERITED_PARAMETER &&
             already_loaded->getLocation().m_source != AVConfig2StorageLocation::PS_DEFAULT_OPTIONAL));

    if (duplicate)
    {
        const AVConfig2StorageLocation *existing_location;
        if (already_loaded == 0)
        {
            existing_location = &existing_ref->getLocation();
        } else
        {
            existing_location = &already_loaded->getLocation();
        }

        QString p1 = name + " (" + location.getFullFilePath() + ")";
        QString p2 = name + " (" + existing_location->getFullFilePath() + ")";

        if (!m_loaded_duplicates.contains(p1))
        {
            m_loaded_duplicates << p1;
        }
        if (!m_loaded_duplicates.contains(p2))
        {
            m_loaded_duplicates << p2;
        }
        return false;
    } else return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2Container::handleVersionTranslation(AVConfig2 *config, QStringList& errors)
{
    // Bail if this config isn't versioned.
    if (config->getVersion() == AVConfig2::NO_VERSION) return false;

    // Retrieve or create the version which is in the config file for the prefix of the config class
    // Keep it so it can be incremented later on.
    QString version_param_name = config->getPrefix() + ".avconfig2_class_version";

    // If no version is yet present in the config file, initialize it with value "0".
    if (m_loaded_data.getElementByName<LoadedParameter>(version_param_name) == nullptr)
    {
        AVConfig2Metadata metadata;
        metadata.m_type = AVConfig2Types::getTypeName<uint>();
        metadata.m_name = version_param_name;
        metadata.setHelp("Used internally for avconfig2 class versioning");

        AVConfig2StorageLocation location = AVConfig2Saver::findStorageLocationForMissingParameter(
                    version_param_name, *this, QRegularExpression(""), QRegularExpression(""));
        addParameter("0", metadata, location);
    }

    const LoadedParameter * version_number = m_loaded_data.getElementByName<LoadedParameter>(version_param_name);
    AVASSERT(version_number != 0);

    int cur_version;
    if (!AVFromString(version_number->getValue(), cur_version))
    {
        AVLogFatal << "Failed to parse version for " << config->getPrefix();
    }

    // Nothing to do if the versions match...
    if (cur_version == config->getVersion()) return false;

    if (cur_version > config->getVersion())
    {
        errors += QString("Unknown version %1 encountered for namespace %2 (max known is %3).").
                arg(cur_version).arg(config->getPrefix()).arg(config->getVersion());
        return false;
    }

    int i;
    for (i=cur_version; i<config->getVersion(); ++i)
    {
        AVLogInfo << "Attempting to translate from version " << i << " to " << i+1
                  << " for namespace " << config->getPrefix();
        if (!config->translateToNextVersion(i))
        {
            AVLogInfo << "Failure.";
            errors += QString("Version translation failed for namespace %1 from version %2 to %3.").
                    arg(config->getPrefix()).arg(i).arg(i+1);
            --i;
            break;
        }
    }

    m_loaded_data.addElement(LoadedParameter(AVToString(i+1), version_number->getMetadata(), version_number->getLocation()));

    return true;
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2::SubconfigTemplateContainer::SubconfigTemplateContainer() : AVConfig2Container()
{
    // Default is lenient checking, which produces verbose warnings.
    // Set to strict to avoid any log output while registering subconfig template parameters.
    // See ATC-3004.
    setCheckingMode(CheckingMode::CM_STRICT);
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2::SubconfigTemplateContainer::~SubconfigTemplateContainer()
{
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2::SubconfigTemplateContainer::createSubconfigTemplateParameters()
{
    for (const ConfigWithRegisteredParameters& config_with_params : m_registered_param)
    {
        for (auto& registered_param : config_with_params.second)
        {
            const AVConfig2Metadata& metadata = registered_param->getMetadata();
            m_loaded_data.addSubconfigTemplateParameter(metadata.m_default_value, metadata, AVConfig2StorageLocation());
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2::SubconfigTemplateContainer::transferSubconfigTemplateData(AVConfig2LoadedData& loaded_data)
{
    // add subconfig template parameters from this container to loaded_data
    for (uint i = 0; i < m_loaded_data.getElementCount<LoadedSubconfigTemplateParameter>(); ++i)
    {
        LoadedSubconfigTemplateParameter param = m_loaded_data.getElementByIndex<LoadedSubconfigTemplateParameter>(i);
        loaded_data.addSubconfigTemplateParameter(param.getValue(), param.getMetadata(), param.getLocation());
    }
}

///////////////////////////////////////////////////////////////////////////////

const QString AVConfig2Global::DEV_CONFIG_SUBDIR        = "dev_configs";
const QString AVConfig2Global::AVCONFIG2_ADD_FACTS           = "AVCONFIG2_ADD_FACTS";
const QString AVConfig2Global::AVCONFIG2_INITIAL_CONFIG_PATH = "AVCONFIG2_INITIAL_CONFIG_PATH";
const QString AVConfig2Global::AVCONFIG2_INITIAL_CONFIG      = "AVCONFIG2_INITIAL_CONFIG";

///////////////////////////////////////////////////////////////////////////////

AVConfig2Global::AVConfig2Global(
        int argc, char**argv, bool allow_extra_cmdline,
        const QString& default_app_name, const QString& default_process_name) :
    m_builtin_params(0),
    m_qt_builtin_params(0),
    m_main_config_loaded(false),
    m_refresh_core_parameters_called(false)
{
    parseCmdlineParams(argc, argv, allow_extra_cmdline);
    m_builtin_params = new (LOG_HERE) AVConfig2BuiltinParams(
            *this, default_app_name, default_process_name);
    m_qt_builtin_params = new (LOG_HERE) AVConfig2QtBuiltinParams(*this);
    RefreshResult res = refreshAllParams();

    if (!res.refreshSuccessful())
    {
        // There shouldn't be any params registered yet...
        AVASSERTNOLOGGER(res.m_missing_parameters.isEmpty());
        AVASSERTNOLOGGER(res.m_metadata_mismatch.isEmpty());

        std::cout << "** ERROR: Failed to read command line params:\n"
                  << qPrintable(res.getErrorMessage()) << "\n"
                  << "Aborting.\n";
        exit(-1);
    }

    // TODO CM move to AVEnvironment
    AVEnvironment::setApplicationName  (m_builtin_params->m_app_name);
    AVEnvironment::setApplicationConfig(m_builtin_params->m_cfg_dir);
    QString env_process_name = AVEnvironment::getEnv("PROCESS_NAME", false);
    if (env_process_name.isEmpty())
    {
        AVEnvironment::setProcessName(default_process_name);
    } else
    {
        AVEnvironment::setProcessName(env_process_name);
    }
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2Global::~AVConfig2Global()
{
    clearKeptConfigInstances();
    delete m_builtin_params;
    delete m_qt_builtin_params;
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2Global& AVConfig2Global::initializeSingleton(
        int argc, char**argv, bool allow_extra_cmdline,
        const QString& app_name, const QString& process_name)
{
    return setSingleton(new (LOG_HERE) AVConfig2Global(
            argc, argv, allow_extra_cmdline, app_name, process_name));
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Global::initialize()
{
    QString env_fact_string = AVEnvironment::getEnv(AVConfig2Global::AVCONFIG2_ADD_FACTS, false, AVEnvironment::NoPrefix);
    if(!env_fact_string.isEmpty())
    {
        AVConfig2StorageLocation::FactContainer env_facts;
        if(!AVFromString(env_fact_string, env_facts))
        {
            AVLogFatal << "error parsing facts added by environment variable (" << AVConfig2Global::AVCONFIG2_ADD_FACTS << "): "
                       << env_fact_string << "\n";
        }

        AVLogInfo << AVCONFIG2_ADD_FACTS << " is " << env_facts;
        QString fact_error = addFacts(env_facts);
        if (!fact_error.isEmpty())
        {
            AVLogFatal << "error adding facts added by environment variable (" << AVConfig2Global::AVCONFIG2_ADD_FACTS << "): "
                       << fact_error << "\n";
        }
    }

    QString fact_error = addFacts(m_builtin_params->m_add_facts);
    if (!fact_error.isEmpty())
    {
        AVLogFatal << "error adding fact: " << fact_error;
    }

    if (m_builtin_params->m_suppress_transient_problems)
    {
        AVLog::setExecutionPhase(AVLog::EP_STARTING_UP);
    }

    if (!m_builtin_params->getInitialConfig().isNull())
    {
        if (!loadConfigInternal(m_builtin_params->getInitialConfig(), false, getInitialConfigSearchPaths()))
        {
            AVLogFatal << "Failed to find initial config \"" << m_builtin_params->getInitialConfig() << "\"";
        } else
        {
            AVLogInfo << "Initial config \""
                      << m_builtin_params->getInitialConfig()
                      << "\" loaded.";
        }
    }

    // load it after initial config, so it will be found in the project's search path
    loadProjectFactsConfig();
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Global::setApplicationHelpText(const QString& text)
{
    m_application_help = text;
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2Global::loadMainConfig(const QString& config, bool optional)
{
    if (m_main_config_loaded)
    {
        AVLogFatal << "AVConfig2Global::loadMainConfig() can be used only once per program; use loadConfig instead.";
    }
    m_main_config_loaded = true;
    QString name = m_builtin_params->m_main_config_override;
    if (name.isEmpty()) name = config;
    bool ret = loadConfig(name, optional);

    // Only make a missing config file fatal if it was explicitly requested via the command line; otherwise this could just be an ordinary
    // "-save" scenario. See SWE-1434.
    if (!ret && !m_builtin_params->m_main_config_override.isEmpty())
    {
        AVLogFatal << "AVConfig2Global::loadMainConfig: failed to load specified main config \""
                   << m_builtin_params->m_main_config_override << "\"";
    }

    return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2Global::refreshCoreParameters()
{
    if (AVProcessTerminated)
        return false;

    if (!m_builtin_params->m_main_config_override.isEmpty() && !m_main_config_loaded)
    {
        AVLogFatal << "A main config override was specified, but no main config "
                   << "was loaded by the application.";
    }

    if (!AVThread::isMainThread())
    {
        AVLogFatal << "AVConfig2Global::refreshCoreParameters not called from main thread - "
                   << "did you call AVApplicationInit()?";
    }

    if (getFacts().isEmpty())
    {
        AVLogInfo << "AVConfig2Global::refreshCoreParameters: No facts in use.";
    } else
    {
        AVLogInfo << "AVConfig2Global::refreshCoreParameters: Our facts are: "
                  << AVToString(getFacts());
    }

    // Do this first so the params changed on the command line are correctly reflected in the help
    // message.
    RefreshResult ret = refreshAllParams();
    checkCmdlineParams(ret.m_cmdline_errors);

    // Treat those errors before handling the builtin params, as we don't want e.g. -save to hide errors in the command
    // line.
    if (!ret.m_cmdline_errors.isEmpty())
    {
        AVLogFatal << "Failed to parse the command line: " << ret.m_cmdline_errors.join("\n");
    }

    if (!checkAndUpdateProjectFactsConfig())
    {
        AVLogFatal << "Project config error, aborting";
        return false;
    }

    if (!handleBuiltinParams()) return false;

    ret.m_errors += m_configuration_errors;

    if (!ret.refreshSuccessful())
    {
        AVLogFatal << ret.getErrorMessage()
                   << "Error loading config, aborting";

        return false;
    }

    AVASSERT(!m_refresh_core_parameters_called);
    m_refresh_core_parameters_called = true;

    // Only auto-save if everything else was OK...
    if (m_save_required)
    {
        AVLogWarning << "Outdated config encountered while loading parameters, doing an auto-save.";
        internalSave();
    }

    clearKeptConfigInstances();

    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2Global::getRefreshCoreParametersCalled() const
{
    return m_refresh_core_parameters_called;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Global::registerConfigurationError(const QString& error)
{
    if (m_refresh_core_parameters_called)
    {
        AVLogFatal << error;
    } else
    {
        m_configuration_errors << error;
    }
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2Global::isApplicationStartupAllowed() const
{
    return m_builtin_params->m_help.isNull() &&
           !m_builtin_params->isSaveRequested() &&
           !m_builtin_params->m_dump_config  &&
           !m_builtin_params->m_version      &&
           m_builtin_params->getUpdateUnusedParamsFile().isNull() &&
           m_builtin_params->getPurgeUnusedParamsFile().isNull();
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2Global::getInitialConfig() const
{
    return m_builtin_params->getInitialConfig();
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2Global::isSaveRequested() const
{
    return m_builtin_params->isSaveRequested();
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2Global::isHelpRequested() const
{
    return !m_builtin_params->m_help.isNull();
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2Global::isDumpConfigRequested() const
{
    return m_builtin_params->m_dump_config;
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2Global::isVersionRequested() const
{
    return m_builtin_params->m_version;
}

///////////////////////////////////////////////////////////////////////////////

uint AVConfig2Global::getStartupDebugLevel() const
{
    return m_builtin_params->m_debug;
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2Global::isColdstartRequested() const
{
    return m_builtin_params->m_coldstart;
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2Format AVConfig2Global::getSaveFormat() const
{
    return m_builtin_params->getSaveFormat();
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Global::internalSave()
{
    AVConfig2Saver saver;
    saver.setForceOptionals(m_builtin_params->getForceSaveOptionals());
    saver.setSaveOrder(m_builtin_params->getSaveOrder());
    saver.setSaveMode(AVConfig2Saver::SM_ONLY_REGISTERED);
    saver.setSaveValueMode(AVConfig2Saver::SVM_KEEP_LOADED_VALUES);
    saver.setConfigFormat(m_builtin_params->getSaveFormat());
    saver.setSaveDir(m_builtin_params->m_save_dir);
    saver.save(*this);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Global::keepOrDeleteConfigInstance(std::unique_ptr<AVConfig2> config)
{
    if (!m_refresh_core_parameters_called)
    {
        m_kept_config_instances.push_back(std::move(config));
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Global::clearKeptConfigInstances()
{
    m_kept_config_instances.clear();
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2Global::getHelp(const QString& group) const
{
    static const uint PADDING_LEFT    = 2;
    static const uint PADDING_DOTS    = 25;
    static const uint MAX_LINE_LENGTH = 80;
    static const uint METADATA_WIDTH  = 18;
    // Length of line until help text
    static const uint HELP_TEXT_OFFSET = PADDING_LEFT + PADDING_DOTS + 4;

    QString ret;
    QTextStream out_stream(&ret, QIODevice::WriteOnly);

    if (!m_application_help.isEmpty())
    {
        out_stream << AVBreakString(m_application_help, MAX_LINE_LENGTH).join("\n") << "\n\n";
    } else
    {
        out_stream << "Usage: " << AVEnvironment::getProcessName() << " [options]\n\n";
        out_stream << "This application does not provide a detailed description "
                   << "of its functionality.\n\n";
    }

    bool any_param_printed = false;

    // Delay writing group header until a parameter with a command line switch is found; this
    // avoids writing empty groups in help text
    QString next_group;
    for (RegisteredParameterContainer::const_iterator config_it = m_registered_param.begin();
         config_it != m_registered_param.end();
         ++config_it)
    {
        const AVConfig2* cur_config = config_it->first;
        const RegisteredParameterList& cur_param_list = config_it->second;

        if (!cur_config->isShowHelpPerDefaultEnabled() && group != "all") continue;

        next_group = cur_config->getHelpGroup();
        if (next_group.isEmpty()) next_group = "???";

        for (RegisteredParameterList::const_iterator param_it = cur_param_list.begin();
                param_it != cur_param_list.end();
                ++param_it)
        {
            const RegisteredParameterBase& cur_param_info = **param_it;
            const AVConfig2Metadata&       cur_metadata   = cur_param_info.getMetadata();
            QString                        cur_param_name = cur_metadata.m_name;

            QString cmdline_switch = cur_param_info.isPureCmdlineOption() ?
                    cur_param_name : cur_param_info.getCommandlineSwitch();
            // Don't output help for entries which cannot be changed on the command line
            if (cmdline_switch.isEmpty()) continue;

            if (!next_group.isEmpty())
            {
                if (!any_param_printed && group.isEmpty())
                {
                    out_stream << "Only showing process-specific parameters.\n"
                               << "For the complete set of options, use \"-help all\".\n\n";
                }

                out_stream << next_group << " options:\n\n";
                next_group = QString::null;
            }

            any_param_printed = true;

            const LoadedParameter * existing_loaded = m_loaded_data.getElementByName<LoadedParameter>(cur_param_name);
            QString value = existing_loaded != nullptr ? existing_loaded->getValue() : "<not found>";

            out_stream << qSetFieldWidth(PADDING_LEFT+1) << qSetPadChar(' ') << "-";
            // Don't display "x" for command line switches (which require no argument)
            QString cmdline_switch_string = cmdline_switch + " ";
            if (cur_param_info.getMetadata().m_type != AVConfig2Types::getTypeName<CmdlineSwitch>())
            {
                cmdline_switch_string += "x ";
            }
            out_stream.setFieldAlignment(QTextStream::AlignLeft);
            out_stream << qSetFieldWidth(PADDING_DOTS) << qSetPadChar('.') << cmdline_switch_string
                       << qSetFieldWidth(0);
            out_stream.setFieldAlignment(QTextStream::AlignRight);
            out_stream << ".. ";

            if (cur_metadata.m_restriction != 0)
            {
                out_stream << AVToString(*cur_metadata.m_restriction) << "\n"
                           << qSetFieldWidth(HELP_TEXT_OFFSET) << qSetPadChar(' ')
                           << " " << qSetFieldWidth(0);
            }

            // Format help text
            QStringList help_split = cur_metadata.getHelp().split(" ");
            uint line_length = HELP_TEXT_OFFSET;
            while(!help_split.isEmpty())
            {
                uint word_length = help_split.front().length();
                if (line_length == HELP_TEXT_OFFSET || // always print at least one word
                        line_length + word_length <= MAX_LINE_LENGTH)
                {
                    out_stream << help_split.front();
                    line_length += word_length;
                    if (line_length < MAX_LINE_LENGTH)
                    {
                        ++line_length;
                        out_stream << " ";
                    }
                    help_split.pop_front();
                } else
                {
                    out_stream << "\n";
                    out_stream << qSetFieldWidth(HELP_TEXT_OFFSET) << qSetPadChar(' ') << ""
                               << qSetFieldWidth(0);
                    line_length = HELP_TEXT_OFFSET;
                }
            }
            out_stream << "\n";

            if (cur_param_info.isPureCmdlineOption())
            {
                out_stream << qSetFieldWidth(HELP_TEXT_OFFSET + METADATA_WIDTH) << qSetPadChar(' ')
                           << "Default value: "
                           << qSetFieldWidth(0)
                           << AVToString(cur_metadata.m_default_value) << "\n";
            } else
            {
                if (!cur_metadata.m_default_value.isNull())
                {
                    out_stream << qSetFieldWidth(HELP_TEXT_OFFSET + METADATA_WIDTH)
                               << qSetPadChar(' ')
                               << "suggested value: "
                               << qSetFieldWidth(0)
                               << cur_metadata.m_default_value << "\n";
                }
                if (!cur_metadata.m_suggested_reference.isNull())
                {
                    out_stream << qSetFieldWidth(HELP_TEXT_OFFSET + METADATA_WIDTH)
                               << qSetPadChar(' ')
                               << "suggested reference: "
                               << qSetFieldWidth(0)
                               << cur_metadata.m_suggested_reference << "\n";
                }

                out_stream << qSetFieldWidth(HELP_TEXT_OFFSET + METADATA_WIDTH) << qSetPadChar(' ')
                           << "current value: "
                           << qSetFieldWidth(0)
                           << value << "\n";
            }

            out_stream << qSetFieldWidth(HELP_TEXT_OFFSET + METADATA_WIDTH) << qSetPadChar(' ')
                       << "Type: "
                       << qSetFieldWidth(0)
                       << cur_metadata.m_type << "\n";

            out_stream << "\n";
        }
    }

    if (!any_param_printed)
    {
        out_stream << "This application does not provide any application-specific command line "
                   << "parameters.\n"
                   << "For the complete set of options, use \"-help all\".\n\n";
    }

    return ret;
}

///////////////////////////////////////////////////////////////////////////////
using GlobalRegisterFunctionsVector = std::vector<AVConfig2Global::ConfigInitializerFunc>;
Q_GLOBAL_STATIC(GlobalRegisterFunctionsVector, global_register_functions)

void AVConfig2Global::registerConfigInitializer(AVConfig2Global::ConfigInitializerFunc func)
{
    global_register_functions->push_back(func);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Global::callConfigSingletonInitializers()
{
    if (AVConfig2Global::isSingletonInitialized())
    {
        for(const auto& func: *global_register_functions)
        {
            func();
        }
    }
    else
    {
        AVLogWarning << Q_FUNC_INFO
                     << ": AVConfig2Global singleton not initialized, global config initializers will not be called.";
    }
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2Global::handleBuiltinParams()
{
    AVASSERT(m_builtin_params != 0);

    int exclusive_count = 0;
    if (!m_builtin_params->m_help.isNull()) ++exclusive_count;
    if (m_builtin_params->isSaveRequested()) ++exclusive_count;
    if (m_builtin_params->m_dump_config)    ++exclusive_count;
    if (m_builtin_params->m_version)        ++exclusive_count;
    if (exclusive_count > 1)
    {
        AVLogFatal << "Only one of -help, -save, -dump_config, -version is allowed.";
    }

    if (!m_builtin_params->m_help.isNull())
    {
        // Output via stdout because logging is disabled.
        std::cout << qPrintable(getHelp(m_builtin_params->m_help));
        AVASSERT(!isApplicationStartupAllowed());
    }
    if (m_builtin_params->isSaveRequested())
    {
        internalSave();
        AVASSERT(!isApplicationStartupAllowed());
    }
    if (m_builtin_params->getUpdateUnusedParamsFile() != QString::null)
    {
        updateUnusedParams(m_builtin_params->getUpdateUnusedParamsFile());
        AVASSERT(!isApplicationStartupAllowed());
    }
    if (m_builtin_params->getPurgeUnusedParamsFile() != QString::null)
    {
        purgeUnusedParams(m_builtin_params->getPurgeUnusedParamsFile());
        AVASSERT(!isApplicationStartupAllowed());
    }
    if (m_builtin_params->m_dump_config)
    {
        dumpConfig();
        AVASSERT(!isApplicationStartupAllowed());
    }
    if (m_builtin_params->m_version)
    {
        printBuildVersion();
        AVASSERT(!isApplicationStartupAllowed());
    }

    return isApplicationStartupAllowed();
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Global::updateUnusedParams(const QString& file_name)
{
    // See whether the file already exists and either update or create it.
    // If updating, we only need to remove our used params from the file.
    // If creating, we need to initialize the file with all loaded params minus the ones used by us.
    // Assumption: all processes share the same set of loaded params.
    QFile unused_file(file_name);
    UnusedParameterContainer unused_info;

    if (unused_file.open(QIODevice::ReadOnly))
    {
        AVLogInfo << "Updating unused params in " << file_name;

        // Retrieve the current value and prepare the file for writing the new value
        {
            QTextStream instream(&unused_file);
            QString unused_string = instream.readLine();
            if (!AVFromString(unused_string, unused_info))
            {
                AVLogFatal << "Failed to parse previous content of " << file_name
                           << ", and refusing to overwrite it.";
            }
            unused_file.close();
            unused_file.open(QIODevice::Truncate | QIODevice::WriteOnly);
        }

        // Actually update the unused params by removing any params that we use.
        for(const ConfigWithRegisteredParameters& pair : m_registered_param)
        {
            const RegisteredParameterList& cur_list = pair.second;
            for (RegisteredParameterList::const_iterator reglist_it = cur_list.begin();
                    reglist_it != cur_list.end();
                    ++reglist_it)
            {
                const RegisteredParameterBase& cur_reg = **reglist_it;
                const QString cur_name = cur_reg.getMetadata().m_name;

                for (UnusedParameterContainer::iterator unused_it = unused_info.begin();
                     unused_it != unused_info.end();
                     /* do nothing */)
                {
                    if (unused_it.value().remove(cur_name))
                    {
                        AVLogInfo << "Removed " << cur_name << " from " << unused_it.key();
                    }

                    if (unused_it.value().isEmpty())
                    {
                        AVLogInfo << unused_it.key() << " has no more unused parameters.";
                        unused_info.erase(unused_it++);
                    } else ++unused_it;
                }
            }
        }
    } else
    {
        AVASSERT(unused_file.open(QIODevice::WriteOnly));
        AVLogInfo << "Saving unused params to " << file_name;

        // First, build a list of all loaded params. Then iterate over registered params and remove
        // those from the set.
        QSet<QString> unused_params;
        for (const QString& key : m_loaded_data.getElementKeys<LoadedParameter>())
        {
            // If this param was loaded twice, just ignore it here. There are other places where
            // this is dealt with.
            unused_params.insert(key);
        }
        for (RegisteredParameterContainer::const_iterator reg_it = m_registered_param.begin();
                reg_it != m_registered_param.end();
                ++reg_it)
        {
            const AVConfig2Container::RegisteredParameterList& cur_list = reg_it->second;

            for (RegisteredParameterList::const_iterator reglist_it = cur_list.begin();
                    reglist_it != cur_list.end();
                    ++reglist_it)
            {
                // If this param wasn't loaded, just ignore it here. There are other places where
                // this is dealt with.
                unused_params.remove((*reglist_it)->getMetadata().m_name);
            }
        }

        // Now, compile the information about unused params
        for (const QString& cur_unused_name: unused_params)
        {
            // TODO CM! handle location here as well...
            const LoadedParameter& cur_unused_param = *m_loaded_data.getElementByName<LoadedParameter>(cur_unused_name);

            // skip parameters not read from a file
            if (cur_unused_param.getLocation().m_format == CF_LAST)
            {
                AVLogInfo << "AVConfig2Global::updateUnusedParams: skipping " << cur_unused_name;
                continue;
            }

            QString cur_format =
                    AVConfig2StorageLocation::configFormatToString(cur_unused_param.getLocation().m_format);
            unused_info[qMakePair(cur_format, cur_unused_param.getLocation().m_name)].
                    insert(cur_unused_name);
        }
    }

    // Save updated info to the file
    QString desc = AVToString(unused_info) + "\n";
    unused_file.write(qPrintable(desc), desc.length());
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Global::purgeUnusedParams(const QString& file_name)
{
    Q_UNUSED(file_name);
    AVLogFatal << "AVConfig2Global::purgeUnusedParams: not yet implemented.";
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Global::loadProjectFactsConfig()
{
    m_project_config.reset(AVConfig2ProjectFactsConfig::createFactsConfig(*this));

    if (!m_project_config.get())
    {
        AVLogInfo << "No project facts configuration found.";
    }
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2Global::checkAndUpdateProjectFactsConfig()
{
    bool ret = true;
    if (m_project_config.get() == nullptr) return ret;

    QSet<QString> errors;

    // First, check the facts passed to our program
    for (QMap<QString, QString>::const_iterator it = getFacts().begin(); it != getFacts().end(); ++it)
    {
        QString name  = it.key();
        QString value = it.value();

        QString error = m_project_config->checkAndUpdateFact(name, value);
        if (!error.isEmpty())
        {
            errors.insert(error);
        }
    }

    // Then, check the fact conditions of all our loaded elements.
    for (uint e=0; e<m_loaded_data.getElementTotalCount(); ++e)
    {
        const LoadedElement& cur_element = m_loaded_data.getElementByTotalIndex(e);
        QString error = m_project_config->checkAndUpdateCondition(cur_element.getLocation());
        if (!error.isEmpty())
        {
            errors.insert(error);
        }
    }

    if (!errors.isEmpty())
    {
        if (isSaveRequested())
        {
            AVLOG_ENTER(saving project facts config);

            AVConfig2Saver saver;
            saver.setParameterSaveFilter(QRegularExpression(AVConfig2ProjectFactsConfig::FACTS_CONFIG_NAMESPACE + "\\..*"));
            saver.setSaveValueMode(AVConfig2Saver::SVM_ADOPT_PROCESS_VALUES);
            saver.setSaveMode(AVConfig2Saver::SM_ONLY_REGISTERED);
            saver.save(*this);

            // Workaround: delete our config, so the values we just saved are not overwritten during the default save operation.
            // Alternatively, we could load the saved parameters...
            m_project_config.reset(nullptr);

            return true; // program won't start up anyway, let other save logic happen.
        } else
        {
            for (const QString& e : errors)
            {
                AVLogError << e;
            }
            return false;
        }
    } else return true;
}

///////////////////////////////////////////////////////////////////////////////

QStringList AVConfig2Global::getInitialConfigSearchPaths()
{
    QSet<QString> ret;

    // Part#1: dev_configs heuristics, searching below $PROJECTS_HOME
    QString projects_home_path = AVEnvironment::getProjectsHome();
    bool search_projects_home = !projects_home_path.isNull();
    if (!search_projects_home)
    {
        AVLogDebug << "AVConfig2Global::getInitialConfigSearchPaths: "
                   << "PROJECTS_HOME not set.";
    } else if (!QFile::exists(projects_home_path))
    {
        // Don't make this fatal as workaround for BS-575.
        // This occurs when using the installed SVN package (e.g. in SAM builds).
        search_projects_home = false;
        AVLogWarning << "AVConfig2Global::getInitialConfigSearchPaths: "
                     << "PROJECTS_HOME "
                     << projects_home_path
                     << " doesn't exist although the environment variable is set.";
    }

    if (search_projects_home)
    {
        // pairs absolute path + nesting depth
        typedef QPair<QString, uint> DirStackEntry;
        typedef QVector<DirStackEntry> DirStack;
        DirStack dirs_to_search;
        dirs_to_search.push_back(qMakePair(projects_home_path, 0u));

        while (!dirs_to_search.isEmpty())
        {
            DirStackEntry d(dirs_to_search.back());
            dirs_to_search.pop_back();

            QString cur_dir   = d.first;
            uint    cur_depth = d.second;

            QStringList subdirs = QDir(cur_dir).entryList(QDir::Dirs);
            for (int sd=0; sd<subdirs.size(); ++sd)
            {
                QString cur_subdir = subdirs[sd];
                if (cur_subdir == "." || cur_subdir == "..") continue;

                cur_subdir = cur_dir + "/" + cur_subdir;

                if (cur_subdir.endsWith(DEV_CONFIG_SUBDIR))
                {
                    ret.insert(cur_subdir);
                } else if (cur_depth != INITIAL_CONFIG_SEARCH_DEPTH)
                {
                    dirs_to_search.push_back(qMakePair(cur_subdir, cur_depth+1));
                }
            }
        }
    }

    // Part#2: evaluate the environment variable
    QStringList initial_config_paths = AVEnvironment::getPath(AVCONFIG2_INITIAL_CONFIG_PATH);
    for (const QString& p : initial_config_paths)
    {
        QString expanded_path = AVEnvString(p).expandEnvString();
        QFileInfo dir_info(expanded_path);
        if (!dir_info.isDir())
        {
            AVLogFatal << AVCONFIG2_INITIAL_CONFIG_PATH << " is not a directory: " << expanded_path;
        }
        ret.insert(expanded_path);
    }

    AVLogDebug << "Search paths for initial config: " << AVEnvironment::getApplicationConfig() << ", "
               << ret.toList().join(", ");

    return ret.toList();
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2::AVConfig2(const QString &prefix, AVConfig2Container &container) :
    m_version(NO_VERSION),
    m_show_help_per_default(true),
    m_last_refresh_params_successful(false),
    m_prefix(prefix),
    m_remove_registered_params_on_destruction(false),
    m_container(&container)
{
    container.addConfigObject(this);
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2::AVConfig2(UnitTestConstructorSelector) :
    m_version(NO_VERSION),
    m_show_help_per_default(true),
    m_last_refresh_params_successful(false),
    m_prefix(QString::null),
    m_remove_registered_params_on_destruction(false),
    m_container(nullptr)
{
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2::~AVConfig2()
{
    if (m_container) m_container->configDeleted(this);
}

///////////////////////////////////////////////////////////////////////////////

int AVConfig2::getVersion() const
{
    return m_version;
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2::translateToNextVersion(uint prev_version)
{
    Q_UNUSED(prev_version);
    AVLogFatal << "AVConfig2::translateToNextVersion not implemented for " << getPrefix();
    return false;
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2::refreshParams(AVConfig2Container::RefreshResult *result)
{
    AVASSERT(m_container);
    m_last_refresh_params_successful = m_container->refreshParams(this, result);
    return m_last_refresh_params_successful;
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2::refreshSuccessful() const
{
    return m_last_refresh_params_successful;
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2::postRefresh()
{
    return QString::null;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2::setConfigMapKey(const QString& key)
{
    m_config_map_key = key;
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2::getConfigMapKey() const
{
    return m_config_map_key;
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2::getPrefix() const
{
    return m_prefix;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2::setHelpGroup(const QString &help_group)
{
    m_help_group = help_group;
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2::getHelpGroup() const
{
    return m_help_group;
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2::isShowHelpPerDefaultEnabled() const
{
    return m_show_help_per_default;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2::setVersion(uint version)
{
    AVASSERT(m_version == NO_VERSION);
    AVASSERT(version > 0);
    m_version = version;
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2::shouldRemoveRegisteredParamOnDestruction() const
{
    return m_remove_registered_params_on_destruction;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2::setRemoveRegisteredParamsOnDestruction(bool remove)
{
    m_remove_registered_params_on_destruction = remove;
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2::hasAssociatedContainer() const
{
    return m_container != nullptr;
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2Container& AVConfig2::getContainer()
{
    AVASSERT(m_container);
    return *m_container;
}

///////////////////////////////////////////////////////////////////////////////

const AVConfig2Container& AVConfig2::getContainer() const
{
    AVASSERT(m_container);
    return *m_container;
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2::translateAddParameter(void *parameter, QString value)
{
    const AVConfig2Container::RegisteredParameterBase *cur_param = getParameter(parameter);
    if (cur_param == 0) return false;

    const AVConfig2Metadata& metadata = cur_param->getMetadata();

    AVConfig2StorageLocation location(AVConfig2StorageLocation::PS_FILE_ORDINARY);

    if (value.isNull()) value = metadata.m_default_value;

    if (!value.isNull())
    {
        m_container->addParameter(value, cur_param->getMetadata(), location);
        AVLogInfo << "AVConfig2::translateAddParameter: added param "
                  << cur_param->getMetadata().m_name;
    } else
    {
        m_container->addReference(AVConfig2Saver::NO_SUGGESTION_REF_VALUE, cur_param->getMetadata(), location);
        AVLogInfo << "AVConfig2::translateAddParameter: added ref "
                  << cur_param->getMetadata().m_name;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2::translateRenameParameter(void *parameter, QString previous_name)
{
    const AVConfig2Container::RegisteredParameterBase *cur_param = getParameter(parameter);
    if (cur_param == 0) return false;

    QString new_name = cur_param->getMetadata().m_name;

    // Handle both references and loaded params - if successfully dereferenced, they are
    // added to the loaded params in AVConfig2Container::resolveReferences().
    // Handle loaded first so we can bail out if the old param is missing

    // We do not fix up parameters referring to an incorrect value here.
    // TODO CM do this if needed

    AVConfig2ImporterClient::LoadedParameter * loaded_param =
        m_container->m_loaded_data.getElementByName<AVConfig2ImporterClient::LoadedParameter>(previous_name);
    if (loaded_param == nullptr)
    {
        return false;
    }

    loaded_param->setName(new_name);
    m_container->m_loaded_data.addElement(*loaded_param);
    m_container->m_loaded_data.removeElementByName<AVConfig2ImporterClient::LoadedParameter>(previous_name);

    AVConfig2ImporterClient::LoadedReference * loaded_ref =
            m_container->m_loaded_data.getElementByName<AVConfig2ImporterClient::LoadedReference>(previous_name);
    if (loaded_ref != nullptr)
    {
        loaded_ref->m_metadata.m_name = new_name;
        m_container->m_loaded_data.addElement(*loaded_ref);
        m_container->m_loaded_data.removeElementByName<AVConfig2ImporterClient::LoadedReference>(previous_name);
    }

    AVLogInfo << "AVConfig2::translateRenameParameter: " << previous_name
              << " was renamed to " << new_name;

    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2::translateChangeMetadata(void *parameter)
{
    const AVConfig2Container::RegisteredParameterBase *cur_param = getParameter(parameter);
    if (cur_param == 0) return false;

    QString name = cur_param->getMetadata().m_name;

    // Handle both references and loaded params - if successfully dereferenced, they are
    // added to the loaded params in AVConfig2Container::resolveReferences().
    // Handle loaded first so we can bail out if the old param is missing

    AVConfig2ImporterClient::LoadedParameter * loaded_param =
        m_container->m_loaded_data.getElementByName<AVConfig2ImporterClient::LoadedParameter>(name);
    if (loaded_param == nullptr) return false;
    loaded_param->updateMetadata(cur_param->getMetadata());

    AVConfig2Container::LoadedReference * loaded_ref =
            m_container->m_loaded_data.getElementByName<AVConfig2ImporterClient::LoadedReference>(name);
    if (loaded_ref != nullptr)
    {
        loaded_ref->m_metadata = cur_param->getMetadata();
    }

    AVLogInfo << "AVConfig2::translateChangeMetadata: " << name << "'s metadata was updated.";

    return true;
}

///////////////////////////////////////////////////////////////////////////////

const AVConfig2Container::RegisteredParameterBase *AVConfig2::getParameter(void *parameter) const
{
    const AVConfig2Container::RegisteredParameterList& registered_list =
            m_container->m_registered_param.getParameterList(this);

    for (AVConfig2Container::RegisteredParameterList::const_iterator it = registered_list.begin();
            it != registered_list.end(); ++it)
    {
        const AVConfig2Container::RegisteredParameterBase& cur_param = **it;
        if (cur_param.pointsToVariable(parameter)) return &cur_param;
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Container::RegisteredParameterContainer::addConfig(const AVConfig2 *config)
{
    for(const ConfigWithRegisteredParameters& pair : qAsConst(*this))
    {
        AVASSERT(pair.first != config);
    }
    push_back(qMakePair(config, RegisteredParameterList()));
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2Container::RegisteredParameterList &AVConfig2Container::RegisteredParameterContainer::getParameterList(
        const AVConfig2 *config)
{
    for(ConfigWithRegisteredParameters& pair : *this)
    {
        if(pair.first == config)
        {
            return pair.second;
        }
    }
    AVASSERT(false);
    static AVConfig2Container::RegisteredParameterList dummy; // keep compiler happy
    return dummy;
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2Container::RegisteredParameterContainer::containsConfig(const AVConfig2 *config) const
{
    for(const ConfigWithRegisteredParameters& pair : *this)
    {
        if(pair.first == config)
            return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Container::RegisteredParameterContainer::removeConfig(const AVConfig2 *config)
{
    auto iter = begin();

    while(iter != end())
    {
        const ConfigWithRegisteredParameters& pair = *iter;
        if(pair.first == config)
        {
            erase(iter);
            break; // It is guaranteed, that there are no duplicates.
        }
        ++iter;
    }
}

// End of file
