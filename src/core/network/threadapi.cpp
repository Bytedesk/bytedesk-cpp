#include "threadapi.h"
#include <QJsonArray>

namespace Bytedesk {

ThreadApi::ThreadApi(HttpClient* httpClient, QObject* parent)
    : ApiBase(httpClient, parent)
{
}

ThreadApi::~ThreadApi()
{
}

void ThreadApi::createThread(const CreateThreadRequest& request, ThreadCallback callback)
{
    qDebug() << "Create thread, type:" << request.type << "uid:" << request.uid;

    httpClient()->post(m_apiPath + "/create", request.toJson(),
        [this, callback](const QJsonObject& response) {
            if (isResponseSuccess(response)) {
                QJsonObject data = getResponseData(response);
                ThreadPtr thread = QSharedPointer<Thread>::create(
                    Thread::fromJson(data)
                );

                qDebug() << "Thread created successfully:" << thread->getUid();

                emit threadCreated(thread);

                if (callback) {
                    callback(thread);
                }
            } else {
                QString message = getResponseMessage(response);
                qWarning() << "Failed to create thread:" << message;

                if (callback) {
                    callback(ThreadPtr());
                }
            }
        },
        [this, callback](const QString& error) {
            qWarning() << "Create thread network error:" << error;
            handleNetworkError(error);

            if (callback) {
                callback(ThreadPtr());
            }
        }
    );
}

void ThreadApi::closeThread(const QString& threadUid, ThreadOperationCallback callback,
                           std::function<void(const QString&)> onError)
{
    qDebug() << "Close thread:" << threadUid;

    QJsonObject request;
    request["uid"] = threadUid;

    httpClient()->post(m_apiPath + "/close", request,
        [this, threadUid, callback](const QJsonObject& response) {
            bool success = isResponseSuccess(response);

            if (success) {
                qDebug() << "Thread closed successfully:" << threadUid;
                emit threadClosed(threadUid);
            } else {
                QString message = getResponseMessage(response);
                qWarning() << "Failed to close thread:" << message;
            }

            if (callback) {
                callback(success);
            }
        },
        [this, onError](const QString& error) {
            qWarning() << "Close thread network error:" << error;
            handleNetworkError(error);
            if (onError) {
                onError(error);
            }
        }
    );
}

void ThreadApi::reopenThread(const QString& threadUid, ThreadOperationCallback callback,
                            std::function<void(const QString&)> onError)
{
    qDebug() << "Reopen thread:" << threadUid;

    QJsonObject request;
    request["uid"] = threadUid;

    httpClient()->post(m_apiPath + "/reopen", request,
        [this, callback](const QJsonObject& response) {
            bool success = isResponseSuccess(response);

            if (success) {
                qDebug() << "Thread reopened successfully";
            } else {
                QString message = getResponseMessage(response);
                qWarning() << "Failed to reopen thread:" << message;
            }

            if (callback) {
                callback(success);
            }
        },
        [this, onError](const QString& error) {
            qWarning() << "Reopen thread network error:" << error;
            handleNetworkError(error);
            if (onError) {
                onError(error);
            }
        }
    );
}

void ThreadApi::getThread(const QString& uid, ThreadCallback callback,
                         std::function<void(const QString&)> onError)
{
    qDebug() << "Get thread:" << uid;

    httpClient()->get(m_apiPath + "/" + uid, QUrlQuery(),
        [this, callback](const QJsonObject& response) {
            if (isResponseSuccess(response)) {
                QJsonObject data = getResponseData(response);
                ThreadPtr thread = QSharedPointer<Thread>::create(
                    Thread::fromJson(data)
                );

                qDebug() << "Got thread:" << thread->getUid();

                if (callback) {
                    callback(thread);
                }
            } else {
                QString message = getResponseMessage(response);
                qWarning() << "Failed to get thread:" << message;
            }
        },
        [this, onError](const QString& error) {
            qWarning() << "Get thread network error:" << error;
            handleNetworkError(error);
            if (onError) {
                onError(error);
            }
        }
    );
}

void ThreadApi::getThreads(std::function<void(const QList<ThreadPtr>&)> callback,
                          std::function<void(const QString&)> onError)
{
    qDebug() << "Get threads";

    httpClient()->get(m_apiPath + "/list", QUrlQuery(),
        [this, callback](const QJsonObject& response) {
            if (isResponseSuccess(response)) {
                QJsonObject data = getResponseData(response);
                QList<ThreadPtr> threads;

                if (data.contains("content")) {
                    QJsonArray contentArray = data["content"].toArray();
                    for (const QJsonValue& value : contentArray) {
                        ThreadPtr thread = QSharedPointer<Thread>::create(
                            Thread::fromJson(value.toObject())
                        );
                        threads.append(thread);
                    }
                }

                qDebug() << "Got threads, count:" << threads.size();

                if (callback) {
                    callback(threads);
                }
            } else {
                QString message = getResponseMessage(response);
                qWarning() << "Failed to get threads:" << message;
            }
        },
        [this, onError](const QString& error) {
            qWarning() << "Get threads network error:" << error;
            handleNetworkError(error);
            if (onError) {
                onError(error);
            }
        }
    );
}

void ThreadApi::getThreadsByType(const QString& type, ThreadsCallback callback,
                                std::function<void(const QString&)> onError)
{
    qDebug() << "Get threads by type:" << type;

    QUrlQuery query;
    query.addQueryItem("type", type);

    httpClient()->get(m_apiPath + "/list", query,
        [this, callback](const QJsonObject& response) {
            if (isResponseSuccess(response)) {
                QJsonObject data = getResponseData(response);
                QList<ThreadPtr> threads;

                if (data.contains("content")) {
                    QJsonArray contentArray = data["content"].toArray();
                    for (const QJsonValue& value : contentArray) {
                        ThreadPtr thread = QSharedPointer<Thread>::create(
                            Thread::fromJson(value.toObject())
                        );
                        threads.append(thread);
                    }
                }

                qDebug() << "Got threads by type, count:" << threads.size();

                if (callback) {
                    callback(threads);
                }
            } else {
                QString message = getResponseMessage(response);
                qWarning() << "Failed to get threads by type:" << message;
            }
        },
        [this, onError](const QString& error) {
            qWarning() << "Get threads by type network error:" << error;
            handleNetworkError(error);
            if (onError) {
                onError(error);
            }
        }
    );
}

void ThreadApi::transferThread(const QString& threadUid, const QString& toAgentUid,
                             ThreadOperationCallback callback,
                             std::function<void(const QString&)> onError)
{
    qDebug() << "Transfer thread:" << threadUid << "to agent:" << toAgentUid;

    QJsonObject request;
    request["threadUid"] = threadUid;
    request["toAgentUid"] = toAgentUid;

    httpClient()->post(m_apiPath + "/transfer", request,
        [this, callback](const QJsonObject& response) {
            bool success = isResponseSuccess(response);

            if (success) {
                qDebug() << "Thread transferred successfully";
            } else {
                QString message = getResponseMessage(response);
                qWarning() << "Failed to transfer thread:" << message;
            }

            if (callback) {
                callback(success);
            }
        },
        [this, onError](const QString& error) {
            qWarning() << "Transfer thread network error:" << error;
            handleNetworkError(error);
            if (onError) {
                onError(error);
            }
        }
    );
}

void ThreadApi::getUnreadCount(std::function<void(int)> callback,
                              std::function<void(const QString&)> onError)
{
    qDebug() << "Get thread unread count";

    httpClient()->get(m_apiPath + "/unread/count", QUrlQuery(),
        [this, callback](const QJsonObject& response) {
            if (isResponseSuccess(response)) {
                QJsonObject data = getResponseData(response);
                int count = data["count"].toInt();

                qDebug() << "Thread unread count:" << count;

                if (callback) {
                    callback(count);
                }
            } else {
                QString message = getResponseMessage(response);
                qWarning() << "Failed to get thread unread count:" << message;
            }
        },
        [this, onError](const QString& error) {
            qWarning() << "Get thread unread count network error:" << error;
            handleNetworkError(error);
            if (onError) {
                onError(error);
            }
        }
    );
}

} // namespace Bytedesk
