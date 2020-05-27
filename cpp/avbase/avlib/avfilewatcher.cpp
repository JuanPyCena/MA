///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2010
//
// Module:     AVLIB - AviBit Library
//
/////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  DI Alexander Wemmer, a.wemmer@avibit.com
    \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   Checks a list of files if they have been modified
*/



#include <QDirIterator>

#include "avlog.h"

#include "avfilewatcher.h"

/////////////////////////////////////////////////////////////////////////////

AVFileWatcher::AVFileWatcher(uint check_intervall_msec, bool usetimer, bool start_timer)
    : m_usetimer(usetimer),
      m_check_intervall_msec(check_intervall_msec),
      m_check_closed_interval_sec(0),
      m_recursive(false)
{
    if (m_usetimer) {
        AVASSERT(connect(&m_check_timer, SIGNAL(timeout()), this, SLOT(slotCheckWatchedFiles())));
        if (start_timer) m_check_timer.start(m_check_intervall_msec);
    }

    //m_file_modify_map.setAutoDelete(true);
    m_dir.setFilter(QDir::Files);
}

/////////////////////////////////////////////////////////////////////////////

AVFileWatcher::~AVFileWatcher()
{
    m_check_timer.stop();
    m_file_modify_map.clear();
}

/////////////////////////////////////////////////////////////////////////////

void AVFileWatcher::start()
{
    if (!m_usetimer) return;
    stop();
    slotCheckWatchedFiles();
    m_check_timer.start(m_check_intervall_msec);
}

/////////////////////////////////////////////////////////////////////////////

void AVFileWatcher::stop()
{
    m_check_timer.stop();
}

/////////////////////////////////////////////////////////////////////////////

bool AVFileWatcher::addFileToWatch(const QString& filename_)
{
    AVLogInfo << "AVFileWatcher::addFileToWatch: " << filename_;
    QString filename = filename_;

    AVASSERT(!filename.isEmpty());

    m_fileinfo.setFile(filename);
    m_fileinfo.refresh();

    if (m_fileinfo.isRelative())
    {
        filename = QDir::current().path() + "/" + filename;
        m_fileinfo.setFile(filename);
        m_fileinfo.refresh();
    }

    QMap<QString, LastModifiedClosed>::iterator iter = m_file_modify_map.find(filename);
    if (iter != m_file_modify_map.end())
    {
        AVLogger->Write(LOG_ERROR, "AVFileWatcher:addFileToWatch: "
                        "File (%s) already watched", qPrintable(filename));
        return false;
    }

    if (m_fileinfo.exists() && !m_fileinfo.isFile())
    {
        AVLogger->Write(LOG_ERROR, "AVFileWatcher:addFileToWatch: "
                        "Given (%s) is not a file", qPrintable(filename));
        return false;
    }

    LastModifiedClosed modified_closed;
    modified_closed.m_last_modified = AVDateTime();
    modified_closed.m_closed = false;
    m_file_modify_map.insert(filename, modified_closed);

    AVLogger->Write(LOG_INFO, "AVFileWatcher:addFileToWatch: "
                    "Added file (%s)", qPrintable(filename));

    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool AVFileWatcher::addFilesToWatch(const QStringList& filelist)
{
    bool ret_val = true;

    QStringList::const_iterator iter = filelist.begin();
    for(; iter != filelist.end(); ++iter)
    {
        if (!addFileToWatch(*iter)) ret_val = false;
    }

    return ret_val;
}

/////////////////////////////////////////////////////////////////////////////

bool AVFileWatcher::addDirToWatch(const QString& dirname_, const QStringList& file_filter_list)
{
    AVLogDebug << "AVFileWatcher::addDirToWatch";

    QString dirname = dirname_;

    AVASSERT(!dirname.isEmpty());

    m_fileinfo.setFile(dirname);
    m_fileinfo.refresh();

    if (m_fileinfo.isRelative())
    {
        dirname = QDir::current().path() + "/" + dirname;
        m_fileinfo.setFile(dirname);
        m_fileinfo.refresh();
    }

    if (m_dir_dict.contains(dirname))
    {
        AVLogger->Write(LOG_ERROR, "AVFileWatcher:addDirToWatch: "
                        "Dir (%s) already watched", qPrintable(dirname));
        return false;
    }

    if (m_fileinfo.exists() && !m_fileinfo.isDir())
    {
        AVLogger->Write(LOG_ERROR, "AVFileWatcher:addDirToWatch: "
                        "Given (%s) is not a directory", qPrintable(dirname));
        return false;
    }

    if (!dirname.endsWith("/"))
    {
        m_dir_dict.insert(dirname+"/", file_filter_list);
    }
    else
    {
        m_dir_dict.insert(dirname, file_filter_list);
    }

    AVLogger->Write(LOG_INFO, "AVFileWatcher:addDirToWatch: "
                    "Added directory (%s)", qPrintable(dirname));

    return true;
}

/////////////////////////////////////////////////////////////////////////////

void AVFileWatcher::slotCheckWatchedFiles()
{
    if (m_recursive)
        checkForSubfolders();

    QStringList added_filelist;
    QStringList modified_filelist;
    QStringList removed_filelist;
    QStringList modified_closed_list;
    if (checkWatchedFiles(added_filelist, modified_filelist,
                          removed_filelist, modified_closed_list))
    {
        QStringList combined_filelist = added_filelist + modified_filelist + removed_filelist;

        if (combined_filelist.size() > 0)
            emit signalFilesChanged(combined_filelist);
        if (added_filelist.size() > 0 || modified_filelist.size() > 0 ||
                removed_filelist.size() > 0)
            emit signalFilesChanged(added_filelist, modified_filelist, removed_filelist);
        if (modified_closed_list.size() > 0)
            emit signalFilesChangedClosed(modified_closed_list);
    }
}

/////////////////////////////////////////////////////////////////////////////

void AVFileWatcher::checkForSubfolders()
{
    AVFileWatcherDirDict::ConstIterator dir_iter = m_dir_dict.begin();
    for(; dir_iter != m_dir_dict.end(); ++dir_iter)
    {
        const QString& dirname = dir_iter.key();
        const QStringList& file_filter_list = dir_iter.value();

        QDirIterator subdir_iter(dirname, QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);

        // and now add all subdirs to watch
        while (subdir_iter.hasNext())
        {
            QFileInfo info(subdir_iter.next());

            if (info.isDir() && !info.isHidden())
            {
                QString sub_dir = info.absoluteFilePath();
                if (!sub_dir.endsWith("/"))
                    sub_dir.append("/");

                if (!m_dir_dict.contains(sub_dir))
                {
                    AVLogInfo << "AVFileWatcher::checkForSubfolders: found new subdir to watch: " << sub_dir;
                    addDirToWatch(sub_dir, file_filter_list);
                }
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

bool AVFileWatcher::checkWatchedFiles(QStringList& added_filelist, QStringList& modified_filelist,
                                      QStringList& removed_filelist)
{
    AVLogDebug1 << "AVFileWatcher::checkWatchedFiles for added/modified/removed";

    added_filelist.clear();
    modified_filelist.clear();
    removed_filelist.clear();

    checkDirectories(removed_filelist);

    uint change_total = removed_filelist.count();

    // loop through the list of files to watch

    QMap<QString, LastModifiedClosed>::iterator iter = m_file_modify_map.begin();
    for(; iter != m_file_modify_map.end();)
    {
        const QString& filename = iter.key();
        //QDateTime* modify_dt = iter.value();
        LastModifiedClosed modified_closed = iter.value();

        bool changed = false;

        m_fileinfo.setFile(filename);
        m_fileinfo.refresh();

        if (!m_fileinfo.exists())
        {
            if (modified_closed.m_last_modified.isValid())
            {
                // file has been removed
                AVLogger->Write(LOG_DEBUG, "AVFileWatcher:checkWatchedFiles: "
                                "File %s has been deleted", qPrintable(filename));

                modified_closed.m_last_modified = AVDateTime();
                changed = true;
                removed_filelist.append(filename);
                ++change_total;
            }
        }
        else if (!modified_closed.m_last_modified.isValid())
        {
            // file has (re)appeared
            modified_closed.m_last_modified = m_fileinfo.lastModified().toUTC();
            changed = true;
            added_filelist.append(filename);
            ++change_total;
        }
        else if (modified_closed.m_last_modified != m_fileinfo.lastModified().toUTC())
        {
            // file has been modified
            modified_closed.m_last_modified = m_fileinfo.lastModified().toUTC();
            changed = true;
            modified_filelist.append(filename);
            ++change_total;
        }

        if (changed)
        {
            AVLogDebug << "AVFileWatcher::checkWatchedFiles (3): " << filename << ", closed: " <<
                    modified_closed.m_closed << ", last mod dt: " <<
                    modified_closed.m_last_modified.toString("hh:mm:ss.zzz");

            m_file_modify_map.insert(filename, modified_closed);
        }

        ++iter;
    }

    if (change_total > 0) {
        AVLogger->Write(LOG_DEBUG, "AVFileWatcher:checkWatchedFiles: "
                        "%d files have changed (%d added, %d modified, %d removed)",
                        change_total, added_filelist.count(),
                        modified_filelist.count(), removed_filelist.count());
        return true;
    }

    return false;
}

/////////////////////////////////////////////////////////////////////////////

bool AVFileWatcher::checkWatchedFiles(QStringList& added_filelist, QStringList& modified_filelist,
                                      QStringList& removed_filelist,
                                      QStringList& modified_closed_list)
{

    AVLogDebug2 << "AVFileWatcher::checkWatchedFiles for added/modified/removed & closed";

    added_filelist.clear();
    modified_filelist.clear();
    removed_filelist.clear();
    modified_closed_list.clear();

    checkDirectories(removed_filelist);

    uint change_total = removed_filelist.count();

    // loop through the list of files to watch

    QMap<QString, LastModifiedClosed>::iterator iter = m_file_modify_map.begin();
    while (iter != m_file_modify_map.end())
    {
        const QString& filename = iter.key();
        //QDateTime* modify_dt = iter.value();
        LastModifiedClosed modified_closed = iter.value();

        bool changed = false;
        bool removed = false;

        m_fileinfo.setFile(filename);
        m_fileinfo.refresh();

        if (!m_fileinfo.exists())
        {
            if (modified_closed.m_last_modified.isValid())
            {
                // file has been removed
                AVLogger->Write(LOG_DEBUG, "AVFileWatcher:slotCheckWatchedFiles: "
                                "File %s has been deleted", qPrintable(filename));

                modified_closed.m_last_modified = AVDateTime();
                modified_closed.m_closed = false;
                changed = true;
                removed_filelist.append(filename);
                ++change_total;

                removed = true;
                //m_file_modify_map.remove(filename);
            }
        }
        else if (!modified_closed.m_last_modified.isValid())
        {
            // file has (re)appeared
            modified_closed.m_last_modified = m_fileinfo.lastModified().toUTC();
            changed = true;
            added_filelist.append(filename);
            ++change_total;
        }
        else if (modified_closed.m_last_modified != m_fileinfo.lastModified().toUTC())
        {
            // file has been modified
            modified_closed.m_last_modified = m_fileinfo.lastModified().toUTC();
            changed = true;
            modified_filelist.append(filename);
            ++change_total;
        }

        if (!removed && !modified_closed.m_closed && modified_closed.m_last_modified.isValid())
        {
            if (modified_closed.m_last_modified.secsTo(QDateTime::currentDateTimeUtc()) >
                    m_check_closed_interval_sec)
            {
                AVLogDebug << "AVFileWatcher::checkWatchedFiles: " << filename <<
                        ", secs since file was closed: "
                        << modified_closed.m_last_modified.secsTo(QDateTime::currentDateTimeUtc());

                modified_closed.m_closed = true;
                changed = true;
                modified_closed_list.append(filename);
                ++change_total;
            }
        }

        if (changed)
        {
            AVLogDebug << "AVFileWatcher::checkWatchedFiles (4): " << filename << ", closed: " <<
                    modified_closed.m_closed << ", last mod dt: " <<
                    modified_closed.m_last_modified.toString("hh:mm:ss.zzz");

            m_file_modify_map.insert(filename, modified_closed);
        }


        ++iter;
    }

    if (change_total > 0) {
        AVLogger->Write(LOG_DEBUG, "AVFileWatcher:checkWatchedFiles: "
                        "%d files have changed (%d added, %d modified, %d removed, "
                        "%d modified/closed)",
                        change_total, added_filelist.count(),
                        modified_filelist.count(), removed_filelist.count(),
                        modified_closed_list.size());
        return true;
    }

    return false;
}

/////////////////////////////////////////////////////////////////////////////

void AVFileWatcher::checkDirectories(QStringList& removed_filelist)
{
    // loop through the list of directories to watch

    AVFileWatcherDirDict::ConstIterator dir_iter = m_dir_dict.begin();
    for(; dir_iter != m_dir_dict.end(); ++dir_iter)
    {
        const QString& dirname = dir_iter.key();
        const QStringList& file_filter_list = dir_iter.value();

        m_fileinfo.setFile(dirname);
        m_fileinfo.refresh();

        if (!m_fileinfo.exists())
        {
            // search for files which were located in the deleted directory

            QMap<QString, LastModifiedClosed>::iterator dict_iter = m_file_modify_map.begin();
            while(dict_iter != m_file_modify_map.end())
            {
                const QString& filename = dict_iter.key();

                if (filename.startsWith(dirname))
                {
                    removed_filelist.append(filename);
                    AVLogger->Write(LOG_DEBUG, "AVFileWatcher:checkDirectories: "
                                    "Removing (%s) from list", qPrintable(filename));

                    dict_iter = m_file_modify_map.erase(dict_iter);
                    continue;
                }

                ++dict_iter;
            }
        }
        else
        {
            if (!m_fileinfo.isDir())
            {
                AVLogger->Write(LOG_ERROR, "AVFileWatcher:checkDirectories: "
                                "(%s) is not a directory", qPrintable(dirname));
                continue;
            }

            m_dir.setPath(dirname);
            m_dir.refresh();

            QStringList dir_filelist;

            if (file_filter_list.count() == 0)
            {
                dir_filelist = m_dir.entryList();
            }
            else
            {
                dir_filelist = m_dir.entryList(file_filter_list);
            }

            // loop through the files of the directory

            QStringList::const_iterator dirlist_iter = dir_filelist.begin();
            for(; dirlist_iter != dir_filelist.end(); ++dirlist_iter)
            {
                QString filename = dirname;
                if (!filename.endsWith("/"))
                    filename.append("/");
                filename.append(*dirlist_iter);

                // look if we already watch the file
                QMap<QString, LastModifiedClosed>::iterator iter = m_file_modify_map.find(filename);
                if (iter == m_file_modify_map.end())
                    addFileToWatch(filename);
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

uint AVFileWatcher::getWatchedFiles(QStringList& filename_list)
{
    QMap<QString, LastModifiedClosed>::iterator iter=m_file_modify_map.begin();
    for(; iter!=m_file_modify_map.end(); ++iter)
    {
        const QString& filename = iter.key();
        filename_list.append(filename);
    }

    return m_file_modify_map.count();
}

// End of file
