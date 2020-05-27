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

#if !defined AVCONSOLE_INT_H_INCLUDED
#define AVCONSOLE_INT_H_INCLUDED

// QT
#include <QObject>
#include <QTimer>
#include <QVector>
#include <QEvent>
#include <QTcpSocket>
#include <QTcpServer>
#include <QTextDecoder>

// AVCOMMON
#include "avlib_export.h"
#include "avconfig2classic.h"
#include "avconsole.h"
#include "avstdinreaderthread.h"
#include "avdataframingdelimiters.h"

///////////////////////////////////////////////////////////////////////////////

class AVLIB_EXPORT AVConsoleTerminalBuffer
{
public:
    AVConsoleTerminalBuffer();
    virtual ~AVConsoleTerminalBuffer();

    //! \param line [out] Will be set to a complete input line which should be processed by the
    //!                   AVConsole.
    //! \param input [in] The user input. Can be a single character, part of a escape code,
    //!                   or a whole escape code (on some systems, escape codes are "split up").
    //! \return           The "reply" to the input. Mostly will be some combination of prompt and
    //!                   cursor control codes which are to be written 1:1 to the terminal.
    QString handleInteractiveInput(QString& line, QString input);
    const QString& getCurInputLine() const;

    QString getNextHistItem();
    QString getPrevHistItem();

    void setEnableAutoCompletion(bool e);

    static const QString ESC;
    static const QString ERASE_LINE;
    static const QString ERASE_SCREEN;
    static const QString RESET_CURSOR;
    static const QString CURSOR_LEFT;
    static const QString CURSOR_RIGHT;
    static const QString CURSOR_UP;
    static const QString CURSOR_DOWN;
    static const QString HOME;
    static const QString END;
    static const QString SCREEN_HOME;
    static const QString SCREEN_END;
    static const QString DEL;

    static const QString CTRL_A;
    static const QString CTRL_C;
    static const QString CTRL_D;
    static const QString CTRL_E;
    static const QString CTRL_K;
    static const QString CTRL_L;
    static const QString CTRL_U;
    static const QString CTRL_Z;
    static const QString BACKSPACE;

    static const QString CRLF;

private:

    //! See http://en.wikipedia.org/wiki/ANSI_escape_code
    static bool isCompleteEscapeCode(const QString& str);

    QString printCommandLine();

    bool m_enable_auto_completion;

    uint    m_cursor_pos;
    QString m_cur_input_line;
    //! Assemble escape codes in this buffer because they are not always sent whole.
    //! If this contains something, it will always start with AVConsoleTerminalBuffer::ESC.
    QString m_cur_escape_code;

    QVector<QString> m_history;
    uint m_hist_position;
};

///////////////////////////////////////////////////////////////////////////////
/**
 * @brief The AVConsoleFileConnection class manages reading AVConsole commands from an input file.
 *
 * Any resulting output is displayed to the user via the connection which was used to trigger reading of the file
 * (the "output connection").
 * The output connection is suspended until all commands of the file have been executed.
 * "disconnecting" the file connection desuspends the output connection and deletes the file connection.
 */
class AVLIB_EXPORT AVConsoleFileConnection : public AVConsoleConnection
{
    Q_OBJECT
public:
    AVConsoleFileConnection(
            const QString& file, AVConsoleConnection *output_connection);
    ~AVConsoleFileConnection() override;

    void disconnect() override;

protected:
    void printInternal(const QByteArray& data) override;
protected slots:
    //! Reading a huge input data file would block the main thread at startup if it were done at
    //! once.
    //! Instead, always schedule the next line to be read via QTimer singleshot.
    void slotReadNextLine();
private:
    //! The stdin connection which triggered processing of the file might
    //! be closed at any time -> use guarded pointer.
    QPointer<AVConsoleConnection> m_output_connection;
    QFile       m_input_file;
    QTextStream m_input_stream;

    //! See m_encountered_error
    QByteArray m_accumulated_output;
    //! See m_encountered_error
    QByteArray m_accumulated_errors;
    /**
     * Goals:
     * - let the console command report an error if any of the commands in the file failed.
     * - only print the failed commands in case of an error so the issue can be tracked down quickly
     *
     * Current implementation / possible improvements:
     * - the test framework detects an error only if the output starts with the ERROR_PREFIX. Thus delay the output
     *   until it is known whether an error occured.
     * - this delay does not matter too much because the test framework waits for the entire output string anyway before
     *   printing anything into the console log file
     * - perhaps the timeout for the console command is not reset as well even if output continually is produced?
     * - possible test framework improvements:
     *   - reset timeout when output is produced
     *   - continually write console log file
     *   - check for error in every output line
     */
    bool m_encountered_error;
};

///////////////////////////////////////////////////////////////////////////////

class AVLIB_EXPORT AVConsoleStdinoutConnection : public AVConsoleConnection
{
    Q_OBJECT
public:
    explicit AVConsoleStdinoutConnection();
    ~AVConsoleStdinoutConnection() override;

protected:
    void printInternal(const QByteArray& data) override;
private slots:
    void slotStdin(const QByteArray& buffer);
    void slotStdinClosed();
private:

    //! Resets the terminal to cooked mode.
    static void resetTerminal();

    AVStdinReaderThread m_stdin_reader_thread;

    //! We want to allow the usage pattern "echo fps|fdp2client".
    //! This requires the program to exit as soon as stdin is closed.
    //! However, we also want to be able to run a program from a script where its input
    //! is closed right away, so don't exit the program if there was no input via stdin at all.
    bool m_had_input;

    static const char *ENV_ENABLE_STDIN;
};

///////////////////////////////////////////////////////////////////////////////

class AVLIB_EXPORT AVConsoleTelnetConnection : public AVConsoleConnection
{
    Q_OBJECT
public:
    explicit AVConsoleTelnetConnection(std::unique_ptr<QTcpSocket> socket);
    ~AVConsoleTelnetConnection() override;

    void disconnect() override;

    static const char* ECHO_LOG_MODES[];
    enum ECHO_LOG_MODE
    {
        ELM_OFF,
        ELM_WARN,
        ELM_ON,
        ELM_LAST
    };

    void setEchoLogMode(ECHO_LOG_MODE mode);
    ECHO_LOG_MODE getEchoLogMode() const;

signals:
    void signalConnectionClosed(AVConsoleTelnetConnection*);

protected:
    void printInternal(const QByteArray& data) override;
private slots:
    void slotReadyRead();
    void slotConnectionClosed();

private:

    //! see arpa/telnet.h or eg http://en.kioskea.net/contents/internet/telnet.php3
    //! defined here to remove dependency on arpa/telnet.h
    static const char TC_IAC;
    static const char TC_WILL;
    static const char TC_DO;
    static const char TC_TELOPT_ECHO;
    static const char TC_TELOPT_SGA;

    std::unique_ptr<QTcpSocket> m_socket;

    QTextDecoder m_utf8_decoder;

    ECHO_LOG_MODE m_echo_log_mode; //!< Controls which log levels are echoed on this connection.
};

///////////////////////////////////////////////////////////////////////////////
/**
 *  Performs the logic to control the AVConsole via telnet:
 *  - listen on a socket for connections
 *  - manage AVConsoleTelnetConnections
 */
class AVLIB_EXPORT AVConsoleTelnetServer : public QTcpServer, public AVLogMainThreadListener
{
    Q_OBJECT
public:
    //! This object is deleted by the QT parent/child mechanism.
    AVConsoleTelnetServer(int port, QObject *parent);
    ~AVConsoleTelnetServer() override;

protected:

    //! Implements the AVLogListener interface.
    //! Writes log output to telnet connection. \sa AVConsoleTelnetConnection::m_echo_log_mode.
    void doAddLogMsg(int logLevel, const QString& message) override;

private slots:
    //! Console function: Control echoing of log entries (telnet connection only).
    //! Possible arguments: on/off/warn. warn shows log entries level "warning" and above.
    void slotConsoleSetEchoLog(AVConsoleConnection& connection, const QStringList& args);
    void slotCompleteSetEchoLog(QStringList& completions, const QStringList& args);

    void slotConnectionClosed(AVConsoleTelnetConnection *connection);

private:
    void incomingConnection(qintptr socketDescriptor) override;

    typedef QVector<AVConsoleTelnetConnection*> ConnectionContainer;
    ConnectionContainer m_connection;
};

///////////////////////////////////////////////////////////////////////////////

class AVLIB_EXPORT AVConsoleDefaultSlots : public QObject
{
    Q_OBJECT
public:
    explicit AVConsoleDefaultSlots(AVConsole *console);
    ~AVConsoleDefaultSlots() override;

    void registerSlots(AVConsole& console);

    static const QString QUIT_CMD;
    static const QString DISCONNECT_CMD;

protected slots:
    //! Console function; Disconnect for telnet connections; noop for all other connections.
    void slotConsoleDisconnect(AVConsoleConnection& connection);
    //! Console function; Rudimentary help: Prints a list of all registered functions
    void slotConsoleHelp(AVConsoleConnection& connection);
    //! Console function; Don't process any more input until the specified time is reached.
    void slotConsoleDelayInputUntil(AVConsoleConnection& connection, const QStringList& args);
    //! Console function; Don't process any more input until the specified numbser of millioseconds
    //! has passed.
    void slotConsoleDelayInputForMsecs(AVConsoleConnection& connection, const QStringList& args);
    void slotCompleteTimestamp(QStringList& completions, const QStringList& args);
    //! Console function; Prints the current time.
    void slotConsolePrintTime(AVConsoleConnection& connection);
    //! Console function; Prints the time reference offset to the current time.
    void slotConsolePrintTimeRefOffset(AVConsoleConnection& connection);
    //! Console function; Prints the time reference id.
    void slotConsolePrintTimeRefParameterStamp(AVConsoleConnection& connection);
    //! Console function: quits the program.
    void slotConsoleQuit(AVConsoleConnection& connection, const QStringList& args);
    //! Console function: set the log level of the specified logger.
    void slotConsoleSetLogLevel(AVConsoleConnection& connection, const QStringList& args);
    void slotCompleteSetLogLevel(QStringList& completions, const QStringList& args);

    void slotConsoleGetLogLevel(AVConsoleConnection& connection, const QStringList& args);
    void slotCompleteGetLogLevel(QStringList& completions, const QStringList& args);
    //! Console function: adds entry to log fatal whitelist
    void slotConsoleAddFatalErrorWhitelistRegex(AVConsoleConnection& connection, const QStringList& args);

    //! Execution phase for suppressing transient warnings/errors in log files. See
    //! https://confluence.avibit.com:2233/x/ZYDa.
    //! TODO CM this should be handled using the replacement for SHM (redis?) in the long term.
    void slotConsoleSetLogExecutionPhase (AVConsoleConnection& connection, const QStringList& args);
    void slotCompleteSetLogExecutionPhase(QStringList& completions, const QStringList& args);

    //! Set the value for a registered variable
    void slotConsoleSet(AVConsoleConnection& connection, const QStringList& args);
    void slotCompleteSet(QStringList& completions, const QStringList& args);
    //! Get the value for a registered variable
    void slotConsoleGet(AVConsoleConnection& connection, const QStringList& args);
    void slotCompleteGet(QStringList& completions, const QStringList& args);

    //! Process an arbitrary input file via the console.
    void slotConsoleProcessFile(AVConsoleConnection& connection, const QStringList& args);
    //! Record the console session to a text file.
    void slotConsoleRecordSession(AVConsoleConnection& connection, const QStringList& args);
    void slotConsoleStopRecording(AVConsoleConnection& connection);

    //! Just call AVConsole helper method for file completion.
    void slotCompleteFilename(QStringList& completions, const QStringList& args);

    void slotConsoleLogMark(AVConsoleConnection& connection, const QStringList& args);

    //! Named time marks can be used for controlling delays in a replay scenario without affecting
    //! the global time reference.
    //!
    //! Currently, every console connection defines its own time marks, allowing completely
    //! independent operation.
    //! TODO CM Rethink this design. Keep track of time marks globally? Use case: simultaneous
    //! replay of input which is split across multiple console input files
    void slotConsoleSetTimeMark(AVConsoleConnection& connection, const QStringList& args);
    void slotCompleteTimeMark(QStringList& completions, const QStringList& args);
    void slotConsoleDelaySinceTimeMark(AVConsoleConnection& connection,
                                            const QStringList& args);

    //! Console function: control whether commands are only executed on instances in EXEC mode.
    void slotConsoleSetExecOnExecOnly(AVConsoleConnection& connection, const QStringList& args);
    void slotCompleteSetExecOnExecOnly(QStringList& completions, const QStringList& args);

    //! Console function: check whether the process is in EXEC mode.
    void slotConsoleIsExec(AVConsoleConnection& connection, const QStringList& args);
    void slotConsoleIsStandby(AVConsoleConnection& connection, const QStringList& args);

    //! Console function: get the config parameter value of the given parameter name
    void slotConsoleGetConfigValue(AVConsoleConnection& connection, const QStringList& args);
    void slotConsoleGetSubconfigs(AVConsoleConnection& connection, const QStringList& args);
    void slotCompleteConfigVariable(QStringList& completions, const QStringList& args);

    void slotConsolePrintShmUsageSummary(AVConsoleConnection& connection);

private:
    static const char* LOG_LEVELS[];
};

///////////////////////////////////////////////////////////////////////////////
//! Hybrid (AVConfig, AVConfig2Container) config class - can operate with both configs.
//! This is implemented here so clients of avconfig don't include avconfig2 and avconfig headers.
//! See AVConsole::m_config.
class AVLIB_EXPORT AVConsoleConfig : public AVConfig2HybridConfig
{
public:
    AVConsoleConfig() : AVConfig2HybridConfig("AVConsole", "")
    {
        m_show_help_per_default = false;

        QString help_port =
                "AVConsole telnet port. Leave at zero to avoid opening a port.";
        QString help_input_file =
                "AVConsole input file. Leave empty if no input file shall be processed.";
        QString help_recording_file =
                "File to record console sessions to. Leave empty to do no recording by default "
                "(also see the recordSession console command).";

        if (isNewConfigUsed())
        {
            registerParameter("console_port", &m_port, help_port).
                              setPureCmdlineOption(0);
            registerParameter("console_input_file", &m_input_file, help_input_file).
                              setPureCmdlineOption("");
            registerParameter("console_recording_file", &m_recording_file, help_recording_file).
                              setPureCmdlineOption("");
            refreshParams();
        } else
        {
            // Macros have been undefined, directly call methods instead...
            AVConfigEntry *entry;

            entry = new (LOG_HERE)
                    AVConfigEntry("port", &m_port, 0, qPrintable(help_port), "console_port");
            Add(entry);

            entry = new (LOG_HERE)
                    AVConfigEntry("input_file", &m_input_file, "", qPrintable(help_input_file),
                                  "console_input_file");
            Add(entry);

            entry = new (LOG_HERE)
                    AVConfigEntry("recording_file", &m_recording_file, "",
                                  qPrintable(help_recording_file),
                                  "console_recording_file");
            Add(entry);

            parseParams(false, true, false);
        }
    }
    ~AVConsoleConfig() override{};

    uint m_port;
    QString m_input_file;
    QString m_recording_file;

protected:
    bool checkForInvalidParameters() override { return true; }
};

#endif

// End of file
