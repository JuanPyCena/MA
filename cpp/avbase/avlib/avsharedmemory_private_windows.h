 //-----------------------------------------------------------------------------
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2015
//
// Module:    AVLIB - Avibit Library
//
//-----------------------------------------------------------------------------

/*! \file
    \author  Ralf Gamillscheg, r.gamillscheg@avibit.com
    \author  QT5-PORT: Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   AVSharedMemoryPrivate header
 */

#ifndef AVSHAREDMEMORY_PRIVATE_WINDOWS_INCLUDED
#define AVSHAREDMEMORY_PRIVATE_WINDOWS_INCLUDED

#include <windows.h>

// Qt includes
#include <qdir.h>

// avlib includes
#include "avenvironment.h"


//! AVSharesMemory implementation for Windows
class AVSharedMemoryPrivate
{
public:
    AVSharedMemoryPrivate(const QString& segment_id, int create_size,
                   const QString& override_application_name = QString());
    virtual ~AVSharedMemoryPrivate();

    bool lock();

    bool unlock();

    char* getSegment() const;

    int getSize() const;

    bool isInitialized() const;

    void setInitialized();

    //! Unit testing only. Avoids cleanup in destructor, leaves stale segment under linux.
    void detachWithoutCleanup();

private:
    bool createSegment(int size);
    //! returns with wrong_size true if an existing segment is found, but with wrong size
    bool openSegment(bool& wrong_size);
    bool closeSegment();
private:
    QString m_segment_id;

    Qt::HANDLE m_shm_handle;           //!< handle for shm segment
    Qt::HANDLE m_mutex_handle;         //!< handle for the mutex object
    QString m_shm_name;           //!< name of the shm segment
    QString m_mutex_name;         //!< name of the mutex object

    int m_shm_id;
    char* m_address;

    int m_size;
};


#endif // AVSHAREDMEMORYPRIVATE_WINDOWS_INCLUDED

// End of file
