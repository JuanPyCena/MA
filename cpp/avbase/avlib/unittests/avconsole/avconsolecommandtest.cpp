///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2013
//
// Module:    TESTS - AVCOMMON Unit Tests
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Wolfgang Aigner, w.aigner@avibit.com
    \brief   Unit tests for avconsole command base class
 */


#include <QtTest>
#include <avunittestmain.h>

// Qt includes
#include <qstring.h>
#include <qstringlist.h>

#include "avconsole.h"
#include "avconsole_int.h"
#include "avconsole_command.h"
#include "helperlib/avconsoleunittestconnection.h"

///////////////////////////////////////////////////////////////////////////////

class AVConsoleCommandExample : public AVConsoleCommand
{
public:

    enum CommandExampleEvents
    {
        INVALID_NUMBER_OF_ARGUMENTS     = 0,
        DEFAULT_COMMAND                 = 1,
        END_OF_FILE                     = 4
    };

    AVConsoleCommandExample() :
        AVConsoleCommand(COMMAND_EXAMPLE_NAME, "Calls example command.")
    {
        setMinimalNumberOfArguments(NUMBER_MIN_ARGUMENTS);
        setMaximalNumberOfArguments(NUMBER_MAX_ARGUMENTS);
    }

    ~AVConsoleCommandExample() override {}

protected:
    bool doExecute(QStringList args) override
    {
        bool ok(false);
        int command_event = args[0].toInt(&ok);
        AVASSERT(ok);

        if (command_event >= COMMAND_EXAMPLE_EVENTS_STRINGS.count())
        {
            AVLogFatal << command_event << " exceeds example event string size";
        }
        print(COMMAND_EXAMPLE_EVENTS_STRINGS[command_event]);

        return true;
    }

    void doComplete(QStringList& completions, const QStringList& args) override
    {
        if (args.count() == 1)
        {
            completions = COMMAND_EXAMPLE_COMPLETE_FIRST;
        }

        if (args.count() == 2)
        {
            completions = COMMAND_EXAMPLE_COMPLETE_SECOND;
        }

        return;
    }

public:
    const static unsigned NUMBER_MIN_ARGUMENTS;
    const static unsigned NUMBER_MAX_ARGUMENTS;

    const static QString COMMAND_EXAMPLE_NAME;

    const static QStringList COMMAND_EXAMPLE_EVENTS_STRINGS;
    const static QStringList COMMAND_EXAMPLE_COMPLETE_FIRST;
    const static QStringList COMMAND_EXAMPLE_COMPLETE_SECOND;

};

const unsigned AVConsoleCommandExample::NUMBER_MIN_ARGUMENTS = 2;
const unsigned AVConsoleCommandExample::NUMBER_MAX_ARGUMENTS = 3;

const QString AVConsoleCommandExample::COMMAND_EXAMPLE_NAME = QString("callExampleCommand");

const QStringList AVConsoleCommandExample::COMMAND_EXAMPLE_EVENTS_STRINGS =
    QStringList() << "INVALID_NUMBER_OF_ARGUMENTS" << "DEFAULT_COMMAND" << "END_OF_FILE";
const QStringList AVConsoleCommandExample::COMMAND_EXAMPLE_COMPLETE_FIRST =
    QStringList() << "complete_first" << "complete_first_a";
const QStringList AVConsoleCommandExample::COMMAND_EXAMPLE_COMPLETE_SECOND =
    QStringList() << "complete_second" << "complete_second_a" << "complete_second_b";


///////////////////////////////////////////////////////////////////////////////

class AVConsoleCommandTest : public QObject
{
    Q_OBJECT

private slots:
    void testFileConnection();
    void testCompletion();
    void testCallable();
    void testCallableCompletion();
    void testCallableLifeTime();

    virtual void init();
    virtual void cleanup();


private:
    AVConsoleCommandExample*    m_console_command;
};

///////////////////////////////////////////////////////////////////////////////

void AVConsoleCommandTest::init()
{
    if (!AVConsole::isSingletonInitialized())
    {
        AVConsole::initializeSingleton();
    }

    m_console_command = new AVConsoleCommandExample();
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleCommandTest::cleanup()
{
    delete m_console_command;
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleCommandTest::testFileConnection()
{
    AVLOG_ENTER_METHOD();

    m_console_command->registerInConsole();

    QString console_file(AVEnvironment::getApplicationConfig() + "/avconsole_example.cmd");

    AVConsoleUnitTestConnection output_connection;
    // will be deleted by AVConsole, just keep a weak pointer
    QPointer<AVConsoleFileConnection> console_pointer(new AVConsoleFileConnection(console_file, &output_connection));

    // process commands until file connection self-destructs via disconnect()
    for (uint i=0; i<50; ++i) // arbitrary limit as safeguard
    {
        qApp->processEvents();
        if (console_pointer == nullptr)
        {
            break;
        }
    }

    if (output_connection.m_received_strings.count() != 2)
    {
        AVLogError << output_connection.m_received_strings;
        QFAIL("Unexpected number of lines printed on console");
    }
    QVERIFY(output_connection.m_received_strings[0].contains(
                "ERROR: Some commands did not execute successfully"));
    QVERIFY(output_connection.m_received_strings[1].contains(
                "Illegal number of arguments"));
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleCommandTest::testCompletion()
{
    AVLOG_ENTER_TESTCASE;

    m_console_command->registerInConsole();

    AVConsole& console = AVConsole::singleton();

    QString input;
    QStringList completions;

    AVLogInfo << "Input before autocompletion: " << input;
    console.autoComplete(input, completions);
    AVLogDebug << "Input after autocompletion: " << input << ", completions: " << completions;
    QVERIFY(completions.contains(AVConsoleCommandExample::COMMAND_EXAMPLE_NAME));

    input = AVConsoleCommandExample::COMMAND_EXAMPLE_NAME;
    input.truncate(10);
    AVLogDebug << "Input before autocompletion: " << input;
    console.autoComplete(input, completions);
    AVLogDebug << "Input after autocompletion: " << input << ", completions: " << completions;
    QCOMPARE(AVConsoleCommandExample::COMMAND_EXAMPLE_NAME + " ", input);
    QCOMPARE(0, completions.count());

    input = AVConsoleCommandExample::COMMAND_EXAMPLE_NAME;
    AVLogDebug << "Input before autocompletion: " << input;
    console.autoComplete(input, completions);
    AVLogDebug << "Input after autocompletion: " << input << ", completions: " << completions;
    QVERIFY(input.endsWith(AVConsoleCommandExample::COMMAND_EXAMPLE_COMPLETE_FIRST[0]));
    QCOMPARE(
        AVConsoleCommandExample::COMMAND_EXAMPLE_COMPLETE_FIRST.count(), completions.count());

    input = AVConsoleCommandExample::COMMAND_EXAMPLE_NAME + " " +
        AVConsoleCommandExample::COMMAND_EXAMPLE_COMPLETE_FIRST[1] + " ";
    AVLogDebug << "Input before autocompletion: " << input;
    console.autoComplete(input, completions);
    AVLogDebug << "Input after autocompletion: " << input << ", completions: " << completions;
    QVERIFY(input.endsWith(AVConsoleCommandExample::COMMAND_EXAMPLE_COMPLETE_SECOND[0]));
    QCOMPARE(
        AVConsoleCommandExample::COMMAND_EXAMPLE_COMPLETE_SECOND.count(), completions.count());

}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleCommandTest::testCallable()
{
    AVLOG_ENTER_TESTCASE;

    AVConsoleUnitTestConnection output_connection;

    AVConsole& console = AVConsole::singleton();

    // Test if lambdas work
    console.registerCallback("testSlot", [](AVConsoleConnection& connection, const QStringList&)
    {
        connection.print("Test Slot");
    }, "help text");

    output_connection.sendInput("testSlot");
    qApp->processEvents();

    QCOMPARE(1, output_connection.m_received_strings.count());
    QCOMPARE(QString("Test Slot"), output_connection.m_received_strings[0]);
    output_connection.m_received_strings.clear();
    console.unregisterSlot("testSlot");

    // Test if std::function works
    std::function<void(AVConsoleConnection& connection, const QStringList& args)> echoFunc = [](AVConsoleConnection& connection, const QStringList& args)
    {
        connection.print(args[0]);
    };
    console.registerCallback("testSlot2", echoFunc, "help text");

    output_connection.sendInput("testSlot2 echo");
    qApp->processEvents();
    QCOMPARE(1, output_connection.m_received_strings.count());
    QCOMPARE(QString("echo"), output_connection.m_received_strings[0]);
    output_connection.m_received_strings.clear();
    console.unregisterSlot("testSlot2");

    // Test if bind works
    class BindTestHelper
    {
    public:
        void method(AVConsoleConnection& connection, const QStringList& args)
        {
            connection.print(args.join(" "));
        }
    };

    BindTestHelper bind_test_helper;
    console.registerCallback("name_space.bindEcho", std::bind(&BindTestHelper::method, &bind_test_helper,
                                                 std::placeholders::_1, std::placeholders::_2),
                           "help text");
    output_connection.sendInput("name_space.bindEcho echo bind");
    qApp->processEvents();
    QCOMPARE(1, output_connection.m_received_strings.count());
    QCOMPARE(QString("echo bind"), output_connection.m_received_strings[0]);
    output_connection.m_received_strings.clear();
    console.unregisterSlot("name_space.bindEcho");
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleCommandTest::testCallableCompletion()
{
    AVLOG_ENTER_TESTCASE;

    AVConsole& console = AVConsole::singleton();

    console.registerCallback("completion", [](AVConsoleConnection&, const QStringList&){}, "help text",
        [](QStringList& completions, const QStringList&)
        {
            completions << "1" << "2";
        });

    QStringList completions;
    QString input = "comp";
    console.autoComplete(input, completions);
    QCOMPARE(0, completions.count());
    QCOMPARE(QString("completion "), input);

    QStringList completions1;
    QString input1 = "completion ";
    console.autoComplete(input1, completions1);
    QCOMPARE(2, completions1.count());
    QCOMPARE(QString("1"), completions1[0]);
    QCOMPARE(QString("2"), completions1[1]);
}

///////////////////////////////////////////////////////////////////////////////

void AVConsoleCommandTest::testCallableLifeTime()
{
    AVLOG_ENTER_TESTCASE;

    auto& console = AVConsole::singleton();
    auto obj = new QObject();
    console.registerCallback("test", obj, [](AVConsoleConnection&, const QStringList&){}, "help 5chars");

    QVERIFY(console.isHandlerRegistered("test") == true);
    delete obj;
    QVERIFY(console.isHandlerRegistered("test") == false);
}


///////////////////////////////////////////////////////////////////////////////

AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(AVConsoleCommandTest,"avlib/unittests/config")
#include "avconsolecommandtest.moc"

// End of file
