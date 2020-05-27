///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2011
//
// Module:     AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////


/*! \file
    \author  Dr. Thomas Leitner, t.leitner@avibit.com
    \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   Implementation of a thread class based on QThread which allows
             for asynchronous notifications to the main thread as well as
             graceful shutdown of the running thread.
*/


// QT includes
#include <QtGlobal>
#include <QApplication>
#include <QEvent>

// system includes
#if (defined (_OS_WIN32_) || defined(Q_OS_WIN32))
#include <windows.h>
#include <float.h>
#endif

// linux specific system includes
#if defined(Q_OS_LINUX) && !defined(__INTEL_COMPILER)
#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif
#if !defined(__USE_GNU)
#define __USE_GNU
#endif
#include <fenv.h>
#endif

#if defined(Q_OS_UNIX)
#if defined(Q_OS_OSF)
#undef _POSIX_SOURCE
#undef _AES_SOURCE
#endif
#include <signal.h>
#include <sys/types.h>
#include <pthread.h>
#endif

// local includes
#include "avthread.h"

#include "avdaemon.h"
#include "avenvironment.h"

const QString AVThread::DEFAULT_THREAD_NAME    = "unnamed";
const QString AVThread::MAIN_THREAD_NAME       = "main";

///////////////////////////////////////////////////////////////////////////////
//! Thread event used in AVThreadNotifier to send a notification from the
//! sub-thread to the main thread's event loop. The AVThreadNotifier class
//! uses that to send itself an event which is executed in the context of
//! the main thread and is used to emit a notifier signal.

class AVThreadNotifyEvent : public QEvent
{
public:
    //! the type of the notify event.
    static const QEvent::Type THREAD_NOTIFY_EVENT_TYPE;

    //! construct a notify event and convey the specified integer to the
    //! main thread.
    /*! \param i the integer to be conveyed.
    */
    explicit AVThreadNotifyEvent(int i) :
        QEvent(THREAD_NOTIFY_EVENT_TYPE),dataSet(false), val(i) {};

    //! construct a notify event and convey the specified bytearray to the
    //! main thread.
    /*! \param dat the bytearray to be conveyed.
    */
    explicit AVThreadNotifyEvent(const QByteArray &dat) :
        QEvent(THREAD_NOTIFY_EVENT_TYPE), dataSet(true), val(0)
    {
        data = dat; //QT4PORT QT4 has implicit sharing
    };

    //! return the currently set integer parameter conveyed in this event
    int      intParam()   { return val;    };

    //! return the currently set bytearray conveyed in this event
    QByteArray &dataParam()  { return data;    };

    //! return true if this event conveys a bytearray, false if it conveys
    //! an integer parameter or a message
    bool     hasData()     { return dataSet; };

private:
    bool     dataSet;              //!< true if the event contains data
    int      val;                  //!< the notification value
    QByteArray data;               //!< notification data
};

///////////////////////////////////////////////////////////////////////////////
const QEvent::Type AVThreadNotifyEvent::THREAD_NOTIFY_EVENT_TYPE =
        static_cast<QEvent::Type>(QEvent::registerEventType());

///////////////////////////////////////////////////////////////////////////////

AVThreadNotifier::AVThreadNotifier()
{
}

///////////////////////////////////////////////////////////////////////////////

AVThreadNotifier::~AVThreadNotifier()
{
}

///////////////////////////////////////////////////////////////////////////////

void AVThreadNotifier::sendNotification(int v)
{
    AVThreadNotifyEvent* event =  new (LOG_HERE) AVThreadNotifyEvent(v);
    QApplication::postEvent(this, event);
}

///////////////////////////////////////////////////////////////////////////////

void AVThreadNotifier::sendNotification(const QByteArray &data)
{
    AVThreadNotifyEvent* event =  new (LOG_HERE) AVThreadNotifyEvent(data);
    QApplication::postEvent(this, event);
}

///////////////////////////////////////////////////////////////////////////////

bool AVThreadNotifier::event(QEvent *e)
{
    AVASSERT(e != 0);
    if ( e->type() == AVThreadNotifyEvent::THREAD_NOTIFY_EVENT_TYPE)
    {
        AVThreadNotifyEvent *ne = static_cast<AVThreadNotifyEvent*>(e);
        if (ne->hasData()) emit signalNotification(ne->dataParam());
        else emit signalNotification(ne->intParam());
        return true;
    }

    return QObject::event(e);
}

///////////////////////////////////////////////////////////////////////////////

AVThread::AVThread(const QString& name, unsigned int stacksize) :
    m_maxStopWait(4000),
    m_stopFlag(false),
    m_verbose(false),
    m_threadNotifier(new (LOG_HERE) AVThreadNotifier),
    m_gotExternalNotifier(false),
    m_thread(stacksize, this, name)
{
    AVASSERT(m_threadNotifier != 0);

    if (stacksize > 0)
        LOGGER_ROOT.Write(LOG_INFO, "AVThread: Using stack size of %d bytes", stacksize);
}

///////////////////////////////////////////////////////////////////////////////

AVThread::AVThread(AVThreadNotifier* thread_notifier, const QString& name, unsigned int stacksize) :
    m_maxStopWait(4000),
    m_stopFlag(false),
    m_verbose(false),
    m_threadNotifier(thread_notifier),
    m_gotExternalNotifier(true),
    m_thread(stacksize, this, name)
{
    AVASSERT(m_threadNotifier != 0);

    if (stacksize > 0)
        LOGGER_ROOT.Write(LOG_INFO, "AVThread: Using stack size of %d bytes", stacksize);
}

///////////////////////////////////////////////////////////////////////////////

AVThread::~AVThread()
{
    if (running()) AVLogger->Write(LOG_ERROR, "AVThread:~AVThread: thread '%s' is still running",
                                   qPrintable(m_thread.objectName()));
//    stop();
    if (!m_gotExternalNotifier && m_threadNotifier != 0) delete m_threadNotifier;
    if (m_verbose) AVLogger->Write(LOG_INFO, "AVThread: thread object destroyed");
}

///////////////////////////////////////////////////////////////////////////////

void AVThread::start(QThread::Priority p)
{
    QMutexLocker locker(&m_startStopMutex);
    bool threadStarted = false;
    while (!threadStarted) {
        // Make sure the thread is not running when calling QThread::start and clearing the stopFlag
        // -> wait for thread termination first.
        if (m_thread.isRunning())
        {
            // don't hold the mutex while waiting to avoid deadlock.
            m_startStopMutex.unlock();
            m_thread.wait();
            m_startStopMutex.lock();
            // Thread could have been started again before lock was acquired -> loop
        } else {
            threadStarted = true;
            m_stopFlag    = false;
            AVASSERT(!m_thread.isRunning());
            m_thread.start(p);
            if(!m_thread.isRunning())
                AVLogFatal << AV_FUNC_PREAMBLE << "Could not start thread: " << m_thread.objectName();

        }
    }
}

///////////////////////////////////////////////////////////////////////////////

bool AVThread::stop(int timeout, bool waitForStop)
{
    QMutexLocker locker(&m_startStopMutex);
    bool rc = true;
    if (m_thread.isRunning()) {

        if (timeout < 0) timeout = m_maxStopWait;
        if (m_verbose) qDebug("AVThread: stopping .......");
        m_stopFlag = true;               // tell the thread we want to stop
        m_swait.wakeOne();               // wake up sleepMs if currently sleeping

        // now call stopThread method
        stopThread();

        // only wait here if it's still running
        if (waitForStop && m_thread.isRunning())
        {
            AVLogger->Write(LOG_DEBUG, "AVThread:stop: waiting for %d ms", timeout);

            bool died = m_thread.wait(timeout);  // wait until it's really dead.
            if (!died) qDebug("AVThread: stop wait timedout");
        }
        rc = !m_thread.isRunning();
    }
    return rc;
}

///////////////////////////////////////////////////////////////////////////////

bool AVThread::sleepMs(int ms)
{
    QMutex mutex;//QT4PORT HACK QWaitCondition::wait needs mutex now FIX
    mutex.lock();
    m_swait.wait(&mutex,ms);//before: m_swait.wait(ms);
    mutex.unlock();
    return checkForStop();
}

///////////////////////////////////////////////////////////////////////////////

void AVThread::connectNotification(QObject *r, const char *slot)
{
    if (m_gotExternalNotifier)
    {
        AVLogger->Write(LOG_FATAL, "AVThread:connectNotification: "
                        "Got a shared external notifier - will not connect.");
        AVASSERT(false);
    }

    QString slot_string = slot;

    if (r == 0)
        m_threadNotifier->disconnect();
    else if (slot_string.contains("int"))
        AVDIRECTCONNECT(m_threadNotifier, SIGNAL(signalNotification(int)), r, slot);
    else
        AVLogger->Write(LOG_FATAL, "The type involved in %s is not supported!!", slot);
}

///////////////////////////////////////////////////////////////////////////////

void AVThread::quitMain()
{
    if (qApp != 0) {
        qApp->exit(); //TODO QT4PORT ckeck if okay, was: qApp->exit_loop();
        QEvent* event = new (LOG_HERE) QEvent(QEvent::Quit);
        QApplication::postEvent(qApp, event);
    }
}

///////////////////////////////////////////////////////////////////////////////

bool AVThread::checkForStop() const
{
    if (m_stopFlag || AVProcessTerminated) return true;
    return false;
}

///////////////////////////////////////////////////////////////////////////////
// perform default FP initializations. Enable OVERFLOW, ZERODIVIDE and
// DENORMAL exceptions

void AVThread::fpinit()
{
    QString afp = AVEnvironment::getEnv("ALLOW_FP_EXCEPTIONS", false);
    if (afp.isNull()) return; // otherwise this overrides the global setting

#if (defined (_OS_WIN32_) || defined(Q_OS_WIN32))

    // Get the default control word
    int cw = _controlfp( 0, 0 );

    // Set the exception masks off, turn exceptions on

    //cw &=~(_EM_OVERFLOW | _EM_UNDERFLOW | _EM_INEXACT |
    //       _EM_ZERODIVIDE | _EM_DENORMAL);
    cw &=~(_EM_OVERFLOW | _EM_ZERODIVIDE | _EM_DENORMAL);

    // Set the new control word
    _controlfp( cw, MCW_EM );
#endif

#if defined(Q_OS_LINUX) && !defined(__INTEL_COMPILER)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
    fesetenv(FE_NOMASK_ENV);
#pragma GCC diagnostic pop
    fedisableexcept(FE_INEXACT);
    fedisableexcept(FE_UNDERFLOW);
#endif
}

///////////////////////////////////////////////////////////////////////////////
// perform per thread initializations

void AVThread::init()
{
    // init floating point stuff
    fpinit();

    // in the threads, disable the INT and TERM signals because
    // they are handled in the main thread already.
#if defined(Q_OS_UNIX)
    sigset_t set, oset;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGTERM);
    sigaddset(&set, SIGUSR1);
    sigaddset(&set, SIGUSR2);
    sigaddset(&set, SIGPIPE);
    pthread_sigmask(SIG_BLOCK, &set, &oset);
#endif
}

///////////////////////////////////////////////////////////////////////////////

QString AVThread::threadName()
{
    return QThread::currentThread()->objectName();
}

///////////////////////////////////////////////////////////////////////////////

void AVThread::setName(const QString& name)
{
    AVASSERT(!running());
    m_thread.setObjectName(name);
}

///////////////////////////////////////////////////////////////////////////////

bool AVThread::isMainThread()
{
    if (!qApp)
    {
        AVLogger->Write(LOG_ERROR, "AVThread::isMainThread: QApplication has not yet been created!");
    }
    return qApp && (qApp->thread() == QThread::currentThread());
}

///////////////////////////////////////////////////////////////////////////////

void AVThread::setMainThread()
{
    AVASSERT(qApp != 0); // QApplication is required
    qApp->thread()->setObjectName(MAIN_THREAD_NAME);
}

///////////////////////////////////////////////////////////////////////////////

void AVThread::run()
{
    init();
    runThread();
}

// End of file
