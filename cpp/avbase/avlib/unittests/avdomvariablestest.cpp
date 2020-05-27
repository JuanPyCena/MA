///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2011
//
// Module:     TESTS - AVCOMMON Unit Tests
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Wolfgang Eder, w.eder@avibit.com
    \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   Test the AVDomVariable classes
*/


// AviBit common includes
#include "avdomvariables.h"
#include <QTest>
#include <avunittestmain.h>


///////////////////////////////////////////////////////////////////////////////

class TestDomObject : public AVDomObject
{
    Q_OBJECT

public:
    TestDomObject(
        AVDomVariable *parent, const QString& tagName, bool autoCreate)
        : AVDomObject(parent, tagName, autoCreate),
          string(this, "string", true, SLOT(slotStringChanged()), "default") { }
protected slots:
    void slotStringChanged() { }
public:
    AVDomString string;
};


///////////////////////////////////////////////////////////////////////////////

class TestDomConfig : public AVDomConfig
{
    Q_OBJECT
public:
    TestDomConfig()
        : AVDomConfig("TestDomConfig"), string(this, "string", true,
                                              SLOT(slotStringChanged()), "default") { }
protected slots:
    void slotStringChanged() { }
public:
    AVDomString string;
};

///////////////////////////////////////////////////////////////////////////////

class AVDomVariablesTest : public QObject
{
  Q_OBJECT
private slots:
    void testDomVariables();
    //! ELSIS-1325
    void testDomDoubleSetValue();
    //! ELSIS-1332
    void test_DomStringListSep_setValue_data();
    void test_DomStringListSep_setValue();
    void test_DomStringListSep_setValue_special_data();
    void test_DomStringListSep_setValue_special();
    void test_DomStringListSep_setText_data();
    void test_DomStringListSep_setText();
};

///////////////////////////////////////////////////////////////////////////////

void AVDomVariablesTest::testDomVariables()
{
    AVLogInfo << "testDomVariables";
    TestDomConfig config1;
    config1.setFileName("avdomvariablestest");
    config1.readFromFile();
    TestDomConfig config2;
    config2.readFromString(
        "<?xml version = '1.0' encoding = 'utf-8'?>\n"
        "<TestDomConfig>\n"
        " <string>value</string>\n"
        "</TestDomConfig>\n");
}

///////////////////////////////////////////////////////////////////////////////

void AVDomVariablesTest::testDomDoubleSetValue()
{
    const double TEST_VALUE = 0.9999999999999;

    AVDomConfig config("DomConfig", true);
    AVDomDouble dom_double(&config, "AVDomDouble", true);

    dom_double.setValue(TEST_VALUE);
    // AVDomDouble value is different to the actual value because of the floating-point
    // rounding errors during the internal conversion between string and double
    QVERIFY(dom_double.value() != TEST_VALUE);
}

///////////////////////////////////////////////////////////////////////////////

void AVDomVariablesTest::test_DomStringListSep_setValue_data()
{
    QTest::addColumn<QStringList>("string_list");

    QTest::newRow("empty")          << QStringList();
    QTest::newRow("2 empty")        << QStringList{"", ""};
    QTest::newRow("standard case")  << QStringList{"first", "second", "third"};
    QTest::newRow("empty at start") << QStringList{"", "first", "second", "third"};
    QTest::newRow("2 empty start")  << QStringList{"", "", "first", "second", "third"};
    QTest::newRow("empty element")  << QStringList{"first", "", "second", "", "third"};
    QTest::newRow("2 empty")        << QStringList{"first", "", "second", "", "third"};
    QTest::newRow("empty at end")   << QStringList{"first", "second", "third", ""};
    QTest::newRow("general case")   << QStringList{"", "first", "", "second", "", "third", ""};
}

///////////////////////////////////////////////////////////////////////////////

void AVDomVariablesTest::test_DomStringListSep_setValue()
{
    AVLogInfo << "---- test_DomStringListSep_setValue launched (" << QTest::currentDataTag() <<") ----";

    QFETCH(QStringList, string_list);
    AVDomConfig config("AVDomConfig", true);
    AVDomStringListSep dom_string_list(&config, "AVDomStringListSep", true, ",");
    dom_string_list.setValue(string_list);
    QCOMPARE(dom_string_list.value(), string_list);

    AVLogInfo << "---- test_DomStringListSep_setValue ended (" << QTest::currentDataTag() <<")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void AVDomVariablesTest::test_DomStringListSep_setValue_special_data()
{
    QTest::addColumn<QStringList>("string_list");
    QTest::addColumn<QStringList>("result");

    // special case: we cannot have a single empty string; this used to cause an assert before ELSIS-1332
    QTest::newRow("empty string")   << QStringList("")                          << QStringList();
    // special case: string contains delimiter, these cause assertions
    /*
    QTest::newRow("delimiter")      << QStringList{"first", "sec,ond", "third"} << QStringList{"first", "sec", "ond", "third"};
    QTest::newRow("only comma")     << QStringList{","}                         << QStringList{"", ""};
    */
}

///////////////////////////////////////////////////////////////////////////////

void AVDomVariablesTest::test_DomStringListSep_setValue_special()
{
    AVLogInfo << "---- test_DomStringListSep_setValue_special launched (" << QTest::currentDataTag() <<") ----";

    QFETCH(QStringList, string_list);
    QFETCH(QStringList, result);
    AVDomConfig config("AVDomConfig", true);
    AVDomStringListSep dom_string_list(&config, "AVDomStringListSep", true, ",");
    dom_string_list.setValue(string_list);
    QCOMPARE(dom_string_list.value(), result);

    AVLogInfo << "---- test_DomStringListSep_setValue_special ended (" << QTest::currentDataTag() <<")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void AVDomVariablesTest::test_DomStringListSep_setText_data()
{
    QTest::addColumn<QString>("string");
    QTest::addColumn<QStringList>("result");

    QTest::newRow("null")           << QString()                  << QStringList();
    QTest::newRow("2 empty")        << ","                        << QStringList{"", ""};
    QTest::newRow("standard case")  << "first,second,third"       << QStringList{"first", "second", "third"};
    QTest::newRow("empty at start") << ",first,second,third"      << QStringList{"", "first", "second", "third"};
    QTest::newRow("2 empty start")  << ",,first,second,third"     << QStringList{"", "", "first", "second", "third"};
    QTest::newRow("empty element")  << "first,,second,,third"     << QStringList{"first", "", "second", "", "third"};
    QTest::newRow("2 empty")        << "first,,,second,,,third"   << QStringList{"first", "", "", "second", "", "", "third"};
    QTest::newRow("empty at end")   << "first,second,third,"      << QStringList{"first", "second", "third", ""};
    QTest::newRow("2 empty")        << "first,second,third,,"     << QStringList{"first", "second", "third", "", ""};
    QTest::newRow("general case")   << ",first,,second,,,third,," << QStringList{"", "first", "", "second", "", "", "third", "", ""};
    // special case: we cannot have a single empty string, for backwards compatibility
    QTest::newRow("empty")          << ""                         << QStringList();
}

///////////////////////////////////////////////////////////////////////////////

void AVDomVariablesTest::test_DomStringListSep_setText()
{
    AVLogInfo << "---- test_DomStringListSep_setText launched (" << QTest::currentDataTag() <<") ----";

    QFETCH(QString, string);
    QFETCH(QStringList, result);
    AVDomConfig config("AVDomConfig", true);
    AVDomStringListSep dom_string_list(&config, "AVDomStringListSep", true, ",");
    dom_string_list.setText(string);
    QCOMPARE(dom_string_list.value(), result);

    AVLogInfo << "---- test_DomStringListSep_setText ended (" << QTest::currentDataTag() <<")    ----";
}

AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(AVDomVariablesTest,"avlib/unittests/config")
#include "avdomvariablestest.moc"


// End of file
