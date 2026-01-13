#include "message.h"
#include <QUuid>
#include <QJsonArray>

namespace Bytedesk {

Message::Message()
    : m_createdAt(QDateTime::currentDateTime())
{
}

Message::Message(const QString& uid)
    : m_uid(uid)
    , m_createdAt(QDateTime::currentDateTime())
{
}

QString Message::getTypeString() const
{
    return typeToString(m_type);
}

QString Message::getStatusString() const
{
    return statusToString(m_status);
}

QString Message::getContentString() const
{
    return m_content.toString();
}

void Message::setType(const QString& typeStr)
{
    m_type = stringToType(typeStr);
}

void Message::setStatus(const QString& statusStr)
{
    m_status = stringToStatus(statusStr);
}

void Message::setContent(const QString& contentStr)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(contentStr.toUtf8(), &error);
    if (error.error == QJsonParseError::NoError && doc.isObject()) {
        m_content = MessageContent::fromJson(doc.object());
    } else {
        // 纯文本
        m_content.text = contentStr;
    }
}

QJsonObject Message::toJson() const
{
    QJsonObject obj;
    obj["uid"] = m_uid;
    obj["type"] = getTypeString();
    obj["status"] = getStatusString();
    obj["content"] = m_content.toJson();
    obj["createdAt"] = m_createdAt.toString(Qt::ISODate);
    obj["threadUid"] = m_threadUid;
    obj["userUid"] = m_userUid;
    obj["userName"] = m_userName;
    obj["userAvatar"] = m_userAvatar;
    if (!m_extra.isEmpty()) {
        obj["extra"] = QJsonDocument::fromJson(m_extra.toUtf8()).object();
    }
    return obj;
}

Message Message::fromJson(const QJsonObject& json)
{
    Message msg;
    msg.setUid(json["uid"].toString());
    msg.setType(json["type"].toString());
    msg.setStatus(json["status"].toString());

    if (json.contains("content")) {
        msg.setContent(json["content"].toObject());
    }

    msg.setCreatedAt(QDateTime::fromString(json["createdAt"].toString(), Qt::ISODate));
    msg.setThreadUid(json["threadUid"].toString());
    msg.setUserUid(json["userUid"].toString());
    msg.setUserName(json["userName"].toString());
    msg.setUserAvatar(json["userAvatar"].toString());

    if (json.contains("extra")) {
        msg.setExtra(QJsonDocument(json["extra"].toObject()).toJson(QJsonDocument::Compact));
    }

    return msg;
}

QString Message::generateUid()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

MessageType Message::stringToType(const QString& typeStr)
{
    static QHash<QString, MessageType> typeMap = {
        {"TEXT", MessageType::TEXT},
        {"IMAGE", MessageType::IMAGE},
        {"FILE", MessageType::FILE},
        {"VIDEO", MessageType::VIDEO},
        {"VOICE", MessageType::VOICE},
        {"TYPING", MessageType::TYPING},
        {"NOTICE", MessageType::NOTICE},
        {"RECALL", MessageType::RECALL},
        {"DELIVERED", MessageType::DELIVERED},
        {"READ", MessageType::READ},
        {"CUSTOM", MessageType::CUSTOM}
    };
    return typeMap.value(typeStr, MessageType::TEXT);
}

QString Message::typeToString(MessageType type)
{
    switch (type) {
        case MessageType::TEXT: return "TEXT";
        case MessageType::IMAGE: return "IMAGE";
        case MessageType::FILE: return "FILE";
        case MessageType::VIDEO: return "VIDEO";
        case MessageType::VOICE: return "VOICE";
        case MessageType::TYPING: return "TYPING";
        case MessageType::NOTICE: return "NOTICE";
        case MessageType::RECALL: return "RECALL";
        case MessageType::DELIVERED: return "DELIVERED";
        case MessageType::READ: return "READ";
        case MessageType::CUSTOM: return "CUSTOM";
        default: return "TEXT";
    }
}

MessageStatus Message::stringToStatus(const QString& statusStr)
{
    static QHash<QString, MessageStatus> statusMap = {
        {"SENDING", MessageStatus::SENDING},
        {"SENT", MessageStatus::SENT},
        {"DELIVERED", MessageStatus::DELIVERED},
        {"READ", MessageStatus::READ},
        {"FAILED", MessageStatus::FAILED},
        {"RECALLED", MessageStatus::RECALLED}
    };
    return statusMap.value(statusStr, MessageStatus::SENDING);
}

QString Message::statusToString(MessageStatus status)
{
    switch (status) {
        case MessageStatus::SENDING: return "SENDING";
        case MessageStatus::SENT: return "SENT";
        case MessageStatus::DELIVERED: return "DELIVERED";
        case MessageStatus::READ: return "READ";
        case MessageStatus::FAILED: return "FAILED";
        case MessageStatus::RECALLED: return "RECALLED";
        default: return "SENDING";
    }
}

} // namespace Bytedesk
