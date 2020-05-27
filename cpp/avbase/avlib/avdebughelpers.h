///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT5
// Copyright:  AviBit data processing GmbH, 2001-2011
//
// Module:     AVLIB - AviBit Library
//
/////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Markus Dauschan, m.dauschan@avibit.com
    \brief   Several helper functions for debugging.
*/


#if !defined(AVDEBUGHELPERS_H_INCLUDED)
#define AVDEBUGHELPERS_H_INCLUDED

#include "avlib_export.h"

//! Prints a stack trace by forking and running gdb
//! Currently only works under Linux. Using gdb was prefered because other methods won't load separated debug files.
AVLIB_EXPORT void AVPrintStackTraceUsingGdb();

#if defined Q_OS_WIN32  
//! Write a minidump <process>.<pid>.dmp to the current directory.
//! First writes the file with suffix ".tmp", then moves it to the final location.
//! This is to avoid housekeeping to work with incomplete dump files.
AVLIB_EXPORT void AVWriteMiniDump(_EXCEPTION_POINTERS *exception_pointers);
#endif

#endif // AVDEBUGHELPERS_H_INCLUDED

// End of file
