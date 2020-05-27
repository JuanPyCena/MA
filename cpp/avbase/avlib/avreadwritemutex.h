///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2011
//
// Module:     AVLIB - AviBit Library
//
/////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  DI Ulrich Feichter, u.feichter@avibit.com
    \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   a timer class, which can handle time speed changes from
             AVTimeReference
*/

#if !defined(AVREADWRITEMUTEX_H_INCLUDED)
#define AVREADWRITEMUTEX_H_INCLUDED

// Qt include
#include <QMutex>
#include <QSemaphore>

#include "avlib_export.h"

///////////////////////////////////////////////////////////////////////////////
//! Readers-Writer Mutex which allowes more simultaneous readers but just one writer
/*! The Mutex uses the standard QMutex and QSemaphore classes to realize the Readers-Writer Mutex.
    The maximum number of simultaneous readers can be configured in the constructor.
    The implementation was taken from http://doc.qt.nokia.com/qq/qq11-mutex.html
    Usage example:

    \code
    ReadWriteMutex mutex;

    void ReaderThread::run()
    {
        ...
        mutex.lockRead();
        read_file();
        mutex.unlockRead();
        ...
    }

    void WriterThread::run()
    {
        ...
        mutex.lockWrite();
        write_file();
        mutex.unlockWrite();
        ...
    }
    \endcode
 */
class AVLIB_EXPORT AVReadWriteMutex
{
public:
    //! constructs the Readers-Writer-Mutex. maxReaders gives the maximum number of allowed
    //! simultaneous readers
    explicit AVReadWriteMutex(int max_readers = 32)
        : m_semaphore(max_readers),
          m_max_readers(max_readers)
    {
    }

    //! lock method for a reader
    void lockRead() { m_semaphore.acquire(); }

    //! unlock method for a reader
    void unlockRead() { m_semaphore.release(); }

    //! lock method for a writer
    void lockWrite() {
        QMutexLocker locker(&m_mutex);
        // we increment the semaphore one by one to not starve: if we would increment it by
        // maxReaders() we would have to wait until no more reader reads.
        for (int i = 0; i < maxReaders(); ++i)
            m_semaphore.acquire();
    }

    //! unlock method for a writer
    void unlockWrite() { m_semaphore.release(m_max_readers-m_semaphore.available()); }

    //! returns the maximum allowed simultaneous readers
    int maxReaders() const { return m_max_readers; }

private:
    QSemaphore m_semaphore;
    QMutex m_mutex;

    int m_max_readers;
};

#endif

// End of file
