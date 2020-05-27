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
    \brief   AVTimeReference header
 */

#ifndef AVTIMEREFERENCE_INCLUDED
#define AVTIMEREFERENCE_INCLUDED

// Qt includes
#include <QObject>

// avlib includes
#include "avlib_export.h"
#include "avmacros.h"

// local includes
#include "avexplicitsingleton.h"
#include "avtimereference/avtimereferenceholder.h"

// forward declarations

///////////////////////////////////////////////////////////////////////////////
//! Class for receiving time information in AviBit processes
/*!
*/
class AVLIB_EXPORT AVTimeReference : public QObject, public AVExplicitSingleton<AVTimeReference>
{
    Q_OBJECT
    AVDISABLECOPY(AVTimeReference);

    //! friend declaration for function level test case
    friend class TstAVTimeReference;
    friend class AVUnitTestTimeReferenceHolder;

    //! The only class which is allowed to modify the time reference:
    friend class AVTimeReferenceAdapter;

public:
    AVTimeReference();
    ~AVTimeReference() override;

    virtual const char *className() const { return "AVTimeReference"; }

    static AVTimeReference& initializeSingleton();

    static QDateTime currentDateTime();
    
    static QDateTime calculateFromSystemTime(const QDateTime& system_time);

    static double getSpeed();

    //! Returns an parameter stamp for the time reference.
    //! In contrast to the session the id also changes on time shifts.
    static QString getTimeReferenceParameterStamp();

    //! Returns the current offset to the system time
    static qint64 getDateTimeOffsetMs();

signals:
    //! Emitted when the time was changed (may also include a speed change)
    void signalTimeJumped(const QDateTime& from, const QDateTime& to);

    //! Emitted when speed changed, but time stays continuous
    void signalSpeedChanged(double from, double to);

    //! Emitted when the time was shifted only (the internal process state should remain)
    void signalTimeShiftedOnly(const QDateTime& from, const QDateTime& to);

private:
    //! Helper method, to be used in unit tests only. Used for overriding the current system time to avoid race conditions.
    //! See SWE-3959.
    void replaceTimeReferenceHolder(std::unique_ptr<AVTimeReferenceHolder> holder);
    //! Pointer indirection so AVUnitTestTimeReferenceHolder can be used instead.
    std::unique_ptr<AVTimeReferenceHolder> m_time_reference_holder;

};

#endif // AVTIMEREFERENCE_INCLUDED

// End of file
