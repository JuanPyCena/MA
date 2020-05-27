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
    \brief   AVTimeReferenceAdapter header
 */

#ifndef AVTIMEREFERENCEADAPTER_INCLUDED
#define AVTIMEREFERENCEADAPTER_INCLUDED

// Qt includes
#include <QObject>

// avlib includes
#include "avlib_export.h"
#include "avmacros.h"

// local includes
#include "avexplicitsingleton.h"

// forward declarations
class AVTimeReferenceHolder;
class AVReplayStateNotifier;

///////////////////////////////////////////////////////////////////////////////
//! Base class for adapters to receive timing information and set the AVTimeReference
/*! Only this class may set the timing information
*/
class AVLIB_EXPORT AVTimeReferenceAdapter : public QObject, public AVExplicitSingleton<AVTimeReferenceAdapter>
{
    Q_OBJECT
    AVDISABLECOPY(AVTimeReferenceAdapter);

    //! friend declaration for function level test case
    friend class TstAVTimeReferenceAdapter;

    friend class AVArchiveReplayer;  //DEPRECATED use only for access to time reference parameters
    friend class ReplayerController; //DEPRECATED use only for access to time reference parameters

public:

    enum class AdapterType {
        AdapterLocal            = 0,
        AdapterServer           = 1,
        AdapterServerController = 2,
        AdapterSharedMem        = 3
    };

    explicit AVTimeReferenceAdapter(AdapterType type);
    ~AVTimeReferenceAdapter() override;

    virtual const char* className() const { return "AVTimeReferenceAdapter"; }

    //! This sets a new speed at the time reference (also remotely, depending on the kind of adapter)
    virtual void setSpeed(double speed) = 0;

    //! This sets the current time at the time reference (also remotely, depending on the kind of adapter)
    virtual void setCurrentDateTime(const QDateTime& current_date_time) = 0;

    //! This sets the current time and speed at the time reference (also remotely, depending on the kind of adapter)
    virtual void setCurrentDateTimeSpeed(const QDateTime& current_date_time, double speed) = 0;

    //! This sets the current at the time reference without jumping for tests (also remotely, depending on the kind of adapter)
    virtual void setCurrentDateTimeShiftedOnly(const QDateTime& current_date_time) = 0;

    //! This starts a new time reference session without changing time or speed
    virtual void startNewTimeReferenceSession() = 0;

    //! Resets all time parameters to the current system time and closes the time reference session
    //! ATTENTION! If you want to change speed etc., you will need to start a new time reference session
    virtual void reset() = 0;

    virtual void registerConsoleSlots();

    AdapterType getAdapterType() { return m_adapter_type; }

    using AVExplicitSingleton<AVTimeReferenceAdapter>::setSingleton;

    /**
     * @brief setExitOnTimeJump is used to indicate that the process can deal with time jumps by resetting its state.
     *
     * Default is true - exit on time jump.
     *
     * Usually a process which sets this to false either
     * - has no state (e.g. HMIs) or
     * - connects to AVTimeReference::signalTimeJumped(), and resets the internal state in the slot implementation.
     *
     * Note that the AVTwinConnection2 clears the persistence if this is true, to provide a useful default behaviour.
     */
    void setExitOnTimeJump(bool exit);
    bool isExitOnTimeJump() const;

    //! This can be used to identify a "session" with same parameters (like former resync counter, see AVTimeReferenceHolder)
    QDateTime getReferenceDateTime() const;

    virtual QSharedPointer<AVReplayStateNotifier> getReplayStateNotifier();

private slots:
    void slotTimeJumped();


protected:

    //! This method allows derived classes to influence the AVTimeReference of this process
    void setProcessTimeReferenceParameters(const QDateTime& reference_date_time, double speed, qint64 offset);

    //! This method allows derived classes to influence the AVTimeReference of this process
    void setProcessDateTime(const QDateTime& time);

    //! This method allows derived classes to influence the AVTimeReference of this process
    void setProcessDateTimeSpeed(const QDateTime& time, double speed);

    //! This method allows derived classes to influence the AVTimeReference of this process
    void setProcessSpeed(double speed);

    //! This method allows derived classes to influence the AVTimeReference of this process
    void setProcessDateTimeShiftedOnly(const QDateTime& date_time);

    //! This method allows derived classes to influence the AVTimeReference of this process
    void startNewTimeReferenceSessionProcess();

    //! This method allows derived classes to influence the AVTimeReference of this process
    void resetProcess();

    const AVTimeReferenceHolder& getLocalTimeReferenceHolder() const;


private:
    bool m_exit_on_time_jump;

    AdapterType m_adapter_type;

};

#endif // AVTIMEREFERENCEADAPTER_INCLUDED

// End of file
