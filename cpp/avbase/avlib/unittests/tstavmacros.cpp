///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2013
//
// Module:    TESTS - AVCOMMON Unit Tests
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   Function level test cases for AviBit macros
 */



#include <QtTest>
#include <avunittestmain.h>
#include <avmacros.h>


class TstAVMacros : public QObject
{
    Q_OBJECT

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
    void testDefineEnum();
    void testDefineTypesafeEnum();
    void testDefineTypesafeEnumInNamespace();

private slots:

public:

#define Enum3Values(enum_value) \
        enum_value(Enum3Value1, =0x1)   \
        enum_value(Enum3Value2, =0x2)   \
        enum_value(Enum3Value3, =0x4)

#define Enum5Values(enum_value) \
        enum_value(Enum5Value1, =0x1)   \
        enum_value(Enum5Value2, =0x2)   \
        enum_value(Enum5Value3, =0x4)

    DEFINE_TYPESAFE_ENUM(Enum3, Enum3Values)

    DEFINE_TYPESAFE_ENUM(Enum4, Enum3Values)

    DEFINE_TYPESAFE_ENUM(Enum5, Enum5Values)
};


namespace enum_namespace
{
    DEFINE_TYPESAFE_ENUM_IN_NAMESPACE(EnumNS, Enum3Values)
}

//! This will be called before the first test function is executed.
//! It is only called once.
void TstAVMacros::initTestCase()
{

}

//! This will be called after the last test function is executed.
//! It is only called once.
void TstAVMacros::cleanupTestCase()
{

}

//! This will be called before each test function is executed.
void TstAVMacros::init()
{

}

//! This will be called after every test function.
void TstAVMacros::cleanup()
{

}


struct TestEnums
{
#define Enum1Values(enum_value) \
        enum_value(Enum1Value1, =0x1)   \
        enum_value(Enum1Value2, =0x2)   \
        enum_value(Enum1Value3, =0x4)

    DEFINE_ENUM(Enum1, Enum1Values)

#define Enum2Values(enum_value) \
        enum_value(Enum2Value1, =0x1)   \
        enum_value(Enum2Value2, =0x2)   \
        enum_value(Enum2Value3, =0x4)

    DEFINE_ENUM(Enum2, Enum2Values)
};

void TstAVMacros::testDefineEnum()
{
    AVLogInfo << "---- testDefineEnum launched ----";

    QCOMPARE(TestEnums::getEnum1Strings(), QStringList() <<"Enum1Value1"<<"Enum1Value2"<<"Enum1Value3");
    QCOMPARE(TestEnums::getEnum2Strings(), QStringList() <<"Enum2Value1"<<"Enum2Value2"<<"Enum2Value3");

    TestEnums::Enum1 value1 = TestEnums::Enum1Value1;
    QVERIFY(value1 == TestEnums::Enum1Value1);
    QCOMPARE(TestEnums::getEnum1FlagStrings(value1), QStringList() << "Enum1Value1");

    int value12 = TestEnums::Enum1Value1 | TestEnums::Enum1Value2;
    QCOMPARE(TestEnums::getEnum1FlagStrings(value12), QStringList() << "Enum1Value1"<<"Enum1Value2");
    QCOMPARE(TestEnums::getFlagStrings(value12, TestEnums::Enum1()), QStringList() << "Enum1Value1"<<"Enum1Value2");

    QCOMPARE(qHash(TestEnums::Enum1Value2), qHash(2));

    AVLogInfo << "---- testDefineEnum ended ----";
}

void TstAVMacros::testDefineTypesafeEnum()
{
    AVLogInfo << "---- testDefineTypesafeEnum launched ----";

    QCOMPARE(TstAVMacros::getEnum3Strings(), QStringList() <<"Enum3Value1"<<"Enum3Value2"<<"Enum3Value3");
    QCOMPARE(TstAVMacros::getEnum4Strings(), QStringList() <<"Enum3Value1"<<"Enum3Value2"<<"Enum3Value3");

    TstAVMacros::Enum3 value1 = TstAVMacros::Enum3::Enum3Value1;
    QVERIFY(value1 == TstAVMacros::Enum3::Enum3Value1);
    QCOMPARE(TstAVMacros::getEnum3FlagStrings(static_cast<int>(value1)), QStringList() << "Enum3Value1");

    int value12 = 3;
    QCOMPARE(TstAVMacros::getEnum3FlagStrings(value12), QStringList() << "Enum3Value1"<<"Enum3Value2");
    QCOMPARE(TstAVMacros::getFlagStrings(value12, TstAVMacros::Enum3()), QStringList() << "Enum3Value1"<<"Enum3Value2");

    TstAVMacros::Enum5 value3 = TstAVMacros::Enum5::Enum5Value3;
    QCOMPARE(TstAVMacros::enumFromInt(1, value3), true);
    QCOMPARE(value3, TstAVMacros::Enum5::Enum5Value1);

    TstAVMacros::Enum5 value4 = TstAVMacros::Enum5::Enum5Value1;
    QCOMPARE(TstAVMacros::enumFromString("Enum5Value2", value4), true);
    QCOMPARE(value4, TstAVMacros::Enum5::Enum5Value2);

    TstAVMacros::Enum5 value5;
    QCOMPARE(AVFromString("\"Enum5Value3\"", value5), true);
    QCOMPARE(value5, TstAVMacros::Enum5::Enum5Value3);
    QCOMPARE(AVFromString("Enum5Value2", value5), true);
    QCOMPARE(AVToString(value5), QString("Enum5Value2"));;

    QCOMPARE(AVGetEnumValues(value5), (QList<TstAVMacros::Enum5> {
                                           TstAVMacros::Enum5::Enum5Value1,
                                           TstAVMacros::Enum5::Enum5Value2,
                                           TstAVMacros::Enum5::Enum5Value3}));

    QCOMPARE(qHash(Enum3::Enum3Value1), qHash(Enum4::Enum3Value1));

    AVLogInfo << "---- testDefineTypesafeEnum ended ----";
}


void TstAVMacros::testDefineTypesafeEnumInNamespace()
{
    AVLogInfo << "---- testDefineTypesafeEnumInNamespace launched ----";

    const enum_namespace::EnumNS value1 = enum_namespace::EnumNS::Enum3Value2;
    QCOMPARE(qHash(value1), qHash(Enum4::Enum3Value2));

    AVLogInfo << "---- testDefineTypesafeEnumInNamespace ended ----";
}


AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(TstAVMacros,"avlib/unittests/config")
#include "tstavmacros.moc"


// End of file
