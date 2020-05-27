///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON
// Copyright: AviBit data processing GmbH, 2001-2014
// QT-Version: QT5
//
// Module:    AVLIB
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   AVReplayStateNotifier implementation
 */

// Qt includes

// avlib includes
#include "avlog.h"
#include "avdatastream.h"

// local includes
#include "avreplaystatenotifier.h"
#include "avtimereference.h"

///////////////////////////////////////////////////////////////////////////////

AVReplayStateNotifier::AVReplayStateNotifier(const QString& process_identifier, const QString& server_hostname, uint server_port) :
    m_process_identifier(process_identifier),
    m_replay_state(Initializing),
    m_reconnect_timer(this),
    m_server_hostname(server_hostname),
    m_server_port(server_port)
{
    connect(&AVTimeReference::singleton(), &AVTimeReference::signalTimeJumped,
            this, &AVReplayStateNotifier::slotTimeJumped);

    m_socket.reset(new QTcpSocket());

    connect(&m_reconnect_timer, &QTimer::timeout, this, &AVReplayStateNotifier::slotConnect);
    connect(m_socket.data(), &QTcpSocket::connected, this, &AVReplayStateNotifier::slotConnected);

    m_reconnect_timer.setInterval(1000);
    m_reconnect_timer.start();

    slotConnect();
}

///////////////////////////////////////////////////////////////////////////////

AVReplayStateNotifier::~AVReplayStateNotifier()
{
}

///////////////////////////////////////////////////////////////////////////////

QByteArray AVReplayStateNotifier::encode(const QString& process_identifier, ReplayState state)
{
    if(process_identifier.isEmpty())
    {
        AVLogError << "AVReplayStateNotifier::encode: cannod encode empty process identifier";
        return QByteArray();
    }

    QByteArray data;
    AVDataStream stream(&data, QIODevice::WriteOnly);

    quint32 version = 1;
    stream << version;

    stream << process_identifier;

    stream << static_cast<qint32>(state);

    return data;
}

///////////////////////////////////////////////////////////////////////////////

bool AVReplayStateNotifier::decode(const QByteArray& data,QString& process_identifier, ReplayState& state)
{
    AVDataStream stream(data);

    quint32 version = 0;
    stream >> version;
    if(version != 1)
    {
        AVLogError << "AVReplayStateNotifier::decode: unknown version "<<version;
        return false;
    }

    if(stream.status() != AVDataStream::Ok)
    {
        AVLogError << "AVReplayStateNotifier::decode: parsing error";
        return false;
    }

    stream >> process_identifier;

    if(stream.status() != AVDataStream::Ok)
    {
        AVLogError << "AVReplayStateNotifier::decode: parsing error";
        return false;
    }

    qint32 state_value;
    stream >> state_value;
    state = ReplayState(state_value);

    if(stream.status() != AVDataStream::Ok)
    {
        AVLogError << "AVReplayStateNotifier::decode: parsing error";
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

void AVReplayStateNotifier::slotSetReplayState(AVReplayStateNotifier::ReplayState state)
{
    if(m_replay_state == Initializing && state == ReadyForInitialData)
    {
        // I have started up and wait for data

        m_replay_state = ReadyForInitialData;
    }
    else if(m_replay_state == ReadyForInitialData && state == ProcessingInitialData)
    {
        // I received the data and process it

        m_replay_state = ProcessingInitialData;
    }
    else if(m_replay_state == ProcessingInitialData  && state == Normal)
    {
        // I processed initial data and am ready for replay

        m_replay_state = Normal;
    }
    else
    {
        AVLogError << "AVReplayStateNotifier::slotSetReplayState: cannot have transition from "
                <<enumToString(m_replay_state) << " to "<< enumToString(state);
        return;
    }

    slotSubmit();
}

///////////////////////////////////////////////////////////////////////////////

void AVReplayStateNotifier::slotTimeJumped()
{
    AVLogInfo << "AVReplayStateNotifier::slotTimeJumped: setting to "<<enumToString(Initializing);

    m_replay_state = Initializing;

    slotSubmit();
}

///////////////////////////////////////////////////////////////////////////////

void AVReplayStateNotifier::slotConnect()
{
    if(m_socket->state() == QAbstractSocket::ConnectedState)
        return;

    if(!m_server_hostname.isEmpty() && m_server_port > 0)
    {
        AVLogInfo << "AVReplayStateNotifier::slotConnect: connecting to time reference server ("
                <<m_server_hostname<<", port "<<m_server_port<<")";

        m_socket->connectToHost(m_server_hostname, m_server_port);
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVReplayStateNotifier::slotConnected()
{
    AVLogInfo << "AVReplayStateNotifier::slotConnected: connected to replay state notification port of time reference server";
}

///////////////////////////////////////////////////////////////////////////////

void AVReplayStateNotifier::slotSubmit()
{
    if(m_socket.isNull() || m_socket->state() != QAbstractSocket::ConnectedState)
    {
        AVLogError << "AVReplayStateNotifier::slotSubmit: cannot submit state "<<enumToString(m_replay_state)
                <<", not connected";
        return;
    }

    QByteArray data = encode(m_process_identifier, m_replay_state);
    AVASSERT(data.size() > 0);

    AVDataStream write_stream(m_socket.data());
    write_stream << data;
}

///////////////////////////////////////////////////////////////////////////////

// End of file
