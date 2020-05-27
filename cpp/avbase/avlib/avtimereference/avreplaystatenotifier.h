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
    \brief   AVReplayStateNotifier header
 */

#ifndef AVREPLAYSTATENOTIFIER_INCLUDED
#define AVREPLAYSTATENOTIFIER_INCLUDED

// Qt includes
#include <QObject>
#include <QSharedPointer>
#include <QTcpSocket>
#include <QTimer>

// avlib includes
#include "avlib_export.h"
#include "avmacros.h"

// local includes

// forward declarations

///////////////////////////////////////////////////////////////////////////////
//! This holds the current replay state for notifying replayers
/*!
*/
class AVLIB_EXPORT AVReplayStateNotifier : public QObject
{
    Q_OBJECT
    AVDISABLECOPY(AVReplayStateNotifier);

    //! friend declaration for function level test case
    friend class TstAVReplayStateNotifier;

public:

    #define REPLAY_STATE_VALUES(val)                                                                    \
    val(Normal,               = 0   /*!< Normal Execution */)                                           \
    val(Initializing,         = 1   /*!< On start-up and immediately after time-jump */)                \
    val(ReadyForInitialData,  = 2   /*!< After initializing, awaiting full-transmission */)             \
    val(ProcessingInitialData,= 3   /*!< When received initial data, when processed -> Normal */)


    DEFINE_ENUM(ReplayState, REPLAY_STATE_VALUES)

public:
    AVReplayStateNotifier(const QString& process_identifier, const QString& server_hostname, uint server_port);
    ~AVReplayStateNotifier() override;

    static QByteArray encode(const QString& process_identifier, ReplayState state);

    static bool decode(const QByteArray& data,QString& process_identifier, ReplayState& state);

public slots:
    void slotSetReplayState(AVReplayStateNotifier::ReplayState state);

private slots:

    void slotTimeJumped();

    void slotConnect();
    void slotConnected();

    void slotSubmit();

private:
    QString m_process_identifier;
    ReplayState m_replay_state;

    QSharedPointer<QTcpSocket> m_socket;
    QTimer m_reconnect_timer;

    QString m_server_hostname;
    uint m_server_port;
};

#endif // AVREPLAYSTATENOTIFIER_INCLUDED

// End of file
