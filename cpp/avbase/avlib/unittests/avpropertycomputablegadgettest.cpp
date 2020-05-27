///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2013
//
// Module:    TESTS - AVCOMMON Unit Tests
//
///////////////////////////////////////////////////////////////////////////////

#include <QtTest>

#include <memory>

#include <avunittestmain.h>

#include "avexpressions.h"

class TestPropertyComputableGadget : public AVPropertyComputable
{
    Q_GADGET
    Q_PROPERTY(bool boolProperty MEMBER m_bool_property)
    Q_PROPERTY(int intProperty MEMBER m_int_property)
    Q_PROPERTY(QString strProperty0 READ strProperty0)
    Q_PROPERTY(QString strProperty1 READ strProperty1 WRITE setStrProperty1)
    Q_PROPERTY(QString strProperty2 READ strProperty2 WRITE setStrProperty2 SCRIPTABLE false)
    Q_PROPERTY(QString strProperty3 READ strProperty3 CONSTANT)
    Q_PROPERTY(AVComputable* nested READ nestedProperty CONSTANT)
    Q_PROPERTY(const AVComputable* constNested READ nestedConstProperty CONSTANT)

public:
    explicit TestPropertyComputableGadget() :
        m_bool_property{false}, m_int_property{0} {}
    virtual ~TestPropertyComputableGadget() = default;

    const QMetaObject *metaObject() const override { return &staticMetaObject; }

    bool boolProperty() const { return m_bool_property; }
    void setBoolProperty(bool bool_property) { m_bool_property = bool_property; }

    int intProperty() const { return m_int_property; }
    void setIntProperty(int int_property) { m_int_property = int_property; }

    const QString &strProperty0() const { return m_str_property[0]; }
    void setStrProperty0(const QString &str_property_0) { m_str_property[0] = str_property_0; }

    const QString &strProperty1() const { return m_str_property[1]; }
    void setStrProperty1(const QString &str_property_1) { m_str_property[1] = str_property_1; }

    const QString &strProperty2() const { return m_str_property[2]; }
    void setStrProperty2(const QString &str_property_2) { m_str_property[2] = str_property_2; }

    const QString &strProperty3() const { return m_str_property[3]; }
    void setStrProperty3(const QString &str_property_3) { m_str_property[3] = str_property_3; }

    TestPropertyComputableGadget* nestedProperty() const
    {
        if (m_nested == nullptr)
            m_nested = std::make_unique<TestPropertyComputableGadget>();
        return m_nested.get();
    }

    const TestPropertyComputableGadget* nestedConstProperty() const
    {
        if (m_nested == nullptr)
            m_nested = std::make_unique<TestPropertyComputableGadget>();
        return m_nested.get();
    }

private:
    bool m_bool_property;
    int m_int_property;
    QString m_str_property[4];
    mutable std::unique_ptr<TestPropertyComputableGadget> m_nested;
};

Q_DECLARE_METATYPE(TestPropertyComputableGadget*)

class AVPropertyComputableGadgetTest : public QObject
{
    Q_OBJECT

public slots:
    void init();
    void cleanup();

private slots:
    void tst_readInvalid();
    void tst_writeInvalid();
    void tst_readBool();
    void tst_writeBool();
    void tst_readInt();
    void tst_writeInt();
    void tst_readStr0Readonly();
    void tst_writeStr0Readonly();
    void tst_readStr1();
    void tst_writeStr1();
    void tst_readStr2NotScriptable();
    void tst_writeStr1NotScriptable();
    void tst_readStr3Constant();
    void tst_writeStr3Constant();
    void tst_readNested();
    void tst_writeNested();
    void tst_readConstNested();
    void tst_writeConstNested();

private:
    std::unique_ptr<TestPropertyComputableGadget> m_computable;
};

///////////////////////////////////////////////////////////////////////////////

//! This will be called before each test function is executed.
void AVPropertyComputableGadgetTest::init()
{
    m_computable = std::make_unique<TestPropertyComputableGadget>();
}

///////////////////////////////////////////////////////////////////////////////

//! This will be called after every test function.
void AVPropertyComputableGadgetTest::cleanup()
{
    m_computable = nullptr;
}

///////////////////////////////////////////////////////////////////////////////

void AVPropertyComputableGadgetTest::tst_readInvalid()
{
    QCOMPARE(m_computable->valueOfLHS(AVHashString("invalid")), QString());
}

///////////////////////////////////////////////////////////////////////////////

void AVPropertyComputableGadgetTest::tst_writeInvalid()
{
    QVERIFY(!m_computable->assignValue(AVHashString("invalid"), QStringLiteral("test")));
}

///////////////////////////////////////////////////////////////////////////////

void AVPropertyComputableGadgetTest::tst_readBool()
{
    m_computable->setBoolProperty(false);
    QCOMPARE(m_computable->valueOfLHS(AVHashString("boolProperty")), QStringLiteral("false"));

    m_computable->setBoolProperty(true);
    QCOMPARE(m_computable->valueOfLHS(AVHashString("boolProperty")), QStringLiteral("true"));
}

///////////////////////////////////////////////////////////////////////////////

void AVPropertyComputableGadgetTest::tst_writeBool()
{
    QVERIFY(m_computable->assignValue(AVHashString("boolProperty"), "false"));
    QCOMPARE(m_computable->boolProperty(), false);

    QVERIFY(m_computable->assignValue(AVHashString("boolProperty"), "true"));
    QCOMPARE(m_computable->boolProperty(), true);
}

///////////////////////////////////////////////////////////////////////////////

void AVPropertyComputableGadgetTest::tst_readInt()
{
    m_computable->setIntProperty(42);
    QCOMPARE(m_computable->valueOfLHS(AVHashString("intProperty")), QStringLiteral("42"));

    m_computable->setIntProperty(-2);
    QCOMPARE(m_computable->valueOfLHS(AVHashString("intProperty")), QStringLiteral("-2"));
}

///////////////////////////////////////////////////////////////////////////////

void AVPropertyComputableGadgetTest::tst_writeInt()
{
    QVERIFY(m_computable->assignValue(AVHashString("intProperty"), "42"));
    QCOMPARE(m_computable->intProperty(), 42);

    QVERIFY(m_computable->assignValue(AVHashString("intProperty"), "-1"));
    QCOMPARE(m_computable->intProperty(), -1);
}

///////////////////////////////////////////////////////////////////////////////

void AVPropertyComputableGadgetTest::tst_readStr0Readonly()
{
    m_computable->setStrProperty0(QStringLiteral("test"));
    QCOMPARE(m_computable->valueOfLHS(AVHashString("strProperty0")), QStringLiteral("test"));
}

///////////////////////////////////////////////////////////////////////////////

void AVPropertyComputableGadgetTest::tst_writeStr0Readonly()
{
    m_computable->setStrProperty0(QStringLiteral("test"));
    QVERIFY(!m_computable->assignValue(AVHashString("strProperty0"), "other"));
    QCOMPARE(m_computable->strProperty0(), QStringLiteral("test"));
    //TODO: check of error log output "property strProperty0 is not writeable"
}

///////////////////////////////////////////////////////////////////////////////

void AVPropertyComputableGadgetTest::tst_readStr1()
{
    m_computable->setStrProperty1(QStringLiteral("test"));
    QCOMPARE(m_computable->valueOfLHS(AVHashString("strProperty1")), QStringLiteral("test"));
}

///////////////////////////////////////////////////////////////////////////////

void AVPropertyComputableGadgetTest::tst_writeStr1()
{
    m_computable->setStrProperty1(QStringLiteral("test"));
    QVERIFY(m_computable->assignValue(AVHashString("strProperty1"), "other"));
    QCOMPARE(m_computable->strProperty1(), QStringLiteral("other"));
}

///////////////////////////////////////////////////////////////////////////////

void AVPropertyComputableGadgetTest::tst_readStr2NotScriptable()
{
    m_computable->setStrProperty2(QStringLiteral("test"));
    QCOMPARE(m_computable->valueOfLHS(AVHashString("strProperty2")), QString());
    //TODO: check of error log output "property strProperty2 is not scriptable"
}

///////////////////////////////////////////////////////////////////////////////

void AVPropertyComputableGadgetTest::tst_writeStr1NotScriptable()
{
    m_computable->setStrProperty2(QStringLiteral("test"));
    QVERIFY(!m_computable->assignValue(AVHashString("strProperty2"), "other"));
    QCOMPARE(m_computable->strProperty2(), QStringLiteral("test"));
    //TODO: check of error log output "property strProperty2 is not scriptable"
}

///////////////////////////////////////////////////////////////////////////////

void AVPropertyComputableGadgetTest::tst_readStr3Constant()
{
    m_computable->setStrProperty3(QStringLiteral("test"));
    QCOMPARE(m_computable->valueOfLHS(AVHashString("strProperty3")), QStringLiteral("test"));
}

///////////////////////////////////////////////////////////////////////////////

void AVPropertyComputableGadgetTest::tst_writeStr3Constant()
{
    m_computable->setStrProperty3(QStringLiteral("test"));
    QVERIFY(!m_computable->assignValue(AVHashString("strProperty3"), "other"));
    QCOMPARE(m_computable->strProperty3(), QStringLiteral("test"));
    //TODO: check of error log output "property strProperty3 is constant"
}

///////////////////////////////////////////////////////////////////////////////

void AVPropertyComputableGadgetTest::tst_readNested()
{
    m_computable->nestedProperty()->setIntProperty(42);
    QCOMPARE(m_computable->valueOfLHS(AVHashString("nested.intProperty")), QStringLiteral("42"));
}

///////////////////////////////////////////////////////////////////////////////

void AVPropertyComputableGadgetTest::tst_writeNested()
{
    m_computable->nestedProperty()->setIntProperty(42);
    QVERIFY(m_computable->assignValue(AVHashString("nested.intProperty"), QStringLiteral("-2")));
    QCOMPARE(m_computable->nestedProperty()->intProperty(), -2);
}

///////////////////////////////////////////////////////////////////////////////

void AVPropertyComputableGadgetTest::tst_readConstNested()
{
    m_computable->nestedProperty()->setIntProperty(42);
    QCOMPARE(m_computable->valueOfLHS(AVHashString("constNested.intProperty")), QStringLiteral("42"));
}

///////////////////////////////////////////////////////////////////////////////

void AVPropertyComputableGadgetTest::tst_writeConstNested()
{
    m_computable->nestedProperty()->setIntProperty(42);
    QVERIFY(!m_computable->assignValue(AVHashString("constNested.intProperty"), QStringLiteral("-2")));
    QCOMPARE(m_computable->nestedProperty()->intProperty(), 42);
}

///////////////////////////////////////////////////////////////////////////////

AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(AVPropertyComputableGadgetTest,"avlib/unittests/config")
#include "avpropertycomputablegadgettest.moc"

// End of file
