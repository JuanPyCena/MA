///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// QT-Version: QT5
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    AVLIB - Avibit Library
// File:      avrawsaver.cpp
// Desc:      Class library for raw data saving
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Dr. Konrad Koeck, k.koeck@avibit.com
    \author  QT4-PORT: Christian Muschick, c.muschick@avibit.com
    \author  Thomas Neidhart, t.neidhart@avibit.com
    \brief   Raw data archiving class.
*/


///////////////////////////////////////////////////////////////////////////////

#include <qdir.h>
#include <qstringlist.h>

#include "avconfig.h"
#include "avlog.h"
#include "avmisc.h"
#include "avrawsaver.h"
#include "avdatetime.h"

///////////////////////////////////////////////////////////////////////////////

bool AVRawSaver::putch(char data)
{
    return writeBlock(&data, 1);
}

///////////////////////////////////////////////////////////////////////////////

bool AVRawSaver::writeBlock(const char *data, int len)
{
    return writeBlock(data, len, AVDateTime::currentDateTimeUtc());
}

///////////////////////////////////////////////////////////////////////////////
bool AVRawSaver::writeBlock(const char *data, int len, const QDateTime &dt)
{
    if (data == 0 || len <= 0)
        return false;

    // open file if required

    if (f == NULL) {
        QString myFname = createFileName();
        AVLogger->Write (LOG_INFO, "Now opening file '%s'", qPrintable(myFname));
        f = new QFile (myFname);
        AVASSERT(f != 0);
        size = 0;
        if (!f->open (QIODevice::WriteOnly)) {
            AVLogger->Write (LOG_ERROR, "Error opening output file '%s'", qPrintable(myFname));
            delete f;
            f = NULL;
            return false;
        }

        ds = new QDataStream(f);
        ds->setVersion(QDataStream::Qt_3_3);
        AVASSERT(ds != 0);

        // remove the old files

        removeOldFiles();
    }

    // Save data


    if (m_bSaveTimeStamp)
    {
        *ds << dt;
        size += sizeof(QDateTime);
    }

    ds->writeRawData(data, len);
    size += len;

    // if maximum size of a file reached -> close it

    if (size >= m_max_filesize)
    {
        delete ds;
        f->close();
        delete f;
        f = NULL;
        size = 0;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

QString AVRawSaver::createFileName()
{
    QDateTime cur = AVDateTime::currentDateTimeUtc();

    QString dts;
    AVsprintf(dts, "%04d%02d%02d_%02d%02d", cur.date().year(),
                                            cur.date().month(),
                                            cur.date().day(),
                                            cur.time().hour(),
                                            cur.time().minute());

    // Get the path of the log directory

    QString ldir;
    QString app_home = AVEnvironment::getApplicationHome();

    if (!m_file_path.isEmpty())
    {
        ldir = m_file_path;
    }
    else if (!app_home.isNull())
    {
        ldir = app_home + "/log";
        QDir dir(ldir);
        if (!dir.exists()) dir.mkdir(ldir);
        ldir = dir.absolutePath();
    }
    else
    {
        QDir dir;
        ldir = dir.absolutePath();
    }

    // Create the filename

    QString ret = ldir + "/" + file_prefix + "_" + dts + ".raw";

    AVLogger->Write(LOG_INFO, "AVRawSaver:createFileName: "
                    "creating file (%s)", qPrintable(ret));
    return ret;
}

///////////////////////////////////////////////////////////////////////////////

void AVRawSaver::removeOldFiles()
{
    int keepfiles = 20;                // Number of files to be kept

    // Get the path of the log directory

    QString ldir;
    QString home = AVEnvironment::getApplicationHome();
    if (!home.isNull()) {
        ldir = home + "/log";
        QDir dir(ldir);
        if (!dir.exists()) dir.mkdir(ldir);
        ldir = dir.absolutePath();
    } else {
        QDir dir;
        ldir = dir.absolutePath();
    }

    QDir dir(ldir, QString(file_prefix + "_*.raw"),
             QDir::Name | QDir::Reversed, QDir::Files);
    QStringList fl = dir.entryList();
    for (int i = 0; i < fl.count(); i++) {
        if (i >= keepfiles) {
            QString afp = dir.absoluteFilePath(fl[i]);
            AVLogger->Write(LOG_INFO, "deleting file %s", qPrintable(afp));
            dir.remove(afp);
        }
    }
}

// end of file

