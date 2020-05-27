

#include "avlogmodel.h"

#include <QtCore/QVariant>
#include <QtCore/QtGlobal>
#include <QtGui/QClipboard>
#include <QtGui/QGuiApplication>
#include <QtGui/QPainter>

#include <fstream>

namespace av
{
namespace auth
{
LogModel::LogModel(QObject* parent) : QAbstractTableModel(parent), mMessages(), mSeverityStates()
{
    mSeverityStates[AVLog::LOG__DEBUG]   = true;
    mSeverityStates[AVLog::LOG__INFO]    = true;
    mSeverityStates[AVLog::LOG__WARNING] = true;
    mSeverityStates[AVLog::LOG__ERROR]   = true;
    mSeverityStates[AVLog::LOG__FATAL]   = true;
}  // LogModel::LogModel

LogModel::~LogModel() {}  // LogModel::~LogModel

bool LogModel::getSeverityState(LogSeverity severity) const
{
    SeverityStateMap::const_iterator ssmit = mSeverityStates.find(severity);
    return ssmit != mSeverityStates.end() ? ssmit->second : false;
}  // LogModel::getSeverityState

void LogModel::setSeverityState(LogSeverity severity, bool state)
{
    if (mSeverityStates[severity] == state)
    {
        return;
    }
    mSeverityStates[severity] = state;

    std::vector<std::size_t> msgIndices;
    for (std::size_t m = 0; m < mMessages.size(); ++m)
    {
        if (mMessages[m].severity == severity)
        {
            msgIndices.push_back(m);
        }
    }

    emit messagesStateChanged(msgIndices, state);
}  // LogModel::setSeverityState

void LogModel::addMessage(QString const& message, LogSeverity severity)
{
    int numMessages = static_cast<int>(mMessages.size());
    beginInsertRows(QModelIndex(), numMessages, numMessages);
    mMessages.push_back(MessageData(
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
        QDateTime::currentDateTime(),
#else
        QDateTime::currentDateTimeUtc(),
#endif
        message, severity));
    numMessages = static_cast<int>(mMessages.size());
    endInsertRows();
    QModelIndex lastRow0 = this->createIndex(numMessages - 1, 0);
    QModelIndex lastRow1 =
        this->createIndex(numMessages - 1, 3 - 1);  // TODO: make the column count a function or something
    Q_EMIT dataChanged(lastRow0, lastRow1);

    if (mSeverityStates[severity] == false)
    {
        std::vector<std::size_t> msgIndices;
        msgIndices.push_back(mMessages.size() - 1);
        emit messagesStateChanged(msgIndices, false);
    }
}  // LogModel::addMessage

void LogModel::clear()
{
    if (mMessages.empty())
    {
        return;
    }

    int numMessages = static_cast<int>(mMessages.size());

    beginRemoveRows(QModelIndex(), 0, numMessages - 1);
    mMessages.clear();
    endRemoveRows();
}  // LogModel::clear

int LogModel::rowCount(QModelIndex const& parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(mMessages.size());
}  // LogModel::rowCount

int LogModel::columnCount(QModelIndex const& parent) const
{
    (void)parent;
    return parent.isValid() ? 0 : 3;
}  // LogModel::columnCount

QString severityString(LogSeverity severity)
{
    switch (severity)
    {
        case AVLog::LOG__DEBUG:
            return QString("<FONT COLOR=\"black\">Debug</FONT>");
        case AVLog::LOG__INFO:
            return QString("<FONT COLOR=\"blue\">Info</FONT>");
        case AVLog::LOG__WARNING:
            return QString("<FONT COLOR=\"orange\">Warn</FONT>");
        case AVLog::LOG__ERROR:
            return QString("<FONT COLOR=\"red\">Error</FONT>");
        case AVLog::LOG__FATAL:
            return QString("<FONT COLOR=\"red\">Fatal</FONT>");
    }
    return QString();
}  // severityString

QVariant LogModel::data(QModelIndex const& index, int role) const
{
    if (role != Qt::DisplayRole)
    {
        return QVariant();
    }

    MessageData const& msgData = mMessages.at(index.row());

    int c = index.column();
    return c == 0 ? QVariant(msgData.timestamp.toString("yyyy-MM-dd hh:mm:ss"))
                  : c == 1 ? QVariant(QString::fromStdString("TODO" /* Log::severityString(msgData.severity)*/))
                           : c == 2 ? QVariant(msgData.message) : QVariant();
}  // LogModel::data

QVariant LogModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (!(orientation == Qt::Horizontal && role == Qt::DisplayRole))
    {
        return QVariant();
    }

    return section == 0 ? QVariant("Timestamp")
                        : section == 1 ? QVariant("Severity") : section == 2 ? QVariant("Message") : QVariant();
}  // LogModel::headerData

}  // namespace auth
}  // namespace av

