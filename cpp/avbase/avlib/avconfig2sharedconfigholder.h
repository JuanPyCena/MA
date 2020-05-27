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
    \author  Christian Muschick, c.muschick@avibit.com
    \brief   Helper class for shared config usage pattern.
 */

///////////////////////////////////////////////////////////////////////////////

#if !defined AVCONFIG2_SHARED_CONFIG_HOLDER_H_INCLUDED
#define AVCONFIG2_SHARED_CONFIG_HOLDER_H_INCLUDED

#include <memory>

#include "avthread.h"
#include "avconfig2.h"

//-----------------------------------------------------------------------------
/**
 * @brief The AVConfig2SharedConfigHolder class implements a shared config usage pattern for library configurations.
 *        See AVAircraftTypeLibrary for usage pattern.
 *
 * *ATTENTION* Instances of this class must be created before the call to refreshCoreParameters(), and must be created
 *             in the main thread context!
 *
 * - The configuration is loaded exactly once, even if it is used in different libraries
 * - Error checking is left to the avconfig2 implementation and refreshCoreParameters
 * - The configuration is destroyed when the last holder is destroyed
 * - The implementation is not thread-safe for simplicity (initialization must happen in main thread context)
 */
template<typename CONFIG>
class AVConfig2SharedConfigHolder
{
public:
    explicit AVConfig2SharedConfigHolder();
    /**
     * Constructor to be used in unit tests only. Let this instance directly work with the given config,
     * bypassing the config sharing mechanism.
     *
     * Also disables all plausibility checks.
     */
    explicit AVConfig2SharedConfigHolder(std::shared_ptr<CONFIG> unit_test_config);
    virtual ~AVConfig2SharedConfigHolder();

    const CONFIG& getConfig() const;

    //! Change the payload of the config holder. For unit test usage only.
    void setUnitTestConfig(std::shared_ptr<CONFIG> unit_test_config);

private:

    //! Note that this can be explicitly specialized if required.
    std::shared_ptr<const CONFIG> createConfig();

    //! Used for new instances to pick up the shared config
    static std::weak_ptr<const CONFIG> m_weak_config_pointer;
    std::shared_ptr<const CONFIG> m_config;
};

//-----------------------------------------------------------------------------

template<typename CONFIG>
std::weak_ptr<const CONFIG> AVConfig2SharedConfigHolder<CONFIG>::m_weak_config_pointer;

//-----------------------------------------------------------------------------

template<typename CONFIG>
AVConfig2SharedConfigHolder<CONFIG>::AVConfig2SharedConfigHolder()
{
    AVASSERT(AVThread::isMainThread());
    if (AVConfig2Global::isSingletonInitialized() && // e.g. unit test
        AVConfig2Global::singleton().getRefreshCoreParametersCalled())
    {
        AVLogFatal << "AVConfig2SharedConfigHolder: instances must be created before calling refreshCoreParameters";
    }

    std::shared_ptr<const CONFIG> existing_config = m_weak_config_pointer.lock();
    if (existing_config)
    {
        m_config = existing_config;
    } else
    {
        m_config = createConfig();
        m_weak_config_pointer = m_config;
    }
}

//-----------------------------------------------------------------------------

template<typename CONFIG>
AVConfig2SharedConfigHolder<CONFIG>::AVConfig2SharedConfigHolder(std::shared_ptr<CONFIG> unit_test_config) :
    m_config(unit_test_config)
{
}

//-----------------------------------------------------------------------------

template<typename CONFIG>
AVConfig2SharedConfigHolder<CONFIG>::~AVConfig2SharedConfigHolder()
{
    AVASSERT(AVThread::isMainThread());
}

//-----------------------------------------------------------------------------

template<typename CONFIG>
const CONFIG &AVConfig2SharedConfigHolder<CONFIG>::getConfig() const
{
    return *m_config;
}

//-----------------------------------------------------------------------------

template<typename CONFIG>
void AVConfig2SharedConfigHolder<CONFIG>::setUnitTestConfig(std::shared_ptr<CONFIG> unit_test_config)
{
    m_config = unit_test_config;
}

//-----------------------------------------------------------------------------

template<typename CONFIG>
std::shared_ptr<const CONFIG> AVConfig2SharedConfigHolder<CONFIG>::createConfig()
{
    return std::make_shared<CONFIG>();
}

#endif // AVCONFIG2_SHARED_CONFIG_HOLDER_H_INCLUDED

// End of file
