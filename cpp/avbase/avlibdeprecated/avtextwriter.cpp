///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// QT-Version: QT5
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Stefan Kunz, s.kunz@avibit.com
    \author  QT5-PORT: Thomas Schiffer, t.schiffer@avibit.com
    \brief   Writes text to an ASCII file.
*/

#define AVTEXTWRITER_CPP

// qt includes
#include <qfileinfo.h>
#include <qdir.h>

// avcommon library
#include "avlog.h"
#include "avmisc.h"

// local includes
#include "avtextwriter.h"


AVTextWriter::AVTextWriter(const QString& dir_name,
                           const QString& file_name,
                           const QString& file_ext,
                           bool append,
                           bool add_date,
                           bool add_time,
                           uint close_interval,
                           uint keep_files,
                           bool thread_safe)
:    m_pMutex(0),
     m_File(),
     m_DirName(dir_name),
     m_FileName(file_name),
     m_FileExt(file_ext),
     m_TextStream(),
     m_bAppend(append),
     m_bAddDate(add_date),
     m_bAddTime(add_time),
     m_CloseInterval(close_interval),
     m_KeepFiles(keep_files)
{
    if ( dir_name.isEmpty() )
        LOGGER_ROOT.Write(LOG_ERROR, "AVTextWriter: Can't handle empty directory name.");
    if ( file_name.isEmpty() )
        LOGGER_ROOT.Write(LOG_ERROR, "AVTextWriter: Can't handle empty file name.");

    QDir dir(dir_name);
    if (! dir.exists())
    {
        if (! dir.mkdir(dir.absolutePath()))
        {
            LOGGER_ROOT.Write(LOG_ERROR, "Unable to create directory %s.",
                              qPrintable(dir.absolutePath()));
        }
    }
    m_DirName = dir.absolutePath();

    if (! add_time && close_interval)
        m_bAddTime = true;

    if (add_time && ! add_date)
        m_bAddDate = true; // time only is currently not supported

    // close interval and keep-files only make sense with dynamic filenames
    if (! (add_date || add_time))
    {
        m_CloseInterval = 0;
        m_KeepFiles = 0;
    }

    if (thread_safe)
    {
        m_pMutex = new QMutex(QMutex::NonRecursive);
        AVASSERT(m_pMutex != 0);
        if ( m_pMutex == 0 )
            LOGGER_ROOT.Write(LOG_ERROR, "AVTextWriter: Out of Memory.");
    }

    openFile(AVDateTime::currentDateTimeUtc());
}

//////////////////////////////////////////////////////////////////////////////

AVTextWriter::~AVTextWriter()
{
    m_File.flush();
    delete m_pMutex; m_pMutex = 0;
    closeFile();
    if (m_KeepFiles > 0) deleteOldFiles();
}

//////////////////////////////////////////////////////////////////////////////

bool AVTextWriter::writeLine(const QString& line)
{
    if (m_pMutex)
        QMutexLocker locker(m_pMutex);

    if (! checkForFile(AVDateTime::currentDateTimeUtc()))
        return false;

    m_TextStream << line << "\n";
    m_File.flush();

    return true;
}

//////////////////////////////////////////////////////////////////////////////

bool AVTextWriter::openFile(const QDateTime &dt)
{
    QString fn = createFileName(dt, false);
    bool success = openFile(fn);
    m_FileNameDt = dt;
    return success;
}

//////////////////////////////////////////////////////////////////////////////

QString AVTextWriter::absoluteFilePath()
{
    if ( m_File.isOpen() )
    {
        QFileInfo fi(m_File);
        return fi.absoluteFilePath();
    }
    else
        return QString();
}

//////////////////////////////////////////////////////////////////////////////

bool AVTextWriter::openFile(const QString &fn)
{
    // close the old file if it is open
    if (m_File.isOpen()) closeFile();

    // construct absolute path
    QFileInfo fi(fn);

    // open the new file
    m_File.setFileName(fi.absoluteFilePath());

    if (! m_File.open(QIODevice::WriteOnly | QIODevice::Text |
                      (m_bAppend ? QIODevice::Append : QIODevice::Truncate)))
    {
        AVLogger->Write(LOG_ERROR,
                        "Cannot  create file %s", qPrintable(fn));

        return false;
    }

    // associate the datastream with this file
    m_TextStream.setDevice(&m_File);
    return true;
}

//////////////////////////////////////////////////////////////////////////////

void AVTextWriter::closeFile()
{
    // just return if the file is not open
    if (! m_File.isOpen()) return;

    // close the file
    m_File.close();

    // rename to contain the closing time
    if (m_bAddTime)
    {
        AVDateTime dt;
        if (m_LastDt.isNull()) dt = AVDateTime::currentDateTimeUtc();
        else                   dt = m_LastDt;
        QString fn = createFileName(dt, true);
        QDir dir;
        if (dir.rename(m_File.fileName(), fn))
            AVLogger->Write(LOG_INFO, "file closed as %s", qPrintable(fn));
        else
            AVLogger->Write(LOG_ERROR, "can't rename %s to %s",
                            qPrintable(m_File.fileName()), qPrintable(fn));
    }
}

//////////////////////////////////////////////////////////////////////////////

QString AVTextWriter::createFileName(const QDateTime& dt, bool for_close)
{
    QString start_date_time, file_name;
    AVsprintf(start_date_time, "%04d%02d%02d", dt.date().year(),
              dt.date().month(), dt.date().day());
    if (m_bAddTime)
    {
        if (for_close && !m_FileNameDt.isNull())
        {
            QTime t0 = m_FileNameDt.time();
            QTime t1 = dt.time();
            QString ti0, ti1;
            // starting date
            AVsprintf(start_date_time, "%04d%02d%02d",
                      m_FileNameDt.date().year(),
                      m_FileNameDt.date().month(),
                      m_FileNameDt.date().day());
            // starting time
            AVsprintf(ti0, "%02d%02d%02d", t0.hour(),t0.minute(),t0.second());
            // closing time
            AVsprintf(ti1, "%02d%02d%02d", t1.hour(),t1.minute(),t1.second());
            start_date_time = start_date_time + "_" + ti0 + "_" + ti1;
        }
        else
        {
            QTime t = dt.time();
            QString ti;
            AVsprintf(ti, "%02d%02d%02d", t.hour(), t.minute(), t.second());
            start_date_time = start_date_time + "_" + ti + "_______";
        }
    }

    file_name = m_DirName + "/" + m_FileName;
    if (m_bAddDate) file_name = file_name + "_" + start_date_time;
    file_name += m_FileExt;

    return file_name;
}

//////////////////////////////////////////////////////////////////////////////

bool AVTextWriter::checkForFile(const QDateTime &dt)
{
    // check if we need to open a new file here.
    if (m_File.isOpen())
    {
        // consider only if increasing times
        if (! m_FileNameDt.isValid() || m_FileNameDt > dt )
            return true;

        // on a new day -> open new archive file in any case
        int day  = dt.date().day();            // day in new msg
        int cday = m_FileNameDt.date().day();  // current day
        if (day == cday)
        {
            // if no close interval is specified -> no new archive file
            // on the same day is opened
            if (m_CloseInterval == 0) return true;

            // here a close interval is specified -> check if it is exceeded
            // If no, return, if yes, open a new archive file
            if (m_FileNameDt.secsTo(dt) < (int)m_CloseInterval * 60)
                return true;
        }
    }

    // open the file
    if (! openFile(dt))
        return false;

    if (m_KeepFiles > 0) deleteOldFiles();

    return true;
}

//////////////////////////////////////////////////////////////////////////////

void AVTextWriter::deleteOldFiles()
{
    QDir dir(m_DirName, m_FileName + QString("*") + m_FileExt,
             QDir::Name | QDir::Reversed, QDir::Files);
    QStringList fl = dir.entryList();
    for (int i = 0; i < fl.count(); ++i)
    {
        if (i >= m_KeepFiles)
        {
            QString f = dir.absoluteFilePath(fl[i]);
            AVLogger->Write(LOG_INFO, "deleting file %s", qPrintable(f));
            dir.remove(f);
        }
    }
}

//#endif

// End of file
