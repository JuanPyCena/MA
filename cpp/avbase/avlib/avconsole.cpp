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
 \author  Christian Muschick, c.muschick@avibit.com
 \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
 \brief   Provides the functionality to call slots via external input for testing / debugging
          purposes
 */


#include <iostream>
#include <regex>

// QT includes
#include <QtGlobal>
#include <QTextStream>

// AVCOMMON includes
#include "avdatetime.h"
#include "avdaemon.h"
#include "avlog.h"
#include "avmisc.h"
#include "avexecstandbychecker.h"

#include "avconsole.h"
#include "avconsole_int.h"


const QString AVConsoleConnection::SUCCESS_PREFIX = "SUCCESS: ";
const QString AVConsoleConnection::ERROR_PREFIX = "ERROR: ";

///////////////////////////////////////////////////////////////////////////////

AVConsoleConnection::AVConsoleConnection() :
        m_raw_mode(false),
        m_terminal_buffer(*new (LOG_HERE) AVConsoleTerminalBuffer),
        m_framing_delimiters(QByteArray(), QByteArray("\n", 1)),
        m_multiline(false),
        m_suspended(false)
{
    AVConsole::singleton().addConnection(this);

    AVDIRECTCONNECT(&m_delay_timer, SIGNAL(timeout()), this, SLOT(slotHandleBufferedInput()));
    AVDIRECTCONNECT(&m_framing_delimiters, SIGNAL(signalGotMessage(const QByteArray&)),
              this, SLOT(slotGotLine(const QByteArray&)));

    QString recording_file = AVConsole::singleton().getDefaultRecordingFile();
    if (!recording_file.isEmpty()) recordSession(recording_file);
}

///////////////////////////////////////////////////////////////////////////////

AVConsoleConnection::~AVConsoleConnection()
{
    if (!m_accumulated_multiline_input.isEmpty())
    {
        AVLogWarning << "AVConsoleConnection::~AVConsoleConnection: "
                     << "Terminated in the middle of a multi-line input. Accumulated input so far: \n"
                     << m_accumulated_multiline_input;
    }
    delete &m_terminal_buffer;
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleConnection::disconnect()
{
    printError("AVConsoleConnection::disconnect: not a telnet connection.");
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleConnection::print(const QString& msg, bool add_frame)
{
    QString msg_with_nl = msg;
    if (!msg_with_nl.endsWith("\n")) msg_with_nl += "\n";

    if (add_frame)
    {
        msg_with_nl.
        prepend("----------------------------------------------------------------------\n");

        msg_with_nl.
        append("----------------------------------------------------------------------\n");
    }

    if (m_record_file.isOpen())
    {
        m_record_file.write(msg_with_nl.toUtf8());
    }

    // TODO CM temporarily disabled, see SWE-2981 - infinite recursion with setEchoLogOutput on
    // AVLogInfo << "AVConsoleConnection::print: " << msg;

    if (m_raw_mode)
    {
        printInternal(msg_with_nl.replace("\n", AVConsoleTerminalBuffer::CRLF).toUtf8());
    } else
    {
        printInternal(msg_with_nl.toUtf8());
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleConnection::printSuccess(const QString& msg)
{
    print(SUCCESS_PREFIX + msg);
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleConnection::printError(const QString& msg)
{
    print(ERROR_PREFIX + msg);
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleConnection::continueMultilineInput()
{
    m_multiline = true;
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleConnection::delayInputForMsecs(uint msecs)
{
    AVASSERT(!m_delay_timer.isActive());
    m_delay_timer.setSingleShot(true);
    m_delay_timer.start(msecs);
}

///////////////////////////////////////////////////////////////////////////////

ConsoleConnectionHandle AVConsoleConnection::suspend()
{
    m_suspended = true;
    return AVConsole::singleton().getConnectionHandle(this);
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleConnection::desuspend()
{
    AVASSERT(m_suspended==true);
    m_suspended = false;
    slotHandleBufferedInput();
}

///////////////////////////////////////////////////////////////////////////////

bool AVConsoleConnection::recordSession(const QString& outfile)
{
    // Mustn't use virtual "print" here, as this method can be called from the connection's
    // constructor.
    if (m_record_file.isOpen()) m_record_file.close();
    m_record_file.setFileName(outfile);
    return m_record_file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Unbuffered);
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleConnection::stopRecording()
{
    if (!m_record_file.isOpen()) printError("Recording has not been started.");
    else
    {
        m_record_file.close();
        print(QString("Recording to %1 has stopped").arg(m_record_file.fileName()));
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleConnection::setTimeMark(const QString &name, const QDateTime &date_time)
{
    m_time_mark_map[name] = date_time;
}

///////////////////////////////////////////////////////////////////////////////

QDateTime AVConsoleConnection::getTimeMark(const QString &name) const
{
    QMap<QString, QDateTime>::ConstIterator it;
    it = m_time_mark_map.find(name);
    if (it != m_time_mark_map.end()) return *it;
    else return AVDateTime();
}

///////////////////////////////////////////////////////////////////////////////

QStringList AVConsoleConnection::getTimeMarkNames() const
{
    return m_time_mark_map.keys();
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleConnection::processInput(const QString& input)
{
    if (m_raw_mode)
    {
        m_terminal_buffer.setEnableAutoCompletion(!m_multiline);
        QString line, reply;
        // We use a loop here to handle pasted input as well as input "pasted together" by a
        // continuously pressed key.
        for (int c=0; c<input.length(); ++c)
        {
            // This happens for telnet input, which delimits lines by \r\0.
            if (input.at(c).isNull()) continue;
            reply = m_terminal_buffer.handleInteractiveInput(line, input.at(c));
            if (!reply.isNull()) printInternal(reply.toUtf8());
            if (!line.isNull()) gotLine(line);
        }
    } else
    {
        m_framing_delimiters.slotGotData(input.toUtf8());
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleConnection::slotHandleBufferedInput()
{
    while (!m_buffered_input.isEmpty())
    {
        // bail if delayed again
        if (m_delay_timer.isActive() || m_suspended) return;
        gotLine(m_buffered_input[0]);
        m_buffered_input.pop_front();
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleConnection::slotGotLine(const QByteArray& line)
{
    gotLine(QString::fromUtf8(line));
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleConnection::gotLine(const QString& input)
{
    if (m_delay_timer.isActive() || m_suspended)
    {
        m_buffered_input += input;
        return;
    }

    if (m_record_file.isOpen())
    {
        m_record_file.write("> ", 2);
        m_record_file.write(input.toUtf8());
        m_record_file.write("\n", 1);
    }

    m_multiline = false;

    if (!m_accumulated_multiline_input.isEmpty())
    {
        m_accumulated_multiline_input += "\n" + input;
        AVConsole::singleton().processInput(*this, m_accumulated_multiline_input);
    } else
    {
        AVConsole::singleton().processInput(*this, input);
    }

    if (!m_multiline)
    {
        m_accumulated_multiline_input = "";
    } else if (m_accumulated_multiline_input.isEmpty())
    {
        m_accumulated_multiline_input += input;
    }
}

///////////////////////////////////////////////////////////////////////////////

AVConsole::RegisteredHandler::RegisteredHandler(const QString& name, const QString& help_text, QObject* obj)
    : m_name(name)
    , m_help_text(help_text)
    , m_object(obj)
    , m_always_allow_standby_execution(false)
{
}

///////////////////////////////////////////////////////////////////////////////

AVConsole::RegisteredHandlerManipulator::RegisteredHandlerManipulator(AVConsole::RegisteredHandler &handler) :
    m_handler(handler)
{
}

///////////////////////////////////////////////////////////////////////////////

AVConsole::RegisteredHandlerManipulator& AVConsole::RegisteredHandlerManipulator::setAlwaysAllowStandbyExecution()
{
    m_handler.setAlwaysAllowStandbyExecution();
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

AVConsole::RegisteredSlot::RegisteredSlot(
        const QString& name, QObject* obj, const QString& slot, const QString& help,
        const QString& slot_autocomplete) :
        RegisteredHandler(name, help, obj),
        m_slot(slot), m_slot_autocomplete(slot_autocomplete)
{
}

///////////////////////////////////////////////////////////////////////////////

void AVConsole::RegisteredSlot::handleCommand(AVConsole &console, AVConsoleConnection &connection, const QStringList &args) const
{
    // Temporarily connect our signalCallFunction() to the proper slot and emit it.
    connect(&console, SIGNAL(signalCallFunction(AVConsoleConnection& ,
                                 const QStringList&)),
            object(), qPrintable(m_slot), Qt::DirectConnection);

    // Now call the registered slot
    emit console.signalCallFunction(connection, args);

    disconnect(&console,
               SIGNAL(signalCallFunction(AVConsoleConnection&,
                              const QStringList&)),
               0, 0);
}

///////////////////////////////////////////////////////////////////////////////

void AVConsole::RegisteredSlot::handleCompletion(AVConsole& console, QStringList& completions, const QStringList& args) const
{
    // Temporarily connect our signalAutoComplete() to the proper slot and emit it.
    connect(&console,
            SIGNAL(signalAutoComplete(QStringList&, const QStringList&)),
            object(), qPrintable(m_slot_autocomplete), Qt::DirectConnection);
    // Now call the registered slot
    emit console.signalAutoComplete(completions, args);
    disconnect(&console,
               SIGNAL(signalAutoComplete(QStringList&, const QStringList&)),
               0, 0);
}

///////////////////////////////////////////////////////////////////////////////

AVConsole::RegisteredCallback::RegisteredCallback(const QString& name, QObject* obj, Callable callback, const QString& help,
                                                  CompleteCallable complete_callback)
    : RegisteredHandler(name, help, obj), m_callback(callback), m_complete_callback(complete_callback)
{
}

///////////////////////////////////////////////////////////////////////////////

void AVConsole::RegisteredCallback::handleCommand(AVConsole&, AVConsoleConnection& connection, const QStringList& args) const
{
    m_callback(connection, args);
}

///////////////////////////////////////////////////////////////////////////////

void AVConsole::RegisteredCallback::handleCompletion(AVConsole&, QStringList& completions, const QStringList& args) const
{
    m_complete_callback(completions, args);
}

///////////////////////////////////////////////////////////////////////////////

AVConsole::RegisteredVariable::RegisteredVariable() :
        m_owner(0),
        m_value(0),
        m_settable(0)
{
}

///////////////////////////////////////////////////////////////////////////////

AVConsole::RegisteredVariable::RegisteredVariable(const AVConsole::RegisteredVariable& other) :
    m_name(other.m_name),
    m_owner(other.m_owner),
    m_value(other.m_value->clone()),
    m_settable(other.m_settable),
    m_observer(other.m_observer)
{
}

///////////////////////////////////////////////////////////////////////////////

AVConsole::RegisteredVariable::RegisteredVariable(
        const QString& name, QObject* owner, TextValuePointerBase *value, bool settable) :
        m_name(name),
        m_owner(owner),
        m_value(value),
        m_settable(settable)
{
}

///////////////////////////////////////////////////////////////////////////////

AVConsole::RegisteredVariable::~RegisteredVariable()
{
    delete m_value;
}

///////////////////////////////////////////////////////////////////////////////

AVConsole::RegisteredVariable& AVConsole::RegisteredVariable::operator=(
        const AVConsole::RegisteredVariable& other)
{
    if (this == &other) return *this;

    delete m_value;

    m_name     = other.m_name;
    m_owner    = other.m_owner;
    m_value    = other.m_value->clone();
    m_settable = other.m_settable;
    m_observer = other.m_observer;

    return *this;
}

///////////////////////////////////////////////////////////////////////////////

AVConsole& AVConsole::initializeSingleton()
{
    return setSingleton(new (LOG_HERE) AVConsole());
}

///////////////////////////////////////////////////////////////////////////////

AVConsole::~AVConsole()
{
    delete m_config;
    // Actual removal from container is done in slotConnectionDestroyed
    while (!m_connection.isEmpty()) delete m_connection.begin().key();
}

///////////////////////////////////////////////////////////////////////////////

void AVConsole::setupDefaultInput()
{
    // see AVConsole::slotSetupDefaultInput
    //
    // This also implies that we cannot assert for the main thread here (because this
    // would require qApp to be initialized)
    QTimer::singleShot(0, this, SLOT(slotSetupDefaultInput()));
}

///////////////////////////////////////////////////////////////////////////////

void AVConsole::registerCategory(const QString& category_name, const QString& description)
{
    Q_UNUSED(category_name);
    Q_UNUSED(description);
}

///////////////////////////////////////////////////////////////////////////////

AVConsole::RegisteredHandlerManipulator AVConsole::registerSlot(
        const QString& name, QObject *obj, const QString& slot,
        const QString& help, const QString& slot_completion)
{
    QMutexLocker locker(&m_mutex);

    validateNameAndHelp(name, help);

    if (!slot.contains("slotConsole"))
    {
        LOGGER_ROOT.Write(LOG_FATAL, "Please stick to the slotConsoleXXX naming convention for "
                "console slots (%s)",
                          qPrintable(slot));
    }
    if (!slot_completion.isEmpty() && !slot_completion.contains("slotComplete"))
    {
        LOGGER_ROOT.Write(LOG_FATAL, "Please stick to the slotCompleteXXX naming convention for "
                "console completion slots (%s)",
                          qPrintable(name));
    }

    m_registered_handlers.push_back(std::make_shared<RegisteredSlot>(name, obj, slot, help, slot_completion));

    AVDIRECTCONNECT(obj, SIGNAL(destroyed(QObject*)),
              this, SLOT(slotObjectDestroyed(QObject*)));

    return RegisteredHandlerManipulator(*m_registered_handlers.back());
}

///////////////////////////////////////////////////////////////////////////////

AVConsole::RegisteredHandlerManipulator AVConsole::registerCallback(
        const QString& name, QObject *owner, std::function<void(AVConsoleConnection&, const QStringList&)> callback,
        const QString& help, std::function<void(QStringList&, const QStringList&)> complete_callback)
{
    QMutexLocker locker(&m_mutex);

    validateNameAndHelp(name, help);

    m_registered_handlers.push_back(std::make_shared<RegisteredCallback>(name, owner, callback, help, complete_callback));

    if (owner != nullptr)
    {
        AVDIRECTCONNECT(owner,  SIGNAL(destroyed(QObject*)),
                        this, SLOT(slotObjectDestroyed(QObject*)));
    }

    return RegisteredHandlerManipulator(*m_registered_handlers.back());
}

///////////////////////////////////////////////////////////////////////////////

bool AVConsole::isHandlerRegistered(const QString& name) const
{
    for (int i=0; i < m_registered_handlers.size(); ++i)
    {
        if (m_registered_handlers[i]->name() == name) return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////

void AVConsole::addVariableObserver(const void *variable, QObject *observer, const QString& slot)
{
    QMutexLocker locker(&m_mutex);

    for (int i=0; i<m_registered_variables.size(); ++i)
    {
        RegisteredVariable& cur_variable = m_registered_variables[i];
        if (cur_variable.m_value->getPointer() == variable)
        {
            // Sanity check: don't let client register same observer twice for the same variable.
            for (int o=0; o<cur_variable.m_observer.size(); ++o)
            {
                if (cur_variable.m_observer[o].first == observer)
                {
                    AVLogger->Write(LOG_FATAL, "AVConsole::addVariableObserver: "
                            "Observer registered twice.");
                }
            }

            cur_variable.m_observer.push_back(qMakePair(observer, slot));
            AVDIRECTCONNECT(observer, SIGNAL(destroyed(QObject*)),
                      this, SLOT(slotVariableObserverDestroyed(QObject*)));

            return;
        }
    }

    AVLogger->Write(LOG_FATAL, "AVConsole::addVariableObserver: "
                    "variable not registered.");
}

///////////////////////////////////////////////////////////////////////////////

AVConsoleConnection *AVConsole::getConnection(const ConsoleConnectionHandle& handle)
{
    ConnectionContainer::iterator it = m_connection.find(handle.first);
    if (it == m_connection.end()) return 0;
    if (it.value() != handle.second) return 0;
    return it.key();
}

///////////////////////////////////////////////////////////////////////////////

const AVConsole::ConnectionContainer AVConsole::getConnections() const
{
    return m_connection;
}

///////////////////////////////////////////////////////////////////////////////

void AVConsole::autoComplete(QString& input, QStringList& completions)
{
    AVASSERT(AVThread::isMainThread());

    completions.clear();

    SplitResult result = splitInput(input);
    QStringList args = result.args;
    QString command = "";

    // If there are any arguments, the first one is our command. Extract it from args.
    if (!args.isEmpty())
    {
        command = args.front();
        args.pop_front();
    }

    // Get indices of all matching commands
    QVector<const RegisteredHandler*> matching_commands;

    // If the input of the command already was completed, see if we know about it.
    if (!args.isEmpty())
    {
        matching_commands.push_back(findRegisteredHandler(command).get());
        // Nothing we can do if we don't know the command.
        if (matching_commands.back() == 0) return;
    } else
    {
        // Still need to complete the command.
        for (int c=0; c<m_registered_handlers.size(); ++c)
        {
            if (m_registered_handlers[c]->name().startsWith(command))
            {
                matching_commands.push_back(m_registered_handlers[c].get());
            }
        }
    }

    if (matching_commands.empty())
    {
        // No registered command matches, do nothing
    } else if (matching_commands.size() == 1)
    {
        const RegisteredHandler& matching_handler = *matching_commands[0];

        if (command != matching_handler.name())
        {
            // Funtion name is not complete.
            // Complete name.
            input = matching_handler.name() + ' ';
        } else
        {
            // Function name is complete.
            // Append space if it isn't there yet (for command completion only)
            if (input.length() == matching_handler.name().length())
            {
                input += ' ';
                // This is the only case in which the args QStringList is empty. To simplify the
                // code below, insert the empty argument.
                AVASSERT(args.isEmpty());
                args.push_back("");
            }

            // Supply additional arguments to command-specific
            // completion fun
            matching_handler.handleCompletion(*this, completions, args);

            if (!completions.empty())
            {
                if (completions.size() == 1)
                {
                    args.pop_back();
                    args.push_back(completions[0]);
                } else
                {
                    completeToCommonPrefix(args.back(), completions);
                }

                input = command;
                for (const QString& arg : args)
                {
                    input += ' ' + AVEscape(arg, {"'", " "}, { "\\'", "\\ " });
                }
            }
        }
    } else
    {
        // Multiple matches, complete to common prefix.
        // Don't complete all functions for a group, but just the group itself (including the '.').
        for (int c=0; c < matching_commands.size(); ++c)
        {
            QString cur_cmd = matching_commands[c]->name();
            int until = cur_cmd.indexOf('.', command.length());
            if (until != -1) cur_cmd = cur_cmd.left(until+1);
            if (!completions.contains(cur_cmd)) completions.push_back(cur_cmd);
        }
        completeToCommonPrefix(input, completions);
        completions.sort();
    }
}

///////////////////////////////////////////////////////////////////////////////

const QString& AVConsole::getDefaultRecordingFile() const
{
    return m_config->m_recording_file;
}

///////////////////////////////////////////////////////////////////////////////

void AVConsole::completeFileName(
        QStringList& completions, const QStringList& args)
{
    if (args.size() != 1) return;

    QString input;
    if (!args[0].isEmpty()) input = args[0];
    else input = "./";

    QDir dir;
    if (input.contains("/"))
    {
        uint p = input.lastIndexOf("/");
        dir.setPath(input.left(p+1));
    }

    QFileInfoList list = dir.entryInfoList();
    // can happen if dir is not readable
    if (list.isEmpty()) return;

    for (QFileInfoList::const_iterator it = list.begin();
         it != list.end();
         ++it)
    {
        const QFileInfo &cur_info = *it;
        if (!cur_info.isReadable()) continue;
        if (cur_info.fileName() == "." ||
            cur_info.fileName() == "..") continue;

        // Pre/append './' and '/' if necessary, or remove './' if not present in input
        QString path = cur_info.filePath();
        if (!input.startsWith("./"))
        {
            if (path.startsWith("./")) path = path.mid(2);
        } else if (cur_info.isRelative() && !path.startsWith("./"))
        {
            path = "./" + path;
        }
        if (cur_info.isDir()) path += "/";

        completions << path;
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConsole::setSuppressStandbyExecution(bool suppress)
{
    m_suppress_standby_execution = suppress;
}

///////////////////////////////////////////////////////////////////////////////

void AVConsole::unregisterHandlers(QObject *obj)
{
    if (obj == nullptr)
    {
        return;
    }

    int s=0;
    while (s < m_registered_handlers.size())
    {
        if (m_registered_handlers[s]->object() == obj)
        {
            m_registered_handlers[s] = m_registered_handlers.back();
            m_registered_handlers.pop_back();
        } else ++s;
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConsole::unregisterHandler(const QString &name)
{
    int s=0;
    while (s < m_registered_handlers.size())
    {
        if (m_registered_handlers[s]->name() == name)
        {
            m_registered_handlers[s] = m_registered_handlers.back();
            m_registered_handlers.pop_back();
            return;
        } else ++s;
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConsole::slotObjectDestroyed(QObject *obj)
{
    unregisterHandlers(obj);
}

///////////////////////////////////////////////////////////////////////////////

void AVConsole::slotVariableOwnerDestroyed(QObject *obj)
{
    int i=0;
    while (i < m_registered_variables.size())
    {
        if (m_registered_variables[i].m_owner == obj)
        {
            m_registered_variables[i] = m_registered_variables.back();
            m_registered_variables.pop_back();
        } else ++i;
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConsole::slotVariableObserverDestroyed(QObject *obj)
{
    for (RegisteredVariableContainer::iterator cur_var = m_registered_variables.begin();
         cur_var != m_registered_variables.end();
         ++cur_var)
    {
        RegisteredVariable::ObserverContainer::iterator cur_observer;
        for (cur_observer = cur_var->m_observer.begin();
             cur_observer != cur_var->m_observer.end();
             /* don't increment */)
        {
            if (cur_observer->first == obj)
            {
                cur_observer = cur_var->m_observer.erase(cur_observer);
            } else ++cur_observer;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConsole::slotConnectionDestroyed(QObject *obj)
{
    // The object already is reduced to a QObject when this method is called - no dynamic cast possible...
    ConnectionContainer::iterator it = m_connection.find(reinterpret_cast<AVConsoleConnection*>(obj));
    AVASSERT(it != m_connection.end());
    m_connection.erase(it);
}

///////////////////////////////////////////////////////////////////////////////

void AVConsole::slotSetupDefaultInput()
{
    if (!m_connection.isEmpty()) return;

    if (m_config->m_port != 0)
    {
        new (LOG_HERE) AVConsoleTelnetServer(m_config->m_port, &AVConsole::singleton());
    }

    AVConsoleStdinoutConnection *stdin_connnection =
            new (LOG_HERE) AVConsoleStdinoutConnection;

    if (!m_config->m_input_file.isEmpty())
    {
        new (LOG_HERE) AVConsoleFileConnection(m_config->m_input_file, stdin_connnection);
    }
}

///////////////////////////////////////////////////////////////////////////////

AVConsole::AVConsole() :
    m_config(new (LOG_HERE) AVConsoleConfig),
    m_connection_handle_counter(0),
    m_suppress_standby_execution(false)
{
    new AVConsoleDefaultSlots(this);
}

///////////////////////////////////////////////////////////////////////////////

std::shared_ptr<AVConsole::RegisteredHandler> AVConsole::findRegisteredHandler(const QString &name)
{
    for (int i=0; i<m_registered_handlers.size(); ++i)
    {
        if (m_registered_handlers[i]->name() == name) return m_registered_handlers[i];
    }
    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////

AVConsole::RegisteredVariable *AVConsole::findRegisteredVariable(const QString& name)
{
    for (int i=0; i < m_registered_variables.size(); ++i)
    {
        if (m_registered_variables[i].m_name == name) return &m_registered_variables[i];
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

AVConsole::RegisteredVariable *AVConsole::findRegisteredVariable(const void *pointer)
{
    for (int i=0; i < m_registered_variables.size(); ++i)
    {
        if (m_registered_variables[i].m_value->getPointer() == pointer)
        {
            return &m_registered_variables[i];
        }
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

void AVConsole::completeToCommonPrefix(QString& input, QStringList& completions)
{
    // First eliminate all completions which don' have input as prefix.
    for (QStringList::iterator it = completions.begin();
         it != completions.end(); /* do nothing */)
    {
        if (!(*it).startsWith(input)) it = completions.erase(it);
        else ++it;
    }

    if (completions.isEmpty()) return;

    for (int cur_pos = input.length();
         /* compare nothing */;
         ++cur_pos)
    {
        if (completions[0].length() <= cur_pos) return;
        QChar next_char = completions[0].at(cur_pos);
        for (int c=1; c<completions.size(); ++c)
        {
            if (completions[c].length() == cur_pos) return;
            if (completions[c].at(cur_pos) != next_char) return;
        }

        input += next_char;
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConsole::processInput(AVConsoleConnection& connection, const QString& input)
{
    AVASSERT(AVThread::isMainThread());

    if (input.startsWith("#")) return;

    AVLogDebug << "AVConsole::processInput: " << input;

    QStringList args;
    QString handler_name;

    //Use legacy splitting for old multi line not containing single quotes
    if (input.contains('\n') && !input.contains('\''))
    {
        args = input.split('\n', QString::KeepEmptyParts);
        handler_name = args[0].section(' ', 0, 0, QString::SectionSkipEmpty);
        // remove handler name from arguments
        args[0] = args[0].section(' ', 1);
        if (args[0].isEmpty()) args.erase(args.begin());
    } else
    {
        auto split_result = splitInput(input.trimmed()); //trimmed to prevent empty last argument
        if (split_result.continue_multiline)
        {
            connection.continueMultilineInput();
            return;
        }
        if (split_result.args.isEmpty())
        {
            return;
        }
        handler_name = split_result.args[0];
        args = std::move(split_result).args.mid(1);
    }

    if (handler_name.isEmpty()) return;

    auto handler = findRegisteredHandler(handler_name);
    if (handler == nullptr)
    {
        connection.printError("Unknown command \"" + handler_name + "\". "
                "Type \"help\" or TAB for a list of commands.");
        return;
    }

    bool is_standby = AVExecStandbyChecker::isSingletonInitialized() && !AVExecStandbyChecker::singleton().isExec();
    if (is_standby && m_suppress_standby_execution && !handler->getAlwaysAllowStandbyExecution())
    {
        // Note that the test framework relies on this magic string.
        connection.print("STANDBY - not executing");
        return;
    }

    handler->handleCommand(*this, connection, args);
}
///////////////////////////////////////////////////////////////////////////////

AVConsole::SplitResult AVConsole::splitInput(const QString& input)
{
    QStringList result;

    bool in_single_char_quote = false;  // '\\'
    bool in_single_quote = false;       // '\''
    QString current_string;
    for (int i = 0; i < input.length(); i++)
    {
        if (in_single_char_quote)
        {
            in_single_char_quote = false;
            //TODO do special handling of \n
        }
        else if (in_single_quote)
        {
            if (input[i] == '\'')
            {
                in_single_quote = false;
                continue;
            }
            if (input[i] == '\\')
            {
                in_single_char_quote = true;
                continue;
            }
        }
        else if (input[i].isSpace())
        {
            while (i+1 < input.size() && input[i] != '\n' && input[i+1].isSpace()) ++i;

            result.append(current_string);
            current_string = "";
            continue;
        }
        else
        {
            switch (input[i].toLatin1())
            {

            case '\'':

                in_single_quote = true;
                continue;
            case '\\':
                in_single_char_quote = true;
                continue;

            }
        }
        current_string.append(input[i]);
    }

    if (!current_string.isNull())
    {
        result.append(current_string);

    }

    return { result, in_single_quote || in_single_char_quote };
}

///////////////////////////////////////////////////////////////////////////////

void AVConsole::addConnection(AVConsoleConnection *connection)
{
    AVASSERT(AVThread::isMainThread());

    m_connection[connection] = m_connection_handle_counter++;
    AVDIRECTCONNECT(connection, SIGNAL(destroyed(QObject*)),
              this, SLOT(slotConnectionDestroyed(QObject*)));
}

///////////////////////////////////////////////////////////////////////////////

ConsoleConnectionHandle AVConsole::getConnectionHandle(AVConsoleConnection *connection) const
{
    ConnectionContainer::const_iterator it = m_connection.find(connection);
    AVASSERT(it != m_connection.end());
    return qMakePair(it.key(), it.value());
}

///////////////////////////////////////////////////////////////////////////////

void AVConsole::validateNameAndHelp(const QString& name, const QString& help)
{
    if (help.length() < 5)
    {
        LOGGER_ROOT.Write(LOG_FATAL, "Please specify a help text for %s", qPrintable(name));
    }

    if (findRegisteredHandler(name) != 0)
    {
        LOGGER_ROOT.Write(LOG_FATAL, "AVConsole::registerSlot: %s already was registered.",
                        qPrintable(name));
    }
}

/////////////////////////////////////////////////////////////////////////////

QTextStream& operator<<(QTextStream& stream, bool b)
{
    stream << (b ? "true" : "false");
    return stream;
}

///////////////////////////////////////////////////////////////////////////////

QTextStream& operator>>(QTextStream& stream, bool& b)
{
    QString r;
    stream >> r;
    AVASSERT(!r.isEmpty());
    if      (r == "true"  || r == "1") b = 1;
    else if (r == "false" || r == "0") b = 0;
    // Put back the last read character so stream.atEnd() will return false.
    // This indicates an error the calling method.
    else stream.device()->ungetChar(*qPrintable(r.right(1)));
    return stream;
}

// End of file
