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
#include "avmisc.h"

#include <QStringBuilder>
#include <QElapsedTimer>

#include "avsharedmemory_private_unix.h"

// define data structure for operating on semaphores
#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
// union semun is defined by including <sys/sem.h>
#else
// according to X/OPEN we have to define it ourselves
union semun
{
    int val;                   // value for SETVAL
    struct semid_ds *buf;      // buffer for IPC_STAT & IPC_SET
    unsigned short int *array; // array for GETALL & SETALL
    struct seminfo *__buf;     // buffer for IPC_INFO
};
#endif

//-----------------------------------------------------------------------------
/**
 *  The SHM header which is managed by this class (its size is added to the requested SHM size).
 */
struct __attribute__ ((__packed__)) AVSharedMemoryHeader
{
    //! The net size of the shared memory segment (AVSharedMemoryHeader not included).
    qint32 m_size;
    //! Whether client code did already initialize the shared memory segment.
    qint32 m_initialized;
};

//-----------------------------------------------------------------------------


const QString AVSharedMemoryPrivate::SHM_FILENAME = "shm";
const uint AVSharedMemoryPrivate::FILE_LOCK_TIMEOUT_MSECS = 500;


//-----------------------------------------------------------------------------

AVSharedMemoryPrivate::AVSharedMemoryPrivate(const QString& segment_id, int create_size, const QString& override_application_name) :
    m_segment_id(segment_id),
    m_size(create_size),
    m_semaphore_id(-1),
    m_shm_id(-1),
    m_address(0),
    m_locked_semaphore(false)
{
    AVASSERT(create_size > 0);

    QString application_name = override_application_name.isNull() ?
            AVEnvironment::getApplicationName() : override_application_name;

    m_shm_file_name = QDir::tempPath() % "/" %
            SHM_FILENAME % "_" % AVGetSystemUsername().toLower() % "_" % application_name.toLower() %
            "_" % m_segment_id;

    openOrCreate();
}

//-----------------------------------------------------------------------------

AVSharedMemoryPrivate::~AVSharedMemoryPrivate()
{
    if (m_locked_semaphore)
    {
        AVLogError << "AVSharedMemoryPrivate::~AVSharedMemoryPrivate: (" << m_segment_id << ") semaphore is still locked!";
        unlock();
    }
    detachOrDestroy(true);
}

//-----------------------------------------------------------------------------

bool AVSharedMemoryPrivate::lock()
{
    if(m_semaphore_id == -1)
    {
        AVLogError << "AVSharedMemoryPrivate::lock: (" << m_segment_id << ") cannot lock SHM, not initialized";
        return false;
    }

    struct sembuf sem_op;
    sem_op.sem_num = 0;
    sem_op.sem_op = -1; //Note: unlocked semaphore has value 1, locked 0
    sem_op.sem_flg = SEM_UNDO; // unlocks semaphore if process terminates

    for (;;)
    {
        int result = semop(m_semaphore_id, &sem_op, 1); //Note: this blocks until semaphore value is >0, then it decrements

        if (result == 0)
            break;
        else if (errno == EINTR)
            continue;
        else
        {
            AVLogErrorErrno << "AVSharedMemoryPrivate::lock: (" << m_segment_id << ") could not lock semaphore.";
            return false;
        }
    }

    m_locked_semaphore = true;

    return true;
}

//-----------------------------------------------------------------------------

bool AVSharedMemoryPrivate::unlock()
{
    if(m_semaphore_id == -1)
    {
        AVLogError << "AVSharedMemoryPrivate::unlock: (" << m_segment_id << ") cannot unlock SHM, not initialized";
        return false;
    }
    if (!m_locked_semaphore)
    {
        // This can happen if client code does not check lock() return value.
        AVLogError << "AVSharedMemoryPrivate::unlock: (" << m_segment_id << ") Tried to unlock semaphore, but we didn't lock it!";
        return false;
    }

    // do this before actually releasing the lock so access to this member is still mutexed - see SWE-1985.
    m_locked_semaphore = false;

    struct sembuf sem_op;
    sem_op.sem_num = 0;
    sem_op.sem_op = 1; //Note: unlocked semaphore has value 1, locked 0
    sem_op.sem_flg = SEM_UNDO; // unlocks semaphore if process terminates
    int result = semop(m_semaphore_id, &sem_op, 1);

    if (result == -1)
    {
        AVLogErrorErrno << "AVSharedMemoryPrivate::unlock: (" << m_segment_id << ") could not unlock semaphore";
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
    return m_address + sizeof(AVSharedMemoryHeader);
}

//-----------------------------------------------------------------------------

int AVSharedMemoryPrivate::getSize() const
{
    return m_size;
}

//-----------------------------------------------------------------------------

bool AVSharedMemoryPrivate::isInitialized() const
{
    AVSharedMemoryHeader *header = reinterpret_cast<AVSharedMemoryHeader*>(m_address);
    return header->m_initialized;
}

//-----------------------------------------------------------------------------

void AVSharedMemoryPrivate::setInitialized()
{
    AVSharedMemoryHeader *header = reinterpret_cast<AVSharedMemoryHeader*>(m_address);
    header->m_initialized = true;
}

//-----------------------------------------------------------------------------

void AVSharedMemoryPrivate::detachWithoutCleanup()
{
    FILE* file_handle = openAndLockFile(m_shm_file_name);
    detach();
    unlockAndCloseFile(file_handle);

    m_semaphore_id = -1;
    m_shm_id = -1;
}

//-----------------------------------------------------------------------------

FILE* AVSharedMemoryPrivate::openAndLockFile(const QString& file_name)
{
    FILE* file_handle = fopen(qPrintable(file_name), "w+");
    if(file_handle == 0)
    {
        AVLogErrorErrno << "AVSharedMemoryPrivate::openAndLockFile: could not open file "<<file_name;
        return 0;
    }

    QElapsedTimer timer;
    timer.start();

    bool failed = true;
    while(timer.elapsed() < FILE_LOCK_TIMEOUT_MSECS)
    {
        failed = flock(fileno(file_handle), LOCK_EX | LOCK_NB);
        if(!failed)
            break;
    }

    if(failed)
    {
        AVLogErrorErrno << "AVSharedMemoryPrivate::openAndLockFile: could not get file lock";
        fclose(file_handle);
        return 0;
    }
    return file_handle;
}

//-----------------------------------------------------------------------------

bool AVSharedMemoryPrivate::unlockAndCloseFile(FILE* file_handle)
{
    bool success = true;
    if(flock(fileno(file_handle), LOCK_UN | LOCK_NB))
    {
        AVLogErrorErrno << "AVSharedMemoryPrivate::openOrCreate: could not unlock file";
        success = false;
    }

    if(fclose(file_handle))
    {
        AVLogErrorErrno << "AVSharedMemoryPrivate::openOrCreate: could not close file";
        success = false;
    }
    return success;
}

//-----------------------------------------------------------------------------

bool AVSharedMemoryPrivate::openOrCreate()
{
    if(m_address != 0)
    {
        AVLogError << "AVSharedMemoryPrivate::openOrCreate: (" << m_segment_id << ") cannot open or create, SHM address exists";
        return false;
    }

    AVLogDebug << "AVSharedMemoryPrivate::openOrCreate: (" << m_segment_id << ") opening SHM file "<<m_shm_file_name;

    FILE* file_handle = openAndLockFile(m_shm_file_name);

    if(file_handle == 0)
        return false;

    key_t key = ftok(qPrintable(m_shm_file_name), 1234);

    if(key == -1)
    {
        AVLogErrorErrno << "AVSharedMemoryPrivate::openOrCreate: (" << m_segment_id << ") Could not get key from SHM file";
        unlockAndCloseFile(file_handle);
        return false;
    }

    AVLogDebug2 << "AVSharedMemoryPrivate::openOrCreate: (" << m_segment_id << ") using SHM key "<<key;

    // try to find the semaphore without creating
    // Remark: need to distinguish between in existing and non-existing, because it needs initialization
    m_semaphore_id = semget(key, 1, 0666);

    if(m_semaphore_id == -1)
    {
        // try to find the semaphore with creating
        m_semaphore_id = semget(key, 1, IPC_CREAT | 0666);

        if(m_semaphore_id == -1)
        {
            AVLogErrorErrno << "AVSharedMemoryPrivate::openOrCreate: (" << m_segment_id << ") could not get or create SHM semaphore";
            unlockAndCloseFile(file_handle);
            return false;
        }

        // initialize the semaphore with 1
        semun sem_val;
        sem_val.val = 1;
        int result = semctl(m_semaphore_id, 0, SETVAL, sem_val);

        if(result == -1)
        {
            AVLogErrorErrno << "AVSharedMemoryPrivate::openOrCreate: (" << m_segment_id << ") could not initialize SHM semaphore";
            m_semaphore_id = -1;
            unlockAndCloseFile(file_handle);
            return false;
        }

        AVLogDebug << "AVSharedMemoryPrivate::openOrCreate: (" << m_segment_id << ") created SHM semaphore ID "<<m_semaphore_id;
    }
    else
    {
        AVLogDebug << "AVSharedMemoryPrivate::openOrCreate: (" << m_segment_id << ") got existing SHM semaphore ID "<<m_semaphore_id;
    }

    bool new_segment = false;
    //try to find the segment without creating
    m_shm_id = shmget(key, m_size + sizeof(AVSharedMemoryHeader), 0666);

    // EINVAL can mean two things:
    // - our requested size was to big (>SHMMAX)
    // - there is already a segment which is smaller than the requested size
    if (m_shm_id == -1 && errno == EINVAL)
    {
        // Try to attach to existing (too small) segment. If it is stale, it will be cleaned up and recreated with the proper size;
        // otherwise the check for correct size will trigger an error later (it is needed anyway for the case in which a smaller
        // segment was requested).
        m_shm_id = shmget(key, 1, 0666);
        if (m_shm_id != -1)
        {
            AVLogWarning << "AVSharedMemoryPrivate::openOrCreate: attached to a segment which is smaller than the requested size.";
        }
    }

    if(m_shm_id == -1)
    {
        //try to find the segment with creating
        m_shm_id = shmget(key, m_size + sizeof(AVSharedMemoryHeader), IPC_CREAT | 0666);

        if(m_shm_id == -1)
        {
            AVLogErrorErrno << "AVSharedMemoryPrivate::openOrCreate: (" << m_segment_id << ") could not get or create SHM";
            unlockAndCloseFile(file_handle);
            return false;
        }

        new_segment = true;
    }

    AVLogDebug << "AVSharedMemoryPrivate::openOrCreate: (" << m_segment_id << ") got SHM id "<<m_shm_id
            <<", currently attached: "<<getNumberOfAttachments();

    //detect stale segments
    if(!new_segment && getNumberOfAttachments() == 0) // existing segment, but no attachments
    {
        AVLogWarning << "AVSharedMemoryPrivate::openOrCreate: (" << m_segment_id << ") detected stale shared memory segment "<<m_shm_id;

        int semaphore_current_value = semctl(m_semaphore_id, 0, GETVAL);
        if (semaphore_current_value != 1) // 0 is locked, 1 is unlocked
        {
            AVLogError << "AVSharedMemoryPrivate::openOrCreate: Semaphore in stale segment was unexpectedly locked.";
            m_locked_semaphore = true;
            unlock();
        }

        // delete existing segment
        shmid_ds shm_descriptor;
        int result = shmctl(m_shm_id, IPC_RMID, &shm_descriptor);
        if(result == -1)
        {
            AVLogErrorErrno << "AVSharedMemoryPrivate::openOrCreate: (" << m_segment_id << ") could not destroy stale SHM";
            m_shm_id = -1;
            unlockAndCloseFile(file_handle);
            return false;
        }

        //create new segment
        m_shm_id = shmget(key, m_size + sizeof(AVSharedMemoryHeader), IPC_CREAT | 0666);

        if(m_shm_id == -1)
        {
            AVLogErrorErrno << "AVSharedMemoryPrivate::openOrCreate: (" << m_segment_id << ") could not get or create SHM";
            unlockAndCloseFile(file_handle);
            return false;
        }

        AVLogInfo << "AVSharedMemoryPrivate::openOrCreate: (" << m_segment_id << ") deleted stale segment and created new one with ID "<<m_shm_id;
        new_segment = true;
    }

    // attach to the memory segment
    m_address = static_cast<char*>(shmat(m_shm_id, 0, 0));
    if(m_address == reinterpret_cast<char*>(-1))
    {
        AVLogErrorErrno << "AVSharedMemoryPrivate::openOrCreate: (" << m_segment_id << ") could attach to SHM";

        m_address = 0;
        unlockAndCloseFile(file_handle);
        return false;
    }

    // initalize segment header or read existing header
    AVSharedMemoryHeader *header = reinterpret_cast<AVSharedMemoryHeader*>(m_address);
    if(new_segment)
    {
        header->m_size        = m_size;
        header->m_initialized = 0;

        AVLogDebug << "AVSharedMemoryPrivate::openOrCreate: (" << m_segment_id << ") created new segment of size "<<m_size;
    }
    else
    {
        int existing_size = getSegmentSize();
        int requested_size = m_size + static_cast<int>(sizeof(AVSharedMemoryHeader));
        if (requested_size != existing_size)
        {
            // This way round it would even work (requested size is smaller than existing size, or we would have failed earlier),
            // but not the other way round. Play it safe and report it as an error.
            AVLogError << "AVSharedMemoryPrivate::openOrCreate: attached to a segment with incorrect size (requested "
                       << requested_size << " vs existing " << existing_size << ") - configuration problem? Detaching.";
            detachOrDestroy(false);
        }
        else
        {
            AVLogDebug << "AVSharedMemoryPrivate::openOrCreate: (" << m_segment_id << ") found existing segment of size " << m_size
                       <<", initialized: "<< header->m_initialized;
        }
    }

    if(!unlockAndCloseFile(file_handle))
        return false;

    AVLogDebug << "AVSharedMemoryPrivate::openOrCreate: (" << m_segment_id << ") closed SHM file";

    return m_address != 0;
}

//-----------------------------------------------------------------------------

void AVSharedMemoryPrivate::detach()
{
    if(m_address != 0)
    {
        int result = shmdt(m_address);
        if(result == -1)
        {
            AVLogErrorErrno << "AVSharedMemoryPrivate::detach: (" << m_segment_id << ") could not detach from SHM";
        }
        m_address = 0;
    }
}

//-----------------------------------------------------------------------------

bool AVSharedMemoryPrivate::detachOrDestroy(bool acquire_lock)
{
    FILE* file_handle = 0;
    if (acquire_lock)
        file_handle = openAndLockFile(m_shm_file_name);

    detach();

    // Delete segment and semaphore if there are no attachments anymore.
    // If there was no way for us to attach to an existing SHM segment, assume that the semaphore can be safely deleted as well.
    if(getNumberOfAttachments() == 0 || m_shm_id == -1)
    {
        if(m_shm_id != -1)
        {
            AVLogDebug << "AVSharedMemoryPrivate::detachOrDestroy: (" << m_segment_id << ") destroying SHM segment id "<<m_shm_id
                    <<", currently attached: "<<getNumberOfAttachments();

            shmid_ds shm_descriptor;
            int result = shmctl(m_shm_id, IPC_RMID, &shm_descriptor);
            if(result == -1)
            {
                AVLogErrorErrno << "AVSharedMemoryPrivate::detachOrDestroy: (" << m_segment_id << ") could not destroy SHM";
                m_shm_id = -1;
                if (file_handle != 0)
                    unlockAndCloseFile(file_handle);
                return false;
            }
            m_shm_id = -1;
        }

        if(m_semaphore_id != -1)
        {
            AVLogDebug << "AVSharedMemoryPrivate::detachOrDestroy: (" << m_segment_id << ") destroying SHM semaphore id "<<m_semaphore_id;

            // Pass in nullptr to avoid valgrind error - see http://beanalby.net/2012/06/valgrinds-syscall-param-semctlarg-points-to-uninitialised-bytes/
            int result = semctl(m_semaphore_id, 0, IPC_RMID, nullptr);

            if(result == -1)
            {
                AVLogErrorErrno << "AVSharedMemoryPrivate::detachOrDestroy: (" << m_segment_id << ") could not destroy SHM semaphore";
                m_semaphore_id = -1;
                if (file_handle != 0)
                    unlockAndCloseFile(file_handle);
                return false;
            }
            m_semaphore_id = -1;
        }
    }
    else
    {
        AVLogDebug2 << "AVSharedMemoryPrivate::detachOrDestroy: (" << m_segment_id << ") not destroying SHM segment "<<m_shm_id
                <<", currently attached: "<<getNumberOfAttachments();
    }

    if (file_handle != 0)
        unlockAndCloseFile(file_handle);
    return true;
}

//-----------------------------------------------------------------------------

int AVSharedMemoryPrivate::getNumberOfAttachments()
{
    if(m_shm_id == -1)
        return -1;

    shmid_ds shm_descriptor;
    int result = shmctl(m_shm_id, IPC_STAT, &shm_descriptor);
    if(result == -1)
    {
        AVLogErrorErrno << "AVSharedMemoryPrivate::getNumberOfAttachments: (" << m_segment_id << ") could get SHM info";
        return -1;
    }
    return shm_descriptor.shm_nattch;
}

//-----------------------------------------------------------------------------

int AVSharedMemoryPrivate::getSegmentSize() const
{
    if(m_shm_id == -1)
        return -1;

    shmid_ds shm_descriptor;
    int result = shmctl(m_shm_id, IPC_STAT, &shm_descriptor);
    if(result == -1)
    {
        AVLogErrorErrno << "AVSharedMemoryPrivate::getSegmentSize: (" << m_segment_id << ") could get SHM info";
        return -1;
    }
    return shm_descriptor.shm_segsz;
}

//-----------------------------------------------------------------------------

// End of file
