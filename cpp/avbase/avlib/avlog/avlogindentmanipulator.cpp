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

// local includes
#include "avlog.h"
#include "avlog/avlogindentmanipulator.h"


///////////////////////////////////////////////////////////////////////////////

AVLogIndentManipulator::AVLogIndentManipulator(AVLogIndent& logIndent) : m_logIndent(logIndent)
{
}

///////////////////////////////////////////////////////////////////////////////

AVLogIndentManipulator::~AVLogIndentManipulator()
{
    m_logIndent.initialize();
}

///////////////////////////////////////////////////////////////////////////////

AVLogIndentManipulator& AVLogIndentManipulator::setSectionName(const QString& fun)
{
    m_logIndent.setSectionName(fun);
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

AVLogIndentManipulator& AVLogIndentManipulator::setLogLevel(int logLevel)
{
    m_logIndent.setLogLevel(logLevel);
    return *this;
}
