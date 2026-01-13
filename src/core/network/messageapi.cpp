#include "messageapi.h"
#include <QJsonArray>

namespace Bytedesk {

MessageApi::MessageApi(HttpClient* httpClient, QObject* parent)
    : ApiBase(httpClient, parent)
{
}

MessageApi::~MessageApi()
{
}

void MessageApi::queryMessages(const PageRequest& request, MessagesCallback callback)
{
    qDebug() << "Query messages, page:" << request.page << "size:" << request.size;

    httpClient()->get(m_apiPath, request.toQuery(),
        [this, callback](const QJsonObject& response) {
            if (isResponseSuccess(response)) {
                QJsonObject data = getResponseData(response);
                PageResult result = PageResult::fromJson(data);

                qDebug() << "Query messages successful, count:" << result.messages.size();

                if (callback) {
                    callback(result);
                }
            } else {
                QString message = getResponseMessage(response);
                qWarning() << "Query messages failed:" << message;

                if (callback) {
                    callback(PageResult());
                }
            }
        },
        [this, callback](const QString& error) {
            qWarning() << "Query messages network error:" << error;
            handleNetworkError(error);

            if (callback) {
                callback(PageResult());
            }
        }
    );
}

void MessageApi::queryMessagesByTopic(const QString& topic, const PageRequest& request, MessagesCallback callback)
{
    qDebug() << "Query messages by topic:" << topic;

    QUrlQuery query = request.toQuery();
    query.addQueryItem("topic", topic);

    httpClient()->get(m_apiPath + "/thread/topic", query,
        [this, callback](const QJsonObject& response) {
            if (isResponseSuccess(response)) {
                QJsonObject data = getResponseData(response);
                PageResult result = PageResult::fromJson(data);

                qDebug() << "Query messages by topic successful, count:" << result.messages.size();

                if (callback) {
                    callback(result);
                }
            } else {
                QString message = getResponseMessage(response);
                qWarning() << "Query messages by topic failed:" << message;

                if (callback) {
                    callback(PageResult());
                }
            }
        },
        [this, callback](const QString& error) {
            qWarning() << "Query messages by topic network error:" << error;
            handleNetworkError(error);

            if (callback) {
                callback(PageResult());
            }
        }
    );
}

void MessageApi::sendMessage(const SendMessageRequest& request, SendMessageCallback callback)
{
    qDebug() << "Send message to thread:" << request.threadUid;

    httpClient()->post(m_apiPath + "/rest/send", request.toJson(),
        [this, callback](const QJsonObject& response) {
            if (isResponseSuccess(response)) {
                QJsonObject data = getResponseData(response);
                MessagePtr message = QSharedPointer<Message>::create(
                    Message::fromJson(data)
                );

                qDebug() << "Message sent successfully:" << message->getUid();

                emit messageSent(message);

                if (callback) {
                    callback(message);
                }
            } else {
                QString message = getResponseMessage(response);
                qWarning() << "Send message failed:" << message;

                if (callback) {
                    callback(MessagePtr());
                }
            }
        },
        [this, callback](const QString& error) {
            qWarning() << "Send message network error:" << error;
            handleNetworkError(error);

            if (callback) {
                callback(MessagePtr());
            }
        }
    );
}

void MessageApi::getMessage(const QString& uid, MessageCallback callback,
                           std::function<void(const QString&)> onError)
{
    qDebug() << "Get message:" << uid;

    httpClient()->get(m_apiPath + "/" + uid, QUrlQuery(),
        [this, callback](const QJsonObject& response) {
            if (isResponseSuccess(response)) {
                QJsonObject data = getResponseData(response);
                MessagePtr message = QSharedPointer<Message>::create(
                    Message::fromJson(data)
                );

                qDebug() << "Got message:" << message->getUid();

                if (callback) {
                    callback(message);
                }
            } else {
                QString message = getResponseMessage(response);
                qWarning() << "Failed to get message:" << message;
            }
        },
        [this, onError](const QString& error) {
            qWarning() << "Get message network error:" << error;
            handleNetworkError(error);
            if (onError) {
                onError(error);
            }
        }
    );
}

void MessageApi::recallMessage(const QString& uid,
                              std::function<void(bool)> callback,
                              std::function<void(const QString&)> onError)
{
    qDebug() << "Recall message:" << uid;

    QJsonObject request;
    request["uid"] = uid;

    httpClient()->post(m_apiPath + "/recall", request,
        [this, uid, callback](const QJsonObject& response) {
            bool success = isResponseSuccess(response);

            if (success) {
                qDebug() << "Message recalled successfully:" << uid;
                emit messageRecalled(uid);
            } else {
                QString message = getResponseMessage(response);
                qWarning() << "Failed to recall message:" << message;
            }

            if (callback) {
                callback(success);
            }
        },
        [this, onError](const QString& error) {
            qWarning() << "Recall message network error:" << error;
            handleNetworkError(error);
            if (onError) {
                onError(error);
            }
        }
    );
}

void MessageApi::markAsRead(const QString& threadUid, const QString& messageUid,
                           std::function<void(bool)> callback)
{
    qDebug() << "Mark message as read:" << messageUid;

    QJsonObject request;
    request["threadUid"] = threadUid;
    request["messageUid"] = messageUid;

    httpClient()->post(m_apiPath + "/read", request,
        [this, callback](const QJsonObject& response) {
            bool success = isResponseSuccess(response);

            if (success) {
                qDebug() << "Message marked as read";
            } else {
                QString message = getResponseMessage(response);
                qWarning() << "Failed to mark message as read:" << message;
            }

            if (callback) {
                callback(success);
            }
        },
        [this, callback](const QString& error) {
            qWarning() << "Mark as read network error:" << error;
            handleNetworkError(error);

            if (callback) {
                callback(false);
            }
        }
    );
}

void MessageApi::getUnreadCount(std::function<void(int)> callback,
                               std::function<void(const QString&)> onError)
{
    qDebug() << "Get unread count";

    httpClient()->get(m_apiPath + "/unread/count", QUrlQuery(),
        [this, callback](const QJsonObject& response) {
            if (isResponseSuccess(response)) {
                QJsonObject data = getResponseData(response);
                int count = data["count"].toInt();

                qDebug() << "Unread count:" << count;

                if (callback) {
                    callback(count);
                }
            } else {
                QString message = getResponseMessage(response);
                qWarning() << "Failed to get unread count:" << message;
            }
        },
        [this, onError](const QString& error) {
            qWarning() << "Get unread count network error:" << error;
            handleNetworkError(error);
            if (onError) {
                onError(error);
            }
        }
    );
}

} // namespace Bytedesk
