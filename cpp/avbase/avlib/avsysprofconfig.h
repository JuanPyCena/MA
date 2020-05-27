///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
// QT-Version: QT4
//
// Module:    AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Wolfgang Eder, w.eder@avibit.com
    \author  QT4-PORT: Wolfgang Eder, w.eder@avibit.com
    \brief

    Provides system profiling facilities.
*/

#ifndef AVSYSPROFCONFIG_H
#define AVSYSPROFCONFIG_H

// AVLib includes
#include "avlib_export.h"
#include "avconfig2classic.h"

///////////////////////////////////////////////////////////////////////////////

//! This is an optional config file that allows to enable system profiling.

class AVLIB_EXPORT AVSysProfConfig : public AVConfig2HybridConfig
{
public:
    AVSysProfConfig() :
        AVConfig2HybridConfig("System Profiling", "processconfig.sysprof")
    {
        if (isNewConfigUsed())
        {
            m_show_help_per_default = false;

            registerParameter("enable_system_profiling", &m_enable_system_profiling,
                    "Enable system profiling")
                    .setOptional(false)
                    .setCmdlineSwitch("sysprof");

            registerParameter("exception_process_names", &m_exception_process_names,
                    "Invert the setting of enable_system_profiling for these processes")
                    .setOptional(EmptyQStringList);

            refreshParams();
        }
        else
        {
            ADDCEP("enable_system_profiling", &m_enable_system_profiling, false,
                   "Enable system profiling", "sysprof");
            ADDCE("exception_process_names",   &m_exception_process_names,  EmptyQStringList,
                  "Invert the setting of enable_system_profiling for these processes");

            standardInit(false);
        }
    }
public:
    bool        m_enable_system_profiling;
    QStringList m_exception_process_names;

    AVDISABLECOPY(AVSysProfConfig);
};

#endif // AVSYSPROFCONFIG_H

// End of file
