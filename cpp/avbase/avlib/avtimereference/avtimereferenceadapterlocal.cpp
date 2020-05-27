///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON
// Copyright: AviBit data processing GmbH, 2001-2014
// QT-Version: QT5
//
// Module:    AVLIB
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   AVTimeReferenceAdapterLocal implementation
 */

// Qt includes

// avlib includes
#include "avdeprecate.h"

// local includes
#include "avtimereferenceadapterlocal.h"
#include "avtimereferenceholder.h"

///////////////////////////////////////////////////////////////////////////////

AVTimeReferenceAdapterLocal::AVTimeReferenceAdapterLocal() :
    AVTimeReferenceAdapter(AdapterType::AdapterLocal)
{
    AVLogInfo << "AVTimeReferenceAdapterLocal::AVTimeReferenceAdapterLocal: instantiating AVTimeReferenceAdapterLocal";
    AVDEPRECATE(AVTimeReferenceAdapterLocal);
}

///////////////////////////////////////////////////////////////////////////////

AVTimeReferenceAdapterLocal::~AVTimeReferenceAdapterLocal()
{
}

///////////////////////////////////////////////////////////////////////////////

void AVTimeReferenceAdapterLocal::setSpeed(double speed)
{
    AVTimeReferenceAdapter::setProcessSpeed(speed);
}

///////////////////////////////////////////////////////////////////////////////

void AVTimeReferenceAdapterLocal::setCurrentDateTime(const QDateTime& current_date_time)
{
    AVTimeReferenceAdapter::setProcessDateTime(current_date_time);
}

///////////////////////////////////////////////////////////////////////////////

void AVTimeReferenceAdapterLocal::setCurrentDateTimeSpeed(const QDateTime& current_date_time, double speed)
{
    AVTimeReferenceAdapter::setProcessDateTimeSpeed(current_date_time, speed);
}

///////////////////////////////////////////////////////////////////////////////

void AVTimeReferenceAdapterLocal::setCurrentDateTimeShiftedOnly(const QDateTime& current_date_time)
{
    AVTimeReferenceAdapter::setProcessDateTimeShiftedOnly(current_date_time);
}

///////////////////////////////////////////////////////////////////////////////

void AVTimeReferenceAdapterLocal::startNewTimeReferenceSession()
{
    AVTimeReferenceAdapter::startNewTimeReferenceSessionProcess();
}

///////////////////////////////////////////////////////////////////////////////

void AVTimeReferenceAdapterLocal::reset()
{
    AVTimeReferenceAdapter::resetProcess();
}

// End of file
