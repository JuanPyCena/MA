///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2013
//
// Module:    AVUNITTESTHELPERLIB - AVCOMMON Unit Test helper library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Christian Muschick, c.muschick@avibit.com
    \brief   Defines for QTestlib unittestcases
 */

#include "avunittestmain.h"

///////////////////////////////////////////////////////////////////////////////

void setModuleTestConfigDirectory(const QString &package_name, const QString &dir)
{
    QString env_name = package_name.toUpper() + "_DIR";
    QString env_string = AVEnvironment::getEnv(env_name, true, AVEnvironment::NoPrefix);
    if (env_string.isNull())
    {
        AVLogFatal << AV_FUNC_PREAMBLE
                   << env_name << " not set";
    }
    QDir config_dir(env_string);
    if (!config_dir.isReadable())
    {
        AVLogFatal << AV_FUNC_PREAMBLE << "config dir "
                   << config_dir.absolutePath() << " is not a readable directory";
    }

    if (!config_dir.cd(dir))
    {
        AVLogFatal << AV_FUNC_PREAMBLE << "config dir "
                   << dir << " is not a readable directory in resource path "
                   << config_dir.absolutePath();
    }

    AVLogInfo << AV_FUNC_PREAMBLE << " setting config directory to " << config_dir.absolutePath();
    AVEnvironment::setApplicationConfig(config_dir.absolutePath());
}


// End of file
