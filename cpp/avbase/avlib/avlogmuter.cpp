//-----------------------------------------------------------------------------
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2018
//
// Module:    AVLIB - AviBit Library
//
//-----------------------------------------------------------------------------

/*! \file
    \brief   AVLogMuter implementation
 */

// local includes
#include "avlogmuter.h"

//-----------------------------------------------------------------------------

AVLogMuter::AVLogMuter(int log_level, AVLog* log) :
    m_old_log_level(log->minLevel()),
    m_log(log)
{
    m_log->setMinLevel(log_level);
}

//-----------------------------------------------------------------------------

AVLogMuter::~AVLogMuter()
{
    m_log->setMinLevel(m_old_log_level);
}

// End of file
