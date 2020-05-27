///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2011
//
// Module:     TESTS - AVCOMMON Unit Tests
//
///////////////////////////////////////////////////////////////////////////////

/*!
  \file
  \author  Michael Gebetsroither, m.gebetsroither@avibit.com
  \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
  \brief   Unit test for the AVFactory class
*/


#include "stdio.h"
#include <QTest>
#include <avunittestmain.h>

#include <QString>


#include "avlog.h"
#include "avfactory.h"
#include "avmisc.h"


#define PRINT_INFO(name) do{\
    AVLogger->Write(LOG_INFO, "----- AVFactoryTest:%s -----", name); \
    AVLogger->setMinLevel(AVLog::LOG__FATAL); \
} while(false)

///////////////////////////////////////////////////////////////////////////////

struct BaseClass
{
    virtual ~BaseClass() {}
    static const int ID = 123;
    virtual int hi() const { return ID; }
};


struct DerivedClass : public BaseClass
{
    ~DerivedClass() override {}
    static const int ID = 456;
    int              hi() const override { return ID; }
};

template<typename T>
class Creator
{
public:
    static BaseClass* create() { return new (LOG_HERE) T; }
};

/////////////////////////////////////////////////////////////////////////////

class AVFactoryUnitTest : public QObject
{
    Q_OBJECT


protected:
    AVFactory<BaseClass> m_factory;

private slots:

    void init()
    {
        AVLogger->setMinLevel(AVLog::LOG__INFO);
    }

    /////////////////////////////////////////////////////////////////////////////

    void cleanup()
    {
    }

    /////////////////////////////////////////////////////////////////////////////

    void testSupports()
    {
        PRINT_INFO("testSupports");

        QVERIFY(!m_factory.supports("should not be found"));
    }

    /////////////////////////////////////////////////////////////////////////////

    void testAddBase()
    {
        PRINT_INFO("testAddBase");

        m_factory.add("a", Creator<BaseClass>::create);
        QVERIFY(m_factory.supports("a"));
    }

    /////////////////////////////////////////////////////////////////////////////

    void testAddDerived()
    {
        PRINT_INFO("testAddDerived");

        m_factory.add("b", Creator<DerivedClass>::create);
        QVERIFY(m_factory.supports("b"));
    }

    /////////////////////////////////////////////////////////////////////////////

    void testAddIfAllreadySupported()
    {
        PRINT_INFO("testAddIfAllreadySupported");

        QVERIFY(m_factory.add("c", Creator<BaseClass>::create));
        QVERIFY(!m_factory.add("c", Creator<BaseClass>::create));
    }

    /////////////////////////////////////////////////////////////////////////////

    void testDel()
    {
        PRINT_INFO("testDel");

        m_factory.add("del", Creator<BaseClass>::create);
        QVERIFY(m_factory.supports("del"));
    }

    /////////////////////////////////////////////////////////////////////////////

    void testGetBase()
    {
        PRINT_INFO("testGetBase");

        m_factory.add("getbase", Creator<BaseClass>::create);
        BaseClass* tmp = m_factory.get("getbase");
        QVERIFY(tmp != 0);
        QVERIFY(tmp->hi() == BaseClass::ID);
        delete tmp;
    }

    /////////////////////////////////////////////////////////////////////////////

    void testGetDerived()
    {
        PRINT_INFO("testGetDerived");

        m_factory.add("getderived", Creator<DerivedClass>::create);
        BaseClass* tmp = m_factory.get("getderived");
        QVERIFY(tmp != 0);
        QVERIFY(tmp->hi() == DerivedClass::ID);
        delete tmp;
    }

    /////////////////////////////////////////////////////////////////////////////

    void testGetUniversalErrorPolicy()
    {
        PRINT_INFO("testGetUniversalErr-orPolicy");

        AVFactory<BaseClass, QString, BaseClass*(*)(), AVFactoryUniversalErrorPolicy> f;
        f.add("test", Creator<BaseClass>::create);
        f.add("teeest", Creator<BaseClass>::create);
        f.add("teeeeest", Creator<BaseClass>::create);
        QVERIFY(!f.get("not available"));
    }

    /////////////////////////////////////////////////////////////////////////////

    void testGetCallbackMap()
    {
        PRINT_INFO("testGetCallbackMap");

        typedef AVFactory<BaseClass> TestFactory;
        TestFactory f;
        f.add("test", Creator<BaseClass>::create);
        f.add("teeest", Creator<DerivedClass>::create);
        f.add("teeeeest", Creator<BaseClass>::create);
        QVERIFY(f.getCallbackMap().size() == 3);
    }

};

AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(AVFactoryUnitTest,"avlib/unittests/config")
#include "avfactorytest.moc"
// vim: foldmethod=marker
// End of file
