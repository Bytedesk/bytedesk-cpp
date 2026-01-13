#ifndef THREADAPI_H
#define THREADAPI_H

#include "apibase.h"
#include "models/thread.h"
#include <QJsonObject>
#include <functional>

namespace Bytedesk {

// 创建会话请求
struct CreateThreadRequest {
    QString type; // AGENT, WORKGROUP, ROBOT, etc.
    QString uid;
    QString topic;

    QJsonObject toJson() const {
        QJsonObject obj;
        obj["type"] = type;
        obj["uid"] = uid;
        if (!topic.isEmpty()) {
            obj["topic"] = topic;
        }
        return obj;
    }
};

// 会话API回调
using ThreadCallback = std::function<void(const ThreadPtr& thread)>;
using ThreadsCallback = std::function<void(const QList<ThreadPtr>& threads)>;
using ThreadOperationCallback = std::function<void(bool success)>;

// 会话API类
class ThreadApi : public ApiBase
{
    Q_OBJECT

public:
    explicit ThreadApi(HttpClient* httpClient, QObject* parent = nullptr);
    ~ThreadApi();

    // 创建会话
    void createThread(const CreateThreadRequest& request, ThreadCallback callback);

    // 关闭会话
    void closeThread(const QString& threadUid, ThreadOperationCallback callback,
                    std::function<void(const QString& error)> onError = nullptr);

    // 重新打开会话
    void reopenThread(const QString& threadUid, ThreadOperationCallback callback,
                     std::function<void(const QString& error)> onError = nullptr);

    // 获取会话详情
    void getThread(const QString& uid, ThreadCallback callback,
                  std::function<void(const QString& error)> onError = nullptr);

    // 获取会话列表
    void getThreads(std::function<void(const QList<ThreadPtr>& threads)> callback,
                   std::function<void(const QString& error)> onError = nullptr);

    // 根据类型获取会话
    void getThreadsByType(const QString& type, ThreadsCallback callback,
                         std::function<void(const QString& error)> onError = nullptr);

    // 转接会话
    void transferThread(const QString& threadUid, const QString& toAgentUid,
                       ThreadOperationCallback callback,
                       std::function<void(const QString& error)> onError = nullptr);

    // 获取会话未读数
    void getUnreadCount(std::function<void(int count)> callback,
                       std::function<void(const QString& error)> onError = nullptr);

signals:
    void threadCreated(const ThreadPtr& thread);
    void threadClosed(const QString& threadUid);
    void threadUpdated(const ThreadPtr& thread);

private:
    QString m_apiPath = "/api/v1/thread";
};

} // namespace Bytedesk

#endif // THREADAPI_H
