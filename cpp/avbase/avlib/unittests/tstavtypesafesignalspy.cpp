//-----------------------------------------------------------------------------
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2020
//
// Module:    TESTS - AVCOMMON Unit Tests
//
//-----------------------------------------------------------------------------

/*! \file
    \author  Daniel Brunner, daniel.brunner@adbsafegate.com
    \brief   Function level test cases for AVTypesafeSignalspy
 */

#include <QtTest>
#include <avunittestmain.h>

#include "avtypesafesignalspy.h"

class TstAVTypesafeSignalspy : public QObject
{
    Q_OBJECT

private slots:
    void test_AVTypesafeSignalspy();
    void test_avSpyFor();
    void test_copyConstructor();
    void test_assignmentOperator();
};

namespace {
class TestSender : public QObject
{
    Q_OBJECT
signals:
    void signalTest(int, QSet<QString>);
};
}

//-----------------------------------------------------------------------------

void TstAVTypesafeSignalspy::test_AVTypesafeSignalspy()
{
    AVLogInfo << "---- test_AVTypesafeSignalspy launched (" << QTest::currentDataTag() << ") ----";

    TestSender sender;

    AVTypesafeSignalSpy<TestSender, int, QSet<QString>> signalspy(&sender, &TestSender::signalTest);
    QVERIFY(signalspy.isEmpty());

    {
        emit sender.signalTest(4, {"a", "b"});

        const decltype(signalspy)::Container expected{
            { 4, {"a", "b"} }
        };
        QCOMPARE(signalspy, expected);
    }

    {
        emit sender.signalTest(6, {});

        const decltype(signalspy)::Container expected{
            { 4, {"a", "b"} },
            { 6, {} }
        };
        QCOMPARE(signalspy, expected);
    }

    AVLogInfo << "---- test_AVTypesafeSignalspy ended (" << QTest::currentDataTag() << ")    ----";
}

//-----------------------------------------------------------------------------

void TstAVTypesafeSignalspy::test_avSpyFor()
{
    AVLogInfo << "---- test_avSpyFor launched (" << QTest::currentDataTag() << ") ----";

    TestSender sender;

    auto signalspy = avSpyFor(&sender, &TestSender::signalTest);
    QVERIFY(signalspy.isEmpty());

    {
        emit sender.signalTest(4, {"a", "b"});

        const decltype(signalspy)::Container expected{
            { 4, {"a", "b"} }
        };
        QCOMPARE(signalspy, expected);
    }

    {
        emit sender.signalTest(6, {});

        const decltype(signalspy)::Container expected{
            { 4, {"a", "b"} },
            { 6, {} }
        };
        QCOMPARE(signalspy, expected);
    }

    AVLogInfo << "---- test_avSpyFor ended (" << QTest::currentDataTag() << ")    ----";
}

//-----------------------------------------------------------------------------

void TstAVTypesafeSignalspy::test_copyConstructor()
{
    AVLogInfo << "---- test_copyConstructor launched (" << QTest::currentDataTag() << ") ----";

    TestSender sender;

    auto signalspy = avSpyFor(&sender, &TestSender::signalTest);

    emit sender.signalTest(4, {"a", "b"});
    auto copy{signalspy};

    {
        const decltype(signalspy)::Container expected{
            { 4, {"a", "b"} }
        };
        QCOMPARE(signalspy, expected);
        QCOMPARE(copy, expected);
    }

    {
        emit sender.signalTest(6, {});

        const decltype(signalspy)::Container expected{
            { 4, {"a", "b"} },
            { 6, {} }
        };
        QCOMPARE(signalspy, expected);
        QCOMPARE(copy, expected);
    }

    AVLogInfo << "---- test_copyConstructor ended (" << QTest::currentDataTag() << ")    ----";
}

//-----------------------------------------------------------------------------

void TstAVTypesafeSignalspy::test_assignmentOperator()
{
    AVLogInfo << "---- test_assignmentOperator launched (" << QTest::currentDataTag() << ") ----";

    TestSender sender0, sender1;

    auto signalspy0 = avSpyFor(&sender0, &TestSender::signalTest);
    auto signalspy1 = avSpyFor(&sender1, &TestSender::signalTest);

    auto signalspy = signalspy0;

    {
        emit sender0.signalTest(4, {"a", "b"});
        const decltype(signalspy)::Container expected{
            { 4, {"a", "b"} }
        };
        QCOMPARE(signalspy, expected);
    }

    signalspy = signalspy1;
    QVERIFY(signalspy1.isEmpty());
    QVERIFY(signalspy.isEmpty());

    {
        emit sender1.signalTest(6, {});
        const decltype(signalspy)::Container expected{
            { 6, {} }
        };
        QCOMPARE(signalspy, expected);
    }

    signalspy = signalspy0;

    {
        const decltype(signalspy)::Container expected{
            { 4, {"a", "b"} }
        };
        QCOMPARE(signalspy, expected);
    }

    AVLogInfo << "---- test_assignmentOperator ended (" << QTest::currentDataTag() << ")    ----";
}

//-----------------------------------------------------------------------------

AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(TstAVTypesafeSignalspy,"avlib/unittests/config")
#include "tstavtypesafesignalspy.moc"
