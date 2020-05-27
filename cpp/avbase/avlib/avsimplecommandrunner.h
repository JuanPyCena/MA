///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// QT-Version: QT5
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*!
  \file
  \author  Kevin Krammer, k.krammer@avibit.com
  \author  QT4-PORT: Karlheinz Wohlmuth, k.wohlmuth@avibit.com
  \brief   Declaration of a class for running configurable commands/executables
*/

#if !defined(AVSIMPLECOMMANDRUNNER_H_INCLUDED)
#define AVSIMPLECOMMANDRUNNER_H_INCLUDED

// Local includes
#include "avlib_export.h"

// Qt includes
#include <qobject.h>

///////////////////////////////////////////////////////////////////////////////

//! Reads a list of commands from a config file and executes any of them on demand
/*! This is a convenience wrapper around QProcess, when the application does not want
    to deal with the details of handling QProcess instances itself.

    The configuration file based on an AVConfigList of a simple AVConfigBase subclass.
    An example configuration file looks like this:
    \code
    [stopbar]
    ,Label of the command, e.g. in dialogs
    label = 133,"Stopbar Alert",1024,"",helpText:
    ,Description of the command, e.g. in dialogs
    description = 133,"Shows an aircraft crossing a red stopbar",1024,"",helpText:
    ,List executable followed by arguments
    command-list = 138,3,1024,helpText:
    "command1.sh"
    "stopbar"
    "alert"

    [vehicleonrunway]
    ,Label of the command, e.g. in dialogs
    label = 133,"Vehicle on Runway",1024,"",helpText:
    ,Description of the command, e.g. in dialogs
    description = 133,"Shows a vehicle entering a runway",1024,"",helpText:
    ,List executable followed by arguments
    command-list = 138,5,1024,helpText:
    "command2.sh"
    "vehicle"
    "on"
    "runway"
    "alert"
    \endcode
    \note then paraset name has to be unique and will be used as the command identifier

    The label and description can be any string, intented usage is for example to use the
    label as a button text and the description as a tooltip.

    The command is a string list, where the first entry is the name of the executable and
    all further entries are passed as parameters.

    Example usage:
    \code
    // create on heap since QProcess requires the event loop
    AVSimpleCommandRunner* runner = new AVSimpleCommandRunner(this);
    AVASSERT(runner != 0);

    if (!runner->readCommandConfig("examplecommands.cfg")) return;

    QStringList commands = runner->commandIDList();
    if (commands.count() == 0) return;

    if (runner->runCommand(commands[0])
    {
        qDebug("successfully started command '%s' (%s)",
               qPrintable(runner->labelForCommand(commands[0])),
               qPrintable(runner->descriptionForCommand(commands[0])));
    }
    \endcode
*/
class AVLIB_EXPORT AVSimpleCommandRunner : public QObject
{
    Q_OBJECT
public:
    //! Creates a command handler
    /*! All QProcess instances will be children of this object and thus deleted when
        the runner is deleted.

        \param parent the QObject parent
        \param name the QObject name
    */
    AVSimpleCommandRunner(QObject* parent = 0, const char* name = 0);

    //! Destroys the instance
    /*! Since all QProcess instances created here are children of this object,
        it will delete them as well.
    */
    ~AVSimpleCommandRunner() override;

    //! Reads command configuration file
    /*! Reading is handled through an AVConfigList of a simple AVConfigBase subclass.
        An example file content can be found in the detailed description of this class.

        \param filename the filename to pass to the AVConfigBase subclass' constructor.

        \return \c true if AVConfigList::readAll() is successfull, \c false when
                the filename is empty or AVConfigList::readAll() returned \c false
    */
    bool readCommandConfig(const QString& filename);

    //! Returns the list of unique command identifiers
    /*! Uses the config's paraset names.

        \return a list of strings, each identifying one command
    */
    QStringList commandIDList() const;

    //! Returns the configured label for a given command
    /*! \warning since the label is configurable it might not be unique, i.e. two commands
                 might have the same label configured.

        \param commandID the identifier of a command to get the label for

        \return the configured label of the given command (could be empty) or
                \c QString() if the given \p commandID is empty or unknown

        \sa commandIDList()
    */
    QString labelForCommand(const QString& commandID) const;

    //! Returns the configured description for a given command
    /*! \param commandID the identifier of a command to get the description for

        \return the configured description of the given command (could be empty) or
                \c QString() if the given \p commandID is empty or unknown

        \sa commandIDList()
    */
    QString descriptionForCommand(const QString& commandID) const;

    //! Starts a given command
    /*! Creates a QProcess for the configured executable+arguments and starts it.

        \note when the started executable is finished, its exit code is emitted through
              signalCommandExited()

        \param commandID the identifier of the command to run

        \return \c true if starting succeeded, \c false if the identifier is empty,
                the identifier is unknown, the configured command list is empty
                or QProcess::start() failed

        \sa commandIDList()
    */
    bool runCommand(const QString& commandID);

signals:
    //! Emitted when a successfully started command executabled finishes
    void signalCommandExited(const QString& commandID, int exitStatus);

private:
    //! Private data class of AVSimpleCommandRunner
    class Data;

    //! Pointer to the internal data object
    Data* m_data;

private slots:
    //! Handles exit signals of started QProcess instances
    void slotCommandExited();

private:
    //! Hidden copy constructor
    AVSimpleCommandRunner(const AVSimpleCommandRunner&);

    //! Hidden assignment operator
    AVSimpleCommandRunner& operator=(const AVSimpleCommandRunner&);
};

#endif

// End of file
