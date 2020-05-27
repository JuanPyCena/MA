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
    \author  QT5-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   AVSharedMemory header
 */

#ifndef AVSHAREDMEMORY_INCLUDED
#define AVSHAREDMEMORY_INCLUDED

// local includes

// Qt includes
#include <QObject>

// avlib includes
#include "avlib_export.h"

// forward declarations
class AVSharedMemoryPrivate;

//-----------------------------------------------------------------------------
//! AviBit SharedMemory class
/*! ATTENTION, when working on the segment the shared memory needs to be locked by using lock() and unlock()
 *
 * The Block consists of a header which contains a qint32 for the size, and a qint32 for the initialized flag.
 * The initialized flag must be set by the user using setInitialized() and can be read by isInitialized().
 */
class AVLIB_EXPORT AVSharedMemory : public QObject
{
    Q_OBJECT

    //! friend declaration for function level test case
    //friend class TstAVSharedMemory;

public:
    /*!
     * \param segment_id                Identifies the shared segment instance (e.g. time reference, process state)
     * \param create_size               Size in bytes of the SHM payload
     * \param override_application_name SHM segments for different application names do not interfere with each other
     */
    AVSharedMemory(const QString& segment_id, int create_size,
                   const QString& override_application_name = QString());
    ~AVSharedMemory() override;

    /**
     * @brief lock obtains a non-recursive lock on the shared memory segment.
     *
     * This is implemented via a SHM semaphore (Linux) or a WINAPI mutex object (windows).
     * Locking the SHM ensures exclusive access across threads and processes.
     *
     * *Attention* non-recursive lock, calling this twice from the same thread will result in deadlock.
     *             Also see SWE-1984.
     */
    bool lock();
    bool unlock();

    /**
     * @brief getSegment returns the pointer to the SHM payload (after the internal header structure).
     */
    char* getSegment() const;

    //! return the actual size of the shm segment
    int getSize() const;

    bool isInitialized() const;

    //! sets the initialized flag of the shm segment.
    /*! If you call this method you must already have acquired the
        mutex.
     */
    void setInitialized();

    inline char* pointer(int offset) const {
        if (getSegment() == 0 || offset < 0) return 0;
        return getSegment() + offset;
    }

    //! get an offset from a pointer into the shm.
    inline int offset(char *pointer) const {
        if (getSegment() == 0) return 0;
        if (pointer < getSegment()) return 0;
        return static_cast<int>(pointer - getSegment());
    }

    //! Used for unit tests only, so stale segements can be created.
    //! Leaves the object in an unusable state, destroy it afterwards.
    void detachWithoutCleanup();

private:
    QScopedPointer<AVSharedMemoryPrivate> m_private;

};

#endif // AVSHAREDMEMORY_INCLUDED

// End of file
