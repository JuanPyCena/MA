///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2013
//
// Module:    TESTS - AVCOMMON Unit Tests
//
///////////////////////////////////////////////////////////////////////////////
#pragma once 

/*! \file
    \author  Wolfgang Aigner, w.aigner@avibit.com
    \brief   Connection for avconsole unit tests
 */

#include "avconsole.h"
#include "avlibunittesthelperlib_export.h"

//-----------------------------------------------------------------------------

class AVLIBUNITTESTHELPERLIB_EXPORT AVConsoleUnitTestConnection : public AVConsoleConnection
{
    Q_OBJECT

public:
    AVConsoleUnitTestConnection();

    ~AVConsoleUnitTestConnection() override;

    //! Automatically appends a trailing newline.
    void sendInput(const QString& str);

protected:
    void printInternal(const QByteArray& str) override;

public:
    QStringList     m_received_strings;
};

// End of file
