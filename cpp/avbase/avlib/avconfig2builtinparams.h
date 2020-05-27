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

#ifndef AVCONFIG2BUILTINPARAMS_INCLUDED
#define AVCONFIG2BUILTINPARAMS_INCLUDED

#include "avlib_export.h"
#include "avconfig2.h"
#include "avconfig2saver.h"

///////////////////////////////////////////////////////////////////////////////

class AVLIB_EXPORT AVConfig2BuiltinParams : public AVConfig2
{
public:
    AVConfig2BuiltinParams(
            AVConfig2Container& config, const
            QString& default_app_name, const QString& default_process_name);
    ~AVConfig2BuiltinParams() override;

    //! Returns CF_LAST to save to the same format the param was loaded from.
    AVConfig2Format getSaveFormat() const;
    QString getUpdateUnusedParamsFile() const;
    QString getPurgeUnusedParamsFile() const;
    AVConfig2SimpleSaver::SaveOrder getSaveOrder() const;

    bool isSaveRequested() const;
    bool getForceSaveOptionals() const;

    QString postRefresh() override;

    //! Either "-config" command line switch or AVCONFIG2_INITIAL_CONFIG environment variable. QString::null if neither
    //! is given.
    QString getInitialConfig() const;

    QString       m_app_name;
    QString       m_cfg_dir;
    QString       m_save_dir;
    //! "-cfg"
    QString       m_main_config_override;
    QString       m_help;
    uint          m_debug;
    CmdlineSwitch m_dump_config;
    CmdlineSwitch m_version;
    QString       m_save_format;
    QString       m_save_order;
    //! See https://confluence.avibit.com:2233/x/ZYDa / AC-554.
    CmdlineSwitch m_suppress_transient_problems;

    //! The facts to be added based on cmd line params
    AVConfig2StorageLocation::FactContainer m_add_facts;

    //! See https://jira.avibit.com:2233/browse/SWE-5487 and https://confluence.avibit.com:2233/display/RDPCI/2019-03-20+Coldstart
    bool          m_coldstart;

private:

    QString       m_save;

    QString       m_initial_config;

    //! If this is not QString::null, update the unused param file and exit the program.
    //! TODO CM really exit, or allow params loaded at runtime?
    QString       m_update_unused_params_file;
    //! If this is not QString::null, use the unused param file to purge the unused params
    //! from the config files.
    QString       m_purge_unused_params_file;

    static const QString FORCE_OPTIONALS_SAVE_OPTION;
    static const QString UNCHANGED_FORMAT_STRING;
    static const QString DEFAULT_UNUSED_PARAMS_FILE;
    static const QString SAVE_ORDER_LOADED;
    static const QString SAVE_ORDER_REGISTERED;
};

///////////////////////////////////////////////////////////////////////////////
/**
 *  This class is used to avoid complaints from the config implementation about "unknown command
 *  line parameters" for parameters known to the QApplication.
 *  Note that the "style=xxx" syntax is not supported except for the qmljsdebugger parameter.
 */
class AVLIB_EXPORT AVConfig2QtBuiltinParams : public AVConfig2
{
public:
    explicit AVConfig2QtBuiltinParams(AVConfig2Container& config);
    QList<QString> m_dummy;
};

#endif // AVCONFIG2BUILTINPARAMS_INCLUDED

// End of file
