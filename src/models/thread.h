#ifndef THREAD_H
#define THREAD_H

#include <QString>
#include <QDateTime>
#include <QSharedPointer>
#include <QJsonObject>
#include "message.h"

namespace Bytedesk {

// 会话类型枚举
enum class ThreadType {
    AGENT = 0,        // 客服会话
    WORKGROUP = 1,    // 工作组
    ROBOT = 2,        // 机器人
    GROUP = 3,        // 群聊
    MEMBER = 4,       // 内部成员
    UNKNOWN = 99
};

// 会话状态枚举
enum class ThreadStatus {
    QUEUEING = 0,     // 排队中
    SERVICING = 1,    // 服务中
    CLOSED = 2,       // 已关闭
    UNKNOWN = 99
};

// 会话模型
class Thread
{
public:
    Thread();
    Thread(const QString& uid);

    // Getters
    QString getUid() const { return m_uid; }
    ThreadType getType() const { return m_type; }
    QString getTypeString() const;
    ThreadStatus getStatus() const { return m_status; }
    QString getStatusString() const;
    QString getTopic() const { return m_topic; }
    QString getTitle() const { return m_title; }
    QString getAvatar() const { return m_avatar; }
    QString getDescription() const { return m_description; }
    QDateTime getUpdatedAt() const { return m_updatedAt; }
    MessagePtr getLastMessage() const { return m_lastMessage; }
    int getUnreadCount() const { return m_unreadCount; }
    bool isPinned() const { return m_isPinned; }
    bool isMuted() const { return m_isMuted; }
    QString getWorkGroupUid() const { return m_workGroupUid; }
    QString getAgentUid() const { return m_agentUid; }
    QString getVisitorUid() const { return m_visitorUid; }

    // Setters
    void setUid(const QString& uid) { m_uid = uid; }
    void setType(ThreadType type) { m_type = type; }
    void setType(const QString& typeStr);
    void setStatus(ThreadStatus status) { m_status = status; }
    void setStatus(const QString& statusStr);
    void setTopic(const QString& topic) { m_topic = topic; }
    void setTitle(const QString& title) { m_title = title; }
    void setAvatar(const QString& avatar) { m_avatar = avatar; }
    void setDescription(const QString& desc) { m_description = desc; }
    void setUpdatedAt(const QDateTime& time) { m_updatedAt = time; }
    void setLastMessage(const MessagePtr& msg) { m_lastMessage = msg; }
    void setUnreadCount(int count) { m_unreadCount = count; }
    void setPinned(bool pinned) { m_isPinned = pinned; }
    void setMuted(bool muted) { m_isMuted = muted; }
    void setWorkGroupUid(const QString& uid) { m_workGroupUid = uid; }
    void setAgentUid(const QString& uid) { m_agentUid = uid; }
    void setVisitorUid(const QString& uid) { m_visitorUid = uid; }

    // 序列化
    QJsonObject toJson() const;
    static Thread fromJson(const QJsonObject& json);

    // 工具方法
    bool isNull() const { return m_uid.isEmpty(); }
    bool isActive() const { return m_status == ThreadStatus::SERVICING; }
    bool isClosed() const { return m_status == ThreadStatus::CLOSED; }
    bool isQueueing() const { return m_status == ThreadStatus::QUEUEING; }

    void incrementUnreadCount() { m_unreadCount++; }
    void clearUnreadCount() { m_unreadCount = 0; }

    void updateLastMessage(const MessagePtr& msg);

    // 静态工具方法
    static ThreadType stringToType(const QString& typeStr);
    static QString typeToString(ThreadType type);
    static ThreadStatus stringToStatus(const QString& statusStr);
    static QString statusToString(ThreadStatus status);

private:
    QString m_uid;
    ThreadType m_type = ThreadType::AGENT;
    ThreadStatus m_status = ThreadStatus::UNKNOWN;
    QString m_topic;
    QString m_title;
    QString m_avatar;
    QString m_description;
    QDateTime m_updatedAt;
    MessagePtr m_lastMessage;
    int m_unreadCount = 0;
    bool m_isPinned = false;
    bool m_isMuted = false;
    QString m_workGroupUid;
    QString m_agentUid;
    QString m_visitorUid;
};

using ThreadPtr = QSharedPointer<Thread>;

} // namespace Bytedesk

#endif // THREAD_H
