#ifndef MOCKQNETWORKACCESSMANAGER_H
#define MOCKQNETWORKACCESSMANAGER_H

#include "avunittesthelperlib_export.h"

#include <QNetworkAccessManager>
#include <QObject>
#include <functional>

class AVUNITTESTHELPERLIB_EXPORT MockQNetworkAccessManager : public QNetworkAccessManager
{
    Q_OBJECT
public:
    struct MockResult
    {
        int http_status_code = 200;
        QByteArray m_content;
    };

    using callback_t = std::function<MockResult(Operation, const QNetworkRequest &, QIODevice *)>;
    explicit MockQNetworkAccessManager(QVector<MockResult> results);
    explicit MockQNetworkAccessManager(callback_t handler);

    callback_t m_handler;
signals:
    void signalRequestEmitted(QNetworkAccessManager::Operation op, QNetworkRequest request, QByteArray outgoingData);

    // QNetworkAccessManager interface
protected:
    QNetworkReply *createRequest(QNetworkAccessManager::Operation op, const QNetworkRequest &request, QIODevice *outgoingData) override;
    QVector<MockResult> m_results;
};

#endif // MOCKQNETWORKACCESSMANAGER_H
