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
    \brief   AVTimeReferenceHolder header
 */

#ifndef AVTIMEREFERENCEHOLDER_INCLUDED
#define AVTIMEREFERENCEHOLDER_INCLUDED

// Qt includes
#include <QObject>
#include <QDateTime>
#include <QReadWriteLock>

// avlib includes
#include "avlib_export.h"
#include "avmacros.h"
#include "avdatetime.h"

// local includes

// forward declarations

///////////////////////////////////////////////////////////////////////////////
//! This class holds the parameters for time reference calculation
/*!
 *
 *  The time is calculated as follows
 *  t = t_0 + (t_sys-t_0)*v + o
 *  where
 *  t_sys... system time
 *  t_0..... reference time
 *  v....... speed
 *  o....... offset
 *  t....... avibit time
 *
 *  The three parameters are mathematically one more than necessary. But for sake of rounding problems and large integers
 *  it is better to have a reference time.
 *
 *  The reference time can be used to "identify" the current "session" of parameters. Since it is always set to the actual
 *  system time, the sessions are uniquely identified.
 *
 *  A time jump always creates a new session (new reference time).
 *
 *  Using the setDateTimeShiftedOnly, no new session is generated. It can be used for tests withoud need for a time jump.
 *  In this case, the reference time may also be null.
 *
 *  A speed change also creates no new session, but the offset is changed.
 *
 *  The signals signalTimeJumped and signalSpeedChanged are emitted if detected accordingly. Exception:
 *  signalTimeJumped is not emitted, if the reference time is unchanged
 *
*/
class AVLIB_EXPORT AVTimeReferenceHolder : public QObject
{
    Q_OBJECT

    //! friend declaration for function level test case
    friend class TstAVTimeReferenceHolder;

public:
    AVTimeReferenceHolder();
    explicit AVTimeReferenceHolder(const AVTimeReferenceHolder& other);
    ~AVTimeReferenceHolder() override;

    QDateTime currentDateTime() const;
    //! Returns the current offset to the system time
    qint64 getDateTimeOffsetMs();

    /**
     * @brief calculateFromSystemTime computes the current value of the time reference based on an arbitrary system time.
     *
     * @param system_time The current datetime (usually obtained via AVDateTime::currentDateTimeUtc()).
     * @return            The time as managed by this class, with applied speed and offset.
     */
    QDateTime calculateFromSystemTime(const QDateTime& system_time) const;

    QDateTime getReferenceDateTime() const;
    double getSpeed() const;
    qint64 getOffset() const;

    //! Returns an parameter stamp for the time reference.
    //! In contrast to the session the id also changes on time shifts.
    QString getTimeReferenceParameterStamp() const;

    //! This emits singalSpeedChanged and/or signalTimeJumped, if applicable
    bool setTimeReferenceParameters(const QDateTime& reference_date_time, double speed, qint64 offset);

    //! This changes the speed, but keeps the current time the same
    //! This emits singalSpeedChanged, if applicable
    void setSpeed(double speed);

    //! This emits signalTimeJumped, if applicable
    void setDateTime(const QDateTime& date_time);

    //! This emits singalSpeedChanged and/or signalTimeJumped, if applicable
    void setDateTimeSpeed(const QDateTime& date_time, double speed);

    //! This does only a shift, no signal is emitted (used for tests, e.g.)
    void setDateTimeShiftedOnly(const QDateTime& date_time);

    //! This starts a new time reference session without changing time or speed
    void startNewTimeReferenceSession();

    //! Resets all parameters, so that system time is returned at speed 1
    //! This emits signalSpeedChanged and/or signalTimeJumped, if applicable
    void resetToSystemTime();

    static const double SPEED_ACCURACY;            //!< Used to check, whether the signal shall be emitted
    static const qint64 TIME_ACCURACY_MSECS;       //!< Used to check, whether the signal shall be emitted

signals:

    //! Emitted when the time was changed (may also include a speed change)
    void signalTimeJumped(const QDateTime& from, const QDateTime& to);

    //! Emitted when speed changed, but time stays continuous
    void signalSpeedChanged(double from, double to);

    //! Emitted when the time was shifted only (the internal process state should remain)
    void signalTimeShiftedOnly(const QDateTime& from, const QDateTime& to);

private:

    //! Updates the reference time by the given date_time (usually current system time)
    //! if the given time is equal to the current reference time it will be increased by 1 ms
    //! to avoid that system-time calls consecutively result into the same reference time
    void updateReferenceDateTime(const QDateTime& date_time);

    //! Calculating the offset between target_simulated_time and reference_date_time but taking into account
    //! that system_time and reference_date_time may not be equal
    qint64 calculateOffset(const QDateTime& target_simulated_time, const QDateTime& reference_date_time, const QDateTime& system_time, double speed);

    //! Virtual for unit test purposes.
    virtual QDateTime getSystemTime() const;

    AVDateTime m_reference_date_time;  //!< The reference time, for start of calculating the current time with speed
    double m_speed;                    //!< The current speed
    qint64 m_offset;                   //!< An additional static offset

    mutable QReadWriteLock m_read_write_lock;
};

#endif // AVTIMEREFERENCEHOLDER_INCLUDED

// End of file
