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
    \brief   AVTimeReferenceAdapterSharedMemory header
 */

#ifndef AVTIMEREFERENCEADAPTERSHAREDMEMORY_INCLUDED
#define AVTIMEREFERENCEADAPTERSHAREDMEMORY_INCLUDED

// Qt includes
#include <QObject>
#include <QScopedPointer>
#include <QMutex>
#include <QTimer>

// avlib includes
#include "avlib_export.h"
#include "avmacros.h"

// local includes
#include "avtimereferenceadapter.h"

// forward declarations
class AVSharedMemory;
class AVConsoleConnection;

///////////////////////////////////////////////////////////////////////////////
//!  Time reference adapter for receiving time information from Shared Memory
/*!
 *
 * The shared memory block is built as follows:
 * int time_offset_s The integer part of the offset
 * double global_speed
 * unsigned long global_resync_counter
 * int time_offset_ms The fractional part of the offset in ms
 * QDateTime reference_time
 *
*/
class AVLIB_EXPORT AVTimeReferenceAdapterSharedMemory : public AVTimeReferenceAdapter
{
    Q_OBJECT
    AVDISABLECOPY(AVTimeReferenceAdapterSharedMemory);

    //! friend declaration for function level test case
    friend class TstAVTimeReferenceAdapterSharedMemory;

public:
    AVTimeReferenceAdapterSharedMemory();
    ~AVTimeReferenceAdapterSharedMemory() override;

    void setSpeed(double speed) override;

    void setCurrentDateTime(const QDateTime& current_date_time) override;

    void setCurrentDateTimeSpeed(const QDateTime& current_date_time, double speed) override;

    void setCurrentDateTimeShiftedOnly(const QDateTime& current_date_time) override;

    void startNewTimeReferenceSession() override;

    void reset() override;

    void registerConsoleSlots() override;

public slots:
    //! retrieves the time reference and updates it locally, if applicable
    void slotCheckTimeReference();


    //! Console function: sets the timeref for the current application
    void slotConsoleSetTimerefence(AVConsoleConnection& connection, const QStringList& args);

    //! Console complete function
    void slotCompleteTimestamp(QStringList& completions, const QStringList& args);

private:
    //! This also increases the resync counter
    void writeParameters(const QDateTime& reference_time, double speed, qint64 offset, bool need_to_lock_shm = true);
    //! This also checks the resync counter and returns true if changed
    bool readParameters(QDateTime& reference_time, double& speed, qint64& offset);

private:

    static const uint DEFAULT_POLLING_INTERVAL;

    QScopedPointer<AVSharedMemory> m_shared_memory;

    ulong m_last_read_resync_counter;

    QTimer m_polling_timer;

};

#endif // AVTIMEREFERENCEADAPTERSHAREDMEMORY_INCLUDED

// End of file
