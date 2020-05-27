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
          purposes. Internal classes.
 */


#include <signal.h>
#include <iostream>

// QT includes
#include <QtGlobal>
#include <QTextStream>
#include <QEvent>
#include <QDir>

// AVCOMMON includes
#include "avconfig.h"
#include "avdatetime.h"
#include "avdaemon.h"
#include "avlog.h"
#include "avmisc.h"
#include "avprocessstate.h"
#include "avtimereference.h"
#include "avtimereference/avtimereferenceadapter.h"
#include "avexecstandbychecker.h"

#if defined(Q_OS_UNIX)
#include <unistd.h>
#endif

#include "avconsole_int.h"


const char* AVConsoleTelnetConnection::ECHO_LOG_MODES[] = { "off", "warn", "on" };

const QString AVConsoleDefaultSlots::QUIT_CMD       = "quit";
const QString AVConsoleDefaultSlots::DISCONNECT_CMD = "disconnect";

//! This matches the enum in AVLog
const char* AVConsoleDefaultSlots::LOG_LEVELS[] =
    { "DEBUG2", "DEBUG1", "DEBUG", "INFO", "WARNING", "ERROR" };


// see http://www.linuxselfhelp.com/howtos/Bash-Prompt/Bash-Prompt-HOWTO-6.html
// for control codes
// Also see http://en.wikipedia.org/wiki/ANSI_escape_code

//! hex 1b is octal 33
const QString AVConsoleTerminalBuffer::ESC          = "\x1b";
const QString AVConsoleTerminalBuffer::ERASE_LINE   = "\x1b[K";
const QString AVConsoleTerminalBuffer::ERASE_SCREEN = "\x1b[2J";
//! Set cursor position to 0;0
const QString AVConsoleTerminalBuffer::RESET_CURSOR = "\x1b[0;0H";
const QString AVConsoleTerminalBuffer::CURSOR_LEFT  = "\x1b[%1D";
const QString AVConsoleTerminalBuffer::CURSOR_RIGHT = "\x1b[%1C";
const QString AVConsoleTerminalBuffer::CURSOR_UP    = "\x1b[%1A";
const QString AVConsoleTerminalBuffer::CURSOR_DOWN  = "\x1b[%1B";
const QString AVConsoleTerminalBuffer::HOME         = "\x1bOH";
const QString AVConsoleTerminalBuffer::END          = "\x1bOF";
const QString AVConsoleTerminalBuffer::SCREEN_HOME  = "\x1b[1~";
const QString AVConsoleTerminalBuffer::SCREEN_END   = "\x1b[4~";
const QString AVConsoleTerminalBuffer::DEL          = "\x1b[3~";

const QString AVConsoleTerminalBuffer::CTRL_A = "\x01";
const QString AVConsoleTerminalBuffer::CTRL_C = "\x03";
const QString AVConsoleTerminalBuffer::CTRL_D = "\x04";
const QString AVConsoleTerminalBuffer::CTRL_E = "\x05";
const QString AVConsoleTerminalBuffer::CTRL_K = "\x0b";
const QString AVConsoleTerminalBuffer::CTRL_L = "\x0c";
const QString AVConsoleTerminalBuffer::CTRL_U = "\x15";
const QString AVConsoleTerminalBuffer::CTRL_Z = "\x1a";
const QString AVConsoleTerminalBuffer::BACKSPACE = "\x7f";

const QString AVConsoleTerminalBuffer::CRLF = "\r\n";

const char AVConsoleTelnetConnection::TC_IAC         = '\xFF'; // interpret as command
const char AVConsoleTelnetConnection::TC_WILL        = '\xFB';
const char AVConsoleTelnetConnection::TC_DO          = '\xFD';
const char AVConsoleTelnetConnection::TC_TELOPT_ECHO = '\x01';
const char AVConsoleTelnetConnection::TC_TELOPT_SGA  = '\x03';    // suppress go ahead

const char *AVConsoleStdinoutConnection::ENV_ENABLE_STDIN = "ENABLE_AVCONSOLE_STDIN";

///////////////////////////////////////////////////////////////////////////////

AVConsoleTerminalBuffer::AVConsoleTerminalBuffer() :
        m_enable_auto_completion(true),
        m_cursor_pos(0),
        m_hist_position(0)
{

}

///////////////////////////////////////////////////////////////////////////////

 AVConsoleTerminalBuffer::~AVConsoleTerminalBuffer()
{

}

///////////////////////////////////////////////////////////////////////////////

QString AVConsoleTerminalBuffer::handleInteractiveInput(QString& line, QString input)
{
    line = "";

    // Assemble escape codes
    if (input.startsWith(ESC))
    {
        if (!m_cur_escape_code.isEmpty())
        {
            AVLogger->Write(LOG_ERROR, "Overlapping escape characters. Ignoring input");
            m_cur_escape_code = "";
            return QString::null;
        }
        m_cur_escape_code = input;
        return QString::null;
    } else if (!m_cur_escape_code.isEmpty())
    {
        m_cur_escape_code += input;
        if (isCompleteEscapeCode(m_cur_escape_code))
        {
            input = m_cur_escape_code;
            m_cur_escape_code = "";
        } else return QString::null;
    }

    line = QString::null;
    QString reply;
    bool got_line = false;

    if (input == CTRL_L)
    {
        reply = ERASE_SCREEN + RESET_CURSOR + printCommandLine();
    } else if (input == CTRL_K)
    {
        reply = ERASE_LINE;
        m_cur_input_line = m_cur_input_line.left(m_cursor_pos);
    } else if (input == CTRL_U)
    {
        m_cur_input_line = m_cur_input_line.mid(m_cursor_pos);
        m_cursor_pos = 0;
        reply = printCommandLine();
    } else if (input == CTRL_C)
    {
        reply = AVConsoleTerminalBuffer::CRLF;
        m_cur_input_line = "";
        m_cursor_pos = 0;
    } else if (input == CURSOR_DOWN.arg(""))
    {
        m_cur_input_line = getPrevHistItem();
        m_cursor_pos = m_cur_input_line.length();
        reply = printCommandLine();
    } else if (input == CURSOR_UP.arg(""))
    {
        m_cur_input_line = getNextHistItem();
        m_cursor_pos = m_cur_input_line.length();
        reply = printCommandLine();
    } else if (input == CURSOR_LEFT.arg(""))
    {
        if (m_cursor_pos != 0)
        {
            reply = CURSOR_LEFT.arg(1);
            --m_cursor_pos;
        }
    } else if (input == CURSOR_RIGHT.arg(""))
    {
        if (m_cursor_pos != static_cast<uint>(m_cur_input_line.length()))
        {
            reply = CURSOR_RIGHT.arg(1);
            ++m_cursor_pos;
        }
    } else if (input == CTRL_A || input == HOME || input == SCREEN_HOME)
    {
        reply = CURSOR_LEFT.arg(m_cursor_pos);
        m_cursor_pos = 0;
    } else if (input == CTRL_E || input == END  || input == SCREEN_END)
    {
        uint move_amount = m_cur_input_line.length() - m_cursor_pos;
        if (move_amount != 0)
        {
            reply = CURSOR_RIGHT.arg(move_amount);
            m_cursor_pos = m_cur_input_line.length();
        }
    } else if (input == BACKSPACE) // Backspace
    {
        if (m_cursor_pos != 0)
        {
            --m_cursor_pos;
            m_cur_input_line.remove(m_cursor_pos, 1);
            reply = printCommandLine();
        }
    } else if (input == CTRL_D || input == DEL)
    {
        if (m_cursor_pos != static_cast<uint>(m_cur_input_line.length()))
        {
            m_cur_input_line.remove(m_cursor_pos, 1);
            reply = printCommandLine();
        }
    } else if (input == "\t") // TAB
    {
        if (m_enable_auto_completion)
        {
            QStringList completions;
            AVConsole::singleton().autoComplete(m_cur_input_line, completions);

            if (completions.size() > 1)
            {
                reply = AVConsoleTerminalBuffer::CRLF;
                for (int c=0; c < completions.size(); ++c)
                {
                    reply += completions[c] + AVConsoleTerminalBuffer::CRLF;
                }
            }
            m_cursor_pos = m_cur_input_line.length();
            reply += printCommandLine();
        }
    // Newline comes in many tastes:
    // "\n"   from testscripts via dconsole
    // "\r\0" telnet session
    // "\r\n" stdin session
    } else if (input.startsWith("\r") ||
               input == "\n")
    {
        reply = AVConsoleTerminalBuffer::CRLF;
        got_line = true;
        m_cursor_pos = 0;
    } else if (input.at(0).isPrint() && input.length() == 1)
    {
        // Simply echo back the input
        m_cur_input_line.insert(m_cursor_pos, input);
        ++m_cursor_pos;
        reply = printCommandLine();
    } else
    {
        reply = "AVConsoleTerminalBuffer::handleInteractiveInput: ignoring input:\n" +
                AVHexDump(input.toUtf8());
        reply.replace("\n", "\r\n");
    }

    if (got_line)
    {
        line = m_cur_input_line;

        if (!m_cur_input_line.isEmpty()) m_history.push_back(m_cur_input_line);
        m_hist_position = m_history.size() - 1;
        m_cur_input_line = "";
    }

    return reply;
}

///////////////////////////////////////////////////////////////////////////////

const QString& AVConsoleTerminalBuffer::getCurInputLine() const
{
    return m_cur_input_line;
}

///////////////////////////////////////////////////////////////////////////////

QString AVConsoleTerminalBuffer::getNextHistItem()
{
    if (m_history.isEmpty()) return "";

    QString ret = m_history[m_hist_position];
    if (m_hist_position == 0) m_hist_position = m_history.size()-1;
    else --m_hist_position;
    return ret;
}

///////////////////////////////////////////////////////////////////////////////

QString AVConsoleTerminalBuffer::getPrevHistItem()
{
    if (m_history.isEmpty()) return "";

    QString ret = m_history[m_hist_position];
    if (m_hist_position == static_cast<uint>(m_history.size()-1)) m_hist_position = 0;
    else ++m_hist_position;
    return ret;
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleTerminalBuffer::setEnableAutoCompletion(bool e)
{
    m_enable_auto_completion = e;
}

///////////////////////////////////////////////////////////////////////////////

bool AVConsoleTerminalBuffer::isCompleteEscapeCode(const QString& str)
{
    AVASSERT(str.startsWith(ESC));
    char last_char = *qPrintable(str.right(1));

    // 2 character control codes
    if (str.length() == 2 &&
        last_char != '['  &&
        last_char != 'O'  && // I don't know where this is documented. Makes HOME and END buttons
                             // work. Hopefully doesn't break anything else.
        last_char >= '@'  && last_char <= '_') return true;

    // More than two characters
    if (str.length() > 2 && last_char >= '@' && last_char <= '~') return true;

    return false;
}

///////////////////////////////////////////////////////////////////////////////

QString AVConsoleTerminalBuffer::printCommandLine()
{
    QString ret = "\r" + ERASE_LINE + m_cur_input_line;

    AVASSERT(m_cursor_pos <= static_cast<uint>(m_cur_input_line.length()));
    uint move_amount = m_cur_input_line.length() - m_cursor_pos;
    if (move_amount != 0)
    {
        ret += CURSOR_LEFT.arg(move_amount);
    }

    return ret;
}

///////////////////////////////////////////////////////////////////////////////

AVConsoleFileConnection::AVConsoleFileConnection(
        const QString& file, AVConsoleConnection *output_connection) :
        m_output_connection(output_connection),
        m_input_file(file),
        m_encountered_error(false)
{
    // ensure that e.g. a telnet connection managed by the test framework gets all of our output before it
    // disconnects
    m_output_connection->suspend();

    if (!m_input_file.open(QIODevice::ReadOnly))
    {
        AVLogError << "AVConsoleFileConnection::AVConsoleFileConnection(): "
                   << "Cannot open input file "
                   << m_input_file.fileName();
        return;
    }

    AVLogInfo << "AVConsoleFileConnection::AVConsoleFileConnection(): "
              << "Processing file " << m_input_file.fileName();

    m_input_stream.setDevice(&m_input_file);
    QTimer::singleShot(0, this, SLOT(slotReadNextLine()));
}

///////////////////////////////////////////////////////////////////////////////

AVConsoleFileConnection::~AVConsoleFileConnection()
{
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleFileConnection::disconnect()
{
    AVLogInfo << "AVConsoleFileConnection: finished executing commands from "
              << m_input_file.fileName();
    if (m_output_connection)
    {
        if (m_encountered_error)
        {
            m_output_connection->printError("Some commands did not execute successfully:");
            m_output_connection->print(m_accumulated_errors);
        } else
        {
            m_output_connection->print(m_accumulated_output);
            m_output_connection->print("Finished executing commands from file.");
        }
        m_output_connection->desuspend();
    }
    deleteLater();
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleFileConnection::printInternal(const QByteArray &data)
{
    if (data.startsWith(qPrintable(ERROR_PREFIX)))
    {
        m_encountered_error = true;
        m_accumulated_errors += data;
    } else
    {
        m_accumulated_output += data;
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleFileConnection::slotReadNextLine()
{
    QString line = m_input_stream.readLine();
    if (line.isNull())
    {
        m_input_file.close();
        // don't directly delete the file connection here, because the executed commands can suspend it.
        // instead, trigger deletion via console command
        processInput("disconnect\n");
    } else
    {
        processInput(line + "\n");
        QTimer::singleShot(0, this, SLOT(slotReadNextLine()));
    }
}

///////////////////////////////////////////////////////////////////////////////

AVConsoleStdinoutConnection::AVConsoleStdinoutConnection() :
        m_had_input(false)
{
    // Assume for now that the std input isn't used under windows.
    // If it is, the reader thread needs to be properly implemented (nonblocking).
    // Rather use putty under windows...
    // stdout is still used under windows to show output from file processing,
    // so we need to instantiate this class (see AVConsole::setupDefaultInput).

    // Defensive programming:
    // Accessing the console via the terminal is a feature normally only used during
    // development and for test scripts.
    // It must be specifically enabled via this environment parameter and will be
    // deactivated in operational setups.
    // See also setall.sh, which sets this environment variable for development environments
    if (AVEnvironment::getEnv(ENV_ENABLE_STDIN, false, AVEnvironment::NoPrefix) == "1")
    {
        // Don't switch to raw mode if "-help" is specified, because the help text is printed
        // directly to stdout and the "\r\n" replacement mechanism from the AVLogWriter won't work.

        // Normally shouldn't come here if startup is not allowed, however the connection can be initialized
        // if the application explicitly triggers the event loop.
        if (!AVConfigBase::option_help && (!AVConfig2Global::isSingletonInitialized() ||
                                           AVConfig2Global::singleton().isApplicationStartupAllowed()))
        {
#if defined(Q_OS_UNIX)
            if(isatty(0))
            {
                if(system("stty raw -echo -isig 2>/dev/null") == 0)
                {
                    AVLogWriter::setPrependCrOnStdout(true);

                    LOGGER_ROOT.Write(LOG_DEBUG, "Interactive console on terminal enabled.");
                    m_raw_mode = true;

                    // Make sure to reset the terminal to cooked mode regardless of exit mode (e.g.
                    // interrupt which causes destructor to be skipped).
                    AVASSERT(atexit(&AVConsoleStdinoutConnection::resetTerminal) == 0);
                }
            }
#endif
        } else
        {
            // This will always happen if started via runtime environment.
            LOGGER_ROOT.Write(LOG_DEBUG, "Couldn't enable interactive console on terminal.");
        }

        AVDIRECTCONNECT(&m_stdin_reader_thread, SIGNAL(signalStdin(const QByteArray&)),
                  this, SLOT(slotStdin(const QByteArray&)));
        AVDIRECTCONNECT(&m_stdin_reader_thread, SIGNAL(signalStdinClosed()),
                  this, SLOT(slotStdinClosed()));

        m_stdin_reader_thread.start();

        AVLogger->Write(LOG_INFO, "AVConsole accepting input from stdin.");
    } else
    {
        AVLogger->Write(LOG_INFO, "AVConsole input from stdin disabled (%s not set)",
                        ENV_ENABLE_STDIN);
    }
}

///////////////////////////////////////////////////////////////////////////////

AVConsoleStdinoutConnection::~AVConsoleStdinoutConnection()
{
    m_stdin_reader_thread.stop();
    if (m_raw_mode) resetTerminal();
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleStdinoutConnection::printInternal(const QByteArray &data)
{
#if defined(Q_OS_OSF) && !defined(__GNUC__)
    cout << str << flush;
#else
    std::cout << data.constData() << std::flush;
#endif
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleStdinoutConnection::slotStdin(const QByteArray& buffer)
{
    m_had_input = true;
    QString input=QString::fromLocal8Bit(buffer.data(), buffer.size());

    if (input == AVConsoleTerminalBuffer::CTRL_C &&
        m_terminal_buffer.getCurInputLine().isEmpty())
    {
        AVASSERT(raise(SIGINT) == 0);
    } else if (input == AVConsoleTerminalBuffer::CTRL_Z)
    {
        // CTRL-Z should still send process into the background.
        // Send the appropriate signal to the own process.
#if defined(Q_OS_WIN32)
        printError(QString("CTRL-Z not yet working under windows"));
#else
        AVASSERT(raise(SIGTSTP) == 0);
#endif
    } else
    {
        processInput(input);
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleStdinoutConnection::slotStdinClosed()
{
    if (m_had_input) processInput(AVConsoleDefaultSlots::QUIT_CMD + " y\n");
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleStdinoutConnection::resetTerminal()
{
    if (system("stty -raw echo") != 0)
    {
        AVLogWarning << "AVConsoleStdinoutConnection::resetTerminal: "
                     << "failed to reset terminal.";
    }
}

///////////////////////////////////////////////////////////////////////////////

AVConsoleTelnetConnection::AVConsoleTelnetConnection(std::unique_ptr<QTcpSocket> socket) :
    m_socket(std::move(socket)),
    m_utf8_decoder(QTextCodec::codecForName("UTF-8")),
    m_echo_log_mode(ELM_OFF)
{
    AV_STATIC_ASSERT(sizeof(ECHO_LOG_MODES) / sizeof(ECHO_LOG_MODES[0]) == ELM_LAST);

    AVDIRECTCONNECT(m_socket.get(), SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
    AVDIRECTCONNECT(m_socket.get(), SIGNAL(disconnected()), this, SLOT(slotConnectionClosed()));

    // We don't know yet whether the other side supports raw mode, so print this before
    // the telnet control codes.
    print(QString("Connected to %1.").arg(AVEnvironment::getProcessName()));

    // Tell the other side we will echo and suppress goahead.
    // Note that an additional newline is appended. AVBlackBoxTest.pm relies on this NL
    // to discard the preamble in execConsoleCommand.
    static const char CONTROL[] =
        { TC_IAC, TC_WILL, TC_TELOPT_ECHO, TC_IAC, TC_WILL, TC_TELOPT_SGA, '\n', '\0' };
    // use internal print to avoid recording
    AVConsoleTelnetConnection::printInternal(CONTROL);
}

///////////////////////////////////////////////////////////////////////////////

AVConsoleTelnetConnection::~AVConsoleTelnetConnection()
{
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleTelnetConnection::disconnect()
{
    // this is important if only a single command goes over the connection before it is closed
    // again (e.g. test scripts)
    m_socket->flush();
    m_socket->close();
    // QT3: QSocket only emits closed signal if the other side cuts the connection.
    // This is no longer necessary in QT4!
    //slotConnectionClosed();
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleTelnetConnection::setEchoLogMode(ECHO_LOG_MODE mode)
{
    m_echo_log_mode = mode;
}

///////////////////////////////////////////////////////////////////////////////

AVConsoleTelnetConnection::ECHO_LOG_MODE AVConsoleTelnetConnection::getEchoLogMode() const
{
    return m_echo_log_mode;
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleTelnetConnection::printInternal(const QByteArray &data)
{
    const char * data_ptr = data.data();
    uint len = data.size();

    uint bytes_written = 0;

    while (bytes_written != len)
    {
        int ret = m_socket->write(data_ptr + bytes_written, len-bytes_written);
        if (ret == -1)
        {
            if (errno == EINTR) continue;
            else
            {
                AVLogErrorErrno << "AVConsoleTelnetConnection::printInternal: failed to write";
                return;
            }
        } else
        {
            bytes_written += ret;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleTelnetConnection::slotReadyRead()
{
    auto bytes = m_socket->bytesAvailable();

    if (bytes == 0)
    {
        AVLogger->Write(LOG_INFO, "AVConsoleTelnetConnection::slotReadyRead: 0 bytes");
        slotConnectionClosed();
        return;
    }

    // If console slot starts an event loop the readyRead() signal might be swallowed, so recheck for available bytes.
    // See SWE-6618 for more details.
    for(; bytes != 0; bytes = m_socket->bytesAvailable())
    {
        QByteArray cur_input(m_socket->readAll());

        if (cur_input.size() == 0 )
        {
            AVLogger->Write(LOG_INFO, "AVConsoleTelnetConnection::slotReadyRead: could not read data");
            slotConnectionClosed();
            return;
        }

        // Ordinarily, ctrl_d does a delete.
        // Only if the input line is empty it disconnects the telnet connection.
        //
        // From test scripts, we can get whole input lines terminated with ctrl_d. The buffering
        // mechanism in AVConsole allows buffering lines only (not control characters), so convert
        // the ctrl_d into a disconnect command here instead (slightly hackish).
        if (cur_input.endsWith(AVConsoleTerminalBuffer::CTRL_D.toLocal8Bit()) &&
            m_terminal_buffer.getCurInputLine().isEmpty())
        {
            cur_input = cur_input.left(cur_input.length()-1);
            if (!cur_input.isEmpty() && !cur_input.endsWith("\n")) cur_input += "\n";
            cur_input += AVConsoleDefaultSlots::DISCONNECT_CMD + "\n";
        }

        // This is the reply from the telnet program to our telnet control sequence (see
        // AVConsoleTelnetConnection::AVConsoleTelnetConnection).
        // If this isn't received, don't switch to raw mode (e.g. netcat, test script)
        static const char REPLY[] =
            { TC_IAC, TC_DO, TC_TELOPT_ECHO, TC_IAC, TC_DO, TC_TELOPT_SGA, '\0' };
        if (cur_input == REPLY)
        {
            m_raw_mode = true;
            m_echo_log_mode = ELM_WARN;
        } else
        {
            processInput(m_utf8_decoder.toUnicode(cur_input));
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleTelnetConnection::slotConnectionClosed()
{
    emit signalConnectionClosed(this);
}

///////////////////////////////////////////////////////////////////////////////

AVConsoleTelnetServer::AVConsoleTelnetServer(int port, QObject *parent) :
        QTcpServer(parent)
{
    setMinLevel(AVLog::LOG__DEBUG2);

    AVConsole::singleton().registerSlot(
            "setEchoLogOutput",      this,
            SLOT(slotConsoleSetEchoLog(AVConsoleConnection&, const QStringList&)),
            "Control echoing of log output on the console (telnet connection only).",
            SLOT(slotCompleteSetEchoLog(QStringList&, const QStringList&))).
            setAlwaysAllowStandbyExecution();

    setMaxPendingConnections(1);

    if (!listen(QHostAddress::Any,port))
    {
        AVLogFatal << "AVConsole: Failed to open port " << port
                   << " for listening. Are there other instances running?";
    } else
    {
        AVLogInfo << "AVConsole listening on port " << port;
    }
}

///////////////////////////////////////////////////////////////////////////////

AVConsoleTelnetServer::~AVConsoleTelnetServer()
{
    AVLogger->unregisterListener(this);
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleTelnetServer::doAddLogMsg(int logLevel, const QString &message)
{
    AVASSERT(AVThread::isMainThread());

    // output date/time stamp
    QDateTime dt      = AVDateTime::currentDateTimeUtc();
    QTime     time    = dt.time();
    QString   timestr = time.toString("hh:mm:ss ");

    // It is essential that the list of connections is copied here. Writing to a connection can always trigger a disconnect (at least in Qt3),
    // which in turn triggers removal of the connection from m_connection - a bad thing if it is currently being looped. See SWE-2033.
    ConnectionContainer cur_connections = m_connection;
    for (ConnectionContainer::iterator it = cur_connections.begin();
         it != cur_connections.end();
         ++it)
    {
        AVConsoleTelnetConnection& con = **it;
        if (con.getEchoLogMode() == AVConsoleTelnetConnection::ELM_OFF) continue;
        if (con.getEchoLogMode() == AVConsoleTelnetConnection::ELM_WARN &&
            logLevel < AVLog::LOG__WARNING) continue;

        con.print(timestr + AVLog::levelToPrefixString(logLevel) + message);
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleTelnetServer::slotConsoleSetEchoLog(
        AVConsoleConnection& connection, const QStringList& args)
{
    AVConsoleTelnetConnection *telnet_connection = 0;
    for (int i=0; i<m_connection.size(); ++i)
    {
        if (m_connection[i] == &connection) telnet_connection = m_connection[i];
    }

    if (telnet_connection == 0)
    {
        connection.printError("This is possible for telnet connections only.");
        return;
    }

    if (args.size() != 1 || (args[0] != "on" && args[0] != "off" && args[0] != "warn"))
    {
        QString cur_mode =
                AVConsoleTelnetConnection::ECHO_LOG_MODES[telnet_connection->getEchoLogMode()];
        connection.printError(QString("possible values: on/off/warn. Currently \"%1\".").arg(cur_mode));
        return;
    }
    if (args[0] == "on")   telnet_connection->setEchoLogMode(AVConsoleTelnetConnection::ELM_ON);
    if (args[0] == "off")  telnet_connection->setEchoLogMode(AVConsoleTelnetConnection::ELM_OFF);
    if (args[0] == "warn") telnet_connection->setEchoLogMode(AVConsoleTelnetConnection::ELM_WARN);

    connection.print("new value for this connection is \"" + args[0] + "\".");
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleTelnetServer::slotCompleteSetEchoLog(
        QStringList& completions, const QStringList& args)
{
    if (args.size() == 1) completions << "on" << "off" << "warn";
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleTelnetServer::slotConnectionClosed(AVConsoleTelnetConnection *connection)
{
    AVLogDebug << "AVConsoleTelnetServer::slotConnectionClosed";

    for (ConnectionContainer::iterator it = m_connection.begin();
         it != m_connection.end();
         ++it)
    {
        if (*it == connection)
        {
            (*it)->deleteLater();
            m_connection.erase(it);
            if (m_connection.isEmpty()) AVLogger->unregisterListener(this);
            return;
        }
    }

    AVLogError << Q_FUNC_INFO << ": telnet connection already closed.";
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleTelnetServer::incomingConnection(qintptr socketDescriptor)
{
    AVLogDebug << "AVConsoleTelnetServer::newConnection";

    if (m_connection.isEmpty()) AVLogger->registerListener(this);

    std::unique_ptr<QTcpSocket> socket(new QTcpSocket());
    AVASSERT(socket->setSocketDescriptor(socketDescriptor,
             QAbstractSocket::ConnectedState,QIODevice::ReadWrite));

    AVConsoleTelnetConnection *connection = new (LOG_HERE) AVConsoleTelnetConnection(std::move(socket));
    m_connection.push_back(connection);

    AVDIRECTCONNECT(connection, SIGNAL(signalConnectionClosed(AVConsoleTelnetConnection*)),
              this, SLOT(slotConnectionClosed(AVConsoleTelnetConnection*)));
}

///////////////////////////////////////////////////////////////////////////////

AVConsoleDefaultSlots::AVConsoleDefaultSlots(AVConsole *console) :
        QObject(console)
{
    registerSlots(*console);
}

///////////////////////////////////////////////////////////////////////////////

AVConsoleDefaultSlots::~AVConsoleDefaultSlots()
{
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleDefaultSlots::registerSlots(AVConsole& console)
{
    console.registerSlot(DISCONNECT_CMD, this,
                         SLOT(slotConsoleDisconnect(AVConsoleConnection&)),
                         "Disconnect (only works for telnet connections).").
            setAlwaysAllowStandbyExecution();
    console.registerSlot("help",         this,
                         SLOT(slotConsoleHelp(AVConsoleConnection&)),
                         "Display this help.").
            setAlwaysAllowStandbyExecution();
    console.registerSlot(QUIT_CMD,       this,
                         SLOT(slotConsoleQuit(AVConsoleConnection&, const QStringList&)),
                         "Tell the application to quit.").
            setAlwaysAllowStandbyExecution();
    console.registerSlot("setLogLevel",  this,
                         SLOT(slotConsoleSetLogLevel(AVConsoleConnection&, const QStringList&)),
                         "Set the log level for arbitrary loggers.",
                         SLOT(slotCompleteSetLogLevel(QStringList&, const QStringList&))).
            setAlwaysAllowStandbyExecution();
    console.registerSlot("getLogLevel", this,
                         SLOT(slotConsoleGetLogLevel(AVConsoleConnection&, const QStringList&)),
                         "Get the log level for arbitrary loggers.",
                         SLOT(slotCompleteGetLogLevel(QStringList&, const QStringList&))).
            setAlwaysAllowStandbyExecution();
    console.registerSlot("addFatalErrorWhitelistRegex", this,
                         SLOT(slotConsoleAddFatalErrorWhitelistRegex(AVConsoleConnection&, const QStringList&)),
                         "Add a regex for error messages which should not trigger a fatal error.").
            setAlwaysAllowStandbyExecution();
    console.registerSlot("setLogExecutionPhase",  this,
                         SLOT(slotConsoleSetLogExecutionPhase(AVConsoleConnection&, const QStringList&)),
                         "Set the execution phase to control log message suppression.",
                         SLOT(slotCompleteSetLogExecutionPhase(QStringList&, const QStringList&))).
            setAlwaysAllowStandbyExecution();
    console.registerSlot("set",  this,
                         SLOT(slotConsoleSet(AVConsoleConnection&, const QStringList&)),
                         "Set the value for registered variables.",
                         SLOT(slotCompleteSet(QStringList&, const QStringList&))).
            setAlwaysAllowStandbyExecution();
    console.registerSlot("get",  this,
                         SLOT(slotConsoleGet(AVConsoleConnection&, const QStringList&)),
                         "Show the value of registered variables.",
                         SLOT(slotCompleteGet(QStringList&, const QStringList&))).
            setAlwaysAllowStandbyExecution();
    console.registerSlot("processFile",  this,
                         SLOT(slotConsoleProcessFile(AVConsoleConnection&, const QStringList&)),
                         "Process an arbitrary input file.",
                         SLOT(slotCompleteFilename(QStringList&, const QStringList&))).
            setAlwaysAllowStandbyExecution();
    console.registerSlot("recordSession",  this,
                         SLOT(slotConsoleRecordSession(AVConsoleConnection&, const QStringList&)),
                         "Record the console session to a file.",
                         SLOT(slotCompleteFilename(QStringList&, const QStringList&))).
            setAlwaysAllowStandbyExecution();
    console.registerSlot("stopRecording",  this,
                         SLOT(slotConsoleStopRecording(AVConsoleConnection&)),
                         "Stop recording the console session to a file.").
            setAlwaysAllowStandbyExecution();
    console.registerSlot("logMark", this,
                         SLOT(slotConsoleLogMark(AVConsoleConnection&, const QStringList&)),
                         "Log the given statement (e.g. used in test scripts)").
            setAlwaysAllowStandbyExecution();
    console.registerSlot("setExecOnExecOnly", this, //
                         SLOT(slotConsoleSetExecOnExecOnly(AVConsoleConnection&, const QStringList&)),
                         "Control whether commands are only executed for instances in EXEC mode.",
                         SLOT(slotCompleteSetExecOnExecOnly(QStringList&, const QStringList&))).
            setAlwaysAllowStandbyExecution();
    console.registerSlot("isExec", this,
                         SLOT(slotConsoleIsExec(AVConsoleConnection&, const QStringList&)),
                         "Check whether the process is in EXEC mode.").
            setAlwaysAllowStandbyExecution();
    console.registerSlot("isStandby", this,
                 SLOT(slotConsoleIsStandby(AVConsoleConnection&, const QStringList&)),
                 "Check whether the process is in Standby mode (not in an intermediate state)").
            setAlwaysAllowStandbyExecution();
    console.registerSlot("getConfigValue", this,
                         SLOT(slotConsoleGetConfigValue(AVConsoleConnection&, const QStringList&)),
                         "Get the value corresponding to the given config parameter.",
                         SLOT(slotCompleteConfigVariable(QStringList&, const QStringList&))).
            setAlwaysAllowStandbyExecution();
    console.registerSlot("getSubconfigs", this,
                         SLOT(slotConsoleGetSubconfigs(AVConsoleConnection&, const QStringList&)),
                         "Get all subconfigs with the given prefix (but not individual parameters).",
                         SLOT(slotCompleteConfigVariable(QStringList&, const QStringList&))).
            setAlwaysAllowStandbyExecution();
    console.registerSlot("printShmUsageSummary", this,
                         SLOT(slotConsolePrintShmUsageSummary(AVConsoleConnection&)),
                         "Print out information about processes and SHM variables").
            setAlwaysAllowStandbyExecution();

    // For the avtimereferenceserver, there is no global time ref
    if (AVTimeReference::isSingletonInitialized())
    {
        console.registerSlot("delayUntil",   this,
                             SLOT(slotConsoleDelayInputUntil(AVConsoleConnection&, const QStringList&)),
                             "Wait until the AVTimeReference reaches the given time before further "
                             "input is processed.",
                             SLOT(slotCompleteTimestamp(QStringList&, const QStringList&))).
                setAlwaysAllowStandbyExecution();
        console.registerSlot("delayMilliseconds",   this,
                             SLOT(slotConsoleDelayInputForMsecs(AVConsoleConnection&, const QStringList&)),
                             "Wait the specified number of milliseconds before further "
                             "input is processed.").
                setAlwaysAllowStandbyExecution();
        console.registerSlot("printTime",    this,
                             SLOT(slotConsolePrintTime(AVConsoleConnection&)),
                             "Print the current simulated time.").
                setAlwaysAllowStandbyExecution();
        console.registerSlot("printTimeRefOffset", this,
                             SLOT(slotConsolePrintTimeRefOffset(AVConsoleConnection&)),
                             "Print the simulated time reference offset to the current time.").
                setAlwaysAllowStandbyExecution();
        console.registerSlot("printTimeRefParameterStamp", this,
                             SLOT(slotConsolePrintTimeRefParameterStamp(AVConsoleConnection&)),
                             "Prints the parameter stamp of the time reference. Changes on "
                             "time jumps, time shifts and speed changes.").
                setAlwaysAllowStandbyExecution();
        console.registerSlot("setTimeMark",   this,
                             SLOT(slotConsoleSetTimeMark(AVConsoleConnection&, const QStringList&)),
                             "Set the named time mark to the current value of the global time reference.",
                             SLOT(slotCompleteTimeMark(QStringList&, const QStringList&))).
                setAlwaysAllowStandbyExecution();
        console.registerSlot("delaySinceTimeMark",   this,
                             SLOT(slotConsoleDelaySinceTimeMark(AVConsoleConnection&, const QStringList&)),
                             "Wait until the given number of ms has passed since the given time mark, before "
                             "further processing input. Does nothing if the time is in the past.",
                             SLOT(slotCompleteTimeMark(QStringList&, const QStringList&))).
                setAlwaysAllowStandbyExecution();
    } else
    {
        AVLogInfo << "AVConsoleDefaultSlots::registerSlots: no time reference; not registering default "
                  << "console slots.";
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleDefaultSlots::slotConsoleDisconnect(AVConsoleConnection& connection)
{
    connection.disconnect();
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleDefaultSlots::slotConsoleHelp(AVConsoleConnection& connection)
{
    connection.print(QString("\nRegistered functions for %1:").
                          arg(AVEnvironment::getProcessName()));
    AVConsole::RegisteredHandlerContainer registered_handlers =
            AVConsole::singleton().m_registered_handlers;
    for (int f=0; f<registered_handlers.size(); ++f)
    {
        QString text;
        QTextStream str(&text);
        str.setFieldAlignment(QTextStream::AlignLeft);
        str.setPadChar('.');
        str.setFieldWidth(20);
        str << registered_handlers[f]->name() + " "
            << " " + registered_handlers[f]->helpText();
        connection.print(text);
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleDefaultSlots::slotConsoleDelayInputUntil(
        AVConsoleConnection& connection, const QStringList& args)
{
    if (args.size() != 1)
    {
        connection.printError("Enter date/time in format YYYYMMDD[hhmmss[.xxx]]");
        return;
    }

    AVDateTime dt(AVParseDateTime(args[0]));
    if (!dt.isValid())
    {
        connection.printError("Invalid date/time specified.");
        return;
    }

    AVDateTime now(AVTimeReference::currentDateTime());
    int msecs = now.msecsTo(dt);
    if (msecs > 0)
    {
        connection.print(QString("Delaying input for %1 msecs").arg(msecs));
        connection.delayInputForMsecs(msecs);
    } else
    {
        connection.printError(args[0] + " is in the past; not delaying input.");
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleDefaultSlots::slotConsoleDelayInputForMsecs(
        AVConsoleConnection& connection, const QStringList& args)
{
    if (args.size() != 1)
    {
        connection.printError("Enter number of milliseconds to delay");
        return;
    }

    bool ok;
    uint msecs = args[0].toUInt(&ok);
    if (!ok)
    {
        connection.printError("Invalid number");
        return;
    }

    connection.print(QString("Delaying input for %1 msecs").arg(msecs));
    connection.delayInputForMsecs(msecs);
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleDefaultSlots::slotCompleteTimestamp(
        QStringList& completions, const QStringList& args)
{
    if (args.size() != 1) return;
    QDateTime dt = AVTimeReference::currentDateTime();
    completions << dt.toString("yyyyMMddhhmmss");
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleDefaultSlots::slotConsolePrintTime(AVConsoleConnection& connection)
{
    QDateTime dt(AVTimeReference::currentDateTime());
    connection.print(AVPrintDateTime(dt));
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleDefaultSlots::slotConsolePrintTimeRefOffset(AVConsoleConnection& connection)
{
    qint64 offset = AVTimeReference::getDateTimeOffsetMs();
    connection.print(AVToString(offset));
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleDefaultSlots::slotConsolePrintTimeRefParameterStamp(AVConsoleConnection& connection)
{
    QString reply;
    if (!AVTimeReferenceAdapter::isSingletonInitialized())
    {
        // used in python API, see AVTimeReferenceServerInterface.wait_for_current_session
        reply = "NO TIME REFERENCE ADAPTER";
    } else
    {
        reply = AVToString(AVTimeReference::getTimeReferenceParameterStamp());
    }
    connection.print(reply);
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleDefaultSlots::slotConsoleQuit(
        AVConsoleConnection& connection, const QStringList& args)
{
    // Don't let users accidentially quit process...
    if (args.empty())
    {
        connection.print("Really? (y)");
        connection.continueMultilineInput();
    } else
    {
        if (args[0] == "y" || args[0] == QUIT_CMD + " y")
        {
            connection.disconnect();
            AVAPPDAEMONQUIT();
        }
        else connection.printError("Not confirmed.");
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleDefaultSlots::slotConsoleSetLogLevel(
        AVConsoleConnection& connection, const QStringList& args)
{
    AVLogFactory& factory = AVLogFactory::getInstance();

    bool args_ok = args.size() == 1 || args.size() == 2;

    uint level;
    uint num_log_levels = sizeof(LOG_LEVELS) / sizeof(LOG_LEVELS[0]);
    for (level=0; level<num_log_levels; ++level)
    {
        if (args.back() == LOG_LEVELS[level]) break;
    }
    if (level == num_log_levels) args_ok = false;

    AVLog *selected_logger = 0;
    if (args.size() == 2)
    {
        if (factory.registeredLoggers().contains(args[0]))
        {
            selected_logger = &factory.getLogger(args[0]);
        } else
        {
            connection.printError("Logger \"" + args[0] + "\" is not registered.");
            args_ok = false;
        }
    } else
    {
        if (AVLogger != 0)
        {
            selected_logger = AVLogger;
        } else
        {
            connection.printError("No default logger.");
            args_ok = false;
        }
    }

    if (!args_ok)
    {
        connection.printError("usage: \"setLogLevel [logger] level\".");
        connection.print("Currently registered loggers: " +
                   factory.registeredLoggers().join(", "));
    } else
    {
        selected_logger->setMinLevel(level);
        connection.print(QString("Set %1 log level to %2").
                   arg(selected_logger->objectName()).
                   arg(selected_logger->minLevel()));
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleDefaultSlots::slotCompleteSetLogLevel(
        QStringList& completions, const QStringList& args)
{
    if (args.size() == 1)
    {
        completions = AVLogFactory::getInstance().registeredLoggers();
        for (int c=0; c < completions.size(); ++c)
        {
            completions[c] += " "; // We expect another argument
        }
    } else if (args.size() == 2)
    {
        for (uint i=0; i<sizeof(LOG_LEVELS) / sizeof (LOG_LEVELS[0]); ++i)
        {
            completions.push_back(LOG_LEVELS[i]);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleDefaultSlots::slotConsoleGetLogLevel(
        AVConsoleConnection& connection, const QStringList& args)
{
    AVLogFactory& factory = AVLogFactory::getInstance();

    bool args_ok = args.size() == 0 || args.size() == 1;

    AVLog *selected_logger = 0;
    if (args.size() == 1)
    {
        if (factory.registeredLoggers().contains(args[0]))
        {
            selected_logger = &factory.getLogger(args[0]);
        }
        else
        {
            connection.printError("Logger \"" + args[0] + "\" is not registered.");
            args_ok = false;
        }
    } else
    {
        if (AVLogger != 0)
        {
            selected_logger = AVLogger;
        } else
        {
            connection.printError("No default logger.");
            args_ok = false;
        }
    }

    if (!args_ok)
    {
        connection.printError("usage: \"getLogLevel [logger]\".");
        connection.print("Currently registered loggers: " +
                   factory.registeredLoggers().join(", "));
    } else
    {
        connection.print(AVToString(LOG_LEVELS[selected_logger->minLevel()]));
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleDefaultSlots::slotCompleteGetLogLevel(
        QStringList& completions, const QStringList& args)
{
    if (args.size() == 1)
    {
        completions = AVLogFactory::getInstance().registeredLoggers();
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleDefaultSlots::slotConsoleAddFatalErrorWhitelistRegex(AVConsoleConnection &connection, const QStringList &args)
{
    const QString pattern = args.join(' ');
    QRegularExpression regex(pattern);

    if (!regex.isValid() || pattern.isEmpty())
    {
        connection.printError("Could not create pattern from: " + pattern + "\nError:" + regex.errorString());
        return;
    }
    AVLog::addFatalErrorWhitelistRegex(regex);
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleDefaultSlots::slotConsoleSetLogExecutionPhase(AVConsoleConnection& connection, const QStringList& args)
{
    if (args.count() != 1)
    {
        connection.printError("Specify one of " + AVLog::getExecutionPhaseStrings().join(", "));
        return;
    }

    AVLog::ExecutionPhase phase;
    if (!AVLog::enumFromString(args[0], phase))
    {
        connection.printError("Unknown phase " + args[0]);
        return;
    }

    AVLog::setExecutionPhase(phase);
    connection.print("Phase now is " + args[0]);
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleDefaultSlots::slotCompleteSetLogExecutionPhase(QStringList& completions, const QStringList& args)
{
    Q_UNUSED(args);

    completions = AVLog::getExecutionPhaseStrings();
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleDefaultSlots::slotConsoleSet(AVConsoleConnection& connection, const QStringList& args)
{
    if (args.size() < 2)
    {
        connection.printError("Usage: set var_name value");
        return;
    }

    AVConsole& console = AVConsole::singleton();
    AVConsole::RegisteredVariable *var = console.findRegisteredVariable(args[0]);
    if (var == 0)
    {
        connection.printError("Variable \"" + args[0] + "\" unknown.");
        return;
    }
    if (!var->m_settable)
    {
        connection.printError("\"" + args[0] + "\" is read-only.");
        return;
    }

    QStringList var_string = args;
    var_string.pop_front();
    bool ret = var->m_value->avFromString(var_string.join(" "));

    if (!ret)
    {
        connection.printError("Failed to parse \"" + var_string.join(" ") + "\"");
        return;
    }

    for (int o=0; o < var->m_observer.size(); ++o)
    {
        // Temporarily connect our signalVariableChanged() to the proper slot and emit it.
        connect(&console, SIGNAL(signalVariableChanged(const void*)),
                var->m_observer[o].first, qPrintable(var->m_observer[o].second));
        // Now call the registered slot
        emit console.signalVariableChanged(var->m_value->getPointer());
        disconnect(&console, SIGNAL(signalVariableChanged(const void*)),
                   0, 0);
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleDefaultSlots::slotCompleteSet(QStringList& completions, const QStringList& args)
{
    AVConsole& console = AVConsole::singleton();
    if (args.size() == 1)
    {
        for (int i=0; i<console.m_registered_variables.size(); ++i)
        {
            if (console.m_registered_variables[i].m_settable)
            {
                completions << console.m_registered_variables[i].m_name + " ";
            }
        }
    } else if (args.size() == 2)
    {
        AVConsole::RegisteredVariable *var = console.findRegisteredVariable(args[0]);
        if (var != 0 && var->m_settable)
        {
            QString val = var->m_value->avToString();
            completions << val;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleDefaultSlots::slotConsoleGet(AVConsoleConnection& connection, const QStringList& args)
{
    if (args.size() != 1)
    {
        connection.printError("Usage: get var_name");
        return;
    }

    AVConsole& console = AVConsole::singleton();
    AVConsole::RegisteredVariable *var = console.findRegisteredVariable(args[0]);
    if (var == 0)
    {
        connection.printError("Variable \"" + args[0] + "\" unknown.");
        return;
    }

    QString val = var->m_value->avToString();
    connection.print(val);
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleDefaultSlots::slotCompleteGet(QStringList& completions, const QStringList& args)
{
    AVConsole& console = AVConsole::singleton();
    if (args.size() == 1)
    {
        for (int i=0; i<console.m_registered_variables.size(); ++i)
        {
            completions << console.m_registered_variables[i].m_name;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleDefaultSlots::slotConsoleProcessFile(
        AVConsoleConnection& connection, const QStringList& args)
{
    if (args.empty())
    {
        connection.printError("Specify filename as argument.");
        return;
    }

    QFile file(args[0]);
    if (!file.open(QIODevice::ReadOnly))
    {
        connection.printError(QString("Cannot read %1.").arg(args[0]));
        return;
    }

    // don't print anything to the console here, because any error needs to be reported first. See m_encountered_error.

    // deletion happens via disconnected which is injected as last command
    new (LOG_HERE) AVConsoleFileConnection(args[0], &connection);
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleDefaultSlots::slotConsoleRecordSession(
        AVConsoleConnection& connection, const QStringList& args)
{
    if (args.size() != 1)
    {
        connection.printError("Please specify the file to record to.");
        return;
    }

    if (connection.recordSession(args[0]))
    {
        connection.print("Recording started " + AVDateTime::currentDateTimeUtc().toString());
    } else
    {
        connection.printError("Failed to open recording file " + args[0]);
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleDefaultSlots::slotConsoleStopRecording(
        AVConsoleConnection& connection)
{
    connection.stopRecording();
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleDefaultSlots::slotCompleteFilename(
        QStringList& completions, const QStringList& args)
{
    AVConsole::completeFileName(completions, args);
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleDefaultSlots::slotConsoleLogMark(
        AVConsoleConnection& connection, const QStringList& args)
{
    Q_UNUSED(connection);
    AVLogInfo << args.join(" ");
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleDefaultSlots::slotConsoleSetTimeMark(
        AVConsoleConnection& connection, const QStringList& args)
{
    if (args.size() != 1)
    {
        connection.printError("Enter name of time mark in format NAME_WITHOUT_WHITESPACES");
        return;
    }

    AVDateTime now(AVTimeReference::currentDateTime());
    connection.setTimeMark(args[0], now);

    connection.print(QString("Set time mark %1 to %2").arg(args[0]).arg(AVPrintDateTime(now)));
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleDefaultSlots::slotCompleteTimeMark(
        QStringList& completions, const QStringList& args)
{
    if (args.size() != 1) return;

    // Strictly speaking this is incorrect. For simplicity we enumerate the time marks from all
    // connections.
    for (AVConsoleConnection* connection : AVConsole::singleton().getConnections().keys())
    {
        completions += connection->getTimeMarkNames();
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleDefaultSlots::slotConsoleDelaySinceTimeMark(
        AVConsoleConnection& connection, const QStringList& args)
{
    if (args.size() != 2)
    {
        connection.printError("Enter delay in milliseconds and name of time mark in format "
                "TIME_MARK_NAME DELAY_MS");
        return;
    }

    AVDateTime time_mark_dt;
    time_mark_dt = connection.getTimeMark(args[0]);
    if (!time_mark_dt.isValid())
    {
        connection.printError("Failed to find time mark with name " + args[0]);
        return;
    }

    bool ok = false;
    uint delay_msecs = args[1].toUInt(&ok);
    if (!ok)
    {
        connection.printError("Invalid number");
        return;
    }

    AVDateTime target_dt(time_mark_dt.addMSecs(delay_msecs));

    AVDateTime now(AVTimeReference::currentDateTime());
    int wait_msecs = now.msecsTo(target_dt);
    if (wait_msecs >= 0)
    {
        connection.print(QString("Delaying input for %1 msecs").arg(wait_msecs));
        connection.delayInputForMsecs(wait_msecs);
    } else
    {
        connection.printError(
                args[0] + QString(" is %1 ms in the past; not delaying input.").
                arg(time_mark_dt.msecsTo(now)));
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleDefaultSlots::slotConsoleSetExecOnExecOnly(
        AVConsoleConnection& connection, const QStringList& args)
{
    bool enable = true;
    if (args.size() >= 1)
    {
        if      (args[0] == "on")  enable = true;
        else if (args[0] == "off") enable = false;
        else
        {
            connection.printError("Invalid Argument " + args[0]);
            return;
        }
    }

    AVConsole::singleton().setSuppressStandbyExecution(enable);
    QString enabled = "OFF";
    if (enable) enabled = "ON";

    connection.print("ExecOnExecOnly turned " + enabled);
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleDefaultSlots::slotCompleteSetExecOnExecOnly(
        QStringList& completions, const QStringList& args)
{
    if (args.size() == 1) completions << "on" << "off";
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleDefaultSlots::slotConsoleIsExec(
        AVConsoleConnection& connection, const QStringList& args)
{
    Q_UNUSED(args);

    if (!AVExecStandbyChecker::isSingletonInitialized())
    {
        connection.printError("Process has no exec standby checker");
        return;
    }

    connection.print(AVToString(AVExecStandbyChecker::singleton().isExec()));
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleDefaultSlots::slotConsoleIsStandby(AVConsoleConnection &connection, const QStringList &args)
{
    Q_UNUSED(args);

    if (!AVExecStandbyChecker::isSingletonInitialized())
    {
        connection.printError("Process has no exec standby checker");
        return;
    }

    connection.print(AVToString(AVExecStandbyChecker::singleton().isStandby()));
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleDefaultSlots::slotConsoleGetConfigValue(AVConsoleConnection& connection,
                                                      const QStringList& args)
{
    if (args.count() != 1)
    {
        connection.printError("(fully qualified) config parameter name expected.");
        return;
    }

    const AVConfig2ImporterClient::LoadedParameter* value =
            AVConfig2Global::singleton().getLoadedParameter(args[0]);

    if(value)
    {
        connection.print(value->getValue());
    } else
    {
        connection.printError("config value " + args[0] + " is not available.");
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleDefaultSlots::slotConsoleGetSubconfigs(AVConsoleConnection &connection, const QStringList &args)
{
    if (args.count() != 1)
    {
        connection.printError("(fully qualified) subconfig prefix expected.");
        return;
    }

    const AVConfig2Container::LoadedParameterContainer& params = AVConfig2Global::singleton().getLoadedParameters(QRegularExpression(args[0] + "\\..+"));
    QSet<QString> config_map_contents;
    for (QString key : params.keys())
    {
        if (!key.startsWith(args[0])) continue;
        QString name = key.mid(args[0].length()+1);
        if (!name.contains(".")) continue;
        name = name.left(name.indexOf("."));

        config_map_contents.insert(name);
    }

    connection.print(AVToString(config_map_contents));
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleDefaultSlots::slotCompleteConfigVariable(QStringList &completions, const QStringList &args)
{
    const AVConfig2LoadedData& data = AVConfig2Global::singleton().getLoadedData();

    if (args.count() != 1) return;

    for (uint i=0; i<data.getElementCount<AVConfig2LoadedData::LoadedParameter>(); ++i)
    {
        const AVConfig2LoadedData::LoadedParameter& cur_param = data.getElementByIndex<AVConfig2LoadedData::LoadedParameter>(i);
        if (cur_param.getName().startsWith(args[0]))
        {
                completions << cur_param.getName();
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleDefaultSlots::slotConsolePrintShmUsageSummary(AVConsoleConnection &connection)
{
    if (AVProcState == nullptr)
    {
        connection.printError("AVProcState is null");
        return;
    }
    connection.print(AVProcState->paramBlockInfoAsString());
}

// End of file
