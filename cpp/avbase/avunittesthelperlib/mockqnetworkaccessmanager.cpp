#include "mockqnetworkaccessmanager.h"

#include <QBuffer>
#include <QNetworkReply>
#include <avmisc.h>

MockQNetworkAccessManager::MockQNetworkAccessManager(QVector<MockResult> results) : m_results(std::move(results))
{
    m_handler = callback_t { [&] (Operation op, const QNetworkRequest & request, QIODevice *) -> MockResult
    {
        AVLogDebug << AV_FUNC_PREAMBLE << "Operation: " << op << " Request URL: " << request.url().toString();
        AVASSERT(!m_results.isEmpty());
        return m_results.takeFirst();
    }};
}

MockQNetworkAccessManager::MockQNetworkAccessManager(MockQNetworkAccessManager::callback_t handler) : m_handler(std::move(handler))
{

}

///////////////////////////////////////////////////////////////////////////////

class MockNetworkReply : public QNetworkReply
{
public:
    MockNetworkReply(const MockQNetworkAccessManager::MockResult & result)
    {
        m_buffer.setData(result.m_content);
        m_buffer.open(QIODevice::ReadOnly);
    }

    qint64 readData(char *data, qint64 maxlen) override
    {
        return m_buffer.read(data, maxlen);
    }

public slots:
    void abort() override
    {
    }

private:
    QBuffer m_buffer;
};

QNetworkReply *MockQNetworkAccessManager::createRequest(QNetworkAccessManager::Operation op, const QNetworkRequest &request, QIODevice *outgoingData)
{
    QByteArray data;
    if (outgoingData)
    {
            data = outgoingData->readAll();
    }

    emit signalRequestEmitted(op, request, data);

    MockResult result = m_handler(op, request, outgoingData);

    auto reply = new MockNetworkReply(result);
    reply->setParent(this);
    reply->open(QIODevice::ReadOnly);

    QTimer::singleShot(0, reply, &QNetworkReply::finished);


    return reply;
}
