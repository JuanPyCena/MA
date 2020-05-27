#pragma once

#include "avdatetime.h"
#include "avlog.h"

#include <QtCore/QAbstractTableModel>
#include <QtCore/QDateTime>

#include <map>
#include <string>
#include <vector>

namespace av
{
namespace auth
{
typedef std::vector<QString> StringVec;
typedef int                  LogSeverity;

/**
 * @brief Implements QAbstractTableModel with list of igsLog messages
 * @ingroup gui
 */
class LogModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    /** Constructor. */
    LogModel(QObject *parent = 0);
    /** Destructor. */
    ~LogModel();

    /** Return whether provided severity is shown. */
    bool getSeverityState(LogSeverity severity) const;
    /** Set whether severity is sown. */
    void setSeverityState(LogSeverity severity, bool state);

Q_SIGNALS:
    /** Signal emitted when visibility of messages has changed. */
    void messagesStateChanged(std::vector<std::size_t> const &, bool);

public:
    /** Structure to carry the information for a single message. */
    struct MessageData
    {
        explicit MessageData(QDateTime tstamp = AVDateTime(), QString const &msg = QString(),
                             LogSeverity sev = AVLog::LOG__INFO)
            : timestamp(tstamp), severity(sev), message(msg)
        {
        }
        QDateTime   timestamp;
        LogSeverity severity;
        QString     message;
    };  // struct MessageData

    typedef std::vector<MessageData> MessageVec;

    /** Get all the messages. */
    MessageVec const &getMessages() const { return mMessages; }

    /** Add a message. */
    void addMessage(QString const &message, LogSeverity severity);

    /** Clear the message buffer .*/
    void clear();

    /** Return the number of rows. */
    virtual int rowCount(QModelIndex const &parent = QModelIndex()) const;

    /** Return the number of columns. */
    virtual int columnCount(QModelIndex const &parent = QModelIndex()) const;

    /** Return the data for the specified model index. */
    virtual QVariant data(QModelIndex const &index, int role = Qt::DisplayRole) const;

    /** Return the data for the table header. */
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

private:
    MessageVec mMessages;

    using SeverityStateMap = std::map<LogSeverity, bool>;
    SeverityStateMap mSeverityStates;

};  // class LogModel

}  // namespace auth
}  // namespace av
