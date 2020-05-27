///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Alexander Randeu, a.randeu@avibit.com
    \author  QT4-PORT: Anton Skrabal, a.skrabal@avibit.com
    \brief   a timer class, which can handle time speed changes from
             AVTimeReference
*/

#if !defined(AVTIMER_H_INCLUDED)
#define AVTIMER_H_INCLUDED

// Qt include
#include <qdatetime.h>
#include <qtimer.h>

#include "avlib_export.h"
#include "avdatetime.h"

/*! A timer class, which handles time speed changes from AVTimeReference.
 *  If the timer is running, and the speed is changed with
 *  AVTimeReference, the internal timer will be restarted for the
 *  remaining time for the new speed.
 *
 *  When the time jumps back, the timer is restarted.
 *  When the time jumps forward after the interval, it is triggered immediately
 *
 */

class AVLIB_EXPORT AVTimer : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(AVTimer);

    friend class TstAVTimer;

    Q_PROPERTY(bool singleShot READ isSingleShot WRITE setSingleShot)
    Q_PROPERTY(bool running READ isActive)
    Q_PROPERTY(qint32 interval READ getInterval WRITE setInterval)
    Q_PROPERTY(int remainingTime READ remainingTime)
    Q_PROPERTY(int realRemainingTime READ realRemainingTime)
public:

    //! creates a new timer object
    explicit AVTimer(QObject* parent = 0);

    //! destroys the timer object
    ~AVTimer() override;

    //! returns true, if the timer is active
    bool isActive() const;

    //! Sets the timer's interval. Restarts the timer with the new interval if it is running.
    void setInterval(qint32 msec);

    //! Returns the timer's interval
    qint32 getInterval() const;

    //! If true the timer is a single shot timer and stops after one timeout,
    //! otherwise it will continue until stop is called.
    void setSingleShot(bool single_shot);

    //! Returns the timer's singleshot property.
    bool isSingleShot() const;

    //! starts the timer
    /*! \param interval after msec <msec> the signal timeout() will be emited
        \param sshot If sshot is true, the timer will be activated only
                     once; otherwise it will continue until it is stopped.

     */
    Q_INVOKABLE void start(int interval, bool single_shot = false);

    //! Starts the timer with the current interval and single-shot settings
    Q_INVOKABLE void start();

    //! stops the timer
    Q_INVOKABLE void stop();

    //! Returns the remaining time in msecs on the timer. It takes the AVTimeReference speed into account.
    //! If the timer is inactive -1 returned.
    //! \sa QTimer::remainingTime()
    int remainingTime() const;

    //! Returns the real remaining time in msecs on the timer. This doesn't take AVTimeReference speed into account.
    //! If the timer is inactive -1 returned.
    //! \sa QTimer::remainingTime()
    int realRemainingTime() const;

signals:
    //! This signal is emitted when the timer is activated (msecs <msec>
    //! after start() including the speed factor)
    void timeout();

protected slots:
    //! slot for the internal timer's timeout() signal
    void slotInternalTimeout();

    //! slot for an speed change from AVTimeReference; this slot
    //! will be automatically connected to the
    //! AVTimeReference::signalSpeedChange() signal.
    void slotSpeedChanged(double from, double to);

    //! slot for a time jump from AVTimeReference; this slot
    //! will be automatically connected to the
    //! AVTimeReference::signalTimeJumped() signal.
    void slotTimeJumped(const QDateTime& from, const QDateTime& to);

private:
    static qint32 calculateScaledInterval(qint32 interval, double speed);

    int startTimer(int interval, Qt::TimerType timerType = Qt::CoarseTimer) = delete;

    QTimer  m_timer;        //!< internal timer

    qint32  m_interval;

    //! The current speed of AVTimeReference. We need to remember this value so timeouts can be
    //! recalculated if the speed changes (\see slotSpeedChange).
    double  m_speed;

    bool m_need_restart;
    AVDateTime m_end_time;
};


#endif

// End of file
