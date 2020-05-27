///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2011
//
// Module:     AVLIB - AviBit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author    DI Alexander Wemmer <a.wemmer@avibit.com>
    \author    QT4-PORT: Christian Muschick, c.muschick@avibit.com
    \brief     Template class for implicit object sharing
*/


#ifndef __AVSHARE_H__
#define __AVSHARE_H__

#include <QString>
#include <QMutex>

// local includes
#include "avlib_export.h"
#include "avmacros.h"

/////////////////////////////////////////////////////////////////////////////
//! Policy class for AVShare allowing empty AVShare instances.
//! Returns 0 as initial object pointer so no default constructor
//! is required. Performs no initial check of the held pointer
//! but checks for a null-pointer on each access.
template <class T> struct AVShareInitPolicyNull
{
    static T* defaultInit()
    {
        return 0;
    }

    static bool checkInitSharedObject(T* )
    {
        return true;
    }

    static bool checkAccessSharedObject(T* object)
    {
        return (object != 0);
    }
};

/////////////////////////////////////////////////////////////////////////////

//! Policy class for AVShare not allowing empty AVShare instances.
//! Uses the default constructor to create the initial object pointer so the
//! type has to provide a default constructor.
//! Only performs the initial check of the held pointer but no checks for
//! a null-pointer on each access.
template <class T> struct AVShareInitPolicyDefault
{
    static T* defaultInit()
    {
        T* instance = new T;
        AVASSERTNOLOGGER(instance != 0);

        return instance;
    }

    static bool checkInitSharedObject(T* object)
    {
        return (object != 0);
    }

    static bool checkAccessSharedObject(T* )
    {
        return true;
    }
};

/////////////////////////////////////////////////////////////////////////////

//! Default Logger Policy that does not log anything
/*! This policy shall be used if AVShare is used within AVLog.

    Example for adifferent policy that does use AVLogger:

    \code

    class AVShareAVLoggerPolicy
    {
    public:
        static void logDebug2(const QString& msg)
        {
            AVLogger->Write(LOG_DEBUG2, "%s", qPrintable(msg));
        }
    };

    \endcode
 */
class AVLIB_EXPORT AVShareEmptyLoggerPolicy
{
public:

    static void logDebug2(const QString&) { }

    static void logDebug2(const QString& /*format*/,
                          const QString& /*param1 = QString::null*/)
    { }

    static void logDebug2(const QString& /*format*/,
                          const QString& /*param1 = QString::null*/,
                          uint /*param2 = 0*/)
    { }

    static void logDebug2(const QString& /*format*/,
                          const QString& /*param1 = QString::null*/,
                          uint /*param2 = 0*/,
                          const QString& /*param3 = QString::null*/,
                          uint /*param4 = 0*/)
    { }
};

/////////////////////////////////////////////////////////////////////////////
template <typename T,
          template <class> class DefaultInitPolicy,
          typename AVShareLoggerPolicy>
class AVShareInternalStruct
{
    typedef DefaultInitPolicy<T> CDefaultInitPolicy;
public:

    //! constructor
    AVShareInternalStruct() : m_shared_object(0)
    {
        AVShareLoggerPolicy::logDebug2("AVShareInternalStruct(std):");

        // init object to share
        m_shared_object = CDefaultInitPolicy::defaultInit();
        setup();
    }

    //! creates an object to be shared via AVShare.
    //! "name" maybe set to ease logging and debugging.
    explicit AVShareInternalStruct(const QString& name) : m_shared_object(0)
    {
        AVShareLoggerPolicy::logDebug2("AVShareInternalStruct(name): " + name);

        // init object to share
        m_shared_object = CDefaultInitPolicy::defaultInit();
        m_name = name;

        setup();
    }

    //! creates an object to be shared via AVShare.
    //! "name" maybe set to ease logging and debugging.
    explicit AVShareInternalStruct(T* object, const QString& name) : m_shared_object(0)
    {
        AVShareLoggerPolicy::logDebug2("AVShareInternalStruct(ptr): " + name);

        // init object to share
        m_shared_object = object;
        m_name = name;

        setup();
    }

    //! destructor
    virtual ~AVShareInternalStruct()
    {
        AVShareLoggerPolicy::logDebug2("~AVShareInternalStruct~: " + m_name);
        AVASSERTNOLOGGER(m_ref_count == 0);
        delete m_shared_object;
        delete m_mutex;
    }

    //-----

    //! returns a const pointer to the shared object
    const T* getSharedObject() const
    {
        AVASSERTNOLOGGER(CDefaultInitPolicy::checkAccessSharedObject(m_shared_object));
        return m_shared_object;
    }

    //! returns a pointer to the shared object
    T* getSharedObject()
    {
        AVASSERTNOLOGGER(CDefaultInitPolicy::checkAccessSharedObject(m_shared_object));
        return m_shared_object;
    }

    //! Returns the number of references
    unsigned int getRefCount() { return m_ref_count; }

    //! Increases the number of references by one
    void incRefCount() { ++m_ref_count; }

    //! Decreases the number of references by one
    void decRefCount() { --m_ref_count; }

    //! Returns the name of this object
    void setName(const QString& name) { m_name = name; }

    //! Returns the name of this object
    const QString& getName() const { return m_name; }

    //! wheter we should use locking or not
    void setLocking(bool do_locking)
    {
        m_do_locking = do_locking;

        if (m_do_locking)
        {
            if(!m_mutex) m_mutex = new QMutex();
            AVASSERTNOLOGGER(m_mutex);
        }
        else
        {
            delete m_mutex;
            m_mutex = 0;
        }
    }

    //! Return true if we use locking, false otherwise
    bool getLocking() const { return m_do_locking; }

    //! Locks the internal mutex
    //! \note This method will return immediately if locking is disabled
    void lock() const
    {
        if (!m_do_locking)
            qFatal("AVShareInternalStruct:lock: %s: "
                   "locking is not enabled", qPrintable(m_name));
        AVASSERTNOLOGGER(m_mutex);
        m_mutex->lock();
    }

    //! Unlocks the internal mutex
    //! \note This method will return immediately if locking is disabled
    void unlock() const
    {
        if (!m_do_locking)
            qFatal("AVShareInternalStruct:lock: %s: "
                   "locking is not enabled", qPrintable(m_name));
        AVASSERTNOLOGGER(m_mutex);
        m_mutex->unlock();
    }

    //! Returns whether the held pointer is null
    bool isNull() const
    {
        return (m_shared_object == 0);
    }

    //! Returns whether the held pointers are equal
    bool compareSharedObject(const AVShareInternalStruct<T,
                             DefaultInitPolicy,
                             AVShareLoggerPolicy>& other) const
    {
         return (m_shared_object == other.m_shared_object);
    }

protected:

    //! the shared object
    T* m_shared_object;

    //! reference counting for the shared object
    unsigned int m_ref_count;

    //! name of the object used for
    QString m_name;

    //! whether we should used locking or not
    bool m_do_locking;

    //! may be used to make access threadsafe.
    //! we only instantiate the mutex if we really need it
    mutable QMutex* m_mutex;

    //-----

    void setup()
    {
        AVASSERTNOLOGGER(CDefaultInitPolicy::checkInitSharedObject(m_shared_object));
        m_ref_count = 1;
        m_do_locking = false;
        m_mutex = NULL;
    }

private:

    //! copy constructor
    AVShareInternalStruct(const AVShareInternalStruct<T,
                                                      DefaultInitPolicy,
                                                      AVShareLoggerPolicy>& other);

    //! assign operator
    AVShareInternalStruct<T, DefaultInitPolicy, AVShareLoggerPolicy>& operator=(
        const AVShareInternalStruct<T, DefaultInitPolicy, AVShareLoggerPolicy>& other);
};

/////////////////////////////////////////////////////////////////////////////

//! The AVShare class is a template class for implicit object sharing.
//! \note For thread-safe operations, the shared object may be locked.
template <typename T,
          typename AVShareLoggerPolicy = AVShareEmptyLoggerPolicy,
          template <class> class DefaultInitPolicy = AVShareInitPolicyDefault>
class AVShare
{
    typedef DefaultInitPolicy<T> CDefaultInitPolicy;
public:

    //! Constructor
    AVShare()
    {
        AVShareLoggerPolicy::logDebug2("AVShare(std):");

        m_shared_object_struct =
                new AVShareInternalStruct<T, DefaultInitPolicy, AVShareLoggerPolicy>();
        AVASSERTNOLOGGER(m_shared_object_struct);
    }

    //! Constructor
    //! "name" may be set to ease logging and debugging.
    explicit AVShare(const QString name)
    {
        AVShareLoggerPolicy::logDebug2("AVShare(name): " + name);

        m_shared_object_struct =
               new AVShareInternalStruct<T, DefaultInitPolicy, AVShareLoggerPolicy>(name);
        AVASSERTNOLOGGER(m_shared_object_struct);
    }

    //! Constructor
    //! "object" is the object to be shared.
    //! \note The given object will be referenced internally and must NOT be
    //! deleted externally!!
    //! "name" may be set to ease logging and debugging.
    AVShare(T* object, const QString name)
    {
        AVShareLoggerPolicy::logDebug2("AVShare(ptr): " + name);

        m_shared_object_struct =
                new AVShareInternalStruct<T, DefaultInitPolicy, AVShareLoggerPolicy>(object, name);
        AVASSERTNOLOGGER(m_shared_object_struct);
    }

    //! Copy constructor
    AVShare(const AVShare<T, AVShareLoggerPolicy, DefaultInitPolicy>& other)
    {
        if (other.getLocking()) other.lock();

        AVASSERTNOLOGGER(other.m_shared_object_struct != 0);

        m_shared_object_struct = other.m_shared_object_struct;
        m_shared_object_struct->incRefCount();
        AVShareLoggerPolicy::logDebug2("AVShare(AVShare): %s: refcount:%d",
                                       qPrintable(m_shared_object_struct->getName()),
                                       m_shared_object_struct->getRefCount());

        if (other.getLocking()) other.unlock();
    }

    //! Destructor
    //! If the reference count of the enclosed shared object reaches 0, the
    //! object will be deleted.
    /*! If getLocking() is true, the shared_object_struct will be locked thus not 2
        or more threads can change the reference counter at the same time. At the
        end of the destructor the shared_object_struct will be unlocked (for the case
        the reference counter is not zero)
     */
    virtual ~AVShare()
    {
        if (getLocking()) lock();

        m_shared_object_struct->decRefCount();

        if (m_shared_object_struct->getRefCount() == 0)
        {
            AVShareLoggerPolicy::logDebug2("~AVShare: %s: no remaining "
                                           "references - deleting object",
                                           qPrintable(m_shared_object_struct->getName()));

            if (getLocking()) unlock();
            delete m_shared_object_struct;
        }
        else
        {
            AVShareLoggerPolicy::logDebug2("~AVShare: %s: object has %d "
                                           "references - no deletion",
                                           qPrintable(m_shared_object_struct->getName()),
                                           m_shared_object_struct->getRefCount());
            if (getLocking()) unlock();
        }
    }

    //-----

    //! assign operator
    const AVShare<T, AVShareLoggerPolicy, DefaultInitPolicy>*
    operator=(const AVShare<T, AVShareLoggerPolicy, DefaultInitPolicy>* other)
    {
        if (other->getLocking()) other->lock();

        AVShareLoggerPolicy::logDebug2("AVShare:operator=(*): %s: refcount:%d "
                                       "/ other: %s: refcount: %d",
                                       qPrintable(m_shared_object_struct->getName()),
                                       m_shared_object_struct->getRefCount(),
                                       qPrintable(other->m_shared_object_struct->getName()),
                                       other->m_shared_object_struct->getRefCount());

        AVASSERTNOLOGGER(other->m_shared_object_struct != 0);

        other->m_shared_object_struct->incRefCount();
        m_shared_object_struct->decRefCount();

        if (m_shared_object_struct->getRefCount() == 0)
        {
            AVShareLoggerPolicy::logDebug2("AVShare:operator=: %s: deleting our "
                                           "shared object",
                                           qPrintable(m_shared_object_struct->getName()));
            delete m_shared_object_struct;
        }

        m_shared_object_struct = other->m_shared_object_struct;

        if (other->getLocking()) other->unlock();

        return this;
    }

    //! assign operator
    const AVShare<T, AVShareLoggerPolicy, DefaultInitPolicy>&
    operator=(const AVShare<T, AVShareLoggerPolicy, DefaultInitPolicy>& other)
    {
        if (other.getLocking()) other.lock();

        AVShareLoggerPolicy::logDebug2("AVShare:operator=(&): %s: refcount:%d "
                                       "/ other: %s: refcount: %d",
                                       qPrintable(m_shared_object_struct->getName()),
                                       m_shared_object_struct->getRefCount(),
                                       qPrintable(other.m_shared_object_struct->getName()),
                                       other.m_shared_object_struct->getRefCount());

        AVASSERTNOLOGGER(other.m_shared_object_struct != 0);
        other.m_shared_object_struct->incRefCount();

        m_shared_object_struct->decRefCount();
        if (m_shared_object_struct->getRefCount() == 0)
        {
            AVShareLoggerPolicy::logDebug2("AVShare:operator=: %s: deleting our "
                                           "shared object",
                                           qPrintable(m_shared_object_struct->getName()));
            delete m_shared_object_struct;
        }

        m_shared_object_struct = other.m_shared_object_struct;

        if (other.getLocking()) other.unlock();

        return *this;
    }

    //! const pointer access operator
    const T* operator->() const
    {
        return m_shared_object_struct->getSharedObject();
    }

    //! pointer access operator
    T* operator->()
    {
        return m_shared_object_struct->getSharedObject();
    }

    const T& operator*() const { return getShare(); }
    T&       operator*()       { return getShare(); }

    //! eqality operator
    inline bool operator==(const AVShare<T, AVShareLoggerPolicy, DefaultInitPolicy>& other) const
    {
         return (m_shared_object_struct->compareSharedObject(*other.m_shared_object_struct));
    }

    //! inequality operator
    inline bool operator!=(const AVShare<T, AVShareLoggerPolicy, DefaultInitPolicy>& other) const
    {
        return (!m_shared_object_struct->compareSharedObject(*other.m_shared_object_struct));
    }

    //-----

    //! const reference access operator
    const T& getShare() const
    {
        AVShareLoggerPolicy::logDebug2("AVShare:operator()(const): %s:",
                                       qPrintable(m_shared_object_struct->getName()));

        return *(m_shared_object_struct->getSharedObject());
    }

    //! reference access operator
    T& getShare()
    {
        AVShareLoggerPolicy::logDebug2("AVShare:operator(): %s:",
                                       qPrintable(m_shared_object_struct->getName()));

        return *(m_shared_object_struct->getSharedObject());
    }

    //-----

    //! Returns the number of references
    unsigned int getRefCount() const { return m_shared_object_struct->getRefCount(); }

    //! Wheter we should use locking or not
    void setLocking(bool do_locking) { m_shared_object_struct->setLocking(do_locking); }

    //! Return true if we use locking, false otherwise
    bool getLocking() const { return m_shared_object_struct->getLocking(); }

    //! Locks the shared object's lock. Locking must be enabled with setLocking() !
    void lock() const { m_shared_object_struct->lock(); }

    //! Locks the shared object's lock. Locking must be enabled with setLocking() !
    void unlock() const { m_shared_object_struct->unlock(); }

    //! Checks if the shared object is null
    bool isNull() const { return m_shared_object_struct->isNull(); }

protected:
    //! class holding the shared object and additional information
    AVShareInternalStruct<T, DefaultInitPolicy, AVShareLoggerPolicy>* m_shared_object_struct;
};

/////////////////////////////////////////////////////////////////////////////

#endif /* __AVSHARE_H__ */

// End of file
