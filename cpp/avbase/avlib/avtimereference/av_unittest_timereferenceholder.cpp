///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    TESTS - AVCOMMON Unit Tests
//
/////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Christian Muschick, c.muschick@avibit.com
*/

#include "av_unittest_timereferenceholder.h"

#include "avtimereference.h"

///////////////////////////////////////////////////////////////////////////////

AVUnitTestTimeReferenceHolder::AVUnitTestTimeReferenceHolder()
{

}

///////////////////////////////////////////////////////////////////////////////

AVUnitTestTimeReferenceHolder::AVUnitTestTimeReferenceHolder(const AVTimeReferenceHolder &other) :
    AVTimeReferenceHolder(other)
{
    AVASSERT(m_system_time_override.isNull());
}

///////////////////////////////////////////////////////////////////////////////

AVUnitTestTimeReferenceHolder::~AVUnitTestTimeReferenceHolder()
{

}

///////////////////////////////////////////////////////////////////////////////

void AVUnitTestTimeReferenceHolder::setSystemTimeOverride(const QDateTime &override)
{
    m_system_time_override = override;
}

///////////////////////////////////////////////////////////////////////////////

void AVUnitTestTimeReferenceHolder::resetSystemTimeOverride()
{
    m_system_time_override = AVDateTime();
    AVASSERT(m_system_time_override.isNull());
}

///////////////////////////////////////////////////////////////////////////////

void AVUnitTestTimeReferenceHolder::setAVTimeReferenceSystemTimeOverride(const QDateTime &override)
{
    // we don't know if the data type is OK yet; for simplicity replace it right away instead of doing a dynamic cast and perhaps replacing it.
    std::unique_ptr<AVUnitTestTimeReferenceHolder> holder = std::make_unique<AVUnitTestTimeReferenceHolder>(
                *AVTimeReference::singleton().m_time_reference_holder.get());
    holder->setSystemTimeOverride(override);
    AVTimeReference::singleton().replaceTimeReferenceHolder(std::move(holder));
}

///////////////////////////////////////////////////////////////////////////////

void AVUnitTestTimeReferenceHolder::resetAVTimeReferenceSystemTimeOverride()
{
    setAVTimeReferenceSystemTimeOverride(AVDateTime());
}

///////////////////////////////////////////////////////////////////////////////

QDateTime AVUnitTestTimeReferenceHolder::getSystemTime() const
{
    if (m_system_time_override.isNull()) return AVDateTime::currentDateTimeUtc();
    else return m_system_time_override;
}


//End of File
