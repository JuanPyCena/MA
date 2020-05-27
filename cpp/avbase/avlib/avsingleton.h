///////////////////////////////////////////////////////////////////////////////
//
// Package:   TBS
// Copyright: AviBit data processing GmbH, 2001-2011
//
// Module:    AVLIB - AviBit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Stefan Kunz, s.kunz@avibit.com
    \author  QT4-PORT: Christian Muschick, c.muschick@avibit.com
    \brief   Template class for creating singleton objects
*/


#ifndef __AVSINGLETON_H__
#define __AVSINGLETON_H__

// STL includes
#include <atomic>

// QT includes
#include <QMutex>

// AVLib includes
#include "avlog.h"
#include "avdaemon.h"


//! The AVSingleton class is a template class for creating singleton objects.
/*! When the static getInstance() method is called for the first time,
    the singleton object is created using the objects' default constructor.
    Every sequential call returns a reference to this instance.
    Alternatively the singleton object can be created already in advance by
    using createInstance(). The instance can be destroyed by calling the
    destroyInstance() method.

    *ATTENTION* Classes used as singletons must inherit from AVSingleton. It is not
    supported to simply call the static methods for arbitrary types. While this appears to
    work, it leads to multiple singleton instances in context of windows DLLs
    (see SWE-4226). Subclassing together with the proper EXPORT statements prevent this problem.

    Example of illegal code:
    typedef AVSingleton<AVUniqueIdGenerator> AVTrackIdGenerator;

    \code

    // Class which can only be used as a singleton
    class MyClassB : public AVSingleton<MyClassB>
    {
        friend class AVSingleton<MyClassB>;

    public:
        void doSomeStuff() { // ... }

    protected:
        //! Standard Constructor
        MyClassB() {}

        //! Destructor
        virtual ~MyClassB() {}

        int m_param1;

    private:
        //! Hidden copy-constructor
        MyClassB(const MyClassB&);
        //! Hidden assignment operator
        const MyClassB& operator = (const MyClassB&);
    };

    // usage of MyClassB
    // will instantiate MyClassB on the first call with default constructor
    MyClassB::getInstance()->doSomeStuff();

    // alternate first time instantiation with non-default constructor
    MyClassB::createInstance(999);
    MyClassB::getInstance()->doSomeStuff();

    \endcode

    This class is thread safe.
*/
template <typename T>
class AV_CLASS_TEMPLATE_EXPORT AVSingleton
{
public:

    //! Rank to register Singleton deletion.
    //! This can be overridden in the actual singleton class.
    static constexpr int DEINIT_RANK = AVDaemonFunctions::DeinitFunctionRanks::USER_RANK;

    //! Returns true if this singleton already has an instance.
    static bool hasInstance();

    //! Creates a new instance of the singleton class T.
    /*! A possibly existing previous instance must be destroyed first!
     */
    template<class... Args>
    static void createInstance(Args&&... args);

    //! Sets a new instance of the singleton class T.
    /*! A possibly existing previous instance must be destroyed first!
        instance must not be 0
     */
    static void setInstance(T* instance);

    //! Returns a pointer to the instance of the singleton class T.
    /*!
        \return A pointer to the instance of the singleton class.
        If there is no instance of this class yet, one will be created.
    */
    static T* getInstance();

    //! Returns a reference to the instance of the singleton class T.
    /*!
        \return A reference to the instance of the singleton class.
        If there is no instance of this class yet, one will be created.
    */
    static T& getInstanceRef();

    //! Destroys the singleton class instance.
    /*!
        Be aware that all references to the single class instance will be
        invalid after this method has been called!
    */
    static void destroyInstance();

protected:

    // shield the constructor and destructor to prevent outside sources
    // from creating or destroying an AVSingleton instance.

    //! Hidden Default constructor
    AVSingleton();

    //! Hidden Destructor
    virtual ~AVSingleton();

private:

    //! Hidden Copy constructor
    AVSingleton(const AVSingleton& source) = delete;

    /**
     * Avoid problems with static member variable initialization order and dll static member ambiguity
     * problems in windows by using a static function variable
     * (same pattern as employed by AVExplicitSingleton). See SWE-4226.
     */
    class SingletonData
    {
    public:
        //! singleton class instance
        std::atomic<T*> m_instance;
        //! mutex for making getInstance() thread safe
        QMutex          m_singleton_mutex;
        SingletonData()
        {
            AVDaemonFunctions::AVDaemonRegisterDeinitFunction(&T::destroyInstance, T::DEINIT_RANK);
        }
    };

    //! \see SingletonData
    static SingletonData& accessSingletonData();
};

//--------------------------------------------------------------------------

template <typename T>
bool AVSingleton<T>::hasInstance()
{
    return accessSingletonData().m_instance.load() != nullptr;
}

//--------------------------------------------------------------------------

template<typename T>
template<typename... Args>
void AVSingleton<T>::createInstance(Args&&... args)
{
    setInstance(new T(std::forward<Args>(args)...));
}

//--------------------------------------------------------------------------

template <typename T>
void AVSingleton<T>::setInstance(T *instance)
{
    AVASSERT(instance);
    AVASSERT(accessSingletonData().m_instance.exchange(instance) == nullptr);
}

//--------------------------------------------------------------------------

template <typename T>
T *AVSingleton<T>::getInstance()
{
    T *temp = accessSingletonData().m_instance.load(std::memory_order_acquire);
    if (!temp)       // only lock-hint check (race condition here)
    {
        QMutexLocker locker(&accessSingletonData().m_singleton_mutex);

        temp = accessSingletonData().m_instance.load(std::memory_order_relaxed);
        if (!temp)   // double check with resolved race condition
        {
            temp = new T;
            AVASSERT(accessSingletonData().m_instance.exchange(temp, std::memory_order_release) == nullptr);
        }
    }

    return temp;
}

//--------------------------------------------------------------------------

template <typename T>
T &AVSingleton<T>::getInstanceRef()
{
    return *getInstance();
}

//--------------------------------------------------------------------------

template <typename T>
void AVSingleton<T>::destroyInstance()
{
    delete accessSingletonData().m_instance.exchange(nullptr);
}

//--------------------------------------------------------------------------

template <typename T>
AVSingleton<T>::AVSingleton() {}

//--------------------------------------------------------------------------

template <typename T>
AVSingleton<T>::~AVSingleton() {}

//--------------------------------------------------------------------------

template <typename T>
typename AVSingleton<T>::SingletonData &AVSingleton<T>::accessSingletonData()
{
    static_assert(std::is_base_of<AVSingleton, T>::value,
                  "T must be derived from AVSingleton<T>.");
    static SingletonData data;
    return data;
}

#endif /* __AVSINGLETON_H__ */

// End of file
