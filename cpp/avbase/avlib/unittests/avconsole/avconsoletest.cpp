///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2013
//
// Module:    TESTS - AVCOMMON Unit Tests
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Christian Muschick, c.muschick@avibit.com
    \brief   Unit tests for avconsole class
 */

#include <QtTest>
#include <avunittestmain.h>

#include "avconsole.h"
#include "avexecstandbycheckerunittestdriver.h"

#include "helperlib/avconsoleunittestconnection.h"

//-----------------------------------------------------------------------------

class AVConsoleTest : public QObject
{
    Q_OBJECT

private slots:
    void testAlwaysAllowStandbyExecution();

    void testSplitInput_data();
    void testSplitInput();

};

//-----------------------------------------------------------------------------

void AVConsoleTest::testAlwaysAllowStandbyExecution()
{
    AVLOG_ENTER_METHOD();

    const QString EXECUTE_ALWAYS  = "executeAlways";
    const QString EXECUTE_DEFAULT = "executeDefault";

    AVExecStandbyCheckerUnitTestDriver driver;
    AVExecStandbyChecker::initializeSingleton(&driver);

    AVConsole::singleton().registerCallback(EXECUTE_ALWAYS, nullptr, [&](AVConsoleConnection& connection, const QStringList&args)
    {
        Q_UNUSED(args);
        connection.print(EXECUTE_ALWAYS);
    }, "no help").setAlwaysAllowStandbyExecution();
    AVConsole::singleton().registerCallback(EXECUTE_DEFAULT, nullptr, [&](AVConsoleConnection& connection, const QStringList&args)
    {
        Q_UNUSED(args);
        connection.print(EXECUTE_DEFAULT);
    }, "no help");

    AVConsole::singleton().setSuppressStandbyExecution(true);
    driver.switchToExec();
    AVConsoleUnitTestConnection connection;
    connection.sendInput(EXECUTE_ALWAYS);
    connection.sendInput(EXECUTE_DEFAULT);
    driver.switchToStandby();
    connection.sendInput(EXECUTE_ALWAYS);
    connection.sendInput(EXECUTE_DEFAULT);
    AVConsole::singleton().setSuppressStandbyExecution(false);

    QCOMPARE(connection.m_received_strings.count(EXECUTE_ALWAYS), 2);
    QCOMPARE(connection.m_received_strings.count(EXECUTE_DEFAULT), 1);
    // Note that the test framework relies on this magic string.
    QVERIFY(connection.m_received_strings.contains("STANDBY - not executing"));
}

void AVConsoleTest::testSplitInput_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QStringList>("args");
    QTest::addColumn<bool>("continue_multiline");

    QTest::newRow("empty")        << ""             << QStringList {} << false;
    QTest::newRow("space")        << " "             << QStringList {"", ""} << false;
    QTest::newRow("normal")       << "bla bla"      << QStringList { "bla", "bla"}  << false;
    QTest::newRow("begin quote")  << "'bla bla'"  << QStringList { "bla bla"}  << false;
    QTest::newRow("open quote")   << "'bla bla"  << QStringList { "bla bla"}  << true;

    QTest::newRow("end space for autocomplete")    << "adsf asdf " << QStringList { "adsf", "asdf", "" }  << false;

    QTest::newRow("end quote")    << "adsf asdf '" << QStringList { "adsf", "asdf", "" }  << true;

    QTest::newRow("quoted begin") << "\\'bla bla"  << QStringList { "'bla", "bla"}  << false;

    QTest::newRow("quote middle") << "a b c 'd e"  << QStringList { "a", "b", "c", "d e"}  << true;

    QTest::newRow("quote middle in text") << "handle:'quote in'middle"
                                          << QStringList { "handle:quote inmiddle"}  << false;
    QTest::newRow("quote middle in text2") << "handle:'quote in'middle 'with quotes' at the end"
                                          << QStringList {"handle:quote inmiddle", "with quotes", "at", "the", "end"}  << false;

    QTest::newRow("space end") << "callExampleCommand 0 arg2 arg3 arg4 "
                                  << QStringList {"callExampleCommand", "0", "arg2", "arg3", "arg4", ""}  << false;

    QTest::newRow("quote in single-quoted text") << "command 'with \\'quote\\' in quote'"
                                                 << QStringList { "command", "with 'quote' in quote" } << false;
    QTest::newRow("ignore double-quote") << "command \"double quote\""
                                                 << QStringList { "command", "\"double", "quote\"" } << false;
    QTest::newRow("keep empty line") << "read\ncommand\n" << QStringList { "read", "command", "" } << false;

    QTest::newRow("escape newline") << "read\\\ncommand" << QStringList { "read\ncommand"} << false;


}

void AVConsoleTest::testSplitInput()
{
    AVLOG_ENTER_TESTCASE;
    QFETCH(QString    , input);
    QFETCH(QStringList, args);
    QFETCH(bool       , continue_multiline);
    AVLogInfo << "Line: " << input;
    auto result = AVConsole::splitInput(input);
    QCOMPARE(result.continue_multiline, continue_multiline);
    QCOMPARE(result.args, args);
}

AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(AVConsoleTest,"avlib/unittests/config")
#include "avconsoletest.moc"

// End of file
