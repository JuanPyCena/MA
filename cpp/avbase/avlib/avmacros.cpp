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
    \author    Stefan Kunz, s.kunz@avibit.com
    \author    QT4-PORT: Christian Muschick, c.muschick@avibit.com
    \brief     Definition of miscellaneous macros.
*/

// local includes
#include "avlog.h"
#include "avmacros.h"

///////////////////////////////////////////////////////////////////////////////

void helper_avassert(int line, const char *file, const char *message)
{
    if (AVLogger)
        AVLogger->Write(line, file, AVLog::LOG__FATAL, QString(message));
    else
        qFatal("ASSERTION FAILED: \"%s\" in file %s, line %d", message, file, line);
}

// End of file
