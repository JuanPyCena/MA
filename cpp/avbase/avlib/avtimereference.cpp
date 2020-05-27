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
    \brief   AVTimeReference implementation
 */

// Qt includes

// avlib includes

// local includes
#include "avtimereference.h"

///////////////////////////////////////////////////////////////////////////////

AVTimeReference::AVTimeReference()
{
    replaceTimeReferenceHolder(std::make_unique<AVTimeReferenceHolder>());
}

///////////////////////////////////////////////////////////////////////////////

AVTimeReference::~AVTimeReference()
{
}

///////////////////////////////////////////////////////////////////////////////

AVTimeReference& AVTimeReference::initializeSingleton()
{
    AVTimeReference* time_reference = new AVTimeReference();
    AVTimeReference::setSingleton(time_reference);
    return *time_reference;
}

///////////////////////////////////////////////////////////////////////////////

QDateTime AVTimeReference::currentDateTime()
{
    return AVTimeReference::singleton().m_time_reference_holder->currentDateTime();
}

///////////////////////////////////////////////////////////////////////////////

QDateTime AVTimeReference::calculateFromSystemTime(const QDateTime& system_time)
{
    return AVTimeReference::singleton().m_time_reference_holder->calculateFromSystemTime(system_time);
}

///////////////////////////////////////////////////////////////////////////////

double AVTimeReference::getSpeed()
{
    return AVTimeReference::singleton().m_time_reference_holder->getSpeed();
}

///////////////////////////////////////////////////////////////////////////////

QString AVTimeReference::getTimeReferenceParameterStamp()
{
    return AVTimeReference::singleton().m_time_reference_holder->getTimeReferenceParameterStamp();
}

///////////////////////////////////////////////////////////////////////////////

qint64 AVTimeReference::getDateTimeOffsetMs()
{
    return AVTimeReference::singleton().m_time_reference_holder->getDateTimeOffsetMs();
}

///////////////////////////////////////////////////////////////////////////////

void AVTimeReference::replaceTimeReferenceHolder(std::unique_ptr<AVTimeReferenceHolder> holder)
{
    m_time_reference_holder = std::move(holder);
    connect(m_time_reference_holder.get(), &AVTimeReferenceHolder::signalSpeedChanged,
            this, &AVTimeReference::signalSpeedChanged);
    connect(m_time_reference_holder.get(), &AVTimeReferenceHolder::signalTimeJumped,
            this, &AVTimeReference::signalTimeJumped);
    connect(m_time_reference_holder.get(), &AVTimeReferenceHolder::signalTimeShiftedOnly,
            this, &AVTimeReference::signalTimeShiftedOnly);
}

// End of file
