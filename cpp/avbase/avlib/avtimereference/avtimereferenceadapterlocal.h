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
    \brief   AVTimeReferenceAdapterLocal header
 */

#ifndef AVTIMEREFERENCEADAPTERLOCAL_INCLUDED
#define AVTIMEREFERENCEADAPTERLOCAL_INCLUDED

// Qt includes
#include <QObject>

// avlib includes
#include "avlib_export.h"
#include "avmacros.h"

// local includes
#include "avtimereferenceadapter.h"

// forward declarations

///////////////////////////////////////////////////////////////////////////////
//! An adapter for only local interaction with time reference
/*!
 *  This can be used e.g. for deprecated behaviour if a process receives its time via a dedicated message.
*/
class AVLIB_EXPORT AVTimeReferenceAdapterLocal : public AVTimeReferenceAdapter
{
    Q_OBJECT
    AVDISABLECOPY(AVTimeReferenceAdapterLocal);

    //! friend declaration for function level test case
    friend class TstAVTimeReferenceAdapterLocal;

public:
    AVTimeReferenceAdapterLocal();
    ~AVTimeReferenceAdapterLocal() override;

    void setSpeed(double speed) override;

    void setCurrentDateTime(const QDateTime& current_date_time) override;

    void setCurrentDateTimeSpeed(const QDateTime& current_date_time, double speed) override;

    void setCurrentDateTimeShiftedOnly(const QDateTime& current_date_time) override;

    void startNewTimeReferenceSession() override;

    void reset() override;

private:

};

#endif // AVTIMEREFERENCEADAPTERLOCAL_INCLUDED

// End of file
