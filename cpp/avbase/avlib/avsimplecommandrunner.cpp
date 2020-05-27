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
  \brief   Implementation of a class for running configurable commands/executables
*/


// Qt includes
#include <qprocess.h>
#include <QHash>

// local includes
#include "avconfig2.h"
#include "avlog.h"
#include "avsimplecommandrunner.h"


///////////////////////////////////////////////////////////////////////////////

class AVSimpleCommandRunnerConfig : public AVConfig2
{
public:
    AVSimpleCommandRunnerConfig(const QString& prefix, AVConfig2Container& config)
            : AVConfig2(prefix, config)
    {
        registerParameter("label", &m_label,
                          "Label of the command, e.g. in dialogs").
            setSuggestedValue(QString());
        registerParameter("description", &m_description,
                          "Description of the command, e.g. in dialogs").
            setSuggestedValue(QString());
        registerParameter("command_list", &m_command,
                          "List executable followed by arguments").
            setSuggestedValue(m_command).
            setDeprecatedName("command-list");
        registerParameter("environment", &m_environment,
                          "List of environment variables").
            setSuggestedValue(m_environment);
    }

public:
    QString m_label;
    QString m_description;
    QStringList m_command;
    QStringList m_environment;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class AVSimpleCommandRunner::Data : public AVConfig2
{
public:
    Data(const QString& filename) : AVConfig2("")
    {
        setHelpGroup("AVSimpleCommandRunner");
        registerSubconfig(m_prefix + filename, &m_config);
        AVConfig2Global::singleton().loadConfig(filename);
    }

    AVSimpleCommandRunnerConfig* findSimpleCommandConfig(const QString& key) {
        AVConfig2Map<AVSimpleCommandRunnerConfig>::iterator it = m_config.find(key);
        if (it == m_config.end()) return 0;
        return it.value().data();
    }

    QHash<void*, QProcess *> m_runningProcesses;

    AVConfig2Map<AVSimpleCommandRunnerConfig> m_config;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

AVSimpleCommandRunner::AVSimpleCommandRunner(QObject* parent, const char* name)
    : QObject(parent),
      m_data(0)
{
    setObjectName(name);
}

///////////////////////////////////////////////////////////////////////////////

AVSimpleCommandRunner::~AVSimpleCommandRunner()
{
    delete m_data;
}

///////////////////////////////////////////////////////////////////////////////

bool AVSimpleCommandRunner::readCommandConfig(const QString& filename)
{
    if (filename.isEmpty()) return false;

    m_data = new Data(filename);
    AVASSERT(m_data != 0);
    AVConfig2Container::RefreshResult result = AVConfig2Global::singleton().refreshAllParams();
    if (!result.refreshSuccessful()) {
        AVLogError << "AVSimpleCommandRunner::readCommandConfig\n" << result.getErrorMessage();
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

QStringList AVSimpleCommandRunner::commandIDList() const
{
    return m_data->m_config.keys();
}

///////////////////////////////////////////////////////////////////////////////

QString AVSimpleCommandRunner::labelForCommand(const QString& commandID) const
{
    if (commandID.isEmpty()) return QString();

    AVSimpleCommandRunnerConfig* config = m_data->findSimpleCommandConfig(commandID);;
    if (config == 0) return QString();

    return config->m_label;
}

///////////////////////////////////////////////////////////////////////////////

QString AVSimpleCommandRunner::descriptionForCommand(const QString& commandID) const
{
    if (commandID.isEmpty()) return QString();

    AVSimpleCommandRunnerConfig* config = m_data->findSimpleCommandConfig(commandID);
    if (config == 0) return QString();

    return config->m_description;
}

///////////////////////////////////////////////////////////////////////////////

bool AVSimpleCommandRunner::runCommand(const QString& commandID)
{
    if (commandID.isEmpty()) return false;

    AVSimpleCommandRunnerConfig* config = m_data->findSimpleCommandConfig(commandID);
    if (config == 0) return false;

    if (config->m_command.isEmpty()) return false;

    QProcess* process = new QProcess(this);
    process->setObjectName(commandID);
    AVASSERT(process != 0);

    // no communication with child process

    process->closeReadChannel(QProcess::StandardOutput);
    process->closeReadChannel(QProcess::StandardError);
    process->closeWriteChannel();
    process->setEnvironment(config->m_environment);
    QStringList arguments = config->m_command;
    if (arguments.count() > 1) {
        arguments.removeFirst();
    }
    process->start(config->m_command.at(0), arguments);
    if (process->waitForStarted())
    {
        AVDIRECTCONNECT(process, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(slotCommandExited()));

        m_data->m_runningProcesses.insert(process, process);

        return true;
    }

    delete process;

    return false;
}

///////////////////////////////////////////////////////////////////////////////

void AVSimpleCommandRunner::slotCommandExited()
{
    //parasoft suppress item pbugs-06
    QProcess* process = m_data->m_runningProcesses.value(static_cast<void*>(QObject::sender()));
    if (process == 0) return;
    //parasoft on
    emit signalCommandExited(QString(process->objectName()), process->exitStatus());
    process->deleteLater();
}

// End of File
