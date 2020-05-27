///////////////////////////////////////////////////////////////////////////////
//
// Package:   ADMAX - Avibit AMAN/DMAN Libraries
// Copyright: AviBit data processing GmbH, 2001-2014
// QT-Version: QT5
//
// Module:    TESTS - AVCOMMON Tests
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Christian Muschick, c.muschick@avibit.com
 */

// Qt5 includes
#include <QTest>
#include <avunittestmain.h>

// avlib includes
#include "avconfig2sharedconfigholder.h"

namespace // avoid clashes with other config test classes (which are not detected at link time :-/ )
{

//-----------------------------------------------------------------------------

class TestConfig
{
public:
    explicit TestConfig(uint payload) { m_payload = payload; m_instance_count++; }
    virtual  ~TestConfig() { m_instance_count--; }

    uint m_payload;
    static uint m_instance_count;
};
uint TestConfig::m_instance_count = 0;

//-----------------------------------------------------------------------------

class TestHolder
{
public:
    //! Demonstrate config access
    uint getPayload() { return m_shared_config.getConfig().m_payload; }

private:
    AVConfig2SharedConfigHolder<TestConfig> m_shared_config;
};


}

// demonstrate creation by template specialization
template<>
std::shared_ptr<const TestConfig> AVConfig2SharedConfigHolder<TestConfig>::createConfig()
{
    return std::make_shared<TestConfig>(666);
}

//-----------------------------------------------------------------------------

class TstAVConfig2SharedConfigHolder : public QObject
{
    Q_OBJECT

private slots:
    void test_lifecycle();
};

//-----------------------------------------------------------------------------

void TstAVConfig2SharedConfigHolder::test_lifecycle()
{
    AVLOG_ENTER_METHOD();

    QCOMPARE(TestConfig::m_instance_count, 0u);
    {
        TestHolder holder1;
        QCOMPARE(TestConfig::m_instance_count, 1u);
        TestHolder holder2;
        QCOMPARE(TestConfig::m_instance_count, 1u);

        QCOMPARE(holder1.getPayload(), 666u);
        QCOMPARE(holder2.getPayload(), 666u);
    }
    QCOMPARE(TestConfig::m_instance_count, 0u);
}

//-----------------------------------------------------------------------------

// don't initialize avconfig2 because of pre-refreshCoreParameters check
AV_QTEST_MAIN_IMPL(TstAVConfig2SharedConfigHolder,"avlib/unittests/config")
#include "tstavconfig2sharedconfigholder.moc"

// end of file
