///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2011
//
// Module:     AVLIB - AviBit Library
//
/////////////////////////////////////////////////////////////////////////////


/*! \file
    \author  Armin Berger, a.berger@avibit.com
    \author  Andreas Schuller, a.schuller@avibit.com
    \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   Template class for creating singleton objects per thread
*/

#ifndef __AVTHREADSINGLETON_H__
#define __AVTHREADSINGLETON_H__

// QT includes
#include <QThreadStorage>

// AVLib includes
#include "avlog.h"

//! The AVThreadSingleton class is a template class for creating singleton objects * per * thread.
/*!
 *  AVThreadSingleton<T> maintains a instance of \c T for every thread. This is in contrast to
 *  the AVSingleton<T> implementation, which ensures that there is only one instance of \c T for
 *  the whole application.
 *
 *  Apart from this importance difference the class API is identical to that of AVSingleton<T>. It
 *  should always be possible to replace the classes by each other.
 *
 *  The AVThreadSingleton comes in place where singletons are the root cause of
 *  explicitly/implicitly shared objects across thread boundaries. For instance, this will be the
 *  case, if a prototype factory singleton is used in multiple threads and no provisions are made to
 *  provide a deep copy of the prototype in a thread-safe manner.
 *
 *   \attention Changes to the state of a singleton will not affect the local
 *   copies of the other threads. Due to the QThreadStorage implementation the number
 *   of different QThreadStorage objects is limited to 256 (i.e. the number of different
 *   AVSingleton instances is limited to 256), whereas the number of threads per
 *   AVSingleton instance is unlimited.
*/

///////////////////////////////////////////////////////////////////////////////

template<typename T>
class AVThreadSingleton
{
    AVDISABLECOPY(AVThreadSingleton);
public:
    //! Returns true if this singleton already has an instance.
    static bool hasInstance()
    {
        InstanceHandle* handle = m_thread_instances.localData();

        if (handle == 0) return false;

        return handle->m_instance != 0;
    }

    //! Creates a new instance of the singleton class T.
    /*! A possibly existing previous instance must be destroyed first!
     */
    static void createInstance(void)
    {
        InstanceHandle* handle = getInstanceHandle();
        AVASSERT(handle != 0);
        AVASSERT(handle->m_instance == 0);
        handle->m_instance = new T;
        AVASSERT(handle->m_instance != 0);
    }

    //! Creates a new instance of the singleton class T.
    /*! \param param1 Template parameter that will be passed to the constructor of T.
        A possibly existing previous instance must be destroyed first!
     */
    template <typename ParamType1>
    static void createInstance(ParamType1 param1)
    {
        InstanceHandle* handle = getInstanceHandle();
        AVASSERT(handle != 0);
        AVASSERT(handle->m_instance == 0);
        handle->m_instance = new T(param1);
        AVASSERT(handle->m_instance != 0);
    }

    //! Creates a new instance of the singleton class T.
    /*! \param param1 Template parameter that will be passed to the constructor of T
               as the first argument.
        \param param2 Template parameter that will be passed to the constructor of T
               as the second argument.
        A possibly existing previous instance must be destroyed first!
     */
    template <typename ParamType1, typename ParamType2>
    static void createInstance(ParamType1 param1, ParamType2 param2)
    {
        InstanceHandle* handle = getInstanceHandle();
        AVASSERT(handle != 0);
        AVASSERT(handle->m_instance == 0);
        handle->m_instance = new T(param1, param2);
        AVASSERT(handle->m_instance != 0);
    }

    //! Creates a new instance of the singleton class T.
    /*! \param param1 Template parameter that will be passed to the constructor of T
               as the first argument.
        \param param2 Template parameter that will be passed to the constructor of T
               as the second argument.
        \param param3 Template parameter that will be passed to the constructor of T
               as the third argument.
        A possibly existing previous instance must be destroyed first!
     */
    template <typename ParamType1, typename ParamType2, typename ParamType3>
    static void createInstance(ParamType1 param1, ParamType2 param2, ParamType3 param3)
    {
        InstanceHandle* handle = getInstanceHandle();
        AVASSERT(handle != 0);
        AVASSERT(handle->m_instance == 0);
        handle->m_instance = new T(param1, param2, param3);
        AVASSERT(handle->m_instance != 0);
    }

    //! Returns a pointer to the instance of the singleton class T.
    /*!
        \return A pointer to the instance of the singleton class.
        If there is no instance of this class yet, one will be created.
    */
    static T* getInstance()
    {
        InstanceHandle* handle = getInstanceHandle();
        AVASSERT(handle != 0);

        if (handle->m_instance == 0)
        {
            handle->m_instance = new T();
            AVASSERT(handle->m_instance != 0);
        }

        return handle->m_instance;
    }

    //! Returns a reference to the instance of the singleton class T.
    /*!
        \return A reference to the instance of the singleton class.
        If there is no instance of this class yet, one will be created.
    */
    static T& getInstanceRef()
    {
        InstanceHandle* handle = getInstanceHandle();
        AVASSERT(handle != 0);

        if (handle->m_instance == 0)
        {
            handle->m_instance = new T();
            AVASSERT(handle->m_instance != 0);
        }

        return *handle->m_instance;
    }

    //! Destroys the singleton class instance.
    /*!
        Be aware that all references to the single class instance will be
        invalid after this method has been called!
    */
    static void destroyInstance()
    {
        InstanceHandle* handle = getInstanceHandle();
        AVASSERT(handle != 0);
        deleteInstance(handle->m_instance);
    }

protected:
    // shield the constructor and destructor to prevent outside sources
    // from creating or destroying a AVSingleton instance.

    //! Hidden Default constructor
    AVThreadSingleton(){}

    //! Hidden Destructor
    virtual ~AVThreadSingleton() {}

private:
    class Handle
    {
        friend class QThreadStorage<Handle*>;
    public:
       Handle() : m_instance(0) {}
       ~Handle() { deleteInstance(m_instance); }
       T* m_instance;
    };

    typedef Handle InstanceHandle;

private:
    static InstanceHandle* getInstanceHandle()
    {
        InstanceHandle* local_data = 0;

        if (!m_thread_instances.hasLocalData())
        {
            local_data = new Handle;
            AVASSERT(local_data != 0);
            m_thread_instances.setLocalData(local_data);
        }
        else
        {
            local_data = m_thread_instances.localData();
        }

        return local_data;
    }

    static void deleteInstance(T*& instance){ delete instance; instance = 0; }

private:
    static QThreadStorage<Handle*> m_thread_instances;
};

///////////////////////////////////////////////////////////////////////////////

template <typename T>
    QThreadStorage<typename AVThreadSingleton<T>::Handle*>
        AVThreadSingleton<T>::m_thread_instances;

#endif /* __AVTHREADSINGLETON_H__ */

// End of file
