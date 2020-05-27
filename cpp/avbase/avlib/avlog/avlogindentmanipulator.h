///////////////////////////////////////////////////////////////////////////////
//
// Package:   ADMAX - Avibit AMAN/DMAN Libraries
// QT-Version: QT4
// Copyright: AviBit data processing GmbH, 2001-2018
//
// Module:    AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \brief   Helper class to enable AVLOG macros the usage of concatenated method invocation
    \see AVLOG_INDENT
*/

#if !defined AVLOG_INDENT_MANIPULATOR_H_INCLUDED
#define AVLOG_INDENT_MANIPULATOR_H_INCLUDED

// QT includes
#include <QString>

// avlib includes
#include "avlib_export.h"

// foward declarations
class AVLogIndent;

class AVLIB_EXPORT AVLogIndentManipulator
{
public:
    explicit AVLogIndentManipulator(AVLogIndent& logIndent);

    ~AVLogIndentManipulator();

    AVLogIndentManipulator& setSectionName(const QString& fun);
    AVLogIndentManipulator& setLogLevel(int logLevel);

private:
    AVLogIndent& m_logIndent;
};

#endif
