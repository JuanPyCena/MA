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
    \brief   AVTimeReferenceAdapter implementation
 */

// Qt includes
#include <QSharedPointer>

// avlib includes
#include "avtimereference.h"
#include "avmacros.h"
#include "avdaemon.h"

// local includes
#include "avtimereferenceadapter.h"

///////////////////////////////////////////////////////////////////////////////

AVTimeReferenceAdapter::AVTimeReferenceAdapter(AdapterType type) :
    m_exit_on_time_jump(false), m_adapter_type(type)
{
    if(!AVTimeReference::isSingletonInitialized())
    {
        AVLogFatal << "AVTimeReferenceAdapter::AVTimeReferenceAdapter: AVTimeReference must be initialized before AVTimeReferenceAdapter";
    }

    connect(&AVTimeReference::singleton(), &AVTimeReference::signalTimeJumped, this, &AVTimeReferenceAdapter::slotTimeJumped);

}

///////////////////////////////////////////////////////////////////////////////

AVTimeReferenceAdapter::~AVTimeReferenceAdapter()
{
}

///////////////////////////////////////////////////////////////////////////////

void AVTimeReferenceAdapter::registerConsoleSlots()
{
}

///////////////////////////////////////////////////////////////////////////////

void AVTimeReferenceAdapter::setExitOnTimeJump(bool exit)
{
    m_exit_on_time_jump = exit;
}

///////////////////////////////////////////////////////////////////////////////

bool AVTimeReferenceAdapter::isExitOnTimeJump() const
{
    return m_exit_on_time_jump;
}

///////////////////////////////////////////////////////////////////////////////

QDateTime AVTimeReferenceAdapter::getReferenceDateTime() const
{
    return getLocalTimeReferenceHolder().getReferenceDateTime();
}

///////////////////////////////////////////////////////////////////////////////

QSharedPointer<AVReplayStateNotifier> AVTimeReferenceAdapter::getReplayStateNotifier()
{
    AVLogWarning << "AVTimeReferenceAdapter::getReplayStateNotifier: this adapter has no notifier, returning 0";
    return QSharedPointer<AVReplayStateNotifier>();
}

///////////////////////////////////////////////////////////////////////////////

void AVTimeReferenceAdapter::slotTimeJumped()
{
    if(m_exit_on_time_jump)
    {
        AVLogInfo << "AVTimeReferenceAdapter::slotTimeJumped: terminating process due to detected time jump.";
        AVAPPDAEMONQUIT();
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVTimeReferenceAdapter::setProcessTimeReferenceParameters(const QDateTime& reference_date_time, double speed, qint64 offset)
{
    if(AVTimeReference::singleton().m_time_reference_holder->setTimeReferenceParameters(reference_date_time, speed, offset))
    {
        AVLogInfo << "AVTimeReferenceAdapter::setProcessTimeReferenceParameters: changed time reference: current time "
                <<AVTimeReference::currentDateTime()<<", speed: "<<AVTimeReference::getSpeed();
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVTimeReferenceAdapter::setProcessDateTime(const QDateTime& time)
{
    AVTimeReference::singleton().m_time_reference_holder->setDateTime(time);

    AVLogInfo << "AVTimeReferenceAdapter::setProcessDateTime: changed time reference: current time "
            <<AVTimeReference::currentDateTime()<<", speed: "<<AVTimeReference::getSpeed();
}

///////////////////////////////////////////////////////////////////////////////

void AVTimeReferenceAdapter::setProcessDateTimeSpeed(const QDateTime& time, double speed)
{
    AVTimeReference::singleton().m_time_reference_holder->setDateTimeSpeed(time, speed);

    AVLogInfo << "AVTimeReferenceAdapter::setProcessDateTimeSpeed: changed time reference: current time "
            <<AVTimeReference::currentDateTime()<<", speed: "<<AVTimeReference::getSpeed();
}

///////////////////////////////////////////////////////////////////////////////

void AVTimeReferenceAdapter::setProcessSpeed(double speed)
{
    AVTimeReference::singleton().m_time_reference_holder->setSpeed(speed);

    AVLogInfo << "AVTimeReferenceAdapter::setProcessSpeed: changed speed: current time "
            <<AVTimeReference::currentDateTime()<<", speed: "<<AVTimeReference::getSpeed();
}

///////////////////////////////////////////////////////////////////////////////

void AVTimeReferenceAdapter::setProcessDateTimeShiftedOnly(const QDateTime& date_time)
{
    AVTimeReference::singleton().m_time_reference_holder->setDateTimeShiftedOnly(date_time);

    AVLogInfo << "AVTimeReferenceAdapter::setProcessDateTimeSpeedShiftedOnly: changed time reference (shifted only): current time "
            <<AVTimeReference::currentDateTime()<<", speed: "<<AVTimeReference::getSpeed();
}

///////////////////////////////////////////////////////////////////////////////

void AVTimeReferenceAdapter::startNewTimeReferenceSessionProcess()
{
    AVTimeReference::singleton().m_time_reference_holder->startNewTimeReferenceSession();

    AVLogInfo << "AVTimeReferenceAdapter::startNewTimeReferenceSessionProcess: started new time reference session";
}

///////////////////////////////////////////////////////////////////////////////

void AVTimeReferenceAdapter::resetProcess()
{
    AVTimeReference::singleton().m_time_reference_holder->resetToSystemTime();

    AVLogInfo << "AVTimeReferenceAdapter::resetProcess: reset time reference: current time "
            <<AVTimeReference::currentDateTime()<<", speed: "<<AVTimeReference::getSpeed();
}

///////////////////////////////////////////////////////////////////////////////

const AVTimeReferenceHolder& AVTimeReferenceAdapter::getLocalTimeReferenceHolder() const
{
    return *AVTimeReference::singleton().m_time_reference_holder.get();
}

///////////////////////////////////////////////////////////////////////////////

// End of file
