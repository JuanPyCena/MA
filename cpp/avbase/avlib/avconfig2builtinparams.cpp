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

#include "avconfig2builtinparams.h"

#include "avenvironment.h"

///////////////////////////////////////////////////////////////////////////////

const QString AVConfig2BuiltinParams::FORCE_OPTIONALS_SAVE_OPTION = "force_optionals";
const QString AVConfig2BuiltinParams::UNCHANGED_FORMAT_STRING     = "unchanged";
const QString AVConfig2BuiltinParams::DEFAULT_UNUSED_PARAMS_FILE  = "unused_params.txt";
const QString AVConfig2BuiltinParams::SAVE_ORDER_LOADED           = "loaded";
const QString AVConfig2BuiltinParams::SAVE_ORDER_REGISTERED       = "registered";

///////////////////////////////////////////////////////////////////////////////

AVConfig2BuiltinParams::AVConfig2BuiltinParams(
        AVConfig2Container& config, const QString& default_app_name,
        const QString& default_process_name) :
        AVConfig2("", config)
{
    setHelpGroup("AVConfig2");
    Q_UNUSED(default_process_name); // Formerly used for default <missing> location

    m_show_help_per_default = false;

    registerParameter("an", &m_app_name, "Application name (has influence on logging dir, "
                      "shared memory, config directory etc)").
            setPureCmdlineOption(default_app_name);
    registerParameter("cfgdir", &m_cfg_dir, "The application config directory.")
            .setPureCmdlineOption(QString::null);
    registerParameter("savedir", &m_save_dir, "The directory to save configs to. "
            "Leave at QString::null to use the directory the config was loaded from.")
            .setPureCmdlineOption(QString::null);
    registerParameter("config", &m_initial_config, "The initial config file to load (without "
            "file extension), if any. Note that directories called " +
            AVConfig2Global::DEV_CONFIG_SUBDIR + " below $PROJECTS_HOME will be added to the "
                    "config search path in this special case.").
            setPureCmdlineOption(QString::null);
    registerParameter("cfg", &m_main_config_override, "If this process has a main config file, "
            "load this file instead of the default.").
            setPureCmdlineOption(QString::null);
    registerParameter("help", &m_help, "Display this help text and terminate the program. "
                      "Suppresses all other output.").
            setValidValues(QStringList() << QString::null << "" << "all").
            setPureCmdlineOption(QString::null);
    registerParameter("debug", &m_debug, "Set the application debug level.").
            setPureCmdlineOption(0).
            setMinMax(0,3);
    registerParameter("dump_config", &m_dump_config, "Dump initially loaded configuration and "
                      "terminate program.").
            setPureCmdlineOption(0);
    registerParameter("version", &m_version, "Print the build version and terminate the program.").
            setPureCmdlineOption(0);
    registerParameter("coldstart", &m_coldstart, "Disregard existing process persistence files.").
            setPureCmdlineOption(false);
    registerParameter("save", &m_save, "Save the current configuration and terminate the program. "
                      "Can be used to fix "
                      "inconsistent metadata or create a template for missing parameters. "
                      "Previously loaded parameters are stored in the format in which they were "
                      "read. Use -save_format to override this. Use \"-save " + FORCE_OPTIONALS_SAVE_OPTION +
                      "\" to save all optional params.").
            setPureCmdlineOption(0);
    registerParameter("update_unused", &m_update_unused_params_file, "[unused_params_file]. Update "
            "or create the specified file containing unused parameters.").
            setPureCmdlineOption(QString::null);
    registerParameter("purge_unused", &m_purge_unused_params_file, "[unused_params_file]. Purge "
            "all parameters listed in the specified file.").
            setPureCmdlineOption(QString::null);
    QStringList possible_formats;
    for (uint i=0; i<CF_LAST; ++i)
    {
        possible_formats << AVConfig2StorageLocation::configFormatToString(static_cast<AVConfig2Format>(i));
    }
    possible_formats << UNCHANGED_FORMAT_STRING;
    registerParameter("save_format", &m_save_format, "The format to use when -save is specified.").
            setPureCmdlineOption(UNCHANGED_FORMAT_STRING).
            setValidValues(possible_formats);

    registerParameter("save_order", &m_save_order,
                      "When saving, use the order from the loaded config file, "
                      "or from program registration? Attention: currently works with cstyle format only!").
            setPureCmdlineOption(SAVE_ORDER_REGISTERED).
            setValidValues(QStringList() << SAVE_ORDER_LOADED << SAVE_ORDER_REGISTERED);

    registerParameter("suppress_transient_problems", &m_suppress_transient_problems,
                      "Start in execution phase \"starting up\" so transient warnings/errors are logged as info?").
            setPureCmdlineOption(false);

    AVConfig2StorageLocation::FactContainer example;
    example["SITE"] = "LOWW";
    example["MODE"] = "ETS";
    registerParameter("add_facts", &m_add_facts, "Maps fact name to value. Facts apply in addition "
            "to already existing facts. Example: " +
                      AVToString(example)).
            setPureCmdlineOption(QMap<QString,QString>());
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2BuiltinParams::~AVConfig2BuiltinParams()
{
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2Format AVConfig2BuiltinParams::getSaveFormat() const
{
    if (m_save_format == UNCHANGED_FORMAT_STRING) return CF_LAST;
    return AVConfig2StorageLocation::getConfigFormatFromString(m_save_format);
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2BuiltinParams::getUpdateUnusedParamsFile() const
{
    if      (m_update_unused_params_file.isNull())  return QString::null;
    else if (m_update_unused_params_file.isEmpty()) return DEFAULT_UNUSED_PARAMS_FILE;
    else return m_update_unused_params_file;
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2BuiltinParams::getPurgeUnusedParamsFile() const
{
    if      (m_purge_unused_params_file.isNull())  return QString::null;
    else if (m_purge_unused_params_file.isEmpty()) return DEFAULT_UNUSED_PARAMS_FILE;
    else return m_purge_unused_params_file;
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2SimpleSaver::SaveOrder AVConfig2BuiltinParams::getSaveOrder() const
{
    return m_save_order == SAVE_ORDER_LOADED ?
            AVConfig2SimpleSaver::SO_LOADED : AVConfig2SimpleSaver::SO_APPLICATION;
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2BuiltinParams::isSaveRequested() const
{
    return !m_save.isNull();
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2BuiltinParams::getForceSaveOptionals() const
{
    return m_save == FORCE_OPTIONALS_SAVE_OPTION;
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2BuiltinParams::postRefresh()
{
    // Avoid trailing '/' on paths because they can make trouble...
    while (m_cfg_dir. endsWith("/") && m_cfg_dir  != "/") m_cfg_dir  = m_cfg_dir. left(m_cfg_dir. length()-1);
    while (m_save_dir.endsWith("/") && m_save_dir != "/") m_save_dir = m_save_dir.left(m_save_dir.length()-1);

    return QString::null;
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2BuiltinParams::getInitialConfig() const
{
    if (!m_initial_config.isNull()) return m_initial_config;
    QString env = AVEnvironment::getEnv(AVConfig2Global::AVCONFIG2_INITIAL_CONFIG, false, AVEnvironment::NoPrefix);
    // This is QString::null if env variable was not found.
    return env;
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2QtBuiltinParams::AVConfig2QtBuiltinParams(AVConfig2Container& config) :
        AVConfig2("", config)
{
    setHelpGroup("Qt5");
    m_show_help_per_default = false;

    for (int i = 0; i < 30; ++i)
    {
        m_dummy.append(QString());
    }

    registerParameter("nograb",       &m_dummy[0], "See Qt docs.").setPureCmdlineOption(QString::null);
    registerParameter("dograb",       &m_dummy[1], "See Qt docs.").setPureCmdlineOption(QString::null);
    registerParameter("sync",         &m_dummy[2], "See Qt docs.").setPureCmdlineOption(QString::null);

    registerParameter("platform",     &m_dummy[3], "See Qt docs.").setPureCmdlineOption(QString::null);
    registerParameter("platformpluginpath",&m_dummy[4], "See Qt docs.").setPureCmdlineOption(QString::null);
    registerParameter("platformtheme",&m_dummy[5], "See Qt docs.").setPureCmdlineOption(QString::null);
    registerParameter("plugin",       &m_dummy[6], "See Qt docs.").setPureCmdlineOption(QString::null);
    registerParameter("style",        &m_dummy[7], "See Qt docs.").setPureCmdlineOption(QString::null);
    registerParameter("stylesheet",   &m_dummy[8], "See Qt docs.").setPureCmdlineOption(QString::null);
    registerParameter("session",      &m_dummy[9], "See Qt docs.").setPureCmdlineOption(QString::null);
    registerParameter("widgetcount",  &m_dummy[10], "See Qt docs.").setPureCmdlineOption(QString::null);
    registerParameter("reverse",      &m_dummy[11], "See Qt docs.").setPureCmdlineOption(QString::null);
    registerParameter("graphicssystem",&m_dummy[12],"See Qt docs.").setPureCmdlineOption(QString::null);
    registerParameter("qmljsdebugger",&m_dummy[13], "See Qt docs. Note that this parameter uses "
            "the '=' syntax.").setPureCmdlineOption(QString::null);

    registerParameter("qwindowgeometry",&m_dummy[14],"See Qt docs.").setPureCmdlineOption(QString::null);
    registerParameter("qwindowicon",    &m_dummy[15],"See Qt docs.").setPureCmdlineOption(QString::null);
    registerParameter("qwindowtitle",   &m_dummy[16],"See Qt docs.").setPureCmdlineOption(QString::null);


    registerParameter("display",      &m_dummy[17], "See Qt docs.").setPureCmdlineOption(QString::null);
    registerParameter("geometry",     &m_dummy[18], "See Qt docs.").setPureCmdlineOption(QString::null);
    registerParameter("font",         &m_dummy[19], "See Qt docs.").setPureCmdlineOption(QString::null);
    registerParameter("background",   &m_dummy[20], "See Qt docs.").setPureCmdlineOption(QString::null);
    registerParameter("foreground",   &m_dummy[21], "See Qt docs.").setPureCmdlineOption(QString::null);
    registerParameter("button",       &m_dummy[22], "See Qt docs.").setPureCmdlineOption(QString::null);
    registerParameter("name",         &m_dummy[23], "See Qt docs.").setPureCmdlineOption(QString::null);
    registerParameter("title",        &m_dummy[24], "See Qt docs.").setPureCmdlineOption(QString::null);
    registerParameter("visual",       &m_dummy[25], "See Qt docs.").setPureCmdlineOption(QString::null);
    registerParameter("ncols",        &m_dummy[26], "See Qt docs.").setPureCmdlineOption(QString::null);
    registerParameter("cmap",         &m_dummy[27], "See Qt docs.").setPureCmdlineOption(QString::null);
    registerParameter("im",           &m_dummy[28], "See Qt docs.").setPureCmdlineOption(QString::null);
    registerParameter("inputstyle",   &m_dummy[29], "See Qt docs.").setPureCmdlineOption(QString::null);
}

// End of file
