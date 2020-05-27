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
    \brief   Definition of a thread class based on QThread which allows
             for asynchronous notifications to the main thread as well as
             graceful shutdown of the running thread.
*/

#if !defined(AVTHREAD_H_INCLUDED)
#define AVTHREAD_H_INCLUDED

// Qt includes
#include <QObject>
#include <QThread>
#include <QThreadStorage>
#include <QEvent>
#include <QWaitCondition>
#include <QMutex>

// AVCOMMON includes
#include "avlib_export.h"
#include "avmacros.h"

// forward declarations
class AVThreadNotifyEvent;

///////////////////////////////////////////////////////////////////////////////
//! Event notifier used to send signals from a sub-thread to a main thread.
/*! The problem with threads is, that the signal/slot mechanism is hard to
    use because a slot is executed under the control of the sub-thread which
    can cause many problems when developers are not aware of that. This class
    uses QApplication::postEvent to send a AVThreadNotifyEvent from the
    sub-thread to the main thread. Upon reception of the event, the main
    thread emits a signal which can be safely connected to by the main program.

    Both, the event sender code and the event receiver code are defined in
    this class, however they are executed by different threads. The event
    sender code is executed by the sub-thread while the event receiver code
    is executed by the main thread.

    This class is basically used by AVThread to implement thread safe
    notifications.
*/
class AVLIB_EXPORT AVThreadNotifier : public QObject
{
    Q_OBJECT
public:
    //! construct a new thread notifier class.
    AVThreadNotifier();

    //! virtual destructor
    ~AVThreadNotifier() override;

    //! this method can be called by the sub-thread to send a notification
    //! event containing the integer 'v' to the main thread.
    void sendNotification(int v);

    //! this method can be called by the sub-thread to send a notification
    //! event containing the bytearray @p data to the main thread.
    //!
    //! *Note* Qt3->Qt5 porting: there is a special implementation for AVMsgRaw signalling in the AVSocketListener implementation.
    //!
    void sendNotification(const QByteArray &data);

signals:
    //! This signal is emitted when a notification event with integer 'v'
    //! was received from the sub-thread. The signal is emitted in the context
    //! of the main thread and the connected slot will also be executed under
    //! the control of the main thread.
    void signalNotification(int v);

    //! This signal is emitted when a notification event with bytearray @p data
    //! was received from the sub-thread. The signal is emitted in the context
    //! of the main thread and the connected slot will also be executed under
    //! the control of the main thread.
    void signalNotification(const QByteArray &data);

private:
    //! internal event filter routine which receives the event and dispatches the signals.
    bool event(QEvent *e) override;
};

///////////////////////////////////////////////////////////////////////////////
//! This is our local QThread implementation called AVThread.
/*! All programs using the avlib need to use AVThread rather than QThread.
    This class has the following features over QThread:

    - provides an asynchronous notification delivery to the main thread
      basically the main thread just calls connectNotification() to connect
      a slot to our signals. The class makes sure that the slots are called
      in the context of the main thread.
    - provides for graceful shutdown.
    - ensures that necessary per-thread initializations are done properly

    A typical usage of this class, including sending notifications to the
    main thread is like this:

    \code
    class MyThread : public AVThread
    {
        void runThread() {         // send out notifications in 2s intervals.
            qDebug("thread 1 running ..........");
            int i = 0;
            while (true) {
                if (sleepMs(2000)) return;       // return if requested to stop
                sendNotification(i);             // notify main thread.
                if (i == 5) quitMain();          // quit after 5 runs.
                i++;
            }
        }
    };
    \endcode

    and in the main code, use this:

    \code

    MyThread *t1 = new MyThread;
    AVASSERT(t1 != 0);
    t1->connectNotification(&receiver, SLOT(slotNotified(int)));
    t1->start();

    bool rc = app.exec();

    t1->stop();
    t1->wait();
    delete t1; // thread must be stopped when being destroyed.
    return rc;

    \endcode
*/
class AVLIB_EXPORT AVThread
{
    AVDISABLECOPY(AVThread);
public:

    static const QString DEFAULT_THREAD_NAME;
    static const QString MAIN_THREAD_NAME;

    //! Construct a new AVThread class.
    /*! \param stacksize If stacksize is greater than zero, the maximum stack size is set to
                         stacksize bytes, otherwise the maximum stack size is automatically
                         determined by the operating system.

         \warning Most operating systems place minimum and maximum limits on thread stack
                  sizes. The thread will fail to start if the stack size is outside these limits.
                  On UNIX you can get the absolute minimum limit on stack size by calling the
                  macro PTHREAD_STACK_MIN, which returns the amount of stack space required for a
                  thread that executes a NULL procedure. Useful threads need more than this, so be
                  very careful when reducing the stack size.

                  \code

                  #include <pthread.h>
                  size_t size = PTHREAD_STACK_MIN + 0x4000;

                  \endcode
     */
    explicit AVThread(const QString& name = DEFAULT_THREAD_NAME, unsigned int stacksize = 0);

    //! Construct a new AVThread class with an external thread notifier.
    //! The thread notifier is used by the connectNotification() and
    //! sendNotification() methods. The external thread notifier will e.g. be used
    //! by the AVSocketListener to avoid killing of a client handle thread
    //! (together with its internal thread notifier) with a potential pending
    //! notification. With the external thread notifier, the notification will
    //! be delivered to the main event loop even if the client handler thread
    //! was killed (e.g. the client disconnected).
    //! WARNING: The external thread notified must not be deleted while any
    //! thread provided with this notifier is still running!!
    /*! \param stacksize If stacksize is greater than zero, the maximum stack size is set to
                         stacksize bytes, otherwise the maximum stack size is automatically
                         determined by the operating system.

         \warning Most operating systems place minimum and maximum limits on thread stack
                  sizes. The thread will fail to start if the stack size is outside these limits.
                  On UNIX you can get the absolute minimum limit on stack size by calling the
                  macro PTHREAD_STACK_MIN, which returns the amount of stack space required for a
                  thread that executes a NULL procedure. Useful threads need more than this, so be
                  very careful when reducing the stack size.

                  \code

                  #include <pthread.h>
                  size_t size = PTHREAD_STACK_MIN + 0x4000;

                  \endcode
     */
    explicit AVThread(AVThreadNotifier* thread_notifier,
                      const QString& name = DEFAULT_THREAD_NAME,
                      unsigned int stacksize = 0);

    //! destroy the AVThread class. If the thread is currently running,
    //! try to stop the thread and wait until it has finished. setStopWait()
    //! can be used to set the maximum wait time in ms.
    virtual ~AVThread();

    //! Mimicks QThread::start(), but clears the stopFlag.
    void start( QThread::Priority = QThread::InheritPriority );
    //! Passes call to m_thread
    bool wait(unsigned long time = ULONG_MAX) { return m_thread.wait(time);  }
    //! Passes call to m_thread
    void terminate()                          {        m_thread.terminate(); }
    //! Passes call to m_thread
    bool finished() const                     { return m_thread.isFinished();  }
    //! Passes call to m_thread
    bool running () const                     { return m_thread.isRunning();   }


    //! connect a notification slot to this thread.
    /*! there are two different types of messages emitted:

        - signalNotification(int) and

        The slots must have the same parameters if they want to received
        any of the signals.
    */
    void connectNotification(QObject *r, const char *slot);

    //! stops the thread (if running) and waits for a max. time until stopped.
    /*! \param timeout the max. time in milliseconds to wait until the thread
               has stopped or -1 to use the default stopWait() time.
        \param waitForStop if true the method waits until the thread has
               really stopped. If false, it triggers the stop so that
               checkForStop() returns true but returns immediately.
        \return true if the thread was stopped successfully, false if not
    */
    bool stop(int timeout = -1, bool waitForStop = true);

    //! set verbose mode.
    void setVerbose(bool v)  { m_verbose = v;      };

    //! return state of verbose mode
    bool verbose() const     { return m_verbose;   };

    //! set the maximum time in ms which the destructor of this class waits
    //! until the thread has finished.
    void setStopWait(unsigned long sw) { m_maxStopWait = sw;   };

    //! returns the maximum time in ms which the destructor of this class waits
    //! until the thread has finished.
    unsigned long  stopWait() const    { return m_maxStopWait; };

    //! sleeps the specified number of ms, but returns immediately if we need
    //! to stop.
    /*! \param ms the number of ms to sleep
        \return true if we need to stop the thread, false if not.
    */
    bool sleepMs(int ms);

    //! allows the thread to quit the main program's exec loop
    //!
    //! *DEPRECATED*, don't use in new code: this method is of dubious value, why does it even exist?
    //! It does not set the AVProcessTerminated flag. Instead, refactor AVSignalHandler to provide a threadsafe way
    //! to terminate the application.
    static void quitMain();

    //! global floating point initialization routine. This routine performs
    //! necessary floating point initializations.
    /*! It is particularly necessary under Windows in order to setup handling
        of floating point exceptions which is required to be done on a
        per-thread basis.
    */
    static void fpinit();

    //! global per-thread initialization routine. This routine performs
    //! necessary per-thread initializations.
    static void init();

    //! Answer the thread name of the currently running thread.
    //! Can return QString::null if thread storage is not properly set up (e.g. no QApplication
    //! object)
    static QString threadName();
    //! Set the thread name for this thread object.
    //! The thread must not be running!
    void setName(const QString& name);
    //! Answer whether the currently running thread is the main thread.
    //! Note that this method works correctly only if setMainThread has been called in the correct
    //! thread context!
    static bool isMainThread();
    //! Set the thread name for the main thread.
    //! It is asserted that the QApplication instance has been created.
    static void setMainThread();

    //! can be used by the thread code to send a notification with integer i
    //! to the main thread.
    void sendNotification(int i)                 { m_threadNotifier->sendNotification(i);  };
    //! can be used by the thread code to send a notification with byte array ba
    //! to the main thread.
    //!
    //! *Note* Qt3->Qt5 porting: there is a special implementation for AVMsgRaw signalling in the AVSocketListener implementation.
    //!
    void sendNotification(const QByteArray& ba ) { m_threadNotifier->sendNotification(ba); };

    //! Is forwarded to corresponding static method from QThread.
    void exit() { QThread::currentThread()->quit(); }

    //! Added during KDAB porting. DEPRECATED.
    //!
    //! Return a pointer to the underlying QThread, only to be used for moveToThread()
    QThread *qthread() { return &m_thread; }

protected:
    //! the thread code (the one in runThread()) should regularly check if
    //! the main thread has requested the thread to stop. This method can
    //! be used for that purpose.
    /*! \returns true if we need to stop, false if not.
    */
    bool checkForStop() const;

    //! the derived class needs to implement this method. It should contain
    //! the actual thread code which will be executed in parallel.
    virtual void runThread() = 0;

    //! The derived class optionally can implement this method. It should contain
    //! cleanup code (e.g. wake up threads waiting on a waitcondition),
    //! and is called within the stop method.
    virtual void stopThread() { }

    //! runs the eventloop of the underlying QThread
    int exec() { return m_thread.exec(); }

    //! Is forwarded to corresponding static method from QThread.
    static void sleep  (unsigned long secs ) { AVInternalThread::sleep(secs);   }
    //! Is forwarded to corresponding static method from QThread.
    static void msleep (unsigned long msecs) { AVInternalThread::msleep(msecs); }
    //! Is forwarded to corresponding static method from QThread.
    static void usleep (unsigned long usecs) { AVInternalThread::usleep(usecs); }

private:
    void run(); //!< Internal run method, called by member m_thread.

    unsigned long     m_maxStopWait;           //!< max. stop waiting time in ms
    bool              m_stopFlag;              //!< flag to tell thread to stop
    bool              m_verbose;               //!< verbose mode
    AVThreadNotifier* m_threadNotifier;        //!< thread notifier
    bool              m_gotExternalNotifier;   //!<
    QWaitCondition    m_swait;                 //!< stop wait condition
    QMutex            m_startStopMutex;        //!< Avoid start/stop race conditions

    //! Bounces back the QThread::run method. Private inheritance from QThread is an alternative,
    //! but makes the base class Qt (which is accessed in some AVThread subclasses) inaccesible,
    //! leading to neccessary (ugly) code changes.
    class AVInternalThread : public QThread
    {
    public:
        AVInternalThread(unsigned int stacksize,
                         AVThread *avThread, const QString& name) :
            m_avThread(avThread) {
            setStackSize(stacksize);
            setObjectName(name);
        }

        static void sleep  (unsigned long secs ) { QThread::sleep(secs);   }
        static void msleep (unsigned long msecs) { QThread::msleep(msecs); }
        static void usleep (unsigned long usecs) { QThread::usleep(usecs); }
        using QThread::exec;
    private:
        void      run() override { m_avThread->run(); }
        AVThread *m_avThread;
    } m_thread;
};

#endif

// End of file
