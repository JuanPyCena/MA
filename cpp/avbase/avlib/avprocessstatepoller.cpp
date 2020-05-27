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


// QT includes
#include <QApplication>
#include <QEvent>

// local includes
#include "avdatetime.h"
#include "avprocessstate.h"
#include "avprocessstatedesc.h"
#include "avprocessstatepoller.h"
#include "avlog.h"


///////////////////////////////////////////////////////////////////////////////

const QEvent::Type AVProcessStatePoller::NotifyEvent::NOTIFY_EVENT_TYPE=
        static_cast<QEvent::Type>(QEvent::registerEventType());

///////////////////////////////////////////////////////////////////////////////

//parasoft suppress item OOP-08
AVProcessStatePoller::AVProcessStatePoller(const QString& process_name,
                                           const QString& param,
                                           uint interval,
                                           uint heartbeat_timeout) :
    AVThread("AVProcessStatePoller"),
    m_process_desc(0),
    m_delete_proc_desc(false),
    m_param_name(param),
    m_polling_interval(interval),
    m_heartbeat_timeout(heartbeat_timeout)
{
    AVASSERT(AVProcState != 0);

    if (process_name.isEmpty()) {
        m_process_desc = AVProcState->getProcessStateDesc();
        if (m_process_desc == 0) {
            LOGGER_ROOT.Write(LOG_FATAL, "AVProcessStatePoller::setPollerParams: "
                              "Process name must be set to query own SHM variables.");
        }
        m_delete_proc_desc = false;
    } else {
        m_process_desc = AVProcState->getProcessStateDesc(process_name);
        m_delete_proc_desc = true;
    }
    AVASSERT(m_process_desc != 0);
}
//parasoft on

///////////////////////////////////////////////////////////////////////////////

AVProcessStatePoller::AVProcessStatePoller(AVProcessStateDesc& desc, const QString& param,
                                           uint interval, uint heartbeat_timeout) :
    AVThread("AVProcessStatePoller"),
    m_process_desc(&desc),
    m_delete_proc_desc(false),
    m_param_name(param),
    m_polling_interval(interval),
    m_heartbeat_timeout(heartbeat_timeout)
{
}

///////////////////////////////////////////////////////////////////////////////

AVProcessStatePoller::~AVProcessStatePoller()
{
    stop();
    if (m_delete_proc_desc) delete m_process_desc;
}

///////////////////////////////////////////////////////////////////////////////

void AVProcessStatePoller::runThread()
{
    AVCVariant initial;
    bool success;
    // first try to get an initial value
    success = getShmValue(initial);

    bool invalid = false;
    // if we got a valid value, send notification
    if (success)
    {
        AVLogger->Write(LOG_DEBUG2,
                        "AVProcessStatePoller:runThread: "
                        "Success: sending Initial");

        sendShmNotification(Initial, initial);
    }
    else
    {
        AVLogger->Write(LOG_DEBUG2,
                        "AVProcessStatePoller:runThread: "
                        "No Success: sending Invalid");

        sendShmNotification(Invalid, AVCVariant());
        invalid = true;
    }

    AVCVariant newValue;
    while (!checkForStop())
    {
        success = getShmValue(newValue);

        if (success)
        {
            if (invalid)
            {
                AVLogger->Write(LOG_DEBUG2,
                                "AVProcessStatePoller:runThread: "
                                "Invalid: sending Changed");

                initial = newValue;
                sendShmNotification(Initial, initial);
                sendShmNotification(Changed, initial);
                invalid = false;
                continue;
            }

            // if the value has changed, send a notification
            if (!initial.compare(newValue)) {
                initial = newValue;
                sendShmNotification(Changed, newValue);
            }

        } else {
            // only send the invalid signal once
            if (!invalid) {
                sendShmNotification(Invalid, AVCVariant());
                invalid = true;
            }
        }

        sleepMs(m_polling_interval);
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVProcessStatePoller::stopThread()
{
}

///////////////////////////////////////////////////////////////////////////////

bool AVProcessStatePoller::getShmValue(AVCVariant& value)
{
    if (m_heartbeat_timeout != 0) {
        AVDateTime dt(m_process_desc->getTimeStamp());
        if (!dt.isValid()) return false;
        AVDateTime now(AVProcessStateDesc::currentDateTime());
        if (dt.msecsTo(now) > static_cast<int>(m_heartbeat_timeout)) return false;
    }

    return m_process_desc->getParameter(m_param_name, value);
}

///////////////////////////////////////////////////////////////////////////////

void AVProcessStatePoller::sendShmNotification(PollingEvent event, const AVCVariant& value)
{
    NotifyEvent *notifyEvent = new NotifyEvent(event, value);
    AVASSERT(notifyEvent != 0);
    QApplication::postEvent(this, notifyEvent);
}

///////////////////////////////////////////////////////////////////////////////

bool AVProcessStatePoller::event(QEvent *e)
{
    AVASSERT(static_cast<int>(e->type()) == NotifyEvent::NOTIFY_EVENT_TYPE);
    NotifyEvent *event = dynamic_cast<NotifyEvent*>(e);
    AVASSERT(event != 0);

    emit signalShmVarChanged(event->m_event, event->m_value);

    return true;
}

// end of file
