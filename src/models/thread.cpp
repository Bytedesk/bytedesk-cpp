#include "thread.h"

namespace Bytedesk {

Thread::Thread()
    : m_updatedAt(QDateTime::currentDateTime())
{
}

Thread::Thread(const QString& uid)
    : m_uid(uid)
    , m_updatedAt(QDateTime::currentDateTime())
{
}

QString Thread::getTypeString() const
{
    return typeToString(m_type);
}

QString Thread::getStatusString() const
{
    return statusToString(m_status);
}

void Thread::setType(const QString& typeStr)
{
    m_type = stringToType(typeStr);
}

void Thread::setStatus(const QString& statusStr)
{
    m_status = stringToStatus(statusStr);
}

QJsonObject Thread::toJson() const
{
    QJsonObject obj;
    obj["uid"] = m_uid;
    obj["type"] = getTypeString();
    obj["status"] = getStatusString();
    obj["topic"] = m_topic;
    obj["title"] = m_title;
    obj["avatar"] = m_avatar;
    obj["description"] = m_description;
    obj["updatedAt"] = m_updatedAt.toString(Qt::ISODate);
    obj["unreadCount"] = m_unreadCount;
    obj["isPinned"] = m_isPinned;
    obj["isMuted"] = m_isMuted;
    obj["workGroupUid"] = m_workGroupUid;
    obj["agentUid"] = m_agentUid;
    obj["visitorUid"] = m_visitorUid;

    if (m_lastMessage && !m_lastMessage->isNull()) {
        obj["lastMessage"] = m_lastMessage->toJson();
    }

    return obj;
}

Thread Thread::fromJson(const QJsonObject& json)
{
    Thread thread;
    thread.setUid(json["uid"].toString());
    thread.setType(json["type"].toString());
    thread.setStatus(json["status"].toString());
    thread.setTopic(json["topic"].toString());
    thread.setTitle(json["title"].toString());
    thread.setAvatar(json["avatar"].toString());
    thread.setDescription(json["description"].toString());

    QString updatedAtStr = json["updatedAt"].toString();
    if (!updatedAtStr.isEmpty()) {
        thread.setUpdatedAt(QDateTime::fromString(updatedAtStr, Qt::ISODate));
    }

    thread.setUnreadCount(json["unreadCount"].toInt());
    thread.setPinned(json["isPinned"].toBool());
    thread.setMuted(json["isMuted"].toBool());
    thread.setWorkGroupUid(json["workGroupUid"].toString());
    thread.setAgentUid(json["agentUid"].toString());
    thread.setVisitorUid(json["visitorUid"].toString());

    if (json.contains("lastMessage")) {
        MessagePtr msg = QSharedPointer<Message>::create(
            Message::fromJson(json["lastMessage"].toObject())
        );
        thread.setLastMessage(msg);
    }

    return thread;
}

void Thread::updateLastMessage(const MessagePtr& msg)
{
    m_lastMessage = msg;
    m_updatedAt = msg->getCreatedAt();
}

ThreadType Thread::stringToType(const QString& typeStr)
{
    static QHash<QString, ThreadType> typeMap = {
        {"AGENT", ThreadType::AGENT},
        {"WORKGROUP", ThreadType::WORKGROUP},
        {"ROBOT", ThreadType::ROBOT},
        {"GROUP", ThreadType::GROUP},
        {"MEMBER", ThreadType::MEMBER}
    };
    return typeMap.value(typeStr, ThreadType::UNKNOWN);
}

QString Thread::typeToString(ThreadType type)
{
    switch (type) {
        case ThreadType::AGENT: return "AGENT";
        case ThreadType::WORKGROUP: return "WORKGROUP";
        case ThreadType::ROBOT: return "ROBOT";
        case ThreadType::GROUP: return "GROUP";
        case ThreadType::MEMBER: return "MEMBER";
        default: return "UNKNOWN";
    }
}

ThreadStatus Thread::stringToStatus(const QString& statusStr)
{
    static QHash<QString, ThreadStatus> statusMap = {
        {"QUEUEING", ThreadStatus::QUEUEING},
        {"SERVICING", ThreadStatus::SERVICING},
        {"CLOSED", ThreadStatus::CLOSED}
    };
    return statusMap.value(statusStr, ThreadStatus::UNKNOWN);
}

QString Thread::statusToString(ThreadStatus status)
{
    switch (status) {
        case ThreadStatus::QUEUEING: return "QUEUEING";
        case ThreadStatus::SERVICING: return "SERVICING";
        case ThreadStatus::CLOSED: return "CLOSED";
        default: return "UNKNOWN";
    }
}

} // namespace Bytedesk
