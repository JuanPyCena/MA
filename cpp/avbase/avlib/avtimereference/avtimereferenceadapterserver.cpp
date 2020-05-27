///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON
// Copyright:  AviBit data processing GmbH, 2001-2014
// QT-Version: QT5
//
// Module:     AVLIB
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   AVTimeReferenceAdapterServer implementation
 */

// Qt includes
#include <QElapsedTimer>

// avlib includes
#include "avdatastream.h"
#include "avdatetime.h"

// local includes
#include "avtimereferenceadapterserver.h"
#include "avdataframinglength.h"
#include "avtimereferenceholder.h"
#include "avdaemon.h"
#include "avtimereferenceadapterserverconfig.h"
#include "avreplaystatenotifier.h"

///////////////////////////////////////////////////////////////////////////////

const quint32 AVTimeReferenceAdapterServer::CURRENT_TIMEREFERENCE_VERSION = 1;
const uint AVTimeReferenceAdapterServer::RECONNECT_INTERVAL = 1000;
const uint AVTimeReferenceAdapterServer::RECEIVE_TIMEOUT = 3000;
const uint AVTimeReferenceAdapterServer::RECEIVE_TIMEOUT_CHECK_INTERVAL = 100;

///////////////////////////////////////////////////////////////////////////////

AVTimeReferenceAdapterServer::AVTimeReferenceAdapterServer(QSharedPointer<AVTimeReferenceAdapterServerConfigBase> config, AdapterType type) :
    AVTimeReferenceAdapter(type),
    m_config(config),
    m_reconnect_timer(this),
    m_check_receive_timeout_timer(this),
    m_current_receive_timout_left(RECEIVE_TIMEOUT),
    m_received_message(false)
{
    initialize();
}

///////////////////////////////////////////////////////////////////////////////

AVTimeReferenceAdapterServer::AVTimeReferenceAdapterServer(const QString& hostname, uint port, AdapterType type) :
    AVTimeReferenceAdapter(type),
    m_config(new AVTimeReferenceAdapterServerConfigBase()),
    m_reconnect_timer(this),
    m_check_receive_timeout_timer(this),
    m_current_receive_timout_left(RECEIVE_TIMEOUT),
    m_received_message(false)
{
    m_config->m_time_server_hostname = hostname;
    m_config->m_time_server_distribution_port = port;

    initialize();
}

///////////////////////////////////////////////////////////////////////////////

AVTimeReferenceAdapterServer::~AVTimeReferenceAdapterServer()
{
    // Ignore final disconnect from the socket when it is destroyed (SWE-4350)
    // This also means that this class won't emit signalDisconnected upon destruction.
    m_socket->disconnect();
}

///////////////////////////////////////////////////////////////////////////////

void AVTimeReferenceAdapterServer::initialize()
{
    AVLogInfo << "AVTimeReferenceAdapterServer::initialize: initialize AVTimeReferenceAdapterServer";

    m_socket.reset(new QTcpSocket());

    m_framing.reset(new AVDataFramingLength(8, 4, AVDataFramingLength::INT));

    if(!m_config->m_time_server_replay_notification_process_identifier.isEmpty() &&
            !m_config->m_time_server_hostname.isEmpty() && m_config->m_time_server_replay_notification_port > 0)
    {
        m_replay_state_notifier.reset(new AVReplayStateNotifier(
                                          m_config->m_time_server_replay_notification_process_identifier,
                                          m_config->m_time_server_hostname,
                                          m_config->m_time_server_replay_notification_port));
    }

    connect(&m_reconnect_timer, &QTimer::timeout, this, &AVTimeReferenceAdapterServer::slotConnect);
    connect(m_socket.get(), &QTcpSocket::connected, this, &AVTimeReferenceAdapterServer::slotConnected);
    // Qt3 QSocket::connectionClosed() is different to Qt5 QTcpSocket::disconnected()
    // QSocket::connectionClosed() is emitted when the other end has closed the connection, where as QTcpSocket::disconnected()
    // is emitted when the socket has been disconnected (also on our end).
    connect(m_socket.get(), &QTcpSocket::disconnected, this, &AVTimeReferenceAdapterServer::slotDisconnected);
    connect(m_socket.get(), &QTcpSocket::readyRead, this, &AVTimeReferenceAdapterServer::slotReadyRead);
    connect(m_framing.data(), &AVDataFramingLength::signalGotFullMessage, this, &AVTimeReferenceAdapterServer::slotReceived);
    connect(&m_check_receive_timeout_timer, &QTimer::timeout, this, &AVTimeReferenceAdapterServer::slotCheckReceiveTimeout);

    slotConnect();

    m_reconnect_timer.setInterval(RECONNECT_INTERVAL);
    m_reconnect_timer.start();

    m_check_receive_timeout_timer.setInterval(RECEIVE_TIMEOUT_CHECK_INTERVAL);
}

///////////////////////////////////////////////////////////////////////////////

void AVTimeReferenceAdapterServer::setSpeed(double speed)
{
    Q_UNUSED(speed);
    AVLogFatal << "AVTimeReferenceAdapterServer::setSpeed: cannot set time/speed using this class, "
                  "use AVTimeReferenceAdapterServerController";
}

///////////////////////////////////////////////////////////////////////////////

void AVTimeReferenceAdapterServer::setCurrentDateTime(const QDateTime& current_date_time)
{
    Q_UNUSED(current_date_time);
    AVLogFatal << "AVTimeReferenceAdapterServer::setCurrentDateTime: cannot set time/speed using this class, "
                  "use AVTimeReferenceAdapterServerController";
}

///////////////////////////////////////////////////////////////////////////////

void AVTimeReferenceAdapterServer::setCurrentDateTimeSpeed(const QDateTime& current_date_time, double speed)
{
    Q_UNUSED(current_date_time);
    Q_UNUSED(speed);
    AVLogFatal << "AVTimeReferenceAdapterServer::setCurrentDateTimeSpeed: cannot set time/speed using this class, "
                  "use AVTimeReferenceAdapterServerController";
}

///////////////////////////////////////////////////////////////////////////////

void AVTimeReferenceAdapterServer::setCurrentDateTimeShiftedOnly(const QDateTime& current_date_time)
{
    Q_UNUSED(current_date_time);
    AVLogFatal << "AVTimeReferenceAdapterServer::setCurrentDateTimeShiftedOnly: cannot set time/speed using this class, "
                  "use AVTimeReferenceAdapterServerController";
}

///////////////////////////////////////////////////////////////////////////////

void AVTimeReferenceAdapterServer::startNewTimeReferenceSession()
{
    AVLogFatal << "AVTimeReferenceAdapterServer::startNewTimeReferenceSession: cannot set time/speed using this class, "
                  "use AVTimeReferenceAdapterServerController";
}

///////////////////////////////////////////////////////////////////////////////

void AVTimeReferenceAdapterServer::reset()
{
    AVLogFatal << "AVTimeReferenceAdapterServer::reset: cannot set time/speed using this class, "
                  "use AVTimeReferenceAdapterServerController";
}

///////////////////////////////////////////////////////////////////////////////

QByteArray AVTimeReferenceAdapterServer::encode(const QDateTime& reference_time, double speed, qint64 offset)
{
    QByteArray data;
    AVDataStream ds1(&data, QIODevice::WriteOnly);
    ds1 << reference_time;
    ds1 << offset;
    ds1 << speed;
    ds1 << reference_time;
    ds1 << reference_time;

    QByteArray full_package;
    AVDataStream ds(&full_package, QIODevice::WriteOnly);
    ds << CURRENT_TIMEREFERENCE_VERSION;
    ds << data; // includes size of byte array

    return full_package;
}

///////////////////////////////////////////////////////////////////////////////

bool AVTimeReferenceAdapterServer::decode(const QByteArray& data, QDateTime& reference_time, double& speed, qint64& offset)
{
    quint32 version = 0;

    AVDataStream read_stream(data);
    read_stream >> version;

    if(version == 1)
    {
        QByteArray parameter_data;
        read_stream >> parameter_data;

        if(read_stream.status() != AVDataStream::Ok)
            return false;

        AVDataStream parameter_read_stream(parameter_data);
        parameter_read_stream >> reference_time;
        parameter_read_stream >> offset;
        parameter_read_stream >> speed;

        if(parameter_read_stream.status() != AVDataStream::Ok)
            return false;
    }
    else
    {
        AVLogError << "AVTimeReferenceAdapterServer::decode: unknown version "<< version;
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVTimeReferenceAdapterServer::waitReceivedMessage(uint timeout)
{
    if(m_socket->state() == QAbstractSocket::UnconnectedState) // not configured
    {
        AVLogInfo << "AVTimeReferenceAdapterServer::waitReceivedMessage: not waiting for a time reference message, "
                     "no time reference server is configured";
        return true;
    }

    AVLogInfo << "AVTimeReferenceAdapterServer::waitReceivedMessage: waiting for connection to time reference server";

    QElapsedTimer timer;
    timer.start();

    while(!AVProcessTerminated && timer.elapsed() < timeout)
    {
        if(m_received_message)
            break;
        runEventLoop(100);
    }

    if(!m_received_message)
        return false;

    AVLogInfo << "AVTimeReferenceAdapterServer::waitReceivedMessage: received initial time information";

    return true;
}

///////////////////////////////////////////////////////////////////////////////

QSharedPointer<AVReplayStateNotifier> AVTimeReferenceAdapterServer::getReplayStateNotifier()
{
    return m_replay_state_notifier;
}

///////////////////////////////////////////////////////////////////////////////

void AVTimeReferenceAdapterServer::slotConnect()
{
    if(m_socket->state() == QAbstractSocket::ConnectedState)
        return;

    m_check_receive_timeout_timer.stop(); // activated on connected

    if(!m_config->m_time_server_hostname.isEmpty() && m_config->m_time_server_distribution_port > 0)
    {
        AVLogInfo << AVLogStream::SuppressRepeated(2)
                  << "AVTimeReferenceAdapterServer::slotConnect: connecting to time reference server ("
                  <<m_config->m_time_server_hostname<<", port "<<m_config->m_time_server_distribution_port<<")";

        QHostAddress host_address;
        if (!AVResolveHost(m_config->m_time_server_hostname, host_address))
        {
            AVLogFatal << "AVTimeReferenceAdapterServer::slotConnect: Failed to resolve host " << m_config->m_time_server_hostname;
        }

        m_socket->connectToHost(host_address.toString(), m_config->m_time_server_distribution_port);
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVTimeReferenceAdapterServer::slotDisconnect()
{
    m_check_receive_timeout_timer.stop();
    m_socket->disconnectFromHost();
}

///////////////////////////////////////////////////////////////////////////////

void AVTimeReferenceAdapterServer::slotReadyRead()
{
    m_framing->slotGotData(m_socket->readAll());
}

///////////////////////////////////////////////////////////////////////////////

void AVTimeReferenceAdapterServer::slotReceived(const QByteArray& data)
{
    AVDateTime reference_time;
    double speed;
    qint64 offset;

    if(!decode(data, reference_time, speed, offset))
    {
        AVLogError << "AVTimeReferenceAdapterServer::slotReceived: could not decode received data";
        slotDisconnect();
        return;
    }

    m_current_receive_timout_left = RECEIVE_TIMEOUT;

    AVLogDebug1 << "AVTimeReferenceAdapterServer::slotReceived: received timing information "
                <<" reference time: "<<reference_time<<", speed: "<<speed<<", offset: "<<offset;

    setProcessTimeReferenceParameters(reference_time, speed, offset);

    m_received_message = true;
}

///////////////////////////////////////////////////////////////////////////////

void AVTimeReferenceAdapterServer::slotConnected()
{
    AVLogInfo << "AVTimeReferenceAdapterServer::slotConnected: connected to time reference server";

    m_current_receive_timout_left = RECEIVE_TIMEOUT;
    m_check_receive_timeout_timer.start();

    emit signalConnected();
}

///////////////////////////////////////////////////////////////////////////////

void AVTimeReferenceAdapterServer::slotDisconnected()
{
    AVLogInfo << "AVTimeReferenceAdapterServer::slotDisconnected: disconnected from time reference server";

    m_framing->reset();
    m_check_receive_timeout_timer.stop();

    emit signalDisconnected();
}

///////////////////////////////////////////////////////////////////////////////

void AVTimeReferenceAdapterServer::slotCheckReceiveTimeout()
{
    m_current_receive_timout_left -= RECEIVE_TIMEOUT_CHECK_INTERVAL;
    if(m_current_receive_timout_left > 0)
        return;

    AVLogError << "AVTimeReferenceAdapterServer::slotCheckReceiveTimeout: timeout detected";
    slotDisconnect();
}

///////////////////////////////////////////////////////////////////////////////

// End of file
