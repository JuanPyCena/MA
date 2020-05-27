///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
// QT4-Equivalent: 
// QT4-Approach: redesign
// QT4-Progress: none
// QT4-Tested: no
// QT4-Problems: 
// QT4-Comment: CM: Depends on old messaging. Document what this is all about!
//
// Module:    AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Stefan Kunz, s.kunz@avibit.com
    \brief   Interface for handling client/server version informations
*/





// Qt includes
#include <qfileinfo.h>

// local includes
#include "avdatetime.h"
#include "avfilewatcher.h"
#include "avversioninfo.h"
#include "avlog.h"
#include "avpackageinfo.h"

///////////////////////////////////////////////////////////////////////////////

void AVVersionInfoFileHandler::slotReadFiles()
{
    AVASSERT(m_container != 0);

    // we do not use the simulated time here to allow on-line update of
    // the version files
    // additionally one second has to be subtracted because
    // QFileInfo::lastModified() only uses rounded second precision
    QDateTime now = AVDateTime::currentDateTimeUtc().addSecs(-1);

    readServerFile(AVMsgVersionInfo::ServerSoftware,
                   m_cfg->serverSoftwareVersionFile.expandPath());
    readServerFile(AVMsgVersionInfo::ServerConfig,
                   m_cfg->serverConfigVersionFile.expandPath());
    readClientFile();

    m_lastChecked = now;
}

///////////////////////////////////////////////////////////////////////////////

void AVVersionInfoFileHandler::readServerFile(
    AVMsgVersionInfo::VersionID versionId, const QString& file)
{
    if (file.isEmpty()) return;

    QFileInfo fileInfo(file);

    bool ok = false, modified = false;
    checkFile(fileInfo, ok, modified);

    if (! ok)
    {
        m_container->setServerVersionInfo(versionId, QString::null);
    }
    else
    {
        if (modified)
        {
            readAndSetServerVersionInfo(versionId, fileInfo);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVVersionInfoFileHandler::readClientFile()
{
    if (m_cfg->clientFile.expandPath().isEmpty()) return;

    QFileInfo fileInfo(m_cfg->clientFile.expandPath());

    bool ok = false, modified = false;
    checkFile(fileInfo, ok, modified);

    if (! ok)
    {
        m_container->clearClientVersionInfos();
    }
    else
    {
        if (modified)
        {
            readAndSetClientVersionInfo(fileInfo);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVVersionInfoFileHandler::checkFile(const QFileInfo& fileInfo,
                                         bool& ok, bool& modified)
{
    if (! fileInfo.exists())
    {
        ok       = false;
        modified = false;
    }
    else
    {
        ok = fileInfo.isReadable();

        if (! m_lastChecked.isValid()                 ||
            fileInfo.created()      >= m_lastChecked  ||
            fileInfo.lastModified() >= m_lastChecked)
            modified = true;
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVVersionInfoFileHandler::readAndSetServerVersionInfo(
    AVMsgVersionInfo::VersionID versionId, const QFileInfo& fileInfo)
{
    AVLogger->Write(LOG_DEBUG, "AVVersionInfoFileHandler::"
                    "readAndSetServerVersionInfo(%s)",
                    fileInfo.absFilePath().latin1());

    QFile file(fileInfo.absFilePath());
    if (! file.open(IO_ReadOnly))
    {
        m_container->setServerVersionInfo(versionId, QString::null);
        return;
    }

    QTextStream stream(&file);
    QString line;
    while (!stream.atEnd())
    {
        line = stream.readLine();
        if (line.isEmpty()) continue;
        // set first non-empty line
        m_container->setServerVersionInfo(versionId, line);
        file.close();
        return;
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVVersionInfoFileHandler::readAndSetClientVersionInfo(
    const QFileInfo& fileInfo)
{
    AVLogger->Write(LOG_DEBUG, "AVVersionInfoFileHandler::"
                    "readAndSetClientVersionInfo(%s)",
                    fileInfo.absFilePath().latin1());

    QFile file(fileInfo.absFilePath());
    if (! file.open(IO_ReadOnly)) return;

    // clear first to allow removal of deprecated clients
    m_container->clearClientVersionInfos();

    QTextStream stream(&file);
    QString line;
    while (!stream.atEnd())
    {
        line = stream.readLine();
        if (line.isEmpty()) continue;
        setClientInfo(line);
    }

    file.close();
}

///////////////////////////////////////////////////////////////////////////////

void AVVersionInfoFileHandler::setClientInfo(const QString& line)
{
    QStringList lineList = QStringList::split(m_cfg->clientFileDelimiter,
                                              line, true);

    bool valid = false;
    QString clientId;
    AVMsgVersionInfo versionInfo;

    if (lineList.count() < m_cfg->clientIdCol + 1)
    {
        AVLogger->Write(LOG_WARNING, "AVVersionInfoFileHandler::setClientInfo: "
                        "Can't read Client ID (Col %d) from line '%s'",
                        m_cfg->clientIdCol, line.latin1());
        return;
    }
    else
    {
        valid = true;
        clientId = lineList[m_cfg->clientIdCol];
    }

    if (lineList.count() < m_cfg->clientSoftwareVersionCol + 1)
    {
        AVLogger->Write(LOG_WARNING, "AVVersionInfoFileHandler::setClientInfo: "
                        "Can't read Client Software Version (Col %d) from "
                        "line '%s'",
                        m_cfg->clientSoftwareVersionCol, line.latin1());
        versionInfo.setVersion(AVMsgVersionInfo::ClientSoftware, QString::null);
    }
    else
    {
        valid = true;
        versionInfo.setVersion(AVMsgVersionInfo::ClientSoftware,
                               lineList[m_cfg->clientSoftwareVersionCol]);
    }

    if (lineList.count() < m_cfg->clientBaseConfigVersionCol + 1)
    {
        AVLogger->Write(LOG_WARNING, "AVVersionInfoFileHandler::setClientInfo: "
                        "Can't read Client Base Config Version (Col %d) from "
                        "line '%s'",
                        m_cfg->clientBaseConfigVersionCol, line.latin1());
        versionInfo.setVersion(AVMsgVersionInfo::ClientBaseConfig, QString::null);
    }
    else
    {
        valid = true;
        versionInfo.setVersion(AVMsgVersionInfo::ClientBaseConfig,
                               lineList[m_cfg->clientBaseConfigVersionCol]);
    }

    if (lineList.count() < m_cfg->clientAdminConfigVersionCol + 1)
    {
        AVLogger->Write(LOG_WARNING, "AVVersionInfoFileHandler::setClientInfo: "
                        "Can't read Client Admin Config Version (Col %d) from "
                        "line '%s'",
                        m_cfg->clientAdminConfigVersionCol, line.latin1());
        versionInfo.setVersion(AVMsgVersionInfo::ClientAdminConfig, QString::null);
    }
    else
    {
        valid = true;
        versionInfo.setVersion(AVMsgVersionInfo::ClientAdminConfig,
                               lineList[m_cfg->clientAdminConfigVersionCol]);
    }

    if (valid)
        m_container->setClientVersionInfo(clientId, versionInfo);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

AVVersionInfoClientFileHandler::AVVersionInfoClientFileHandler(const QString& baseConfigFile,
        const QString& adminConfigFile, const QString& clientId, uint checkIntervall)
    : AVVersionInfoHandler(),
      m_baseConfigFile(baseConfigFile),
      m_adminConfigFile(adminConfigFile),
      m_clientId(clientId),
      m_fileWatcher(0)
{
    AVASSERT(!m_clientId.isEmpty());

    m_fileWatcher = new AVFileWatcher(checkIntervall, true);
    AVASSERT(m_fileWatcher != 0);

    AVDIRECTCONNECT(m_fileWatcher, SIGNAL(signalFilesChanged(QStringList&)),
              this,          SLOT(slotFilesChanged(QStringList&)));
}

///////////////////////////////////////////////////////////////////////////////

AVVersionInfoClientFileHandler::~AVVersionInfoClientFileHandler()
{
    delete m_fileWatcher;
}

///////////////////////////////////////////////////////////////////////////////

void AVVersionInfoClientFileHandler::setContainer(AVVersionInfoContainer* container)
{
    AVVersionInfoHandler::setContainer(container);

    QStringList files;
    m_fileWatcher->getWatchedFiles(files);

    slotFilesChanged(files);
}

///////////////////////////////////////////////////////////////////////////////

void AVVersionInfoClientFileHandler::checkFiles()
{
    m_fileWatcher->slotCheckWatchedFiles();
}

///////////////////////////////////////////////////////////////////////////////

QString AVVersionInfoClientFileHandler::readFile(const QString& filename)
{
    QFile file(filename);
    if (!file.open(IO_ReadOnly)) return QString::null;

    QTextStream stream(&file);

    // return first non-empty line
    QString version;
    while (!stream.atEnd())
    {
        version = stream.readLine();
        if (!version.isEmpty()) return version;
    }

    return QString::null;
}

///////////////////////////////////////////////////////////////////////////////

void AVVersionInfoClientFileHandler::slotFilesChanged(QStringList& files)
{
    AVMsgVersionInfo msg = m_container->clientVersionInfo(m_clientId);

    msg.setVersion(AVMsgVersionInfo::ClientSoftware, AVPackageInfo::getVersion());

    QStringList::iterator it    = files.begin();
    QStringList::iterator endIt = files.end();
    for (; it != endIt; ++it)
    {
        QString filename = *it;
        QString version  = readFile(filename);

        if (filename == m_adminConfigFile)
            msg.setVersion(AVMsgVersionInfo::ClientAdminConfig, version);
        else if (filename == m_baseConfigFile)
            msg.setVersion(AVMsgVersionInfo::ClientBaseConfig, version);
        else
            AVLogger->Write(LOG_ERROR, "AVVersionInfoClientFileHandler::slotFilesChanged: "
                            "got change notification for unknown file '%s'",
                            filename.latin1());
    }

    m_container->setClientVersionInfo(m_clientId, msg);
}

// End of file
