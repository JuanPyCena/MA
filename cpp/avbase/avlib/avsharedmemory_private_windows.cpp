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
    \brief   AVSharedMemoryPrivate implementation
 */

// local includes
#include "avsharedmemory.h"

#include <QStringBuilder>
#include <QElapsedTimer>


#include "avsharedmemory_private_windows.h"


//-----------------------------------------------------------------------------

AVSharedMemoryPrivate::AVSharedMemoryPrivate(const QString& segment_id, int create_size, const QString& override_application_name) :
    m_segment_id(segment_id),
    m_shm_handle(0),
    m_mutex_handle(0),
    m_shm_id(-1),
    m_address(0),
	m_size(create_size)
{
    QString application_name = override_application_name.isNull() ?
            AVEnvironment::getApplicationName() : override_application_name;

    m_shm_name.sprintf("%s_%s_shm", qPrintable(application_name.toLower()), qPrintable(segment_id.toLower()));
    m_mutex_name.sprintf("%s_%s_mutex", qPrintable(application_name.toLower()), qPrintable(segment_id.toLower()));

    bool wrong_size = false;
    if (!openSegment(wrong_size) && !wrong_size)
    {
        AVLogInfo << "AVSharedMemoryPrivate::AVSharedMemoryPrivate: could not open existing shm "
                              "segment, now trying to create a new one...";


        // if the open command failed, try to create one
        if (!createSegment(create_size))
        {
			closeSegment();
		}
    }
}

//-----------------------------------------------------------------------------

AVSharedMemoryPrivate::~AVSharedMemoryPrivate()
{
	closeSegment();
}

//-----------------------------------------------------------------------------

bool AVSharedMemoryPrivate::lock()
{
    WaitForSingleObject(m_mutex_handle, INFINITE);
    return true;
}

//-----------------------------------------------------------------------------

bool AVSharedMemoryPrivate::unlock()
{
    if(!ReleaseMutex(m_mutex_handle))
    {
        AVLogWarning << "AVSharedMemoryPrivate::unlock: got error in unlocking the mutex";
        return false;
    }
    return true;
}

//-----------------------------------------------------------------------------

char* AVSharedMemoryPrivate::getSegment() const
{
    if(m_address == 0)
    {
        AVLogError << "AVSharedMemoryPrivate::getSegment: (" << m_segment_id << ") not initialized";
        return 0;
    }
    return m_address + sizeof(qint32) + sizeof(qint32);
}

//-----------------------------------------------------------------------------

int AVSharedMemoryPrivate::getSize() const
{
    return m_size;
}

//-----------------------------------------------------------------------------

bool AVSharedMemoryPrivate::isInitialized() const
{
    qint32* initialized_ptr = reinterpret_cast<qint32*>(m_address+sizeof(qint32));
    return *initialized_ptr;
}

//-----------------------------------------------------------------------------

void AVSharedMemoryPrivate::setInitialized()
{
    qint32* initialized_ptr = reinterpret_cast<qint32*>(m_address+sizeof(qint32));
    *initialized_ptr = true;
}

//-----------------------------------------------------------------------------

void AVSharedMemoryPrivate::detachWithoutCleanup()
{
    // nothing to do in windows, as the destructor also does nothing.
    // SHM segments without attached processes are cleaned up by operating system.
}

//-----------------------------------------------------------------------------

bool AVSharedMemoryPrivate::createSegment(int size)
{
    // create a security descriptor to share our shm segment with forground
    // processes even if we are a NT service

    PSECURITY_DESCRIPTOR pSD;
    SECURITY_ATTRIBUTES sa;
    pSD = (PSECURITY_DESCRIPTOR) LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
    if (pSD == NULL) return false;
    if (!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION))
    {
        LocalFree(pSD);

        AVLogError << "AVSharedMemoryPrivate::createSegment: could not initialize security descriptor";
        return false;
    }

    if (!SetSecurityDescriptorDacl(pSD, true, (PACL) NULL, false))
    {
        LocalFree(pSD);
        AVLogError << "AVSharedMemoryPrivate::createSegment: could not set acl in security descriptor";
        return false;
    }

    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = pSD;
    sa.bInheritHandle = true;

    // Create the mutex
    m_mutex_handle = CreateMutex(&sa, false, m_mutex_name.toLatin1().data());
    if (m_mutex_handle == (HANDLE) 0)
    {
        AVLogError << "AVSharedMemoryPrivate::createSegment:  could not create the mutex";
        return false;
    }

    // create the mapping file
    m_shm_handle = CreateFileMapping(INVALID_HANDLE_VALUE,
                                     &sa, PAGE_READWRITE, 0,
                                     (DWORD) (size+ sizeof(qint32)+sizeof(qint32)), m_shm_name.toLatin1().data());
    LocalFree(pSD);
    if (m_shm_handle == (HANDLE) 0)
    {
        AVLogError << "AVSharedMemoryPrivate::createSegment: could not create file mapping for shm segment";
        return false;
    }

    m_address = (char *) MapViewOfFile(m_shm_handle, FILE_MAP_ALL_ACCESS,
                                        0, 0, 0);
    if (m_address == 0)
    {
        CloseHandle(m_shm_handle);
        AVLogError << "AVSharedMemoryPrivate::createSegment:  could not attach to shm segment";
        return false;
    }

    qint32* size_ptr = reinterpret_cast<qint32*>(m_address);
    qint32* initialized_ptr = reinterpret_cast<qint32*>(m_address+sizeof(qint32));
    *size_ptr = m_size;
    *initialized_ptr = 0;

    AVLogDebug << "AVSharedMemoryPrivate::createSegment: (" << m_segment_id << ") created new segment of size "<<m_size;

    return true;
}

//-----------------------------------------------------------------------------

bool AVSharedMemoryPrivate::openSegment(bool& wrong_size)
{
    wrong_size = false;
    
    m_mutex_handle = OpenMutex(MUTEX_ALL_ACCESS, false,
                               m_mutex_name.toLatin1().data());
    if (m_mutex_handle == (HANDLE) 0)
    {
        AVLogWarning << "AVSharedMemoryPrivate::openSegment: failed to open mutex object";
        return false;

    }

    m_shm_handle = OpenFileMapping(FILE_MAP_ALL_ACCESS, false,
                                   m_shm_name.toLatin1().data());
    if (m_shm_handle == (HANDLE) 0)
    {
        AVLogWarning << "AVSharedMemoryPrivate::openSegment: failed to open shm segment";

        return false;
    }

    m_address = (char *) MapViewOfFile(m_shm_handle, FILE_MAP_ALL_ACCESS,
                                        0, 0, 0);
    if (m_address == 0)
    {
        CloseHandle(m_shm_handle);
        AVLogWarning << "AVSharedMemoryPrivate::openSegment: failed to attach to shm segment";
        return false;
    }

    qint32* size_ptr = reinterpret_cast<qint32*>(m_address);
    qint32* initialized_ptr = reinterpret_cast<qint32*>(m_address+sizeof(qint32));


    if(m_size != *size_ptr)
    {
        AVLogError << "AVSharedMemoryPrivate::openSegment: attached to a segment with incorrect size (requested "
                   << m_size << " vs existing " << *size_ptr << ") - configuration problem? Detaching.";

        closeSegment();
        wrong_size = true;
        return false;
    }

    AVLogDebug << "AVSharedMemoryPrivate::openSegment: (" << m_segment_id << ") found existing segment of size "<<m_size
            <<", initialized: "<<*initialized_ptr;

    return true;
}

//-----------------------------------------------------------------------------

bool AVSharedMemoryPrivate::closeSegment()
{
    bool success = true;
    if (m_address != 0)
    {
        if (!UnmapViewOfFile(m_address))
        {
            AVLogWarning << "AVSharedMemoryPrivate::closeSegment: failed to detach from shm segment";
            success = false;
        }
        else
        {
            m_address = 0;
        }
    }

    if (m_shm_handle != (HANDLE) 0)
    {
        if (!CloseHandle(m_shm_handle))
        {
            AVLogWarning << "AVSharedMemoryPrivate::closeSegment: failed to close shm segment handle";
            success = false;
        }
        else
        {
            m_shm_handle = (HANDLE) 0;
        }

    }

    if (m_mutex_handle != (HANDLE) 0)
    {
        if (!CloseHandle(m_mutex_handle))
        {
            AVLogWarning << "AVSharedMemoryPrivate::closeSegment: failed to close mutex handle";
            success = false;
        }
        else
        {
            m_mutex_handle = (HANDLE) 0;
        }
    }

    return success;
}

//-----------------------------------------------------------------------------

// End of file
