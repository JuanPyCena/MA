///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2011
//
// Module:     AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Thomas Neidhart, t.neidhart@avibit.com
    \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   A thread to poll process state entries.
*/

#if !defined(AVPROCESSSTATEPOLLER_H_INCLUDED)
#define AVPROCESSSTATEPOLLER_H_INCLUDED

// QT includes
#include <QEvent>

// AVlib includes
#include "avlib_export.h"
#include "avcvariant.h"
#include "avthread.h"

// forward declarations
class AVProcessStateDesc;

///////////////////////////////////////////////////////////////////////////////

//! AVProcessStatePoller class
/*! This class is used to poll a specified param value of a process state
    and emit a signal if it has changed.
    \see PollingEvent for possible signal values.
*/
class AVLIB_EXPORT AVProcessStatePoller : public QObject, public AVThread
{
    Q_OBJECT
public:
    enum PollingEvent { Initial,   //!< initial value
                        Changed,   //!< value changed
                        Invalid    //!< parameter not valid
    };

    /*! \param process_name The name of the process to poll. Leave empty to poll the own process.
        \param param The parameter name to be polled.
        \param interval the polling interval (in msec), default = 5000 msec
        \param heartbeat_timeout Consider the variable to be invalid if the SHM was not
                                 updated for this time (in msec, 0 to disable).
     */
    AVProcessStatePoller(const QString& process_name, const QString& param,
                         uint interval = 5000, uint heartbeat_timeout = 0);
    /*! Same as above, but with a predefined process state descriptor.
     *  \param desc The process state descriptor to use for polling the SHM. Note that ownership of
     *             the desc object will *not* be claimed.
     *  \param param The parameter name to be polled.
     *  \param interval The polling interval (in msec), default = 5000 msec
     *  \param heartbeat_timeout Consider the variable to be invalid if the SHM was not
     *                           updated for this time (in msec, 0 to disable).
     */
    AVProcessStatePoller(AVProcessStateDesc& desc, const QString& param,
                         uint interval = 5000, uint heartbeat_timeout = 0);

    ~AVProcessStatePoller() override;

signals:
    //! \param event \see PollingEvent
    //! \param new_value The new value in SHM. Only use this if event is not Invalid.
    void signalShmVarChanged(AVProcessStatePoller::PollingEvent event, const AVCVariant& new_value);

protected:
    void runThread() override;
    void stopThread() override;

private:
    //! Avoid accidential use of previous mechanism by making this private
    using AVThread::connectNotification;

    class NotifyEvent : public QEvent
    {
    public:
        NotifyEvent(PollingEvent event, const AVCVariant& value) :
            QEvent(NOTIFY_EVENT_TYPE), m_event(event), m_value(value) {}


        static const QEvent::Type NOTIFY_EVENT_TYPE;

        PollingEvent m_event;
        AVCVariant m_value;
    };

    //! Retrieves the value of the variable from SHM, checking whether the process is still alive.
    //! \param value [out] Will be set to the retrieved variable on success.
    //! \return False if the variable is invalid (not present or process dead), true otherwise.
    bool getShmValue(AVCVariant& value);
    void sendShmNotification(PollingEvent event, const AVCVariant& value);
    //! Internal event filter routine which receives the NotifyEvent and dispatches the signals.
    bool event(QEvent* e) override;

    AVProcessStatePoller(const AVProcessStatePoller&);
    AVProcessStatePoller& operator=(const AVProcessStatePoller&);

    AVProcessStateDesc *m_process_desc;      //!< The PSD for the polled process.
                                             //!< \see m_delete_proc_state
    bool                m_delete_proc_desc;  //!< Whether to delete m_process_desc (depends on
                                             //!< whether we're polling the own process).
    QString             m_param_name;        //!< The name of the polled SHM variable
    uint                m_polling_interval;  //!< The polling interval in milliseconds.
    uint                m_heartbeat_timeout; //!< The polled variable is considered to be invalid if
                                             //!< the SHM of the polled process is not updated for
                                             //!< this long (in milliseconds).
};

#endif

// End of file
