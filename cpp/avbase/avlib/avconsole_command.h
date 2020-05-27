///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT5
// Copyright:  AviBit data processing GmbH, 2001-2014
//
// Module:     AVLIB - AviBit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
 \author  Wolfgang Aigner, w.aigner@avibit.com
 \author  Andreas Niederl, a.niederl@avibit.com
 \brief   Provides a base class for AVConsole commands.
 */

#ifndef AVCONSOLE_COMMAND_H
#define AVCONSOLE_COMMAND_H

// Qt includes
#include <qobject.h>
#include <qmutex.h>

// AviBit common includes
#include "avlib_export.h"
#include "avconsole.h"

///////////////////////////////////////////////////////////////////////////////

/**
 * The class AVConsoleCommand provides a base class for AVConsole commands.
 * The method registerInConsole registers the command (slotConsoleExecuteCommand
 * and slotCompleteCommand) in the AVConsole singleton.
 * These two slots call the virtual methods doExecute and doComplete respectively.
 *
*/

class AVLIB_EXPORT AVConsoleCommand : public QObject
{
    Q_OBJECT
public:

    AVConsoleCommand(const QString& name,
                     const QString& help_text);

    ~AVConsoleCommand() override;

    //! registers the command in AVConsole
    void registerInConsole();

    //! getter for command name as given to constructor
    const QString& getCommandName();

    //! getter for help text as given to constructor
    const QString& getCommandHelp();


public slots:
    //! slot which calls the virtual execute method of the actual command object
    //! if neither success() nor error() are used within doExecute() a generic error/success
    //! message will be printed to the console connection depending on the return value of
    //! doExecute().
    void slotConsoleExecuteCommand(AVConsoleConnection& connection,
                                   const QStringList& args);

    //! slot which calls the virtual complete method of the actual command object
    void slotCompleteCommand(QStringList& completions,
                             const QStringList& args);

protected:
    //! Set an arguments usage text which can be displayed on invalid argument errors,
    //! e.g. "[opt_arg] <arg1>" will result in the help text "command [opt_arg] <arg1>" on
    //! invalid arguments.
    void setArgumentsUsage(const QString& usage);

    //! Specify a minimal number of arguments which have to be provided for this command.
    //! Calling the command with less arguments will result in an invalid argument error.
    //!
    //! Minimal argument default is 0.
    void setMinimalNumberOfArguments(uint number);

    //! Specify a maximal number of arguments which can be provided for this command.
    //! Calling the command with more arguments will result in an invalid argument error.
    //! The maximal number of arguments will only be checked if this method has been called
    //! at least once.
    //!
    //! Default is to not check the maximum number of arguments.
    void setMaximalNumberOfArguments(uint number);

protected:
    //! Print an error message on the console connection.
    //! \sa setPrintDefaultResultMessage
    void error(const QString& msg);

    //! Print a success message on the console connection.
    //! \sa setPrintDefaultResultMessage
    void success(const QString& msg);

    //! Print text on the console connection.
    void print(const QString& text);

    //! Access the current console connection which is used to execute the command.
    AVConsoleConnection& connection() const;

    //! Specify if a success or error message should be printed after the command has finished
    //! depending on the return value of doExecute.
    //!
    //! Per default, a result message will automatically be printed if neither error() nor success() is called during
    //! command execution. This behaviour can be disabled for the currently executed command by calling this method.
    void setPrintDefaultResultMessage(bool print);

private:
    //! Reset the member variables of the command to their initial values.
    virtual void reset();

    //! pure virtual method for implementing a command; can use methods for printing or use m_connection directly
    virtual bool doExecute(QStringList args) = 0;

    //! virtual method which provides console completion for the command
    virtual void doComplete(QStringList& completions,
                            const QStringList& args) = 0;

private:
    //! Stores the console connection for the current command execution. 0 when no command is currently executed.
    AVConsoleConnection*     m_connection;

    //! \see setPrintDefaultResultMessage
    bool                     m_print_default_result_message;

private:
    //! name of the command
    QString     m_command_name;

    //! help text for the command
    QString     m_command_help;

    //! textual description of argument list for usage help
    QString     m_arguments_usage;

    //! minimal number of arguments
    uint        m_minimal_arguments;

    //! maximal number of arguments
    uint        m_maximal_arguments;

    //! determines if the maximum number of arguments is checked
    bool        m_check_maximal_arguments;

    AVDISABLECOPY(AVConsoleCommand);
};

#endif // AVCONSOLE_COMMAND_H

// End of file
