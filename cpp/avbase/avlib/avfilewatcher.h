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
    \author  DI Alexander Wemmer, a.wemmer@avibit.com
    \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   Checks a list of files if they have been modified
*/

#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QTimer>
#include <QFileInfo>
#include <QDir>
#include <QMap>

#ifndef __AVFILEWATCHER_H__
#define __AVFILEWATCHER_H__

#include "avdatetime.h"

#include "avlib_export.h"

typedef QMap<QString, QStringList> AVFileWatcherDirDict;

//! Checks a list of files if they have been modified
class AVLIB_EXPORT AVFileWatcher : public QObject
{
    Q_OBJECT

public:

    //! Standard Constructor
    /*! \param usetimer if true the internal timer is used and files/directories are checked
                        every check_intervall_msec seconds. If false no timer is used and
                        slotCheckWatchedFiles() or checkWatchedFiles() has to be called from
                        outside.
        \param start_timer if true (and usetimer is true) the check timer will be started
     */
    explicit AVFileWatcher(uint check_intervall_msec, bool usetimer = true,
                           bool start_timer = true);

    //! Destructor
    ~AVFileWatcher() override;

    //! starts the check timer
    void start();

    // stops the check timer
    void stop();

    // sets the check intervall
    void setInterval(uint intervall) { m_check_intervall_msec = intervall; }

    //! Adds the given file to the checker. \sa addFilesToWatch()
    //! Returns true on success, false otherwise.
    /*! If the filename to watch is relative, the current working dir is
        added. Each file may only be added once to be watched.
        If the file is removed signalFilesChanged is emitted.
     */
    bool addFileToWatch(const QString& filename);

    //! Same as addFileToWatch() but with a list of files
    bool addFilesToWatch(const QStringList& filelist);

    //! Adds the given directory to the checker.
    //! Returns true on success, false otherwise.
    /*! All files (or if "file_filter_list" is set only the files matching the
        filters) currently located in the directory will be
        added. Additionally on each check cycle new files will be added and
        deleted files will be removed from the file check list.
     */
    bool addDirToWatch(const QString& dirname, const QStringList& file_filter_list = QStringList());

    //! Adds all watched filenames (the given list will not be cleared!) to
    //! the given list. Returns the number of files added.
    uint getWatchedFiles(QStringList& filename_list);

    //! Check if one or more files have changed in any way
    /*! Convenience overload for checkWatchedFiles(QStringList&, QStringList&, QStringList&)
        which combined the three kind of changed into one list.

        \note The list is cleared before adding to them.

        \param changed_filelist files which got changed (added/modified/removed) since last check

        \return \c true if at least one file has changed
     */
    bool checkWatchedFiles(QStringList& changed_filelist)
    {
        QStringList added_filelist;
        QStringList removed_filelist;
        QStringList modified_closed_list;

        bool result = checkWatchedFiles(added_filelist, changed_filelist, removed_filelist,
                                        modified_closed_list);

        changed_filelist += added_filelist + removed_filelist;
        return result;
    }

    void setCheckForCloseInterval(int interval)
    { m_check_closed_interval_sec = interval; }

    void setRecursiveFolderCheck(bool recursive)
    { m_recursive = recursive; }

    //! Check if one or more files have been added, modified or removed
    /*! \note All lists are cleared before adding to them.

        \param added_filelist files which got added since last check
        \param modified_filelist files which got modified since last check
        \param removed_filelist files which got removed since last check

        \return \c true if at least one file has changed
     */
    bool checkWatchedFiles(QStringList& added_filelist, QStringList& modified_filelist,
                           QStringList& removed_filelist);

    bool checkWatchedFiles(QStringList& added_filelist, QStringList& modified_filelist,
                           QStringList& removed_filelist, QStringList& modified_closed_list);

signals:

    //! Emitted if one or more watched files changed
    void signalFilesChanged(const QStringList& changed_file_list);

    //! Emitted if one or more watched files are changed
    /*! Since there are three kind of changes (newly created file, file modified,
        file deleted), the simple signal signalFilesChanged(QStringList&) might not
        be sufficient for the application.

        This variant has one list for each type of change.

        \param added_file_list list of files which are new, i.e. have not been seen before
        \param modified_file_list list of files which have potentially be modifed, i.e.
                                  have been seen before but their modification timestamp
                                  differs
        \param removed_file_list list of files which are no longer present
    */
    void signalFilesChanged(const QStringList& added_file_list,
                            const QStringList& modified_file_list,
                            const QStringList& removed_file_list);

    void signalFilesChangedClosed(const QStringList& modified_closed_list);

public slots:

   //! Called when we should check the watched files for changes
   void slotCheckWatchedFiles();

private:

    struct LastModifiedClosed
    {
        AVDateTime m_last_modified;
        bool m_closed;
    };

    void checkForSubfolders();

protected:

    //----- fields

    //! contains the last modification times of the watched files
    QMap<QString, LastModifiedClosed> m_file_modify_map;

    //! list of directories to check
    AVFileWatcherDirDict m_dir_dict;

    //! used to trigger the file checking
    QTimer m_check_timer;

    //! used to check the watched files
    QFileInfo m_fileinfo;

    //! used to check watched directories
    QDir m_dir;

    //! if to use the check timer
    bool m_usetimer;

    //! check interval in msecs
    uint m_check_intervall_msec;

    //! interval until a file is seen as closed (after for example copy/write)
    int m_check_closed_interval_sec;

    //! whether to search folders for subfolders or not
    bool m_recursive;

    //----- methods

    void checkDirectories(QStringList& removed_filelist);

private:

    //! Hidden copy-constructor
    AVFileWatcher(const AVFileWatcher&);
    //! Hidden assignment operator
    const AVFileWatcher& operator = (const AVFileWatcher&);
};

#endif /* __AVFILEWATCHER_H__ */

// End of file
