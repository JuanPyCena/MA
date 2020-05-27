///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2010
//
// Module:     AVLIB - AviBit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author    Wolfgang Eder, w.eder@avibit.com
    \author    QT4-PORT: Christian Muschick, c.muschick@avibit.com
    \brief     This file declares the AVExplicitSingleton and
               AVDomConfigSingleton template classes.
*/

#ifndef AVEXPLICITSINGLETON_H
#define AVEXPLICITSINGLETON_H

// AviBit common includes
#include "avlib_export.h"
#include "avlog.h"
#include "avmisc.h"
#include <avdaemon.h>

#include <typeinfo>

///////////////////////////////////////////////////////////////////////////////

//! This is the AVExplicitSingleton class template.
//! It is a base class for all explicit singletons, except subclasses of
//! AVDomConfig. Subclasses of AVDomConfig should use AVDomConfigSingleton
//! instead.
//! Subclasses are supposed to specify themselves as the template parameter, and
//! provide a static method initializeSingleton().
//!
//! \code
//! class MySingleton : public AVExplicitSingleton<MySingleton>
//! {
//! public:
//!     static MySingleton& initializeSingleton(params...)
//!         { return setSingleton(new MySingleton(params...); }
//! public:
//!     MySingleton(params...);
//! };
//! \endcode
//!
//! This class template is similar to AVSingleton. The differences are:
//! 1. AVExplicitSingleton is initialized explicitly;
//!    AVSingleton is lazy initialized.
//! 2. AVExplicitSingleton subclasses can be polymorphic, e.g. ColorConfig
//! 3. AVExplicitSingleton does not need a QMutex

template <class T>
class AV_CLASS_TEMPLATE_EXPORT AVExplicitSingleton
{

public:

    //! Rank to register Singleton deletion.
    //! This can be overridden in the actual singleton class.
    static constexpr int DEINIT_RANK = AVDaemonFunctions::DeinitFunctionRanks::USER_RANK;

    //! Default constructor
    AVExplicitSingleton() = default;

    //! The destructor automatically clears the singleton instance.
    virtual ~AVExplicitSingleton();

    //! Returns true if the singleton was initialized
    static bool isSingletonInitialized();

    //! Return the global singleton instance
    static T& singleton();

    //! Delete the global singleton instance
    static void deleteSingleton();

    //! Set the singleton instance
    static T& setSingleton(T *singleton);


    //! Creates a new instance of the singleton class T.
    /*! A possibly existing previous instance must be destroyed first!
     */
    template<class... Args>
    static void createInstance(Args&&... args);

private:
    Q_DISABLE_COPY(AVExplicitSingleton)

    //! Helper data structure to store singleton data
    class SingletonData
    {
    public:
        SingletonData() {
            AVDaemonFunctions::AVDaemonRegisterDeinitFunction(&T::deleteSingleton, T::DEINIT_RANK);
        }

        T   *m_singleton = nullptr;
        bool m_used = false;
    };

    //! Internal method to access the static  singleton data
    //! We don't use a static variable for storing the instance to avoid initialization order problems
    static SingletonData& accessSingletonData();

};

///////////////////////////////////////////////////////////////////////////////

template<class T>
AVExplicitSingleton<T>::~AVExplicitSingleton()
{
    if (this == accessSingletonData().m_singleton) accessSingletonData().m_singleton = 0;
}

///////////////////////////////////////////////////////////////////////////////

template<class T>
bool AVExplicitSingleton<T>::isSingletonInitialized()
{
    return accessSingletonData().m_singleton != 0;
}

///////////////////////////////////////////////////////////////////////////////

template<class T>
T &AVExplicitSingleton<T>::singleton()
{
    AVASSERTMSG(accessSingletonData().m_singleton != 0,
                qPrintable("AVExplicitSingleton::singleton: Explicit singleton was not initialized: " +
                           QString(typeid(T).name())));

    accessSingletonData().m_used = true;
    return *accessSingletonData().m_singleton;
}

///////////////////////////////////////////////////////////////////////////////

template<class T>
void AVExplicitSingleton<T>::deleteSingleton()
{
    if (accessSingletonData().m_singleton == 0) return;
    if (!accessSingletonData().m_used)
    {
        AVLogInfo << "Singleton " << QString(typeid(T).name())
                  << " was probably never used";
    }
    delete accessSingletonData().m_singleton; // the destructor sets the pointer to 0
    AVASSERT(accessSingletonData().m_singleton == 0);
}

///////////////////////////////////////////////////////////////////////////////

template<class T>
T &AVExplicitSingleton<T>::setSingleton(T *singleton)
{
    AVASSERT(singleton != 0);
    AVASSERT(accessSingletonData().m_singleton == 0);
    accessSingletonData().m_singleton = singleton;
    accessSingletonData().m_used = false;
    return *accessSingletonData().m_singleton;
}

///////////////////////////////////////////////////////////////////////////////

template<class T>
typename AVExplicitSingleton<T>::SingletonData &AVExplicitSingleton<T>::accessSingletonData()
{
    static_assert(std::is_base_of<AVExplicitSingleton, T>::value,
                  "T must be derived from AVExplicitSingleton<T>.");
    static SingletonData data;
    return data;
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
template<typename... Args>
void AVExplicitSingleton<T>::createInstance(Args&&... args)
{
    setSingleton(new T(std::forward<Args>(args)...));
}

///////////////////////////////////////////////////////////////////////////////

//! This is the AVExplicitSingleton class for subclasses of DomConfig (but not
//! AVConfigBase).

template <class T>
class AV_CLASS_TEMPLATE_EXPORT AVDomConfigSingleton : public AVExplicitSingleton<T>
{
public:
    //! This helper method is used by initializeSingleton() and by unit tests
    static T& setSingleton(T *singleton, bool readFromFile=true)
    {
        T& result = AVExplicitSingleton<T>::setSingleton(singleton);
        if (readFromFile)
            result.readFromFile();
        return result;
    }

    //! Tries to set the singleton, but leaves it to the client to check whether
    //! the initialization was successful.
    //! Helper method used by initializeSingletonChecked() and by unit tests.
    //! Will delete the passed singleton object if initialization fails.
    static T* setSingletonChecked(T *singleton)
    {
        AVASSERT(singleton != 0);
        if (!singleton->readFromFileChecked()) {
            delete singleton;
            return 0;
        }
        T& result = AVExplicitSingleton<T>::setSingleton(singleton);
        return &result;
    }
};

#endif // AVEXPLICITSINGLETON_H

// End of file
