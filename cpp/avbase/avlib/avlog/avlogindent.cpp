///////////////////////////////////////////////////////////////////////////////
//
// Package:   ADMAX - Avibit AMAN/DMAN Libraries
// QT-Version: QT4
// Copyright: AviBit data processing GmbH, 2001-2011
//
// Module:    AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

#include "avlogindent.h"

#include "../avlog.h"

///////////////////////////////////////////////////////////////////////////////

AVLogIndent::AVLogIndent(const QString& section_name) :
        m_section_name(section_name),
        m_log_level(AVLog::LOG__INFO),
        m_line_number(0),
        m_initialized(false)
{
    initialize();
}

///////////////////////////////////////////////////////////////////////////////

AVLogIndent::AVLogIndent(const QString& section_name, int lineNumber, const QString& fileName) :
        m_section_name(section_name),
        m_log_level(AVLog::LOG__INFO),
        m_line_number(lineNumber),
        m_file_name(fileName),
        m_initialized(false)
{
}

///////////////////////////////////////////////////////////////////////////////

AVLogIndent::~AVLogIndent()
{
    finalize();
}

///////////////////////////////////////////////////////////////////////////////

void AVLogIndent::initialize()
{
    AVASSERT(!m_initialized);
    m_initialized = true;
    if (AVLogger == 0) return;
    AVLogger->Write(m_line_number, m_file_name, m_log_level, "-> %s", m_section_name.toLatin1().constData());
    AVLogger->incIndent();
}

///////////////////////////////////////////////////////////////////////////////

void AVLogIndent::finalize()
{
    AVASSERT(m_initialized);
    if (AVLogger == 0) return;
    AVLogger->decIndent();
    AVLogger->Write(m_line_number, m_file_name, m_log_level, "<- %s", m_section_name.toLatin1().constData());
}

///////////////////////////////////////////////////////////////////////////////

void AVLogIndent::setSectionName(const QString& section)
{
    m_section_name = section;
}

///////////////////////////////////////////////////////////////////////////////

void AVLogIndent::setLogLevel(int logLevel)
{
    m_log_level = logLevel;
}
