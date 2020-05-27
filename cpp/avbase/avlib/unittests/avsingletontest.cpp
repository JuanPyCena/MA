///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    TESTS - AVCOMMON Unit Tests
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Stefan Kunz, s.kunz@avibit.com
    \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   Tests for the AVSingleton class template
*/



// system includes

// qt includes

// AVLib Includes
#include "avlog.h"
#include <QTest>
#include <avunittestmain.h>
#include "avsingleton.h"

//locales

#include "helperlib/avsingletontestclasses.h"

class DerivedSingleton : public AVSingleton<DerivedSingleton>
{

    friend class AVSingleton<DerivedSingleton>;

public:
    //! Increments the member variable
    void incrementParam() { ++m_param; }

    //! Returns the member variable
    int param() const { return m_param; }

    //! Standard Constructor
    DerivedSingleton() : m_param(0)
    {
        qDebug("DerivedSingleton::DerivedSingleton(): %p", this);
    }

    //! Alternate Constructor
    explicit DerivedSingleton(int param) : m_param(param)
    {
        qDebug("DerivedSingleton::DerivedSingleton(%d): %p", param, this);
    }

    //! Destructor
    ~DerivedSingleton() override { qDebug("DerivedSingleton::~DerivedSingleton(): %p", this); }

protected: // members
    int m_param;

private:
    //! Hidden copy-constructor
    DerivedSingleton(const DerivedSingleton&);
    //! Hidden assignment operator
    const DerivedSingleton& operator = (const DerivedSingleton&);
};

////////////////////////////////////////////////////////////////////////////////

class AVSingletonTest : public QObject
{
    Q_OBJECT


private slots:
    void init()
    {
    }

    void cleanup()
    {
        DerivedSingleton::destroyInstance();
    }

    // This test showed multiple singleton instances when using windows dlls.
    // Previous code showing the problem was (now illegal):
    // AVSingleton<TestClassUsedAsSingleton>::createInstance();
    // ...
    // See SWE-4226.
    void testTemplateInstantiations()
    {
        qDebug("AVSingletonTest::testTemplateInstantiations");

        TestClassUsedAsSingleton::createInstance();

        QVERIFY( TestClassSingletonInfo::getAddrOfSingleton() ==
                        TestClassUsedAsSingleton::getInstance() );

        TestClassUsedAsSingleton::destroyInstance();
    }

    void testsParameterConstructor()
    {
        DerivedSingleton::createInstance(42);
        QCOMPARE(DerivedSingleton::getInstanceRef().param(), 42);
    }

    void derivedUsageTest()
    {
        qDebug("AVSingletonTest::derivedUsageTest");

        DerivedSingleton* singleton = DerivedSingleton::getInstance();
        QVERIFY(singleton != 0);
        QVERIFY(singleton == DerivedSingleton::getInstance());
        // can be used either way
        QVERIFY(AVSingleton<DerivedSingleton>::getInstance() ==
                       DerivedSingleton::getInstance());
        QCOMPARE(singleton->param(), 0);
        singleton->incrementParam();
        QCOMPARE(DerivedSingleton::getInstance()->param(), 1);

        // old singleton must be destroyed before creating a new one
        DerivedSingleton::destroyInstance();
        DerivedSingleton::setInstance(new DerivedSingleton(99));
        singleton = DerivedSingleton::getInstance();
        QVERIFY(singleton != 0);
        QCOMPARE(singleton->param(), 99);
        singleton->incrementParam();
        QCOMPARE(DerivedSingleton::getInstance()->param(), 100);

        // not possible with the above definition
        // DerivedSingleton* anotherSingleton = new DerivedSingleton;
    }
};

AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(AVSingletonTest,"avlib/unittests/config")
#include "avsingletontest.moc"


// End of file
