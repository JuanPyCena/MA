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


#if !defined(AVTEXTWRITER_H_INCLUDED)
#define AVTEXTWRITER_H_INCLUDED

class AVTextWriter;
#if defined(AVTEXTWRITER_CPP)
AVTextWriter *TextWriter = 0;
#else
extern AVTextWriter *TextWriter;
#endif

// QT includes
#include <qstring.h>
#include <qmutex.h>
#include <qfile.h>

// Local includes
#include "avlibdeprecated_export.h"

#include "avdatetime.h"

//! This class writes text to an ASCII file.
//! TODO CM: why does this class exist? use logger instead? at least move this away from avlib...
class AVLIBDEPRECATED_EXPORT AVTextWriter
{
public:
    //! Constructor
    /*! Constructs a new Text Writer.
        \param dir_name The directory name where the file(s) shall be saved
        \param file_name Name of the file to write to (excluding extension and
                         possible date/time information)
        \param file_ext Extension of the file name to write to
        \param append Append data to possibly existing file name
        \param add_date Add the opening date between filename and extension
        \param add_time Add opening and closing time between filename + date and
                        extension. Before the file is closed the closing time
                        will consist only of underscores (______).
        \param close_interval Interval in minutes in which a dynamic
                              date/time file shall be closed/reopened with the
                              current date/time.
        \param keep_files Number of dynamic date/time files to keep
        \param thread_safe Make this class thread-safe
     */
    AVTextWriter(const QString& dir_name,
                 const QString& file_name,
                 const QString& file_ext,
                 bool append = false,
                 bool add_date = false,
                 bool add_time = false,
                 uint close_interval = 0,
                 uint keep_files = 0,
                 bool thread_safe = false);

    //! Destructor
    virtual ~AVTextWriter();

    //! Writes the specified text to the file
    /*! \param line Text to add to the file
     */
    bool writeLine(const QString& line);

    //! Returns the absolute path including the file name.
    QString absoluteFilePath();

protected: // methods
    //! Creates a filename with the current settings
    QString createFileName(const QDateTime& dt, bool for_close = false);

    //! Opens a file by date
    /*! This method creates a suitable output file name from the specified
        date/time variable and opens the file.
        \param dt Date/time stamp.
     */
    bool openFile(const QDateTime &dt);

    //! Opens the specified file
    /*! \param fn Filename
     */
    bool openFile(const QString &fn);

    //! Closes the current file
    void closeFile();

    //! Check if a new output file is required to be opened
    /*! This is the case when there is no open file yet, or if a new day
        occurs.
    */
    bool checkForFile(const QDateTime &dt);

    //! Deletes old files
    void deleteOldFiles();

protected: // members
    QMutex*                m_pMutex;       //!< Mutex for providing access
                                           //!< serialization between threads
    QFile                  m_File;         //!< Current output file
    QString                m_DirName;      //!< Current file directory
    QString                m_FileName;     //!< Current file name
    QString                m_FileExt;      //!< Current file extension
    QTextStream            m_TextStream;   //!< Textstream used for file writing
    bool                   m_bAppend;      //!< Append data to existing file
    bool                   m_bAddDate;     //!< Add date to filename
    bool                   m_bAddTime;     //!< Add time to filename
    uint                   m_CloseInterval;//!< Closing interval for dynamic
                                           //!< time file names (in minutes)
    int                    m_KeepFiles;    //!< Number of files to keep
    AVDateTime             m_FileNameDt;   //!< Date and Time of the current
                                           //!< file name
    AVDateTime             m_LastDt;       //!< Last output date/time

private:
    //! Hidden copy-constructor
    AVTextWriter(const AVTextWriter&);
    //! Hidden assignment operator
    const AVTextWriter& operator = (const AVTextWriter&);
};


#endif

// End of file
