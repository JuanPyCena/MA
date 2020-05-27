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
    \brief   Function level test cases for AVTimeReferenceAdapterServer
 */

#include <QtTest>
#include <avunittestmain.h>
#include "avtimereference/avtimereferenceadapterserver.h"
#include "avtimereference/avtimereferenceadapterserverconfig.h"
#include "avdatastream.h"
#include <QTcpServer>
#include "avtimereference/avtimereferenceholder.h"
#include "avdataframinglength.h"

class TstAVTimeReferenceAdapterServer : public QObject
{
    Q_OBJECT

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
    void test_AVTimeReferenceAdapterServer();
    void test_encode();
    void test_decode();
    void test_slotConnect();
    void test_slotDisconnect();
    void test_slotReceived();
    void test_slotConnected();
    void test_slotDisconnected();
    void test_slotCheckReceiveTimeout();

};

///////////////////////////////////////////////////////////////////////////////

//! Declaration of unknown metatypes
// Q_DECLARE_METATYPE(long long)

///////////////////////////////////////////////////////////////////////////////

//! This will be called before the first test function is executed.
//! It is only called once.
void TstAVTimeReferenceAdapterServer::initTestCase()
{

}

///////////////////////////////////////////////////////////////////////////////

//! This will be called after the last test function is executed.
//! It is only called once.
void TstAVTimeReferenceAdapterServer::cleanupTestCase()
{

}

///////////////////////////////////////////////////////////////////////////////

//! This will be called before each test function is executed.
void TstAVTimeReferenceAdapterServer::init()
{

}

///////////////////////////////////////////////////////////////////////////////

//! This will be called after every test function.
void TstAVTimeReferenceAdapterServer::cleanup()
{
    AVTimeReferenceAdapterServer instance("", 0);
    instance.resetProcess();
}

///////////////////////////////////////////////////////////////////////////////

void TstAVTimeReferenceAdapterServer::test_AVTimeReferenceAdapterServer()
{
    AVLogInfo << "---- test_AVTimeReferenceAdapterServer launched (" << QTest::currentDataTag() << ") ----";

    AVTimeReferenceAdapterServer instance("localhost", 1234);

    QVERIFY(!instance.m_config.isNull());
    QCOMPARE(instance.m_config->m_time_server_hostname, QString("localhost"));
    QCOMPARE(instance.m_config->m_time_server_distribution_port, 1234u);
    QVERIFY(instance.m_socket != nullptr);
    QVERIFY(instance.m_reconnect_timer.isActive());
    QVERIFY(!instance.m_framing.isNull());
    QVERIFY(!instance.m_check_receive_timeout_timer.isActive());

    QCOMPARE(instance.m_reconnect_timer.interval(), 1000);
    QCOMPARE(instance.m_check_receive_timeout_timer.interval(), 100);
    QCOMPARE(instance.m_check_receive_timeout_timer.isSingleShot(), false);
    QCOMPARE(instance.m_current_receive_timout_left, 3000);

    QVERIFY(instance.m_socket->state() != QAbstractSocket::UnconnectedState);

    AVLogInfo << "---- test_AVTimeReferenceAdapterServer ended (" << QTest::currentDataTag() << ")    ----";
}

///////////////////////////////////////////////////////////////////////////////


void TstAVTimeReferenceAdapterServer::test_encode()
{
    AVLogInfo << "---- test_encode launched (" << QTest::currentDataTag() << ") ----";

    AVDateTime reference_time(AVDateTime::currentDateTimeUtc().addMSecs(-100));
    double speed = 1.2;
    qint64 offset = 135421;

    QByteArray data = AVTimeReferenceAdapterServer::encode(reference_time, speed, offset);

    AVDataStream stream(data);
    quint32 version;
    stream >> version;
    QCOMPARE(version, 1u);

    QByteArray parameter_data;
    stream >> parameter_data;

    AVDataStream parameter_stream(parameter_data);

    AVDateTime read_reference_time;
    parameter_stream >> read_reference_time;
    QCOMPARE(read_reference_time, reference_time);

    qint64 read_offset;
    parameter_stream >> read_offset;
    QCOMPARE(read_offset, offset);

    double read_speed;
    parameter_stream >> read_speed;
    QCOMPARE(read_speed, speed);



    AVLogInfo << "---- test_encode ended (" << QTest::currentDataTag() << ")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVTimeReferenceAdapterServer::test_decode()
{
    AVLogInfo << "---- test_decode launched (" << QTest::currentDataTag() << ") ----";

    AVDateTime reference_time(AVDateTime::currentDateTimeUtc().addMSecs(-100));
    double speed = 1.2;
    qint64 offset = 135421;

    // this is tested in test_encode
    QByteArray data = AVTimeReferenceAdapterServer::encode(reference_time, speed, offset);

    AVDateTime read_reference_time;
    double read_speed;
    qint64 read_offset;
    QVERIFY(AVTimeReferenceAdapterServer::decode(data, read_reference_time, read_speed, read_offset));

    QCOMPARE(read_reference_time, reference_time);
    QCOMPARE(read_speed, speed);
    QCOMPARE(read_offset, offset);

    AVLogInfo << "---- test_decode ended (" << QTest::currentDataTag() << ")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVTimeReferenceAdapterServer::test_slotConnect()
{
    AVLogInfo << "---- test_slotConnect launched (" << QTest::currentDataTag() << ") ----";

    QTcpServer server;
    server.listen(QHostAddress::Any);
    AVTimeReferenceAdapterServer instance("localhost", server.serverPort());

    QSignalSpy connect_spy(&server, SIGNAL(newConnection()));

    instance.slotConnect();

    QVERIFY(connect_spy.wait(1000));

    AVLogInfo << "---- test_slotConnect ended (" << QTest::currentDataTag() << ")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVTimeReferenceAdapterServer::test_slotDisconnect()
{
    AVLogInfo << "---- test_slotDisconnect launched (" << QTest::currentDataTag() << ") ----";

    QTcpServer server;
    server.listen(QHostAddress::Any);

    AVTimeReferenceAdapterServer instance("localhost", server.serverPort());

    QTest::qWait(200);

    QVERIFY(server.hasPendingConnections());

    QTcpSocket* socket = server.nextPendingConnection();

    QVERIFY(socket->state() == QAbstractSocket::ConnectedState);

    instance.slotDisconnect();

    QTest::qWait(200);
    QVERIFY(socket->state() != QAbstractSocket::ConnectedState);

    AVLogInfo << "---- test_slotDisconnect ended (" << QTest::currentDataTag() << ")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVTimeReferenceAdapterServer::test_slotReceived()
{
    AVLogInfo << "---- test_slotReceived launched (" << QTest::currentDataTag() << ") ----";

    QDateTime reference_time = AVDateTime::currentDateTimeUtc().addMSecs(-100);
    double speed = 1.2;
    qint64 offset = 135421;

    AVTimeReferenceAdapterServer instance("localhost", 1234);

    QByteArray msg = AVTimeReferenceAdapterServer::encode(reference_time, speed, offset);

    instance.slotReceived(msg);

    QCOMPARE(instance.getLocalTimeReferenceHolder().getReferenceDateTime(), reference_time);
    QCOMPARE(instance.getLocalTimeReferenceHolder().getOffset(), offset);
    QCOMPARE(instance.getLocalTimeReferenceHolder().getSpeed(), speed);

    AVLogInfo << "---- test_slotReceived ended (" << QTest::currentDataTag() << ")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVTimeReferenceAdapterServer::test_slotConnected()
{
    AVLogInfo << "---- test_slotConnected launched (" << QTest::currentDataTag() << ") ----";

    AVTimeReferenceAdapterServer instance("localhost", 1234);
    instance.m_current_receive_timout_left = 0;
    instance.slotConnected();

    QCOMPARE(instance.m_current_receive_timout_left > 0, true);
    QVERIFY(instance.m_check_receive_timeout_timer.isActive());

    AVLogInfo << "---- test_slotConnected ended (" << QTest::currentDataTag() << ")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVTimeReferenceAdapterServer::test_slotDisconnected()
{
    AVLogInfo << "---- test_slotDisconnected launched (" << QTest::currentDataTag() << ") ----";

    AVTimeReferenceAdapterServer instance("localhost", 1234);
    instance.slotConnected();
    instance.m_framing->slotGotData("bla");
    instance.slotDisconnected();

    QVERIFY(!instance.m_check_receive_timeout_timer.isActive());

    AVLogInfo << "---- test_slotDisconnected ended (" << QTest::currentDataTag() << ")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVTimeReferenceAdapterServer::test_slotCheckReceiveTimeout()
{
    AVLogInfo << "---- test_slotCheckReceiveTimeout launched (" << QTest::currentDataTag() << ") ----";

    QTcpServer server;
    server.listen(QHostAddress::Any);
    AVTimeReferenceAdapterServer instance("localhost", server.serverPort());

    QSignalSpy connect_spy(&server, SIGNAL(newConnection()));

    instance.slotConnect();

    QVERIFY(connect_spy.wait(1000));

    QTcpSocket* socket = server.nextPendingConnection();

    QVERIFY(socket->state() == QAbstractSocket::ConnectedState);

    instance.m_current_receive_timout_left = 300;

    instance.slotCheckReceiveTimeout();

    QCOMPARE(instance.m_current_receive_timout_left, 200);

    instance.slotCheckReceiveTimeout();

    QCOMPARE(instance.m_current_receive_timout_left, 100);

    instance.slotCheckReceiveTimeout();

    QCOMPARE(instance.m_current_receive_timout_left, 0);

    QTest::qWait(50);

    QVERIFY(socket->state() != QAbstractSocket::ConnectedState);

    AVLogInfo << "---- test_slotCheckReceiveTimeout ended (" << QTest::currentDataTag() << ")    ----";
}

///////////////////////////////////////////////////////////////////////////////

AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(TstAVTimeReferenceAdapterServer,"avlib/unittests/config")
#include "tstavtimereferenceadapterserver.moc"

// End of file
