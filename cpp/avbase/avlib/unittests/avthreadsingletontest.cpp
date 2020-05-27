///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    TESTS - AVCOMMON Unit Tests
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Andreas Schuller, a.schuller@avibit.com
    \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   Tests for the AVThreadSingleton class template
*/



// system includes

// qt includes

// AVLib Includes
#include "avlog.h"
#include <QTest>
#include <avunittestmain.h>
#include "avthreadsingleton.h"

//locales

#include "helperlib/avsingletontestclasses.h"


//! Class that can only be used as a singleton
class ThreadSingletonTestDerivedClass : public AVThreadSingleton<ThreadSingletonTestDerivedClass>
{
    friend class AVThreadSingleton<ThreadSingletonTestDerivedClass>;

public:
    //! Increments the member variable
    void incrementParam() { ++m_param; }

    //! Returns the member variable
    int param() const { return m_param; }

protected: // methods
    //! Standard Constructor
    ThreadSingletonTestDerivedClass() : m_param(0)
    {
        qDebug("ThreadSingletonTestDerivedClass::ThreadSingletonTestDerivedClass(): %p", this);
    }

    //! Alternate Constructor
    explicit ThreadSingletonTestDerivedClass(int param) : m_param(param)
    {
        qDebug("ThreadSingletonTestDerivedClass::ThreadSingletonTestDerivedClass(%d): %p",
               param, this);
    }

    //! Destructor
    ~ThreadSingletonTestDerivedClass() override
    {
        qDebug("ThreadSingletonTestDerivedClass::~ThreadSingletonTestDerivedClass(): %p",
               this);
    }

protected: // members
    int m_param;

private:
    //! Hidden copy-constructor
    ThreadSingletonTestDerivedClass(const ThreadSingletonTestDerivedClass&);
    //! Hidden assignment operator
    const ThreadSingletonTestDerivedClass& operator = (const ThreadSingletonTestDerivedClass&);
};


////////////////////////////////////////////////////////////////////////////////

//! Class that can optionally be used as a singleton
class ThreadSingletonTestNormalClass
{
public:
    //! Standard Constructor
    ThreadSingletonTestNormalClass() : m_param(0)
    {
        qDebug("ThreadSingletonTestNormalClass::ThreadSingletonTestNormalClass(): %p", this);
    }

    //! Alternate Constructor
    explicit ThreadSingletonTestNormalClass(int param) : m_param(param)
    {
        qDebug("ThreadSingletonTestNormalClass::ThreadSingletonTestNormalClass(%d): %p",
               param, this);
    }

    //! Destructor
    virtual ~ThreadSingletonTestNormalClass()
    {
        qDebug("ThreadSingletonTestNormalClass::~ThreadSingletonTestNormalClass(): %p", this);
    }

    //! Increments the member variable
    void incrementParam() { ++m_param; }

    //! Returns the member variable
    int param() const { return m_param; }

protected: // members
    int m_param;

private:
    //! Hidden copy-constructor
    ThreadSingletonTestNormalClass(const ThreadSingletonTestNormalClass&);
    //! Hidden assignment operator
    const ThreadSingletonTestNormalClass& operator = (const ThreadSingletonTestNormalClass&);
};


////////////////////////////////////////////////////////////////////////////////

class AVThreadSingletonTest : public QObject
{
    Q_OBJECT


private slots:
    void init()
    {
    }

    void cleanup()
    {
        ThreadSingletonTestDerivedClass::destroyInstance();
        AVThreadSingleton<ThreadSingletonTestNormalClass>::destroyInstance();
    }

    void derivedUsageTest()
    {
        qDebug("AVThreadSingletonTest::derivedUsageTest");

        ThreadSingletonTestDerivedClass* singleton = ThreadSingletonTestDerivedClass::getInstance();
        QVERIFY(singleton != 0);
        QVERIFY(singleton == ThreadSingletonTestDerivedClass::getInstance());
        // can be used either way
        QVERIFY(AVThreadSingleton<ThreadSingletonTestDerivedClass>::getInstance() ==
                       ThreadSingletonTestDerivedClass::getInstance());
        QCOMPARE(singleton->param(), 0);
        singleton->incrementParam();
        QCOMPARE(ThreadSingletonTestDerivedClass::getInstance()->param(), 1);

        // old singleton must be destroyed before creating a new one
        ThreadSingletonTestDerivedClass::destroyInstance();
        ThreadSingletonTestDerivedClass::createInstance<int>(99);
        singleton = ThreadSingletonTestDerivedClass::getInstance();
        QVERIFY(singleton != 0);
        QCOMPARE(singleton->param(), 99);
        singleton->incrementParam();
        QCOMPARE(ThreadSingletonTestDerivedClass::getInstance()->param(), 100);

        // not possible with the above definition
        // ThreadSingletonTestDerivedClass* anotherSingleton = new ThreadSingletonTestDerivedClass;
    }

    void explicitUsageTest()
    {
        qDebug("AVThreadSingletonTest::explicitUsageTest");

        ThreadSingletonTestNormalClass* singleton =
                AVThreadSingleton<ThreadSingletonTestNormalClass>::getInstance();
        QVERIFY(singleton != 0);
        QVERIFY(singleton ==
                AVThreadSingleton<ThreadSingletonTestNormalClass>::getInstance());
        QCOMPARE(singleton->param(), 0);
        singleton->incrementParam();
        QCOMPARE(AVThreadSingleton<ThreadSingletonTestNormalClass>::getInstance()->param(), 1);

        // old singleton must be destroyed prior to creating a new one
        AVThreadSingleton<ThreadSingletonTestNormalClass>::destroyInstance();
        AVThreadSingleton<ThreadSingletonTestNormalClass>::createInstance<int>(99);
        singleton = AVThreadSingleton<ThreadSingletonTestNormalClass>::getInstance();
        QVERIFY(singleton != 0);
        QCOMPARE(singleton->param(), 99);
        singleton->incrementParam();
        QCOMPARE(AVThreadSingleton<ThreadSingletonTestNormalClass>::getInstance()->param(), 100);

        // also possible here
        ThreadSingletonTestNormalClass* normalInstance = new ThreadSingletonTestNormalClass;
        QVERIFY(normalInstance != 0);
        QVERIFY(normalInstance !=
                AVThreadSingleton<ThreadSingletonTestNormalClass>::getInstance());
        delete normalInstance;
    }
};

AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(AVThreadSingletonTest,"avlib/unittests/config")
#include "avthreadsingletontest.moc"


// End of file
