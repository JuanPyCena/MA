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
    \brief   AVTimeReferenceHolder implementation
 */

// Qt includes
#include <QReadLocker>
#include <QWriteLocker>

// avlib includes
#include "avlog.h"
#include "avmisc.h"
#include "avdatetime.h"

// local includes
#include "avtimereferenceholder.h"

///////////////////////////////////////////////////////////////////////////////

const double AVTimeReferenceHolder::SPEED_ACCURACY = 1E-12;
const qint64 AVTimeReferenceHolder::TIME_ACCURACY_MSECS = 3;

///////////////////////////////////////////////////////////////////////////////

AVTimeReferenceHolder::AVTimeReferenceHolder() :
    m_speed(1.),
    m_offset(0)
{
}

///////////////////////////////////////////////////////////////////////////////

AVTimeReferenceHolder::AVTimeReferenceHolder(const AVTimeReferenceHolder &other) :
    QObject(),
    m_reference_date_time(other.m_reference_date_time),
    m_speed(other.m_speed),
    m_offset(other.m_offset)
{

}

///////////////////////////////////////////////////////////////////////////////

AVTimeReferenceHolder::~AVTimeReferenceHolder()
{
}

///////////////////////////////////////////////////////////////////////////////

QDateTime AVTimeReferenceHolder::currentDateTime() const
{
    return calculateFromSystemTime(getSystemTime());
}

///////////////////////////////////////////////////////////////////////////////

qint64 AVTimeReferenceHolder::getDateTimeOffsetMs()
{
    return getSystemTime().msecsTo(currentDateTime());
}

///////////////////////////////////////////////////////////////////////////////

QDateTime AVTimeReferenceHolder::calculateFromSystemTime(const QDateTime& system_time) const
{
    // get current time and correct it by the offset
    QDateTime dt(system_time);

    QReadLocker locker(&m_read_write_lock);

    if (m_reference_date_time.isValid())
    {
        qint64 msecs_since_reference = m_reference_date_time.msecsTo(dt);

        double speed_increment_d = msecs_since_reference * m_speed;

        qint64 speed_increment = static_cast<qint64>(speed_increment_d);

        dt = m_reference_date_time.addMSecs(speed_increment);
    }
    else
    {
        AVASSERT(m_speed == 1.0); // when a speed is not 1, a reference time must be set
    }

    // apply offset to real time to the simulated time
    dt = dt.addMSecs(m_offset);

    return dt;
}

///////////////////////////////////////////////////////////////////////////////

qint64 AVTimeReferenceHolder::calculateOffset(const QDateTime& target_simulated_time, const QDateTime& reference_date_time, const QDateTime& system_time, double speed)
{
    qint64 offset_correction = reference_date_time.msecsTo(system_time) * speed;

    qint64 offset = reference_date_time.msecsTo(target_simulated_time) - offset_correction;

    return offset;
}

///////////////////////////////////////////////////////////////////////////////

QDateTime AVTimeReferenceHolder::getReferenceDateTime() const
{
    QReadLocker locker(&m_read_write_lock);
    return m_reference_date_time;
}

///////////////////////////////////////////////////////////////////////////////

QString AVTimeReferenceHolder::getTimeReferenceParameterStamp() const
{
    // rounds speed to two decimal places, to avoid floating point issues
    const QString speed = QString::number(getSpeed(), 'f', 2);
    // It is OK if the reference date time is uninitialized (null) - this can happen e.g. if no simulated time has
    // yet been set, or if only time shifts occured.
    return AVToString(getReferenceDateTime()) + "O" + AVToString(getOffset()) + "S" + speed;
}

///////////////////////////////////////////////////////////////////////////////

double AVTimeReferenceHolder::getSpeed() const
{
    QReadLocker locker(&m_read_write_lock);
    return m_speed;
}

///////////////////////////////////////////////////////////////////////////////

qint64 AVTimeReferenceHolder::getOffset() const
{
    QReadLocker locker(&m_read_write_lock);
    return m_offset;
}

///////////////////////////////////////////////////////////////////////////////

bool AVTimeReferenceHolder::setTimeReferenceParameters(const QDateTime& reference_date_time, double speed, qint64 offset)
{
    QDateTime system_time(getSystemTime());
    QDateTime before = calculateFromSystemTime(system_time);
    QDateTime old_time_at_new_reference_time = calculateFromSystemTime(reference_date_time);

    QWriteLocker locker(&m_read_write_lock);

    bool speed_changed = !AVFLOATEQ(speed, m_speed, SPEED_ACCURACY);
    bool reference_time_changed = m_reference_date_time.isNull() != reference_date_time.isNull() ||
            m_reference_date_time != reference_date_time;
    bool offset_changed = m_offset != offset;
    if(!speed_changed && !reference_time_changed && ! offset_changed) // no parameters changed
        return false;

    double old_speed = m_speed;
    QDateTime old_reference_date_time = m_reference_date_time;
    m_speed = speed;

    m_reference_date_time = reference_date_time;
    m_offset = offset;

    locker.unlock(); // must be unlocked here, so that emitting signals do not run into locks

    bool time_changed = false;
    if(!reference_date_time.isNull()) // use reference time to determine time change (same as used on server)
    {
        QDateTime new_time_at_new_reference_time = calculateFromSystemTime(reference_date_time);
        time_changed = qAbs(old_time_at_new_reference_time.msecsTo(new_time_at_new_reference_time))
            > TIME_ACCURACY_MSECS;
    }
    else if (!old_reference_date_time.isNull()) // a reset took place (now null)
    {
        time_changed = true; // jump if reference date time was resetted
    }
    else // both old and new reference are null, check if offset changed
    {
        time_changed = offset_changed;
    }

    QDateTime after = calculateFromSystemTime(system_time);
    if(reference_time_changed) // if reference time changes -> jump, else shift
        emit signalTimeJumped(before, after);
    else if(time_changed)
        emit signalTimeShiftedOnly(before, after);
    if(speed_changed)
        emit signalSpeedChanged(old_speed, speed);

    return true;
}

///////////////////////////////////////////////////////////////////////////////

void AVTimeReferenceHolder::setSpeed(double speed)
{
    if(AVFLOATEQ(speed, m_speed, SPEED_ACCURACY))
        return;

    QDateTime system_time(getSystemTime());
    QDateTime current = calculateFromSystemTime(system_time);

    QWriteLocker locker(&m_read_write_lock);

    if(m_reference_date_time.isNull())
    {
        AVLogError << "AVTimeReferenceHolder::setSpeed: Cannot set speed without time refererence session, "
                      "did you call setDateTime() before?";
        return;
    }

    double old_speed = m_speed;
    m_speed = speed;

    // a speed change leaves the sesssion the same, the reference time stays constant
    // the offset is corrected. using the following formula, calculateDateTime gives now the same time as before the speed change
    m_offset = calculateOffset(current, m_reference_date_time, system_time, m_speed);

    locker.unlock(); // must be unlocked here, so that emitting signals do not run into locks

    emit signalSpeedChanged(old_speed, speed);
}

///////////////////////////////////////////////////////////////////////////////

void AVTimeReferenceHolder::setDateTime(const QDateTime& date_time)
{
    QDateTime system_time(getSystemTime());
    QDateTime current = calculateFromSystemTime(system_time);

    QWriteLocker locker(&m_read_write_lock);

    // the reference time is set to the current system time
    updateReferenceDateTime(system_time);

    // the offset is set, so that the time gives the new date_time
    m_offset = calculateOffset(date_time, m_reference_date_time, system_time, m_speed);


    locker.unlock(); // must be unlocked here, so that emitting signals do not run into locks

    emit signalTimeJumped(current, date_time);
}

///////////////////////////////////////////////////////////////////////////////

void AVTimeReferenceHolder::setDateTimeSpeed(const QDateTime& date_time, double speed)
{
    QDateTime system_time(getSystemTime());
    QDateTime current = calculateFromSystemTime(system_time);

    QWriteLocker locker(&m_read_write_lock);

    bool speed_changed = !AVFLOATEQ(speed, m_speed, SPEED_ACCURACY);

    double old_speed = m_speed;
    m_speed = speed;
    // the reference time is set to the current system time
    updateReferenceDateTime(system_time);

    // the offset is set, so that the time gives the new date_time
    m_offset = calculateOffset(date_time, m_reference_date_time, system_time, m_speed);

    locker.unlock(); // must be unlocked here, so that emitting signals do not run into locks

    emit signalTimeJumped(current, date_time);
    if(speed_changed)
        emit signalSpeedChanged(old_speed, speed);
}

///////////////////////////////////////////////////////////////////////////////

void AVTimeReferenceHolder::setDateTimeShiftedOnly(const QDateTime& date_time)
{
    QDateTime system_time(getSystemTime());
    QDateTime current = calculateFromSystemTime(system_time);

    qint64 offset = current.msecsTo(date_time);

    QWriteLocker locker(&m_read_write_lock);

    if(m_reference_date_time.isNull())
    {
        AVLogError << "AVTimeReferenceHolder::setDateTimeShiftedOnly: Cannot shift time without time refererence session, "
                      "did you call setDateTime() before?";
        return;
    }

    m_offset += offset;

    locker.unlock(); // must be unlocked here, so that emitting signals do not run into locks

    emit signalTimeShiftedOnly(current, date_time);
}

///////////////////////////////////////////////////////////////////////////////

void AVTimeReferenceHolder::startNewTimeReferenceSession()
{
    QDateTime system_time(getSystemTime());
    QDateTime current = calculateFromSystemTime(system_time);

    QWriteLocker locker(&m_read_write_lock);

    updateReferenceDateTime(system_time);
    // this sets the offset, so that the actual time stays the same
    m_offset = calculateOffset(current, m_reference_date_time, system_time, m_speed);


    locker.unlock(); // must be unlocked here, so that emitting signals do not run into locks

    emit signalTimeJumped(current, current);
}

///////////////////////////////////////////////////////////////////////////////

void AVTimeReferenceHolder::resetToSystemTime()
{
    QDateTime system_time = getSystemTime();
    QDateTime current = calculateFromSystemTime(system_time);

    QWriteLocker locker(&m_read_write_lock);

    bool speed_changed = !AVFLOATEQ(1. , m_speed, SPEED_ACCURACY);
    bool reference_time_changed = !m_reference_date_time.isNull();
    bool offset_changed = m_offset != 0;

    double old_speed = m_speed;
    m_speed = 1.;
    m_reference_date_time = AVDateTime();
    m_offset = 0;

    locker.unlock(); // must be unlocked here, so that emitting signals do not run into locks

    if(reference_time_changed)
        emit signalTimeJumped(current, system_time);
    else if (offset_changed)
        emit signalTimeShiftedOnly(current, system_time);
    if(speed_changed)
        emit signalSpeedChanged(old_speed, 1.);
}

///////////////////////////////////////////////////////////////////////////////

QDateTime AVTimeReferenceHolder::getSystemTime() const
{
    return AVDateTime::currentDateTimeUtc();
}

///////////////////////////////////////////////////////////////////////////////
void AVTimeReferenceHolder::updateReferenceDateTime(const QDateTime& now)
{
    if(now <= m_reference_date_time)
    {
        m_reference_date_time = m_reference_date_time.addMSecs(1);
    }
    else
    {
        m_reference_date_time = now;
    }
}

///////////////////////////////////////////////////////////////////////////////

// End of file
