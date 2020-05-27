///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Alexander Randeu, a.randeu@avibit.com
    \author  QT4-PORT: Anton Skrabal, a.skrabal@avibit.com
    \brief   a timer class, which can handle time speed changes from
             AVTimeReference
*/


#include <math.h>

// local includes
#include "avlog.h"
#include "avtimer.h"
#include "avtimereference.h"


///////////////////////////////////////////////////////////////////////////////

AVTimer::AVTimer(QObject* parent) :
    QObject(parent),
    m_timer(this),
    m_interval(0),
    m_speed(1.),
    m_need_restart(false)
{
    m_timer.setTimerType(Qt::PreciseTimer);

    connect(&m_timer, &QTimer::timeout, this, &AVTimer::slotInternalTimeout);

    connect(&AVTimeReference::singleton(), &AVTimeReference::signalSpeedChanged,
            this, &AVTimer::slotSpeedChanged);

    connect(&AVTimeReference::singleton(), &AVTimeReference::signalTimeJumped,
            this, &AVTimer::slotTimeJumped);

    connect(&AVTimeReference::singleton(), &AVTimeReference::signalTimeShiftedOnly,
            this, &AVTimer::slotTimeJumped);

    m_speed = AVTimeReference::getSpeed();
}

///////////////////////////////////////////////////////////////////////////////

AVTimer::~AVTimer()
{
}

///////////////////////////////////////////////////////////////////////////////

void AVTimer::start(int interval, bool single_shot)
{
    m_timer.setSingleShot(single_shot);
    m_interval = interval;
    start();
}

///////////////////////////////////////////////////////////////////////////////

void AVTimer::start()
{
    m_end_time = AVTimeReference::currentDateTime().addMSecs(m_interval);
    m_timer.setInterval(calculateScaledInterval(m_interval, m_speed));
    m_timer.start();
}

///////////////////////////////////////////////////////////////////////////////

void AVTimer::stop()
{
    m_timer.stop();
}

///////////////////////////////////////////////////////////////////////////////

int AVTimer::remainingTime() const
{
    if (isActive())
        return AVTimeReference::currentDateTime().msecsTo(m_end_time);
    return -1;
}

///////////////////////////////////////////////////////////////////////////////

int AVTimer::realRemainingTime() const
{
    if (m_speed == 0 || !isActive())
        return -1;
    return qRound(remainingTime() / m_speed);
}

///////////////////////////////////////////////////////////////////////////////

bool AVTimer::isActive() const
{
    return m_timer.isActive();
}

///////////////////////////////////////////////////////////////////////////////

void AVTimer::setInterval(int msec)
{
    m_interval = msec;
    if(isActive())
        start();
}

///////////////////////////////////////////////////////////////////////////////

qint32 AVTimer::getInterval() const
{
    return m_interval;
}

///////////////////////////////////////////////////////////////////////////////

void AVTimer::setSingleShot(bool single_shot)
{
    m_timer.setSingleShot(single_shot);
}

///////////////////////////////////////////////////////////////////////////////

bool AVTimer::isSingleShot() const
{
    return m_timer.isSingleShot();
}

///////////////////////////////////////////////////////////////////////////////

void AVTimer::slotInternalTimeout()
{
    emit timeout();

    if(!isSingleShot())
    {
        if(m_need_restart)
        {
            stop();
            start(); // sets m_end_time
        }
        else
        {
            m_end_time = AVTimeReference::currentDateTime().addMSecs(m_interval);
        }
    }
    m_need_restart = false;
}

///////////////////////////////////////////////////////////////////////////////

void AVTimer::slotSpeedChanged(double from, double to)
{
    // "from" can be inconsistent with m_from after a time jump (which emits both
    // signalTimeJumped and signalSpeedChanged) - ignore it. Also see SWE-4772.
    Q_UNUSED(from);

    if (qFuzzyCompare(to, m_speed))
    {
        AVLogDebug << "AVTimer::slotSpeedChanged: current speed=" << m_speed <<
            " is equal to updated speed. Skipping speed update.";
        return;
    }

    if(isActive())
    {
        qint32 remaining_old = m_timer.remainingTime();

        qint32 remaining_new = static_cast<qint32>(m_speed/to * remaining_old);

        m_timer.stop();
        m_timer.setInterval(remaining_new);
        m_timer.start();

        m_need_restart = true; // this resets the interval to the current scaled value
    }
    m_speed = to;
}

///////////////////////////////////////////////////////////////////////////////

void AVTimer::slotTimeJumped(const QDateTime& from, const QDateTime& to)
{
    if(!isActive())
        return;

    m_speed = AVTimeReference::getSpeed(); // may have changed too

    if(to >= m_end_time)
    {
        // emit and restart
        stop();
        start();
        emit timeout();
    }
    else if(to > from)
    {
        m_timer.stop();
        qint32 remaining = to.msecsTo(m_end_time);
        m_timer.setInterval(calculateScaledInterval(remaining, m_speed));
        m_timer.start();
        // m_end_time stays the same

        m_need_restart = true; // this resets the interval to the current scaled value
    }
    else // to < from
    {
        // simple restart
        stop();
        start();
    }
}

///////////////////////////////////////////////////////////////////////////////

qint32 AVTimer::calculateScaledInterval(qint32 interval, double speed)
{
    if (interval <= 0)       // immediately or in past -> trigger now
        return 0;
    if (qAbs(speed) < 1e-10) // speed is 0; start timer nonetheless
        return INT_MAX;

    return avroundtoint(interval/speed);
}

///////////////////////////////////////////////////////////////////////////////


// End of file
