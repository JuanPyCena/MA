///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// QT-Version: QT5
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Andreas Schuller
    \author  QT4-PORT: Christian Muschick, c.muschick@avibit.com
    \brief   Implementation of AVObserver and AVObservable
 */


// Qt includes
#include <qmutex.h>
#include <QList>

// AVLib
#include "avlog.h"
#include "avobserver.h"
#include "avmisc.h"
#include "avshare.h"


///////////////////////////////////////////////////////////////////////////////

class ObserverHandle
{
public:
    ObserverHandle() : id(0), observer(0), activeNotifactions(0)  {}
    explicit ObserverHandle(AVObserver* instance)
        : id(instance), observer(instance), activeNotifactions(0)
    {}
    void*               id;
    AVObserver*         observer;
    QMutex              mutex;
    int                 activeNotifactions;
    static void* getId(AVObserver* ptr) { return ptr; }
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class ObserverSharedHandle : public AVShare<ObserverHandle>
{
public:
    ObserverSharedHandle();

    explicit ObserverSharedHandle(AVObserver* instance);

    ObserverSharedHandle(const ObserverSharedHandle& other);

    ObserverSharedHandle& operator=(const ObserverSharedHandle& other);

    ~ObserverSharedHandle() override;
};

///////////////////////////////////////////////////////////////////////////////

ObserverSharedHandle::ObserverSharedHandle()
    : AVShare<ObserverHandle>(new (LOG_HERE) ObserverHandle(), QString())
{
    setLocking(true);
}

///////////////////////////////////////////////////////////////////////////////

ObserverSharedHandle::ObserverSharedHandle(AVObserver* instance)
    : AVShare<ObserverHandle>(new (LOG_HERE) ObserverHandle(instance), QString())
{
    setLocking(true);
}

///////////////////////////////////////////////////////////////////////////////

ObserverSharedHandle::ObserverSharedHandle(const ObserverSharedHandle& other)
    : AVShare<ObserverHandle>(other)
{}

///////////////////////////////////////////////////////////////////////////////

ObserverSharedHandle& ObserverSharedHandle::operator=(const ObserverSharedHandle& other)
{
    if (&other == this) return *this;
    AVShare<ObserverHandle>::operator=(other);
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

ObserverSharedHandle::~ObserverSharedHandle()
{
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class AVObserver::Data
{
public:
    explicit Data(AVObserver* observer)
        : m_ref(observer) {}

    ObserverSharedHandle m_ref;
};

///////////////////////////////////////////////////////////////////////////////

AVObserver::AVObserver()
    : m_d(0)
{
    m_d = new Data(this);
    AVASSERT(m_d != 0);
}

///////////////////////////////////////////////////////////////////////////////

AVObserver::~AVObserver()
{
    m_d->m_ref->mutex.lock();

    m_d->m_ref->observer = 0;

    if (m_d->m_ref->activeNotifactions > 0)
    {
        AVLogError << "AVObserver::~AVObserver[" << QString().sprintf("%p", this) << "]: "
                   << "object is destroyed while processing notifications "
                   << "in another thread context. Unsubscribe the observer before it get "
                   << "deleted to fix this error!";
    }

    m_d->m_ref->mutex.unlock();

    delete m_d;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class AVObservable::Data
{
public:
    QMutex m_mutex;
    typedef QList<ObserverSharedHandle> ObserverList;
    ObserverList m_observers;
};

///////////////////////////////////////////////////////////////////////////////

AVObservable::AVObservable()
    : m_d(0)
{
    m_d = new Data;
    AVASSERT(m_d != 0);
}

///////////////////////////////////////////////////////////////////////////////

AVObservable::~AVObservable()
{
    delete m_d;
}

///////////////////////////////////////////////////////////////////////////////

void AVObservable::subscribe(AVObserver* observer)
{
    AVASSERT(observer != 0);

    QMutexLocker locker(&m_d->m_mutex);

    Data::ObserverList::iterator it  = m_d->m_observers.begin();
    Data::ObserverList::iterator end = m_d->m_observers.end();
    while (it != end)
    {
        ObserverSharedHandle& ref = *it;
        // use the subscription also for clean-up of already gone observers
        if (ref->observer == 0)
        {
            it = m_d->m_observers.erase(it);
            continue;
        }

        AVASSERT(ref->id != ObserverHandle::getId(observer));
        ++it;
    }

    // qDebug("AVObservable(%p)::subscribe: %p", this, observer);

    m_d->m_observers.push_back(observer->m_d->m_ref);
}
///////////////////////////////////////////////////////////////////////////////

void AVObservable::unsubscribe(AVObserver* observer)
{
    AVASSERT(observer != 0);

    QMutexLocker locker(&m_d->m_mutex);

    Data::ObserverList::iterator it  = m_d->m_observers.begin();
    Data::ObserverList::iterator end = m_d->m_observers.end();
    for (; it != end; ++it)
    {
        ObserverSharedHandle& ref = *it;
        if (ref->id == ObserverHandle::getId(observer))
        {
            m_d->m_observers.erase(it);
            return;
        }
    }

    AVLogDebug << "Observable::unsubscribe: observer "
                   << QString().sprintf("%p", observer)
                   << " is not subscribed - nothing to do";
}

///////////////////////////////////////////////////////////////////////////////

void AVObservable::unsubscribeAll()
{
    QMutexLocker locker(&m_d->m_mutex);
    m_d->m_observers.clear();
}

///////////////////////////////////////////////////////////////////////////////

void AVObservable::notifyObservers(const AVObservableEvent& msg)
{
    Data::ObserverList observers;
    {
        QMutexLocker locker(&m_d->m_mutex);
        observers = m_d->m_observers;
    }

    msg.m_sender = this;

    Data::ObserverList::iterator it  = observers.begin();
    Data::ObserverList::iterator end = observers.end();
    for (; it != end; ++it)
    {
        ObserverSharedHandle& ref = *it;

        AVObserver* observer = 0;

        {
            QMutexLocker ref_locker(&ref->mutex);

            // Check if observer still exists
            if (ref->observer == 0)
            {
                continue;
            }

            observer = ref->observer;

            ++ref->activeNotifactions;
        }

        observer->gotNotification(msg);

        ref->mutex.lock();
        --ref->activeNotifactions;
        ref->mutex.unlock();
    }
}

///////////////////////////////////////////////////////////////////////////////

int AVObservable::countObservers() const
{
    QMutexLocker locker(&m_d->m_mutex);
    return m_d->m_observers.count();
}

///////////////////////////////////////////////////////////////////////////////

// End of file
