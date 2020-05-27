///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:     AVLIB - AviBit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Daniela Kengyel, d.kengyel@avibit.com
    \brief
 */

#if !defined(AVSIGNALHANDLER_H_INCLUDED)
#define AVSIGNALHANDLER_H_INCLUDED

// qt includes
#include <QString>
#include <QTimer>
#include <QObject>
#include <QDateTime>
#include <QSocketNotifier>

// local includes
#include "avlib_export.h"
#include "avexplicitsingleton.h"

/**
 * @brief AVSignalHandler encapsulates the differences in signal handling for windows & linux.
 *
 * - Linux:
 *   Operations in a signal handler are severly limited, e.g. malloc cannot be used as it is not reentrant.
 *   Also see http://stackoverflow.com/questions/3941271/why-are-malloc-and-printf-said-as-non-reentrant and
 *   the "man 7 signal"
 *   The AVSignalHandler class converts OS signals to Qt signals, as described here:
 *   http://doc.qt.io/qt-5/unix-signals.html
 *
 * - Windows:
 *   Under Windows, CTRL+C handling is done in a new thread. See MSDN "signal" documentation:
 *   https://msdn.microsoft.com/en-us/library/xdkz3x12.aspx
 *   The AVSignalHandler class handles the event in the main thread context.
 */
class AVLIB_EXPORT AVSignalHandler : public QObject, public AVExplicitSingleton<AVSignalHandler>
{
    Q_OBJECT

public:

    //! Ensure that the signal handler is deleted later.
    static constexpr int DEINIT_RANK = AVDaemonFunctions::DeinitFunctionRanks::BASE_RANK;

    ~AVSignalHandler() override;

    static AVSignalHandler& initializeSingleton();

private:

    /**
     * @brief installHandler creates our socket pair and installs the signal handlers, but does not
     *                       yet create the socket notifier (qApp is required for that).
     *
     * This method is designed so it can be called immediately after process startup (does not depend on logger or QApplication).
     */
    void installHandler();

    /**
     * @brief initialize must be called after the QApplication and logger objects exist, and actually orimes the signal handling
     *                   (installs socket notifier in linux, and releases semaphore in windows).
     */
    void initialize();

public slots:

    //! This slot is always called in the main thread context.
    void slotQuitInternal();

    //! Called in Linux if the signal handler wrote to our socket pair.
    //! Not inside #if because the MOC cannot handle it...
    void slotLinuxSignal();

private:

    AVSignalHandler();

    /**
     * @brief isHandlerDisabled checks whether registering a signal handler has been disabled via environment variable.
     *
     * TODO CM This is ancient functionality and probably never used...
     */
    bool isHandlerDisabled() const;

    //! The actual signal handler registered with the OS.
    static void signalHandler(int s);

#if defined(Q_OS_LINUX)
    static int m_socket_pair[2];
    std::unique_ptr<QSocketNotifier> m_socket_notifier;
#endif
};


#endif

// End of file
