///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    UNITTESTS
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   Function level test cases for AVReplayStateNotifier
 */

#include <QtTest>
#include <QTcpServer>
#include <avunittestmain.h>
#include "avtimereference/avreplaystatenotifier.h"
#include "avdatastream.h"

class TstAVReplayStateNotifier : public QObject
{
    Q_OBJECT

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
    void test_AVReplayStateNotifier();
    void test_encode();
    void test_decode();
    void test_slotSetReplayState();
    void test_slotTimeJumped();
    void test_slotConnect();
    void test_slotConnected();
    void test_slotSubmit();

};

///////////////////////////////////////////////////////////////////////////////

//! Declaration of unknown metatypes
// Q_DECLARE_METATYPE(AVReplayStateNotifier::ReplayState)
// Q_DECLARE_METATYPE(QList<AVReplayStateNotifier::ReplayState>)

///////////////////////////////////////////////////////////////////////////////

//! This will be called before the first test function is executed.
//! It is only called once.
void TstAVReplayStateNotifier::initTestCase()
{

}

///////////////////////////////////////////////////////////////////////////////

//! This will be called after the last test function is executed.
//! It is only called once.
void TstAVReplayStateNotifier::cleanupTestCase()
{

}

///////////////////////////////////////////////////////////////////////////////

//! This will be called before each test function is executed.
void TstAVReplayStateNotifier::init()
{

}

///////////////////////////////////////////////////////////////////////////////

//! This will be called after every test function.
void TstAVReplayStateNotifier::cleanup()
{

}

///////////////////////////////////////////////////////////////////////////////

void TstAVReplayStateNotifier::test_AVReplayStateNotifier()
{
    AVLogInfo << "---- test_AVReplayStateNotifier launched (" << QTest::currentDataTag() << ") ----";

    AVReplayStateNotifier instance("my_proc", "localhost", 1234);
    QCOMPARE(instance.m_process_identifier, QString("my_proc"));
    QCOMPARE(instance.m_replay_state, AVReplayStateNotifier::Initializing);
    QVERIFY(!instance.m_socket.isNull());
    QVERIFY(instance.m_reconnect_timer.isActive());
    QCOMPARE(instance.m_server_hostname, QString("localhost"));
    QCOMPARE(instance.m_server_port, 1234u);

    QVERIFY(instance.m_socket->state() != QAbstractSocket::UnconnectedState);

    AVLogInfo << "---- test_AVReplayStateNotifier ended (" << QTest::currentDataTag() << ")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVReplayStateNotifier::test_encode()
{
    AVLogInfo << "---- test_encode launched (" << QTest::currentDataTag() << ") ----";

    QString process_identifier = "my_proc";
    AVReplayStateNotifier::ReplayState state = AVReplayStateNotifier::ReadyForInitialData;

    QByteArray data = AVReplayStateNotifier::encode(process_identifier, state);

    QString process_identifier1;
    AVReplayStateNotifier::ReplayState state1;
    QVERIFY(AVReplayStateNotifier::decode(data, process_identifier1, state1));

    QCOMPARE(process_identifier1, process_identifier);
    QCOMPARE(state1, state);

    process_identifier = "my_other_proc";
    state = AVReplayStateNotifier::Normal;


    data = AVReplayStateNotifier::encode(process_identifier, state);
    QVERIFY(AVReplayStateNotifier::decode(data, process_identifier1, state1));

    QCOMPARE(process_identifier1, process_identifier);
    QCOMPARE(state1, state);

    AVLogInfo << "---- test_encode ended (" << QTest::currentDataTag() << ")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVReplayStateNotifier::test_decode()
{
    AVLogInfo << "---- test_decode launched (" << QTest::currentDataTag() << ") ----";

    // tested in test_encode

    AVLogInfo << "---- test_decode ended (" << QTest::currentDataTag() << ")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVReplayStateNotifier::test_slotSetReplayState()
{
    AVLogInfo << "---- test_slotSetReplayState launched (" << QTest::currentDataTag() << ") ----";

    QTcpServer server;
    server.listen(QHostAddress::Any);

    AVReplayStateNotifier instance("process1", "localhost", server.serverPort());

    QTest::qWait(200);

    QVERIFY(instance.m_socket->state() == QAbstractSocket::ConnectedState);
    QTcpSocket* server_socket = server.nextPendingConnection();
    QVERIFY(server_socket != 0);

    instance.slotSetReplayState(AVReplayStateNotifier::ReadyForInitialData);

    QTest::qWait(200);

    QByteArray received_data = server_socket->readAll();
    AVDataStream read_stream(received_data);
    QByteArray data;
    read_stream >> data;

    QString process_identifier1;
    AVReplayStateNotifier::ReplayState state1;
    QVERIFY(AVReplayStateNotifier::decode(data, process_identifier1, state1));

    QCOMPARE(process_identifier1, QString("process1"));
    QCOMPARE(state1, AVReplayStateNotifier::ReadyForInitialData);

    AVLogInfo << "---- test_slotSetReplayState ended (" << QTest::currentDataTag() << ")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVReplayStateNotifier::test_slotTimeJumped()
{
    AVLogInfo << "---- test_slotTimeJumped launched (" << QTest::currentDataTag() << ") ----";

    AVReplayStateNotifier instance("process1", "localhost", 1234);
    instance.slotSetReplayState(AVReplayStateNotifier::ReadyForInitialData);
    instance.slotSetReplayState(AVReplayStateNotifier::ProcessingInitialData);

    QCOMPARE(instance.m_replay_state, AVReplayStateNotifier::ProcessingInitialData);

    instance.slotTimeJumped();

    QCOMPARE(instance.m_replay_state, AVReplayStateNotifier::Initializing);

    AVLogInfo << "---- test_slotTimeJumped ended (" << QTest::currentDataTag() << ")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVReplayStateNotifier::test_slotConnect()
{
    AVLogInfo << "---- test_slotConnect launched (" << QTest::currentDataTag() << ") ----";

    QTcpServer server;
    server.listen(QHostAddress::Any);

    AVReplayStateNotifier instance("proc1", "localhost", server.serverPort()); // calls slotConnect

    QTest::qWait(200);

    QVERIFY(server.hasPendingConnections());
    QVERIFY(instance.m_socket->state() == QAbstractSocket::ConnectedState);

    AVLogInfo << "---- test_slotConnect ended (" << QTest::currentDataTag() << ")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVReplayStateNotifier::test_slotConnected()
{
    AVLogInfo << "---- test_slotConnected launched (" << QTest::currentDataTag() << ") ----";

    AVLogInfo << "---- test_slotConnected ended (" << QTest::currentDataTag() << ")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVReplayStateNotifier::test_slotSubmit()
{
    AVLogInfo << "---- test_slotSubmit launched (" << QTest::currentDataTag() << ") ----";

    // tested in test_slotSetReplayState

    AVLogInfo << "---- test_slotSubmit ended (" << QTest::currentDataTag() << ")    ----";
}

///////////////////////////////////////////////////////////////////////////////

AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(TstAVReplayStateNotifier,"avlib/unittests/config")
#include "tstavreplaystatenotifier.moc"

// End of file
