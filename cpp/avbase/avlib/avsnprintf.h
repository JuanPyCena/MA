///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2011
//
// Module:     AVLIB - AviBit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author    Dr. Thomas Leitner, t.leitner@avibit.com
    \author    QT4-PORT: Christian Muschick, c.muschick@avibit.com
    \brief     Defines vsnprintf and snprintf for systems lacking these
               functions.
*/

#if !defined(AVSNPRINTF_H_INCLUDED)
#define AVSNPRINTF_H_INCLUDED

#include <qglobal.h>

#if defined(Q_OS_OSF)
extern "C" int vsnprintf(char *str, size_t n, char const *fmt, va_list ap);
extern "C" int snprintf(char *str, size_t n, char const *fmt, ...);
#endif

#if defined(Q_OS_WIN32)
#include <cstdio>
#define vsnprintf _vsnprintf
#define snprintf _snprintf
#endif

#endif

// End of file
