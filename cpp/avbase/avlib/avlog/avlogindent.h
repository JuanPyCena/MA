///////////////////////////////////////////////////////////////////////////////
//
// Package:   ADMAX - Avibit AMAN/DMAN Libraries
// QT-Version: QT4
// Copyright: AviBit data processing GmbH, 2001-2011
//
// Module:    AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

// QT includes
#include <QString>

#include "avlogindentmanipulator.h"

#include "avlib_export.h"

#if !defined AVLOG_INDENT_H_INCLUDED
#define AVLOG_INDENT_H_INCLUDED

//helper macros to create unique variable names for AVLogIndent object
#define PRIVATE_AVLOG_CONCAT(x, y) x ## y
#define PRIVATE_AVLOG_UNIQUE(x, y) PRIVATE_AVLOG_CONCAT(x, y)

//! Prints section entry and exit statements, and causes log entries to be printed with indentation.
//! Macro invocation can be concatenated with methods from AVLogIndentManipulator
//! example usage: AVLOG_INDENT.setSectionName("testFunction").setLogLevel(AVLog::LOG__ERROR);
//! \sa AVLogIndent
//! \sa AVLogIndentManipulator
#define AVLOG_INDENT AVLogIndent PRIVATE_AVLOG_UNIQUE(logIndent, __LINE__)(Q_FUNC_INFO, __LINE__, __FILE__); \
    AVLogIndentManipulator(PRIVATE_AVLOG_UNIQUE(logIndent, __LINE__)).setLogLevel(AVLog::LOG__INFO)

/////////////////////////////////////////////////////////////////////////////
/**
 * @brief The AVLogIndent class is a helper class to implement scope-based log indentation.
 *
 * See AVLOG_INDENT macro
 *
 * Example:
 *
 * 16:37:38 INFO:    -> Fdp2Interface::handleNewConnection
 * 16:37:38 INFO:       incoming connection to MINIFPLS on port 9626
 * 16:37:38 INFO:       Fdp2Interface::handleNewConnection: waiting for DAQ to report connection to the external system.
 * 16:37:38 INFO:    <- Fdp2Interface::handleNewConnection
 *
 */
class AVLIB_EXPORT AVLogIndent
{
public:
    //! DEPRECATED, don't use in new code. Rather implement manipulator "suppressEmpty" and "disableIf" functionality.
    //! Constructor for manual usage, no manipulator. Calls initialize immediately.
    explicit AVLogIndent(const QString& section_name);
    //! Constructor for macro usage with manipulator. initialize() is called by manipulator.
    explicit AVLogIndent(const QString& section_name, int lineNumber, const QString& fileName);
    ~AVLogIndent();

    //! Called by the AVLogIndentManipulator destructor when all properties have been set.
    void initialize();
    //! Called when the AVLogIndent object is destroyed.
    void finalize();

    void setSectionName(const QString& section);
    void setLogLevel(int logLevel);

private:
    //! The enclosing function name by default, but can be overridden via setSectionName
    QString m_section_name;
    int m_log_level;
    //! Used for warning/error messages only
    int m_line_number;
    //! Used for warning/error messages only
    QString m_file_name;

    bool m_initialized;
};

// --- legacy macros below ---

//! Prints method entry and exit statements, and causes log entries to be printed
//! with indentation inside the method. Method name will be automatically substituted.
#define AVLOG_ENTER_METHOD() AVLogIndent logIndent(Q_FUNC_INFO, __LINE__, __FILE__); AVLogIndentManipulator(logIndent).setLogLevel(AVLog::LOG__INFO)

//! Prints method entry and exit statements, and causes log entries to be printed
//! with indentation inside the method. Method name will be automatically substituted.
#define AVLOG_ENTER_METHOD_EX(logLevel) AVLogIndent logIndent(Q_FUNC_INFO, __LINE__, __FILE__); AVLogIndentManipulator(logIndent).setLogLevel(logLevel)

//! Prints entry and exit statements for arbitrary code blocks, and causes log entries to be printed
//! with indentation inside the block.
#define AVLOG_ENTER(m) AVLogIndent logIndent(#m, __LINE__, __FILE__); AVLogIndentManipulator(logIndent).setLogLevel(AVLog::LOG__INFO)

//! Extension of macro AVLOG_ENTER that allows to pass in the required log level
#define AVLOG_ENTER_EX(m, logLevel) \
        AVLogIndent logIndent(#m, __LINE__, __FILE__); AVLogIndentManipulator(logIndent).setLogLevel(logLevel)

#endif
