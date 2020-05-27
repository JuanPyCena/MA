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


// AviBit common includes
#include <avlog.h>
#include <avthread.h>

#include "avconsole_command.h"

///////////////////////////////////////////////////////////////////////////////

AVConsoleCommand::AVConsoleCommand(const QString& name,
                                   const QString& help_text) :
    m_connection(0),
    m_print_default_result_message(true),
    m_command_name(name),
    m_command_help(help_text),
    m_arguments_usage(""),
    m_minimal_arguments(0),
    m_maximal_arguments(0),
    m_check_maximal_arguments(false)
{
}

///////////////////////////////////////////////////////////////////////////////

AVConsoleCommand::~AVConsoleCommand()
{
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleCommand::registerInConsole()
{
    AVConsole& console = AVConsole::singleton();

    console.registerSlot(m_command_name,
                         this,
                         SLOT(slotConsoleExecuteCommand(AVConsoleConnection&,const QStringList&)),
                         m_command_help,
                         SLOT(slotCompleteCommand(QStringList&,const QStringList&)));
}

///////////////////////////////////////////////////////////////////////////////

const QString& AVConsoleCommand::getCommandName()
{
    return m_command_name;
}

///////////////////////////////////////////////////////////////////////////////

const QString& AVConsoleCommand::getCommandHelp()
{
    return m_command_help;
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleCommand::slotConsoleExecuteCommand(AVConsoleConnection& connection,
                                                 const QStringList& args)
{
    AVASSERT(AVThread::isMainThread());

    reset();
    m_connection = &connection;

    if ((args.size() < static_cast<int>(m_minimal_arguments)) ||
        (m_check_maximal_arguments && args.size() > static_cast<int>(m_maximal_arguments)))
    {
        // connection.printError("Illegal number of arguments, usage: " + m_usage_description);
        error("Illegal number of arguments, usage: " + m_command_name + " " + m_arguments_usage);
        return;
    }

    bool result = doExecute(args);

    if (m_print_default_result_message)
    {
        if (result)
            success("Command executed successfully.");
        else
            error("Command failed.");
    }

    m_connection = 0;
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleCommand::slotCompleteCommand(QStringList& completions,
                                           const QStringList& args)
{
    doComplete(completions, args);
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleCommand::setArgumentsUsage(const QString& usage)
{
    m_arguments_usage = usage;
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleCommand::setMinimalNumberOfArguments(uint number)
{
    m_minimal_arguments = number;
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleCommand::setMaximalNumberOfArguments(uint number)
{
    m_maximal_arguments       = number;
    m_check_maximal_arguments = true;
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleCommand::error(const QString& msg)
{
    AVASSERT(m_connection);
    m_connection->printError(msg);
    m_print_default_result_message = false;
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleCommand::success(const QString& msg)
{
    AVASSERT(m_connection);
    m_connection->printSuccess(msg);
    m_print_default_result_message = false;
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleCommand::print(const QString& text)
{
    AVASSERT(m_connection);
    m_connection->print(text);
}

///////////////////////////////////////////////////////////////////////////////

AVConsoleConnection& AVConsoleCommand::connection() const
{
    AVASSERT(m_connection);
    return *m_connection;
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleCommand::setPrintDefaultResultMessage(bool print)
{
    // Only makes sense during command execution.
    AVASSERT(m_connection != 0);
    m_print_default_result_message = print;
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleCommand::reset()
{
    m_print_default_result_message = true;
}

// End of file
