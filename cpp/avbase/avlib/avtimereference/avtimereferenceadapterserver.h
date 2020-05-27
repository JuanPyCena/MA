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
    \brief   AVTimeReferenceAdapterServer header
 */

#ifndef AVTIMEREFERENCEADAPTERSERVER_INCLUDED
#define AVTIMEREFERENCEADAPTERSERVER_INCLUDED

// Qt includes
#include <QObject>
#include <QSharedPointer>
#include <QTimer>
#include <limits>

// avlib includes
#include "avlib_export.h"
#include "avmacros.h"

// local includes
#include "avtimereferenceadapter.h"

// forward declarations
class AVDataFramingLength;
class AVTimeReferenceAdapterServerConfigBase;
class AVReplayStateNotifier;

// avoid mixup with macro in windows
#ifdef max
#undef max
#endif


///////////////////////////////////////////////////////////////////////////////
//! Time reference adapter for receiving time information from the AviBit time reference server
/*!
*/
class AVLIB_EXPORT AVTimeReferenceAdapterServer : public AVTimeReferenceAdapter
{
    Q_OBJECT
    AVDISABLECOPY(AVTimeReferenceAdapterServer);

    //! friend declaration for function level test case
    friend class TstAVTimeReferenceAdapterServer;

public:
    explicit AVTimeReferenceAdapterServer(QSharedPointer<AVTimeReferenceAdapterServerConfigBase> config,
                                          AdapterType type = AdapterType::AdapterServer);
    explicit AVTimeReferenceAdapterServer(const QString& hostname, uint port,
                                          AdapterType type = AdapterType::AdapterServer);
    ~AVTimeReferenceAdapterServer() override;

    static const quint32 CURRENT_TIMEREFERENCE_VERSION;

    //! Dummy method, this can't be used, as this class only receives reference time
    void setSpeed(double speed) override;

    //! Dummy method, this can't be used, as this class only receives reference time
    void setCurrentDateTime(const QDateTime& current_date_time) override;

    //! Dummy method, this can't be used, as this class only receives reference time
    void setCurrentDateTimeSpeed(const QDateTime& current_date_time, double speed) override;

    //! Dummy method, this can't be used, as this class only receives reference time
    void setCurrentDateTimeShiftedOnly(const QDateTime& current_date_time) override;

    //! Dummy method, this can't be used, as this class only receives reference time
    void startNewTimeReferenceSession() override;

    //! Dummy method, this can't be used, as this class only receives reference time
    void reset() override;

    static QByteArray encode(const QDateTime& reference_time, double speed, qint64 offset);

    static bool decode(const QByteArray& data, QDateTime& reference_time, double& speed, qint64& offset);

    bool waitReceivedMessage(uint timeout = std::numeric_limits<uint>::max());

    QSharedPointer<AVReplayStateNotifier> getReplayStateNotifier() override;

    static const uint RECONNECT_INTERVAL;
    static const uint RECEIVE_TIMEOUT;
    static const uint RECEIVE_TIMEOUT_CHECK_INTERVAL;

public slots:
    void slotConnect();
    void slotDisconnect();

private slots:
    void slotReadyRead();

    void slotReceived(const QByteArray& data);

    void slotConnected();
    void slotDisconnected();

    void slotCheckReceiveTimeout();

private:
    //! called in constructor
    void initialize();

signals:
    void signalConnected();
    //! Not emitted during destruction of this object.
    void signalDisconnected();

private:

    QSharedPointer<AVTimeReferenceAdapterServerConfigBase> m_config;

    std::unique_ptr<QTcpSocket> m_socket;
    QTimer m_reconnect_timer;

    QScopedPointer<AVDataFramingLength> m_framing;

    QTimer m_check_receive_timeout_timer;
    int m_current_receive_timout_left;

    bool m_received_message;

    //! Used to send replay state notifications to the server.
    /**
     *  This will be 0 if the process did not opt in for sending replay state notifications using
     *  AVApplicationInitParams::enableSendingReplayStateNotifications().
     */
    QSharedPointer<AVReplayStateNotifier> m_replay_state_notifier;
};

#endif // AVTIMEREFERENCEADAPTERSERVER_INCLUDED

// End of file
