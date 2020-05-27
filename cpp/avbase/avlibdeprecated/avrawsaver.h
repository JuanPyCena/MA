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
    \author  Dr. Konrad Koeck, k.koeck@avibit.com
    \author  QT4-PORT: Christian Muschick, c.muschick@avibit.com
    \author  Thomas Neidhart, t.neidhart@avibit.com
    \author  Thomas Pock, t.pock@avibit.com
    \brief   Raw data archiving class definition.
*/

///////////////////////////////////////////////////////////////////////////////

#if !defined(AVRAWSAVER_H_INCLUDED)
#define AVRAWSAVER_H_INCLUDED

#include <qstring.h>
#include <qfile.h>
#include <qdatetime.h>
#include <qdatastream.h>
#include "avlibdeprecated_export.h"
///////////////////////////////////////////////////////////////////////////////
//! A raw data saver class.
/*! Used for saving raw data to a file.
*/
class AVLIBDEPRECATED_EXPORT AVRawSaver {

public:
    //! construct a raw message saver class
    explicit AVRawSaver (const QString& prefix, 
                         bool bSaveTimeStamp = false,
                         const QString& file_path = QString(),
                         unsigned long max_filesize = 2048*1024) :
        f(0), size(0), file_prefix(prefix), m_bSaveTimeStamp(bSaveTimeStamp), 
        ds(0), m_file_path(file_path), m_max_filesize(max_filesize)
    {}

    //! write a single character to the raw archive
    /*! \param data the raw data
    */
    bool putch(char data);

    //! write a raw data block
    /*! \param data a pointer to the raw data block
        \param len the length of the data block
    */
    bool writeBlock(const char *data, int len);
    //! write a raw data block
    /*! \param data a pointer to the raw data block
        \param len the length of the data block
        \param dt current date and time
        \param msecs Milliseconds for more exact time specification
    */
    bool writeBlock(const char *data, int len, const QDateTime &dt);

protected:
    QFile *f;                  //!< Current open file for writing
    unsigned long size;        //!< size of the file that is currently open
    QString file_prefix;       //!< raw file prefix
    bool m_bSaveTimeStamp;     //!< Specifies if the timestamp should be saved
    QDataStream *ds;           //!< Stream to write out the data
    QString m_file_path;       //!< path to save the RAW files to

    //! max. filesize before opening a new file
    unsigned long m_max_filesize;

private:
    //! Creates the file name for raw logging
    QString createFileName();

    //! Removes the old raw log files
    void removeOldFiles();
};

#endif

// End of file
