//-----------------------------------------------------------------------------
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2019
//
// Module:    AVLIB - AviBit Library
//
//-----------------------------------------------------------------------------

/*! \file
    \brief   AVProcessWatchdog implementation
 */

// class header include
#include "avprocesswatchdog.h"

#include <QThread>

// system/Qt includes

// common includes
#include "avdatetime.h"

// local includes

//-----------------------------------------------------------------------------
constexpr int PING_INTERVAL_MSEC = 1000;
constexpr int DEFAULT_INFO_THRESHOLD_MSEC = 15000;
constexpr int DEFAULT_DEADLOCK_THRESHOLD_MSEC = 30000;

//-----------------------------------------------------------------------------

class AVProcessWatchdogThreadObject : public QObject
{
    Q_OBJECT
public:
    AVProcessWatchdogThreadObject()
    {
        m_ping_timer.setInterval(PING_INTERVAL_MSEC);

        connect(&m_ping_timer, &QTimer::timeout, this, &AVProcessWatchdogThreadObject::signalPing);
        m_ping_timer.start();
        QTimer::singleShot(0,this, &AVProcessWatchdogThreadObject::signalPing);
    }

signals:
    void signalPing();
private:
    QTimer m_ping_timer { this };
};

//-----------------------------------------------------------------------------

class AVProcessWatchdogMonitor : public QObject
{
    Q_OBJECT
public:
    AVProcessWatchdogMonitor(QThread* sentinel_thread, const AVProcessWatchdogConfig& config) :
        m_thread_object { new AVProcessWatchdogThreadObject },
        m_deadlock_timer { this },
        m_thread_name { m_thread_object->thread()->objectName() },
        m_config { config }
    {
        AVASSERT(m_thread_object->thread() != sentinel_thread); // has to be executed in context of the monitored thread

        moveToThread(sentinel_thread);

        QObject::connect(m_thread_object, &AVProcessWatchdogThreadObject::signalPing,
                         this, &AVProcessWatchdogMonitor::onPing);

        m_deadlock_timer.setInterval(m_config.m_deadlock_limit_msec);

        QObject::connect(&m_deadlock_timer, &QTimer::timeout, this, &AVProcessWatchdogMonitor::onDeadlockTimer);
        QObject::connect(QThread::currentThread(), &QThread::finished, &m_deadlock_timer, &QTimer::stop);

        if (m_config.m_deadlock_limit_msec > 0)
        {
            AVASSERT(m_deadlock_timer.thread() == sentinel_thread);
            //start timer in the correct thread
            QTimer::singleShot(0, &m_deadlock_timer, QOverload<>::of(&QTimer::start));
        }

    }

    void onDeadlockTimer()
    {
        auto level = AVProcessWatchdog::singleton().isAbortOnDeadlockEnabled() ? AVLog::LOG__FATAL
                                                                                  : AVLog::LOG__INFO;
        AVLogStream(LOG_HERE, level, AVLogger).getStream()
                << AV_FUNC_PREAMBLE << "No ping back from thread \'" << m_thread_name
                << "\' received since " << m_last_receive_time;
    }

    void onPing()
    {
        QDateTime now = AVDateTime::currentDateTimeUtc();
        auto diff = m_last_receive_time.msecsTo(now);

        if (m_config.m_info_limit_msec > 0
                && diff > m_config.m_info_limit_msec)
        {
            QString message = QString("%1: Long delay in thread '%2' detected: %3 ms").
                    arg(QString(AV_FUNC_INFO)).arg(m_thread_name).arg(diff);
            if (AVProcessWatchdog::singleton().isAbortOnDeadlockEnabled())
            {
                AVLogError << message;
            } else
            {
                AVLogInfo << message;
            }
        }
        else
        {
            AVLogDebug2 << AV_FUNC_PREAMBLE << "Received ping from \'" << m_thread_name << "\' with duration: "
                        << diff << " ms from " << m_last_receive_time << " to " << now;
        }

        if (m_config.m_deadlock_limit_msec > 0)
        {
            m_deadlock_timer.start();
        }

        m_last_receive_time = now;
    }

    ~AVProcessWatchdogMonitor()
    {
        m_thread_object->deleteLater();
    }

    AVProcessWatchdogThreadObject* m_thread_object; //lives in the watched thread

    QTimer m_deadlock_timer;
    AVDateTime m_last_receive_time { AVDateTime::currentDateTimeUtc() };
    QString m_thread_name;

    const AVProcessWatchdogConfig& m_config;
};

AVProcessWatchdog::AVProcessWatchdog(bool abort_on_deadlock) : m_abort_on_deadlock_enabled(abort_on_deadlock)
{
    m_sentinel_thread.setObjectName("AVProcessWatchdog thread");
    if (!qApp || QThread::currentThread() != qApp->thread())
    {
        AVLogFatal << AV_FUNC_PREAMBLE << "Must be initialized in main thread.";
    }
}

//----------------------------------------------------------------------------

AVProcessWatchdog::~AVProcessWatchdog()
{
    {
        std::lock_guard<std::mutex> lock(m_map_mutex);
        QStringList thread_names;
        std::transform(m_monitored_threads.begin(), m_monitored_threads.end(),
                       std::back_inserter(thread_names), [](auto pair) {
            const QThread* monitored_thread = pair.first;
            return monitored_thread->objectName();
        });

        thread_names.removeOne(AVThread::MAIN_THREAD_NAME);
        if (thread_names.size() > 0)
        {
            AVLogError << AV_FUNC_PREAMBLE << "The AVProcessWatchdog has to outlive all watched threads. Next to the main thread, still running: " << thread_names;
        }

        for(auto entry: m_monitored_threads)
        {
            entry.second->deleteLater();
        }
        m_monitored_threads.clear();
    }
    m_sentinel_thread.quit();
    m_sentinel_thread.wait();
}

//----------------------------------------------------------------------------

void AVProcessWatchdog::registerThread()
{
    if (m_config.m_deadlock_limit_msec <= 0
            && m_config.m_info_limit_msec <= 0)
    {
        return;
    }
    if (!m_sentinel_thread.isRunning())
    {
        m_sentinel_thread.start();
    }
    QThread * const current_thread = QThread::currentThread();

    auto* monitor = new AVProcessWatchdogMonitor(&m_sentinel_thread, m_config);
    monitor->moveToThread(&m_sentinel_thread);
    {
        std::lock_guard<std::mutex> lock(m_map_mutex);

        AVASSERT(m_monitored_threads.find(current_thread) == m_monitored_threads.end());
        m_monitored_threads[current_thread] = monitor;
    }

    QObject::connect(current_thread, &QThread::finished, monitor, [current_thread, this]() {
        AVASSERT(QThread::currentThread() == current_thread);

        std::lock_guard<std::mutex> lock(m_map_mutex);
        auto iter = m_monitored_threads.find(current_thread);
        AVASSERT(iter != m_monitored_threads.end());
        iter->second->deleteLater();
        m_monitored_threads.erase(iter);
    }, Qt::DirectConnection);
}

//----------------------------------------------------------------------------

void AVProcessWatchdog::reportAlive()
{
    QThread * const current_thread = QThread::currentThread();
    std::lock_guard<std::mutex> lock(m_map_mutex);
    auto iter = m_monitored_threads.find(current_thread);
    AVASSERT(iter != m_monitored_threads.end());

    AVASSERT(iter->second->thread() != current_thread);
    QTimer::singleShot(0, iter->second, &AVProcessWatchdogMonitor::onPing);
}

//----------------------------------------------------------------------------

void AVProcessWatchdog::setAbortOnDeadlockEnabled(bool enabled)
{
    m_abort_on_deadlock_enabled = enabled;
}

//----------------------------------------------------------------------------

bool AVProcessWatchdog::isAbortOnDeadlockEnabled()
{
    return m_abort_on_deadlock_enabled;
}

//----------------------------------------------------------------------------

AVProcessWatchdogConfig::AVProcessWatchdogConfig() : AVConfig2("processconfig.thread_monitor")
{
    registerParameter("deadlock_limit_msec", &m_deadlock_limit_msec, "The hard time limit when a deadlock is assumed.\n"
                      "Set 0 to disable.").
            setOptional(DEFAULT_DEADLOCK_THRESHOLD_MSEC);

    registerParameter("info_limit_msec", &m_info_limit_msec, "The threshold time limit when a process is assumed to be in "
                                                             "danger of mistakenly being considered in a deadlock state.").
            setOptional(DEFAULT_INFO_THRESHOLD_MSEC);
    refreshParams();
}

//----------------------------------------------------------------------------

#include "avprocesswatchdog.moc"
