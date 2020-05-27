///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2013
//
// Module:    TESTS - AVCOMMON Unit Tests
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Wolfgang Aigner, w.aigner@avibit.com
    \brief   Unit tests for avconsole command base class
 */

#include "avconsoleunittestconnection.h"

//-----------------------------------------------------------------------------

AVConsoleUnitTestConnection::AVConsoleUnitTestConnection()
{
}

//-----------------------------------------------------------------------------

AVConsoleUnitTestConnection::~AVConsoleUnitTestConnection()
{
}

//-----------------------------------------------------------------------------

void AVConsoleUnitTestConnection::sendInput(const QString &str)
{
    processInput(str + "\n");
}

//-----------------------------------------------------------------------------

void AVConsoleUnitTestConnection::printInternal(const QByteArray &str)
{
    QString temp_string(str);
    m_received_strings.append(temp_string.remove('\n'));

    AVLogDebug1 << "AVConsoleUnitTestConnection::printInternal: line processed in printInternal: " << temp_string;
}

// End of file
