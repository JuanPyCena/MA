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

#if !defined AVCONSOLE_H_INCLUDED
#define AVCONSOLE_H_INCLUDED

// STL includes
#include <functional>

// QT
#include <QMutex>
#include <QObject>
#include <QTimer>
#include <QVector>
#include <QTextStream>

// AVCOMMON
#include "avlib_export.h"
#include "avmacros.h"
#include "avexplicitsingleton.h"
#include "avstdinreaderthread.h"
#include "avdataframingdelimiters.h"

class AVConsoleTerminalBuffer;
class AVConsole;
class AVConsoleConnection;

typedef QPair<AVConsoleConnection*, uint> ConsoleConnectionHandle;
const ConsoleConnectionHandle NullConsoleConnectionHandle = qMakePair(static_cast<AVConsoleConnection*>(0), 0u);

//! Convenience macro for quickly defining a static variable which is settable via
//! AVConsole for debugging purposes.
#define REGISTER_STATIC_CONSOLE_VAR(type, name, var, init_value) \
    static type var = init_value; \
    { static bool first = true; if (first) \
    { first = false; AVConsole::singleton().registerVariable(#name, 0, &var, true); } }

///////////////////////////////////////////////////////////////////////////////
/**
 *  Used for handling multiple connections to AVConsole, e.g. multiple telnet
 *  connections or telnet and stdin at the same time. Also see \ref avconsole_intro.
 *
 *  Console connections register themselves with the console and are deleted
 *  when the console is.
 */
class AVLIB_EXPORT AVConsoleConnection : public QObject
{
    Q_OBJECT

    AVDISABLECOPY(AVConsoleConnection);

public:
    explicit AVConsoleConnection();
    ~AVConsoleConnection() override;

    //! Only prints a message per default.
    virtual void disconnect();

    //! Write an output string for the user to see, taking into account newline issues depending
    //! on whether we are interactive (=raw terminal or telnet connection).
    //! Always adds a newline at the end of the message if there is none.
    //! If add_frame is true, a leading and trailing "----" line will be added to the message.
    void print(const QString& msg, bool add_frame = false);

    //! DEPRECATED, don't use this in new code!
    //! There currently is no known use case for this type of success message. If a method does not print an error,
    //! the assumption is that it succeeded.
    void printSuccess(const QString& msg);

    //! Defined to have a unified prefix for error messages.
    /**
     *  Note that perl and python tests fail per default if a console slot logs an error via this method.
     *  It is encouraged to use this facility to avoid redundant result checking in test script code.
     */
    void printError(const QString& msg);

    //! Must be called only from within a registered slot.
    //! Causes the slot to be called again after the next line of input has been entered,
    //! accumulating input over multiple lines (so the
    //! registered slot doesn't have to accumulate input itself).
    //! The elements of the QStringList passed to the slot will correspond to the lines
    //! of input after calling this method for the first time.
    //! If the input is finally processed by the slot, simply don't call this method again.
    void continueMultilineInput();

    //! Usefule e.g. for replaying a scenario file. \sa AVConsoleDefaultSlots::slotDelayInputUntil.
    void delayInputForMsecs(uint msecs);

    //! Can be used by a called slot if the console function cannot be executed immediately (e.g. it
    //! has to wait for a message from a remote system).
    //! The given handle can later be used to safely  retrieve the connection object
    //! and print a reply (see AVConsole::getConnection).
    //! No other input is processed by the connection in the
    //! meantime. Don't forget to call desuspend!
    ConsoleConnectionHandle suspend();
    //! Only call this on suspended connections.
    void desuspend();

    //! Record all commands and output to the given file. Will close the file when destroyed,
    //! when calling this function again, or when calling stopRecording().
    //! \return Whether the recording file has been opened successfully and recording has started.
    bool recordSession(const QString& outfile);
    void stopRecording();

    //! Store a named date/time
    //! \see AVDefaultSlots::slotConsoleSetTimeMark
    void setTimeMark(const QString &name, const QDateTime &date_time);
    //! Request a named date/time
    //! \return Stored date/time for given mark name or QDateTime() if name does not exist
    //! \see AVDefaultSlots::slotConsoleSetTimeMark
    QDateTime getTimeMark(const QString &name) const;
    //! \see AVDefaultSlots::slotConsoleSetTimeMark
    QStringList getTimeMarkNames() const;

    static const QString SUCCESS_PREFIX;
    static const QString ERROR_PREFIX;

protected:

    //! Write an output string for the user to see via the connection.
    //! Directly writes the given string without any modification.
    virtual void printInternal(const QByteArray& data) = 0;

    //! Called for every single input character in raw mode, or for every input line.
    //! It's fine to call this with empty input (can happen e.g. for incomplete unicode characters)
    void processInput(const QString& input);

    //! If this is true, we need to print "\r\n" instead of just "\n".
    //! This will be the case for stdin/stdout under Linux, and for telnet connections (in contrast
    //! to netcat connections, which don't operate in raw mode).
    //! \sa
    bool m_raw_mode;

    // Reference to avoid header dependency
    AVConsoleTerminalBuffer& m_terminal_buffer;

private slots:
    //! Used internally; called by m_delay_timer;
    void slotHandleBufferedInput();

    //! Only in use if handling input non-interactively, via data framing.
    void slotGotLine(const QByteArray& line);

private:
    //! This is called for lines which should actually be processed.
    void gotLine(const QString& line);

    //! Only in use if non-interactive.
    AVDataFramingDelimiters m_framing_delimiters;

    //! Temporarily set by registered slot to request continued multiline input.
    bool m_multiline;
    //! Active while the input is being delayed.
    QTimer m_delay_timer;
    //! Console is suspended by the application.
    bool m_suspended;
    //! Delayed / suspended input is buffered here.
    QStringList m_buffered_input;

    QString m_accumulated_multiline_input;

    //! Opened by a call to recordSession.
    QFile m_record_file;

    //! name/qdatetime map for time marks
    QMap<QString, QDateTime> m_time_mark_map;
};

///////////////////////////////////////////////////////////////////////////////
/**
 *  Use this class to easily make functions callable and variables set/gettable via text
 *  terminal / telnet / input file. This mechanism can be used for debugging, development,
 *  process investigation and the like. For usage examples, see \ref avconsole_intro.
 *
 *  Register your functions using registerSlot(), and your variables using registerVariable().
 *  If you want to be notified if a variable is changed via the console, use addVariableObserver().
 */
class AVLIB_EXPORT AVConsole : public QObject, public AVExplicitSingleton<AVConsole>
{
    friend class AVConsoleDefaultSlots;
    friend class AVConsoleConnection;
    friend class AVConsoleTest;
    class RegisteredHandler;

    Q_OBJECT

public:

    typedef QMap<AVConsoleConnection*, uint> ConnectionContainer;
    class AVLIB_EXPORT RegisteredHandlerManipulator
    {
    public:
        explicit RegisteredHandlerManipulator(AVConsole::RegisteredHandler& handler);
        //! Use for basic console commands which do not care about the current EXEC/Standby state and
        //! should always be executed. Also see AVConsole::setSuppressStandbyExecution.
        RegisteredHandlerManipulator& setAlwaysAllowStandbyExecution();
    private:
        RegisteredHandler& m_handler;
    };

    static AVConsole& initializeSingleton();
    ~AVConsole() override;

    //! Sets up input via telnet, stdin or input file, depending on the configuration.
    void setupDefaultInput();

    //! TODO CM not yet implemented
    //! Register a category for functions and variables, and provide a meaningful description.
    //! Functions and variables are assigned to a category by using the following syntax when
    //! registering: "category.functionname" or "category.variablename".
    //! Categories can be nested.
    void registerCategory(const QString& category_name, const QString& description);

    //! Register a slot to be called via the console.
    //! This association is automatically removed when the passed object is destroyed.
    //!
    //! Called slots can use the passed connection object to print replies to the user, see
    //! AVConsoleConnection::print.
    //!
    //! The completion slot can optionally take into account the input done so far, or simply
    //! write all valid arguments to the QStringList passed to the slot.
    //!
    //! Qt5: note that slots are always called in the main thread context, regardless of the thread
    //! association of the passed object (=a direct connection is used).
    //!
    //! \param name The function name to type in the console to call this slot.
    //! \param obj  The object for which to call the slot.
    //! \param slot The slot to be called. See AVConsole::signalCallFunction for the signature.
    //! \param help A one-line describing the basic functionality of the registered method.
    //!             Printed if the user types "help".
    //! \param slot_completion A slot designed to complete arguments for this function.
    //!                        For the method signature, see AVConsole::signalAutoComplete.
    AVConsole::RegisteredHandlerManipulator  registerSlot(
            const QString& name, QObject *obj, const QString& slot, const QString& help,
            const QString& slot_completion = "");

    //! Register a callable to be called via the console.
    //! This association must be removed manually.
    //! \sa registerSlot
    //!
    //! \param name The function name to type in the console to call this slot.
    //! \param obj Optional object that specifies the lifetime of the callback.
    //! \param callback The callable to be called. The signature is void(AVConsoleConnection&, const QStringList&).
    //! \param help A one-line describing the basic functionality of the registered method.
    //!             Printed if the user types help
    //! \param complete_callback The callable to complete arguments for this function.
    //!                          The signature is void(QStringList&, const QStringList&).
    template<typename Cb, typename CompleteCb>
    AVConsole::RegisteredHandlerManipulator registerCallback(
            const QString& name, Cb callback, const QString& help, CompleteCb complete_callback);
    //! Same as above, but automatically removes the callback registration if the given owner object is destroyed.
    template<typename Cb, typename CompleteCb>
    AVConsole::RegisteredHandlerManipulator registerCallback(
            const QString& name, QObject *owner, Cb callback, const QString& help, CompleteCb complete_callback);

    //! Same as above, but without auto complete support.
    template<typename Cb>
    AVConsole::RegisteredHandlerManipulator registerCallback(const QString& name, Cb callback, const QString& help);
    //! Same as above, but with auto-deregister and without auto complete support.
    template<typename Cb>
    AVConsole::RegisteredHandlerManipulator registerCallback(
            const QString& name, QObject *owner, Cb callback, const QString& help);

    AVConsole::RegisteredHandlerManipulator registerCallback(
            const QString& name, QObject *owner,
            std::function<void(AVConsoleConnection&, const QStringList&)> callback,
            const QString& help, std::function<void(QStringList&, const QStringList&)> complete_callback);

    bool isHandlerRegistered(const QString& name) const;
    bool isSlotRegistered(const QString& name) const { return isHandlerRegistered(name); }

    //! Unregister the console handlers that are registered for the object.
    void unregisterHandlers(QObject *obj);
    void unregisterSlots(QObject* obj) { unregisterHandlers(obj); }

    //! Unregister the console handler with the given name
    void unregisterHandler(const QString& name);
    void unregisterSlot(const QString& name) { unregisterHandler(name); }

    //! Register an arbitrary variable to be set/gettable via the console. AVToString and AVFromString
    //! must be implemented for the variable's type.
    //!
    //! TODO CM it would be possible to only require AVToString for non-settable types, implement if necessary.
    //!
    //! Internally, a pointer to the variable is stored. The association is automatically removed
    //! when the variable owner object is destroyed. Alternatively, unregisterVariable can be called
    //! manually, and no owner specified (if the owner object is not a QObject).
    //!
    //! \param name     The name used in the console to get/set the variable.
    //! \param owner    The owner object. If this object is destroyed, the variable is deregistered
    //!                 automatically. Can also be 0 if the variable has no owner object (i.e. a
    //!                 static or global variable), or if the owner is not a QObject.
    //! \param pointer  The pointer to the variable. Don't register stack variables!
    //! \param settable Whether the variable should be settable via the console.
    template<typename T>
    void registerVariable(const QString& name, QObject *owner, T *pointer,
                          bool settable = false);
    //! Manually unregister a variable registered with registerVariable(). Note
    //! that the variable must not have an owner.
    //! \param pointer  The pointer to the variable which was registered.
    template<typename T>
    void unregisterVariable(const QString& name, T *pointer);

    //! The observer will get notified if the variable is changed via the console.
    //!
    //! \param variable A variable previously registered via registerVariable.
    //! \param observer If the variable is changed, call a slot from this object.
    //! \param slot     The slot name to call if the variable is changed. For the signature, see
    //!                 AVConsole::signalVariableChanged.
    //!                 Note: The slot method has to be passed using the SLOT macro
    void addVariableObserver(const void *variable, QObject *observer, const QString& slot);

    //! \sa AVConsoleConnection::suspend.
    //! \return A pointer to the previously suspended connection, or 0 if there was a disconnet
    //!         in the meantime.
    AVConsoleConnection *getConnection(const ConsoleConnectionHandle& handle);
    const ConnectionContainer getConnections() const;

    //! Performs autocompletion on a partial input line.
    //! non-const because we need to emit signalAutoComplete.
    //!
    //! \param input [in,out]    The input to complete. The last word will be completed to the
    //!                          longest common prefix of possible completions.
    //! \param completions [out] Will be set to the list of possible completions for the last word
    //!                          in input. To this end, the completion slot of registered functions
    //!                          is used.
    void autoComplete(QString& input, QStringList& completions);

    //! Returns the name of the recording file from the process configuration. If not empty,
    //! any new connection immediately starts recording to this file.
    const QString& getDefaultRecordingFile() const;

    //! Auto completion for functions requiring a file name as argument. Provided here for
    //! convenience of client code.
    static void completeFileName(QStringList& completions, const QStringList& args);

    virtual const char *className() const { return "AVConsole"; }

    /**
     * @brief setSuppressExecutionOnStandby controls whether (most) console slots are executed only when EXEC.
     *
     * This is primarily used for running tests in a deployed setup, where the test framework connects to the AVConsole
     * of both exec and standby processes, and most operations should be performed on the EXEC only.
     * It is also used for local exec/standby testing.
     *
     * Also see AVConsole::RegisteredHandlerManipulator::setAlwaysAllowStandbyExecution; this can be used at slot
     * registration time to ignore this setting for specific slots (which is done for most built-in console slots).
     */
    void setSuppressStandbyExecution(bool suppress);

signals:
    //! Don't directly connect to this signal! Register your slots instead with registerSlot().
    void signalCallFunction(AVConsoleConnection& connection, const QStringList& args);
    //! Don't directly connect to this signal! Register your slots instead with registerSlot().
    //!
    //! Auto completion functions should append a trailing space to the completions if the function
    //! expects another argument.
    void signalAutoComplete(QStringList& completions, const QStringList& args);
    //! Don't directly connect to this signal! Register your variables instead with
    //! registerVariable().
    void signalVariableChanged(const void *variable);

private slots:
    //! Used internally; removes any registered slots for the destroyed object.
    void slotObjectDestroyed(QObject *obj);
    //! Used internally; removes any registered variables belonging to the destroyed object.
    void slotVariableOwnerDestroyed(QObject *obj);
    //! Used internally; removes any observer slots for the destroyed object.
    void slotVariableObserverDestroyed(QObject *obj);
    //! Used internally; removes the internal reference to the console connection object.
    void slotConnectionDestroyed(QObject *obj);
    //! Console connections need the event loop to work. For programs without event loop
    //! (e.g. daq_smru), starting the thread would break e.g. "ctrl+c".
    //! Solution: trigger starting the connections via an event.
    void slotSetupDefaultInput();

private:

    class TextValuePointerBase
    {
    public:
        virtual ~TextValuePointerBase() {}

        virtual QString avToString() const               = 0;
        virtual bool avFromString(const QString& string) = 0;
        virtual TextValuePointerBase* clone() const      = 0;
        virtual const void *getPointer() const           = 0;
    };

    template <typename T>
    class TextValuePointer : public TextValuePointerBase
    {
    public:
        explicit TextValuePointer(T *pointer);
        ~TextValuePointer() override {}

        QString               avToString() const override;
        bool                  avFromString(const QString& string) override;
        TextValuePointerBase* clone() const override;
        const void*           getPointer() const override;

    private:
        T *m_pointer;
    };

    class RegisteredHandler
    {
    public:
        explicit RegisteredHandler(const QString& name, const QString& help_text, QObject* obj);
        virtual ~RegisteredHandler() {}

        virtual void handleCommand(AVConsole& console, AVConsoleConnection& connection, const QStringList& args) const = 0;
        virtual void handleCompletion(AVConsole& console, QStringList& completions, const QStringList& args) const = 0;

        const QString& name() const { return m_name; }
        const QString& helpText() const { return m_help_text; }
        QObject* object() const { return m_object; }

        void setAlwaysAllowStandbyExecution()       { m_always_allow_standby_execution = true; }
        bool getAlwaysAllowStandbyExecution() const { return m_always_allow_standby_execution; }

    private:
        QString  m_name;
        QString  m_help_text;
        QObject* m_object;
        bool     m_always_allow_standby_execution;
    };

    class RegisteredSlot : public RegisteredHandler
    {
    public:
        RegisteredSlot(const QString& name, QObject* obj, const QString& slot, const QString& help,
                       const QString& slot_autocomplete="");
        ~RegisteredSlot() override {}

        void handleCommand(AVConsole &console, AVConsoleConnection &connection, const QStringList &args) const override;
        void handleCompletion(AVConsole& console, QStringList& completions, const QStringList& args) const override;

    private:
        QString  m_slot;
        QString  m_slot_autocomplete;
    };

    class RegisteredCallback : public RegisteredHandler
    {
        using Callable = std::function<void(AVConsoleConnection&, const QStringList&)>;
        using CompleteCallable = std::function<void(QStringList&, const QStringList&)>;
    public:
        RegisteredCallback();
        RegisteredCallback(const QString& name, QObject* obj, Callable callback, const QString& help, CompleteCallable complete_callback);
        ~RegisteredCallback() override {}

        void handleCommand(AVConsole &console, AVConsoleConnection &connection, const QStringList &args) const override;
        void handleCompletion(AVConsole& console, QStringList& completions, const QStringList& args) const override;

    private:
        Callable          m_callback;
        CompleteCallable  m_complete_callback;
    };

    typedef QVector<std::shared_ptr<RegisteredHandler>> RegisteredHandlerContainer;

    class AVLIB_EXPORT RegisteredVariable
    {
    public:
        RegisteredVariable();
        RegisteredVariable(const RegisteredVariable& other);
        RegisteredVariable(const QString& name, QObject* owner, TextValuePointerBase *value,
                           bool settable);
        virtual ~RegisteredVariable();

        RegisteredVariable& operator=(const RegisteredVariable& other);

        QString               m_name;
        QObject              *m_owner;
        TextValuePointerBase *m_value;
        bool                  m_settable;

        typedef QVector<QPair<QObject*, QString> > ObserverContainer;
        //! Observer objects which are to be notified if this variable is changed via the console.
        //! For the signature of the slot, see AVConsole::signalVariableChanged.
        ObserverContainer m_observer;
    };
    typedef QVector<RegisteredVariable> RegisteredVariableContainer;

    AVConsole();
    AVConsole(const AVConsole&);
    AVConsole& operator=(const AVConsole&);

    //! \param name The name under which the handler was registered
    std::shared_ptr<AVConsole::RegisteredHandler> findRegisteredHandler(const QString &name);
    //! \param name The name under which the variable was registered
    RegisteredVariable *findRegisteredVariable(const QString& name);
    RegisteredVariable *findRegisteredVariable(const void *pointer);

    //! First, drops all strings from completions which don't have input as prefix (this is done to
    //! allow completion functions to simply specify all possible completions, without checking
    //! the arguments).
    //! Then completes input to the longest common prefix for all strings still in completions.
    static void completeToCommonPrefix(QString& input, QStringList& completions);

    //! Called by the \see AVConsoleConnection when a new line of input should be processed.
    void processInput(AVConsoleConnection& connection, const QString& input);

    struct SplitResult
    {
        QStringList args; //!< Splitted arguments.
        bool continue_multiline = false; //!< Is True when quotes have not been closed and input shall continue.
    };

    //!
    //! \brief splitInput Performs a bash like splitting of parameters, but recognizing only for single quotes and backslash
    //! Also empty parts will be preserved (i.a. to facilitate autocomplete)
    //! \param input The complete console input.
    //! \return See \sa SplitResult
    //!
    static SplitResult splitInput(const QString& input);

    //! Add an active connection. This connection is deleted when the console is (AVConsole takes
    //! ownership). Should only be called by connection implementations!
    void addConnection(AVConsoleConnection *connection);

    //! The connection might be already gone when we need it again, so return a handle.
    //! \sa AVConsoleConnection::suspend().
    ConsoleConnectionHandle getConnectionHandle(AVConsoleConnection *connection) const;

    void validateNameAndHelp(const QString& name, const QString& help);

    //! This is a pointer to avoid avconfig and avconfig2 includes in this header
    //! (which potentially break files including this header)
    //! Implementation of this class is in avconsole_int.{h.cpp}
    class AVConsoleConfig *m_config;

    //! The handlers and variables registered for calling by the console.
    RegisteredHandlerContainer m_registered_handlers;
    RegisteredVariableContainer m_registered_variables;

    //! Pairs connection pointer and "unique" number. Those two together form a connection handle.
    ConnectionContainer m_connection;
    uint m_connection_handle_counter;

    //! This mutex only protects the AVConsole interface functions, so things can be registered
    //! from different threads.
    //! Console functions / variable observers etc. are always called in the main thread context.
    QMutex m_mutex;

    //! AVConsole::setSuppressStandbyExecution.
    bool m_suppress_standby_execution;
};

///////////////////////////////////////////////////////////////////////////////

AVLIB_EXPORT QTextStream& operator<<(QTextStream& stream, bool b);
//! This method assumes that stream contains *only* the bool ("true" or "false") for error
//! handling.
AVLIB_EXPORT QTextStream& operator>>(QTextStream& stream, bool& b);

///////////////////////////////////////////////////////////////////////////////

template<typename T>
AVConsole::TextValuePointer<T>::TextValuePointer(T *pointer) :
    m_pointer(pointer)
{

}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
QString AVConsole::TextValuePointer<T>::avToString() const
{
    return AVToString(*m_pointer);
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
bool AVConsole::TextValuePointer<T>::avFromString(const QString& string)
{
    return AVFromString(string, *m_pointer);
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
AVConsole::TextValuePointerBase* AVConsole::TextValuePointer<T>::clone() const
{
    TextValuePointer* pointer = new TextValuePointer(m_pointer);
    AVASSERT(pointer != 0);
    return pointer;
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
const void *AVConsole::TextValuePointer<T>::getPointer() const
{
    return m_pointer;
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
void AVConsole::registerVariable(const QString& name, QObject *owner, T *pointer,
                                 bool settable)
{
    QMutexLocker locker(&m_mutex);

    if (name.toLower() != name)
    {
        AVLogFatal << "Please stick to the lowercase_with_underscore naming convention (" + name + ")";
    }
    if (!name.contains("."))
    {
        AVLogFatal << "Please use a hierarchical naming scheme for console variables, e.g. \"fdp2.is_connected\" (" + name + ")";
    }

    if (findRegisteredVariable(name)    != 0 ||
        findRegisteredVariable(pointer) != 0)
    {
        AVLogger->Write(LOG_FATAL, "AVConsole::registerVariable: %s already was registered.",
                        qPrintable(name));
    }

    TextValuePointerBase *new_pointer = new TextValuePointer<T>(pointer);
    AVASSERT(new_pointer != 0);

    m_registered_variables.push_back(RegisteredVariable(name, owner, new_pointer, settable));

    if (owner != 0)
    {
        AVDIRECTCONNECT(owner, SIGNAL(destroyed(QObject*)),
                  this, SLOT(slotVariableOwnerDestroyed(QObject*)));
    }
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
void AVConsole::unregisterVariable(const QString& name, T *pointer)
{
    AVASSERT(pointer != nullptr);

    QMutexLocker locker(&m_mutex);

    const RegisteredVariable* variable = findRegisteredVariable(name);
    AVASSERT(variable != nullptr);
    AVASSERT(variable->m_owner == nullptr);
    AVASSERT(variable->m_value != nullptr);
    AVASSERT(variable->m_value->getPointer() == pointer);

    auto registered_variable = m_registered_variables.end();
    for(auto it = m_registered_variables.begin();
        it != m_registered_variables.end(); it++)
    {
        if(it == variable)
        {
            registered_variable = it;
            break;
        }
    }

    AVASSERT(registered_variable != m_registered_variables.end());
    m_registered_variables.erase(registered_variable);
}

///////////////////////////////////////////////////////////////////////////////

template<typename Cb>
AVConsole::RegisteredHandlerManipulator AVConsole::registerCallback(
        const QString& name, Cb callback, const QString& help)
{
    return registerCallback(name, nullptr, callback, help, nullptr);
}

///////////////////////////////////////////////////////////////////////////////

template<typename Cb>
AVConsole::RegisteredHandlerManipulator AVConsole::registerCallback(
        const QString& name, QObject *owner, Cb callback, const QString& help)
{
    return registerCallback(name, owner, callback, help, nullptr);
}

///////////////////////////////////////////////////////////////////////////////

template<>
AVConsole::RegisteredHandlerManipulator AVConsole::registerCallback(
        const QString& name, QObject *obj,
        std::function<void(AVConsoleConnection&, const QStringList&)> callback,
        const QString& help, std::function<void(QStringList&, const QStringList&)> complete_callback);

///////////////////////////////////////////////////////////////////////////////

template<typename Cb, typename CompleteCb>
AVConsole::RegisteredHandlerManipulator AVConsole::registerCallback(
        const QString& name, QObject *owner, Cb callback, const QString& help, CompleteCb complete_callback)
{
    std::function<void(AVConsoleConnection&, const QStringList&)> callback_func = callback;
    std::function<void(QStringList&, const QStringList&)> complete_callback_func = complete_callback;
    return registerCallback(name, owner, callback_func, help, complete_callback_func);
}

///////////////////////////////////////////////////////////////////////////////

template<typename Cb, typename CompleteCb>
AVConsole::RegisteredHandlerManipulator AVConsole::registerCallback(
        const QString& name, Cb callback, const QString& help, CompleteCb complete_callback)
{
    return registerCallback(name, nullptr, callback, help, complete_callback);
}

#endif // AVCONSOLE_H_INCLUDED

// End of file
