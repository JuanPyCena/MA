//-----------------------------------------------------------------------------
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2018
//
// Module:    AVLIB - AviBit Library
//
//-----------------------------------------------------------------------------

/*! \file
    \brief   AVLogMuter header
 */

#pragma once

// Qt includes
#include <QtGlobal>

// avlib includes
#include "avlog.h"

//-----------------------------------------------------------------------------
//! Increases the minimum log level of AVLog for the lifetime of AVLogMuter object
/*! This class can be used to mute the warnings and errors while loading huge config files.
 *! This makes the unittest log output more readable.
 *! The destructor will restore the old log level again.
*/
class AVLIB_EXPORT AVLogMuter
{
    //! friend declaration for function level test case
    //friend class TstAVLogMuter;

public:
    explicit AVLogMuter(int log_level = AVLog::LOG__FATAL, AVLog* log = AVLog::getRootLogger());
    virtual ~AVLogMuter();

private:
    int m_old_log_level;
    AVLog* m_log;

    Q_DISABLE_COPY(AVLogMuter)
};

// End of file
