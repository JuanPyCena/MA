///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON
// Copyright:  AviBit data processing GmbH, 2001-2014
// QT-Version: QT5
//
// Module:     AVLIB
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   AVTimeReferenceAdapterSharedMemory implementation
 */

// Qt includes

// avlib includes
#include "avsharedmemory.h"
#include "avenvironment.h"
#include "avshm_varnames.h"
#include "avdatastream.h"
#include "avdatetime.h"
#include "avconsole.h"

// local includes
#include "avtimereferenceadaptersharedmemory.h"
#include "avtimereferenceholder.h"
#include "avtimereference.h"

///////////////////////////////////////////////////////////////////////////////

const uint AVTimeReferenceAdapterSharedMemory::DEFAULT_POLLING_INTERVAL = 500;

///////////////////////////////////////////////////////////////////////////////

AVTimeReferenceAdapterSharedMemory::AVTimeReferenceAdapterSharedMemory() :
    AVTimeReferenceAdapter(AdapterType::AdapterSharedMem),
    m_last_read_resync_counter(0),
    m_polling_timer(this)
{
    AVLogInfo << "AVTimeReferenceAdapterSharedMemory::AVTimeReferenceAdapterSharedMemory: instantiating AVTimeReferenceAdapterSharedMemory";

    m_polling_timer.setInterval(DEFAULT_POLLING_INTERVAL);
    connect(&m_polling_timer, &QTimer::timeout, this, &AVTimeReferenceAdapterSharedMemory::slotCheckTimeReference);
    m_polling_timer.start();

    AVLogInfo << "AVTimeReferenceAdapterSharedMemory::initialize: creating SHM access";
    m_shared_memory.reset(new AVSharedMemory(AVEnvironment::getApplicationName()+AV_SHM_TIMEREF_POSTFIX, 1024));

    // This is needed, because this adapter performs time shifts and speed changes
    m_shared_memory->lock();
    if (!m_shared_memory->isInitialized())
    {
        AVTimeReferenceAdapter::startNewTimeReferenceSessionProcess();

        const AVTimeReferenceHolder& holder = getLocalTimeReferenceHolder();
        writeParameters(holder.getReferenceDateTime(), holder.getSpeed(), holder.getOffset(), false);
    }
    m_shared_memory->unlock();

    // detect the first "time jump" when reading the initial time session before exit on time jump is enabled
    slotCheckTimeReference();
}

///////////////////////////////////////////////////////////////////////////////

AVTimeReferenceAdapterSharedMemory::~AVTimeReferenceAdapterSharedMemory()
{
}

///////////////////////////////////////////////////////////////////////////////

void AVTimeReferenceAdapterSharedMemory::setSpeed(double speed)
{
    setProcessSpeed(speed);

    const AVTimeReferenceHolder& holder = getLocalTimeReferenceHolder();
    writeParameters(holder.getReferenceDateTime(), holder.getSpeed(), holder.getOffset());
}

///////////////////////////////////////////////////////////////////////////////

void AVTimeReferenceAdapterSharedMemory::setCurrentDateTime(const QDateTime& current_date_time)
{
    setProcessDateTime(current_date_time);

    const AVTimeReferenceHolder& holder = getLocalTimeReferenceHolder();
    writeParameters(holder.getReferenceDateTime(), holder.getSpeed(), holder.getOffset());
}

///////////////////////////////////////////////////////////////////////////////

void AVTimeReferenceAdapterSharedMemory::setCurrentDateTimeSpeed(const QDateTime& current_date_time, double speed)
{
    setProcessDateTimeSpeed(current_date_time, speed);

    const AVTimeReferenceHolder& holder = getLocalTimeReferenceHolder();
    writeParameters(holder.getReferenceDateTime(), holder.getSpeed(), holder.getOffset());
}

///////////////////////////////////////////////////////////////////////////////

void AVTimeReferenceAdapterSharedMemory::setCurrentDateTimeShiftedOnly(const QDateTime& current_date_time)
{
    setProcessDateTimeShiftedOnly(current_date_time);

    const AVTimeReferenceHolder& holder = getLocalTimeReferenceHolder();
    writeParameters(holder.getReferenceDateTime(), holder.getSpeed(), holder.getOffset());
}

///////////////////////////////////////////////////////////////////////////////

void AVTimeReferenceAdapterSharedMemory::startNewTimeReferenceSession()
{
    AVTimeReferenceAdapter::startNewTimeReferenceSessionProcess();

    const AVTimeReferenceHolder& holder = getLocalTimeReferenceHolder();
    writeParameters(holder.getReferenceDateTime(), holder.getSpeed(), holder.getOffset());
}

///////////////////////////////////////////////////////////////////////////////

void AVTimeReferenceAdapterSharedMemory::reset()
{
    resetProcess();

    const AVTimeReferenceHolder& holder = getLocalTimeReferenceHolder();
    writeParameters(holder.getReferenceDateTime(), holder.getSpeed(), holder.getOffset());
}

///////////////////////////////////////////////////////////////////////////////

void AVTimeReferenceAdapterSharedMemory::registerConsoleSlots()
{
    AVTimeReferenceAdapter::registerConsoleSlots();

    AVConsole::singleton().registerSlot("setTimeref",   this,
                                        SLOT(slotConsoleSetTimerefence(AVConsoleConnection&, const QStringList&)),
                                        "Set the time reference.",
                                        SLOT(slotCompleteTimestamp(QStringList&, const QStringList&)));
}

///////////////////////////////////////////////////////////////////////////////

void AVTimeReferenceAdapterSharedMemory::slotCheckTimeReference()
{
    AVLogDebug2 << "AVTimeReferenceAdapterSharedMemory::slotCheckTimeReference: ";

    AVDateTime reference_date_time;
    double speed =1.;
    qint64 offset = 0;

    if(readParameters(reference_date_time, speed, offset))
    {
        setProcessTimeReferenceParameters(reference_date_time, speed, offset);
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVTimeReferenceAdapterSharedMemory::slotConsoleSetTimerefence(AVConsoleConnection& connection, const QStringList& args)
{
    if (args.size() != 1)
    {
        connection.printError("Enter date/time in format yyyy-MM-ddThh:mm:ss[.zzz] OR yyyMMddhhmmss[.zzz]"
                              "to set the global time reference");
        return;
    }

    AVDateTime dt;
    if (AVFromString(args[0], dt))
    {
        setCurrentDateTimeShiftedOnly(dt);

        connection.print("Timeref now is " + dt.toString("yyyyMMddhhmmss"));
    }
    else
    {
        connection.printError("Invalid date/time specified.");
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVTimeReferenceAdapterSharedMemory::slotCompleteTimestamp(
        QStringList& completions, const QStringList& args)
{
    if (args.size() != 1) return;
    QDateTime dt = AVTimeReference::currentDateTime();
    completions << dt.toString("yyyyMMddhhmmss");
}

///////////////////////////////////////////////////////////////////////////////

bool AVTimeReferenceAdapterSharedMemory::readParameters(QDateTime& reference_time, double& speed, qint64& offset)
{
    AVASSERT(!m_shared_memory.isNull());

    if (!m_shared_memory->lock())
    {
        AVLogError << "AVTimeReferenceAdapterSharedMemory::readParameters: Could not lock SHM";
        return false;
    }

    bool updated = false;

    if(m_shared_memory->isInitialized())
    {
        char *p = m_shared_memory->getSegment();
        if (p != 0)
        {
            int *time_offset_s_ptr = reinterpret_cast<int*>(p);
            p += sizeof(int);
            double *speed_ptr = reinterpret_cast<double*>(p);
            p += sizeof(double);
            unsigned long *global_resync_counter_ptr = reinterpret_cast<unsigned long*>(p);
            p += sizeof(unsigned long);
            int *time_offset_ms_ptr = reinterpret_cast<int*>(p);
            p += sizeof(int);
            char *reference_time_ptr = p;

            QByteArray reference_time_ba;
            reference_time_ba.setRawData(reference_time_ptr, 8);
            AVDataStream ds(&reference_time_ba, QIODevice::ReadOnly);
            ds >> reference_time;
            reference_time_ba.clear();

            speed = *speed_ptr;
            offset = (*time_offset_s_ptr) * static_cast<qint64>(1000) + (*time_offset_ms_ptr);
            ulong global_resync_counter = *global_resync_counter_ptr;
            if(global_resync_counter != m_last_read_resync_counter)
            {
                updated = true;
                m_last_read_resync_counter = global_resync_counter;
            }
        }
        else
        {
            AVLogError << "AVTimeReferenceAdapterSharedMemory::readParameters: No SHM segment!";
        }
    }

    m_shared_memory->unlock();

    return updated;
}

///////////////////////////////////////////////////////////////////////////////

void AVTimeReferenceAdapterSharedMemory::writeParameters(const QDateTime& reference_time, double speed, qint64 offset, bool need_to_lock_shm)
{
    AVASSERT(!m_shared_memory.isNull());

    if (need_to_lock_shm && !m_shared_memory->lock())
    {
        AVLogError << "AVTimeReferenceAdapterSharedMemory::writeParameters: Could not lock SHM";
        return;
    }

    char *p = m_shared_memory->getSegment();
    if (p != 0)
    {


        int *time_offset_s_ptr = reinterpret_cast<int*>(p);
        p += sizeof(int);
        double *speed_ptr = reinterpret_cast<double*>(p);
        p += sizeof(double);
        unsigned long *global_resync_counter_ptr = reinterpret_cast<unsigned long*>(p);
        p += sizeof(unsigned long);
        int *time_offset_ms_ptr = reinterpret_cast<int*>(p);
        p += sizeof(int);
        char *reference_time_ptr = p;

        QByteArray reference_time_ba;
        AVDataStream ds(&reference_time_ba, QIODevice::WriteOnly);
        ds << reference_time;
        memcpy(reference_time_ptr, reference_time_ba.data(), reference_time_ba.size());

        *speed_ptr = speed;
        *time_offset_s_ptr = static_cast<int>(offset/1000);
        *time_offset_ms_ptr = static_cast<int>(offset % 1000);

        if (m_shared_memory->isInitialized())
            m_last_read_resync_counter = *global_resync_counter_ptr;
        ++m_last_read_resync_counter;
        *global_resync_counter_ptr = m_last_read_resync_counter;

        m_shared_memory->setInitialized();
    }
    else
    {
        AVLogError << "AVTimeReferenceAdapterSharedMemory::writeParameters: No SHM segment!";
    }

    if (need_to_lock_shm)
        m_shared_memory->unlock();
}

///////////////////////////////////////////////////////////////////////////////

// End of file
