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

#ifndef AVSHAREDMEMORY_PRIVATE_UNIX_INCLUDED
#define AVSHAREDMEMORY_PRIVATE_UNIX_INCLUDED

#include <stdio.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

// Qt includes
#include <qdir.h>

// avlib includes
#include "avenvironment.h"


//! AVSharesMemory implementation for Unix
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
    bool openOrCreate();

    //! Detach from memory segment - m_address is 0 afterwards.
    void detach();
    bool detachOrDestroy(bool acquire_lock);

    //! \return -1 if not m_shm_id is -1 or an error occurs, the number of processes currently attached to the SHM otherwise.
    int getNumberOfAttachments();

    //! We don't want to rely on the "size" field in our shared memory header when initializing and potentially working with stale segments.
    //!
    //! \return The gross number of bytes (including AVSharedMemoryHeader) allocated for the SHM segment for m_shm_id, or -1 if
    //!         - m_shm_id is -1 or
    //!         - some other error occured.
    int getSegmentSize() const;

    static FILE* openAndLockFile(const QString& file_name);
    static bool unlockAndCloseFile(FILE* file_handle);

private:

    static const QString SHM_FILENAME;
    static const uint FILE_LOCK_TIMEOUT_MSECS;

    QString m_shm_file_name;

    QString m_segment_id;
    //! The net size of the shared memory segment (AVSharedMemoryHeader not included).
    int m_size;

    int m_semaphore_id;
    int m_shm_id;

    //! Actual pointer into our shared memory segment.
    char* m_address;

    //! Avoid freeing somebody else's lock by remembering whether we locked the semaphore.
    bool m_locked_semaphore;
};

#endif // AVSHAREDMEMORY_PRIVATE_UNIX_INCLUDED

// End of file
