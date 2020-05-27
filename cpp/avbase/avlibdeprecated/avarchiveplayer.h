///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// QT-Version: QT5
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Kevin Krammer, k.krammer@avibit.com
    \author  QT4-PORT: Robert Koenighofer, r.koenighofer@avibit.com
    \brief   Abstract base class for Archive Players
*/

#if !defined(AVARCHIVEPLAYER_H_INCLUDED)
#define AVARCHIVEPLAYER_H_INCLUDED

// Qt includes
#include <qobject.h>
#include <avdatetime.h>
#include "avlibdeprecated_export.h"

// forward declarations
class AVMsgRaw;

///////////////////////////////////////////////////////////////////////////////

/*! Base class for Archive Players.
    An archive player is a class which can replay an AVMsg message archive.
*/

class AVLIBDEPRECATED_EXPORT AVArchivePlayer : public QObject
{
    Q_OBJECT
public:
    //! Creates an archive player
    /*! \param parent the parent QObject if automatic deletion is wanted.
        \param name the object's name
    */
    explicit AVArchivePlayer(QObject* parent = 0, const char* name = 0);

    //! Destructor
    ~AVArchivePlayer() override;

    //! Returns the name of the current archive
    /*! Returns the internal identifier of the archive currently opened.

        \return the current archive's name or
                QString() if no archive is open
    */
    virtual QString archive() const { return QString(); }

    //! Returns the minimum time of the current archive
    /*! The minimum time is the time stamp of the first message in the archive.

        \return the maximum time or an invalid QDateTime if no archive is set.
        \sa QDateTime::isValid()
    */
    virtual QDateTime minArchiveTime() const = 0;

    //! Returns the maximum time of the current archive
    /*! The maximum time is the time stamp of the lasr message in the archive.

        \return the maximum time or an invalid QDateTime if no archive is set.
        \sa QDateTime::isValid()
    */
    virtual QDateTime maxArchiveTime() const = 0;

    //! Returns the minimum replay time
    /*! The minimum replay is the lower boundary of a playback, i.e. the
        playback will start at a message having a timestamp equal or later than
        this value.
        The returned value is always valid, within minArchiveTime and
        maxArchiveTime and earlier or equal maxReplayTime IF an archive is set.
        It will be invalid like minArchiveTime if no archive is set.

        \return the minimum replay time or
                an invalid QDateTime if no archive is open.
        \sa QDateTime::isValid()
    */
    virtual QDateTime minReplayTime() const {
        if (m_minReplayTime.isValid())
            return m_minReplayTime;
        else
            return minArchiveTime();
    }

    //! Returns the maximum replay time
    /*! The maximum replay is the upper boundary of a playback, i.e. the
        playback will stop at a message having a timestamp equal than this
        value, or earlier if the next message in the archive has a timestamp
        after this value.
        The returned value is always valid, within minArchiveTime and
        maxArchiveTime and later or equal minReplayTime IF an archive is set.
        It will be invalid like maxArchiveTime if no archive is set.

        \return the maximum replay time or
                an invalid QDateTime if no archive is open.
            \sa QDateTime::isValid()
    */
    virtual QDateTime maxReplayTime() const {
        if (m_maxReplayTime.isValid())
            return m_maxReplayTime;
        else
            return maxArchiveTime();
    }

    //! Returns the current time
    /*! The current time is the time of the last replayed message or the last
        message resulting from a seek in the archive.
        Implementations in subclasses should return an invalid QDateTime if
        no archive is open.

        \return the current archive time
        \sa QDateTime::isValid()
    */
    virtual QDateTime currentTime() const = 0;

    //! Returns the speed factor of the replay
    /*! The speed factor is a ratio between the normal time progress of
        the archive's messages and the desired progress speed.
        Factor 1.0 means normal speed, 2.0 double speed, 0.5 half speed.

        \return a value decribing the current replay speed
    */
    virtual double speedFactor() const { return m_speedFactor; }

    //! Selects an archive for replay
    /*! Selecting an archive is dependend on the player implementation.
        Thus the player's user can only pass a pointer to a widget which will
        serve as the parent of any dialog the player needs to display for
        archive selection.

        \param parent the parent widget to pass to selection dialogs

        \return false if selection was cancelled or the archive could not be
                opened.
    */
    virtual bool selectArchive(QWidget* parent) = 0;

    //! Selects an archive for replay
    /*! Selects a given archive for replay. The archive string is
        dependend on the player implementation.

        \param archive the archive to select

        \return false if the archive could not be opened.
    */
    virtual bool selectArchive(const QString& archive) = 0;

    //! Sets the minimum replay time.
    /*! The specified time is only set as the new minimum replay time if
        it is valid and between minArchiveTime and maxArchiveTime or equal to
        one of the two limits. Both limits must be valid times themselves.
        If the value is used and the current maxReplayTime is earlier,
        maxReplayTime will be set to the same value.
        Any change will not effect a currently running playback, when using this
        base implementation.

        \param minTime the new time to be set as the minReplayTime
    */
    virtual void setMinReplayTime(const QDateTime& minTime);

    //! Sets the maximum replay time.
    /*! The specified time is only set as the new maximum replay time if
        it is valid and between minArchiveTime and maxArchiveTime or equal to
        one of the two limits. Both limits must be valid times themselves.
        If the value is used and the current minReplayTime is laters,
        minReplayTime will be set to the same value.
        Any change will not effect a currently running playback, when using this
        base implementation.

        \param maxTime the new time to be set as the maxReplayTime
    */
    virtual void setMaxReplayTime(const QDateTime& maxTime);

    //! Set a message source filter
    /*! Normally the player replays all messages from the archive.
        However under certain circumstances, it might be necessary to
        replay only messages which have a certain SRC field set in the
        header. This method can be used to set a filter to replay only
        those messages.
        \param on true to perform src filtering, false for normal operation
                without filter
        \param source source address to filter on
    */
    virtual void setSourceFilter(bool on, uchar source[6]) = 0;

signals:
    //! Emitted when a new archive is selected
    /*! This signal is emitted when the currently used archive is changed.
        It is recommened to read any archive related values (times)
        from the player if this signal activated a slot.

        \param the name of the newly selected archive. May be QString() if
               an archive was only closed an no new one got selected.
    */
    void signalArchiveChanged(const QString& archive);

    //! Emitted when the player starts replaying an archive
    /*! This signal is emitted when the player changes its internal state to
        replay. This usually happens during execution of slotPlay but could
        be delayed if the player asynchronosly waits for a reply of a backend.

        \param the date the archive replay starts. Usually currentTime().
    */
    void signalReplayStarted(const QDateTime&);

    //! Emitted when the player stops replaying an archive
    /*! This signal is emitted when the player changes its internal state to
        stopped. This usually happens during execution of slotStop but could
        be delayed if the player asynchronosly waits for a reply of a backend.

        \param the date the archive replay stopped. Usually the timestamp of
               of the last replayed message and thus equal to currentTime().
    */
    void signalReplayStopped(const QDateTime&);

    //! Emitted when the player's internal time changes
    /*! The player's time usually changes when forward or rewind slots are
      called, a message is being replayed, or a new archive is opened.

        \param the date the archive changed to. Usually equal to current time.
    */
    void signalTimeChanged(const QDateTime&);

    //! Emitted when a new message is coming from the archive
    /*! This signal transports the most recent message being replayed by the
        player.

        \param an archived message
    */
    void signalReplayMessage(AVMsgRaw& msg);

public slots:
    //! Starts the playback with the current parameter set.
    /*! Starts an archive replay between current time and maxReplayTime
        using the currently set speed factor.
        If there is any reason the player cannot start a replay, e.g no archive
        set, already playing, etc., this should return false.
        The playback has officially started once the player emitted
        signalReplayStarted.

        \return false if the playback could not be started.
    */
    virtual bool slotPlay() = 0;

    //! Stops the playback.
    /*! Stops an currently running archive replay.
        If there is any reason the player cannot stop a replay, e.g no archive
        set, not playing playing, etc., this should return false.
        The playback has officially stopped once the player emitted
        signalReplayStopped.

        \return false if no playback was stopped.
    */
    virtual bool slotStop() = 0;

    //! Skips ahead in the archive
    /*! Skips to a later time in the archive. Usually the next message or next
        second.

        \return should return false if skipping was not possible.
    */
    virtual bool slotForward() = 0;

    //! Moves back in the archive
    /*! Moves to an earlier time in the archive. Usually the
        previous message or one second before current time.

        \return should return false if skipping was not possible.
    */
    virtual bool slotRewind() = 0;

    //! Moves to the archive's beginning.
    /*! Moves to the first message in the archive.

        \return should return false if skipping was not possible.
    */
    virtual bool slotArchiveFullForward() {
        return slotChangeTime(maxArchiveTime());
    }

    //! Moves to the archives end.
    /*! Moves to the last message in the archive.

        \return should return false if skipping was not possible.
    */
    virtual bool slotArchiveFullRewind() {
        return slotChangeTime(minArchiveTime());
    }

    //! Moves to the lower replay limit.
    /*! Moves to the first message at or after minReplayTime.

        \return should return false if skipping was not possible.
    */
    virtual bool slotReplayFullForward() {
        return slotChangeTime(maxReplayTime());
    }

    //! Moves to the upper replay limit.
    /*! Moves to the last message at or bore maxReplayTime.

        \return should return false if skipping was not possible.
    */
    virtual bool slotReplayFullRewind(){
        return slotChangeTime(minReplayTime());
    }

    //! Changes the current time.
    /*! Changes the position in the archives timeline to a message which has a
        timestamp equal or later the given value unless the given value is
        beyond the archives uppper limit.

        \param time the time to change to.
        \return should return false if skipping was not possible.
    */
    virtual bool slotChangeTime(const QDateTime& time) = 0;

    //! Changes the current replay speed.
    /*! This sets the speed factor to the replay. Depending on the
        implementation this can also effect a currently running replay.

        \param speedFactor the new speed factor
        \sa speedFactor()
    */
    virtual void setSpeedFactor(double speedFactor) = 0;

protected:
    AVDateTime m_minReplayTime; //!< lower replay time limit
    AVDateTime m_maxReplayTime; //!< upper replay time limit
    double    m_speedFactor;   //!< replay speed ratio

private:
    //! Dummy Copy constructor
    AVArchivePlayer (const AVArchivePlayer&);

    //! Dummy assignment operator
    AVArchivePlayer& operator= (const AVArchivePlayer&);
};

#endif

// End of file
