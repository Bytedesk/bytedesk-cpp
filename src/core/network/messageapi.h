#ifndef MESSAGEAPI_H
#define MESSAGEAPI_H

#include "apibase.h"
#include "models/message.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QUrlQuery>
#include <functional>

namespace Bytedesk {

// 分页请求参数
struct PageRequest {
    int page = 0;
    int size = 20;
    QString threadUid;
    QString sort = "createdAt,desc";

    QUrlQuery toQuery() const {
        QUrlQuery query;
        query.addQueryItem("page", QString::number(page));
        query.addQueryItem("size", QString::number(size));
        if (!threadUid.isEmpty()) {
            query.addQueryItem("threadUid", threadUid);
        }
        if (!sort.isEmpty()) {
            query.addQueryItem("sort", sort);
        }
        return query;
    }
};

// 分页响应
struct PageResult {
    int totalPages = 0;
    long totalElements = 0;
    int currentPage = 0;
    int pageSize = 0;
    bool hasNext = false;
    bool hasPrevious = false;

    QList<QSharedPointer<Message>> messages;

    static PageResult fromJson(const QJsonObject& json) {
        PageResult result;
        if (json.contains("content")) {
            QJsonArray contentArray = json["content"].toArray();
            for (const QJsonValue& value : contentArray) {
                MessagePtr message = QSharedPointer<Message>::create(
                    Message::fromJson(value.toObject())
                );
                result.messages.append(message);
            }
        }

        if (json.contains("totalPages")) {
            result.totalPages = json["totalPages"].toInt();
        }
        if (json.contains("totalElements")) {
            result.totalElements = json["totalElements"].toVariant().toLongLong();
        }
        if (json.contains("number")) {
            result.currentPage = json["number"].toInt();
        }
        if (json.contains("size")) {
            result.pageSize = json["size"].toInt();
        }
        if (json.contains("hasNext")) {
            result.hasNext = json["hasNext"].toBool();
        }
        if (json.contains("hasPrevious")) {
            result.hasPrevious = json["hasPrevious"].toBool();
        }

        return result;
    }
};

// 发送消息请求
struct SendMessageRequest {
    QString threadUid;
    QString type = "TEXT";
    QString content; // JSON字符串

    QJsonObject toJson() const {
        QJsonObject obj;
        obj["threadUid"] = threadUid;
        obj["type"] = type;
        obj["content"] = content;
        obj["clientType"] = "HTTP";
        return obj;
    }
};

// 消息API回调
using MessagesCallback = std::function<void(const PageResult& result)>;
using SendMessageCallback = std::function<void(const MessagePtr& message)>;
using MessageCallback = std::function<void(const MessagePtr& message)>;

// 消息API类
class MessageApi : public ApiBase
{
    Q_OBJECT

public:
    explicit MessageApi(HttpClient* httpClient, QObject* parent = nullptr);
    ~MessageApi();

    // 查询消息列表
    void queryMessages(const PageRequest& request, MessagesCallback callback);

    // 根据会话主题查询消息
    void queryMessagesByTopic(const QString& topic, const PageRequest& request, MessagesCallback callback);

    // 通过REST发送消息
    void sendMessage(const SendMessageRequest& request, SendMessageCallback callback);

    // 获取单条消息详情
    void getMessage(const QString& uid, MessageCallback callback,
                   std::function<void(const QString& error)> onError = nullptr);

    // 撤回消息
    void recallMessage(const QString& uid,
                      std::function<void(bool success)> callback,
                      std::function<void(const QString& error)> onError = nullptr);

    // 标记消息已读
    void markAsRead(const QString& threadUid, const QString& messageUid,
                   std::function<void(bool success)> callback);

    // 获取未读消息数
    void getUnreadCount(std::function<void(int count)> callback,
                       std::function<void(const QString& error)> onError = nullptr);

signals:
    void messageReceived(const MessagePtr& message);
    void messageSent(const MessagePtr& message);
    void messageRecalled(const QString& messageUid);

private:
    QString m_apiPath = "/api/v1/message";
};

} // namespace Bytedesk

#endif // MESSAGEAPI_H
