#ifndef MESSAGE_H
#define MESSAGE_H

#include <QString>
#include <QDateTime>
#include <QSharedPointer>
#include <QJsonObject>
#include <QJsonDocument>

namespace Bytedesk {

// 消息类型枚举
enum class MessageType {
    TEXT = 0,
    IMAGE = 1,
    FILE = 2,
    VIDEO = 3,
    VOICE = 4,
    TYPING = 5,
    NOTICE = 6,
    RECALL = 7,
    DELIVERED = 8,
    READ = 9,
    CUSTOM = 10
};

// 消息状态枚举
enum class MessageStatus {
    SENDING = 0,
    SENT = 1,
    DELIVERED = 2,
    READ = 3,
    FAILED = 4,
    RECALLED = 5
};

// 消息内容结构
struct MessageContent {
    QString text;
    QString imageUrl;
    QString fileUrl;
    QString fileName;
    qint64 fileSize = 0;
    int duration = 0; // 音视频时长（秒）
    int width = 0;    // 图片/视频宽度
    int height = 0;   // 图片/视频高度

    QJsonObject toJson() const {
        QJsonObject obj;
        if (!text.isEmpty()) obj["text"] = text;
        if (!imageUrl.isEmpty()) obj["imageUrl"] = imageUrl;
        if (!fileUrl.isEmpty()) obj["fileUrl"] = fileUrl;
        if (!fileName.isEmpty()) obj["fileName"] = fileName;
        if (fileSize > 0) obj["fileSize"] = fileSize;
        if (duration > 0) obj["duration"] = duration;
        if (width > 0) obj["width"] = width;
        if (height > 0) obj["height"] = height;
        return obj;
    }

    static MessageContent fromJson(const QJsonObject& json) {
        MessageContent content;
        content.text = json["text"].toString();
        content.imageUrl = json["imageUrl"].toString();
        content.fileUrl = json["fileUrl"].toString();
        content.fileName = json["fileName"].toString();
        content.fileSize = json["fileSize"].toVariant().toLongLong();
        content.duration = json["duration"].toInt();
        content.width = json["width"].toInt();
        content.height = json["height"].toInt();
        return content;
    }

    QString toString() const {
        return QJsonDocument(toJson()).toJson(QJsonDocument::Compact);
    }
};

// 消息模型
class Message
{
public:
    Message();
    Message(const QString& uid);

    // Getters
    QString getUid() const { return m_uid; }
    MessageType getType() const { return m_type; }
    QString getTypeString() const;
    MessageStatus getStatus() const { return m_status; }
    QString getStatusString() const;
    MessageContent getContent() const { return m_content; }
    QString getContentString() const;
    QDateTime getCreatedAt() const { return m_createdAt; }
    QString getThreadUid() const { return m_threadUid; }
    QString getUserUid() const { return m_userUid; }
    QString getUserName() const { return m_userName; }
    QString getUserAvatar() const { return m_userAvatar; }
    QString getExtra() const { return m_extra; }

    // Setters
    void setUid(const QString& uid) { m_uid = uid; }
    void setType(MessageType type) { m_type = type; }
    void setType(const QString& typeStr);
    void setStatus(MessageStatus status) { m_status = status; }
    void setStatus(const QString& statusStr);
    void setContent(const MessageContent& content) { m_content = content; }
    void setContent(const QString& contentStr);
    void setContent(const QJsonObject& contentJson) { m_content = MessageContent::fromJson(contentJson); }
    void setCreatedAt(const QDateTime& time) { m_createdAt = time; }
    void setThreadUid(const QString& uid) { m_threadUid = uid; }
    void setUserUid(const QString& uid) { m_userUid = uid; }
    void setUserName(const QString& name) { m_userName = name; }
    void setUserAvatar(const QString& avatar) { m_userAvatar = avatar; }
    void setExtra(const QString& extra) { m_extra = extra; }

    // 序列化
    QJsonObject toJson() const;
    static Message fromJson(const QJsonObject& json);

    // 工具方法
    bool isNull() const { return m_uid.isEmpty(); }
    bool isSelf(const QString& currentUserUid) const { return m_userUid == currentUserUid; }

    // 消息类型判断
    bool isTextMessage() const { return m_type == MessageType::TEXT; }
    bool isImageMessage() const { return m_type == MessageType::IMAGE; }
    bool isFileMessage() const { return m_type == MessageType::FILE; }
    bool isVideoMessage() const { return m_type == MessageType::VIDEO; }
    bool isVoiceMessage() const { return m_type == MessageType::VOICE; }
    bool isSystemMessage() const {
        return m_type == MessageType::TYPING ||
               m_type == MessageType::NOTICE ||
               m_type == MessageType::DELIVERED ||
               m_type == MessageType::READ ||
               m_type == MessageType::RECALL;
    }

    // 静态工具方法
    static QString generateUid();
    static MessageType stringToType(const QString& typeStr);
    static QString typeToString(MessageType type);
    static MessageStatus stringToStatus(const QString& statusStr);
    static QString statusToString(MessageStatus status);

private:
    QString m_uid;
    MessageType m_type = MessageType::TEXT;
    MessageStatus m_status = MessageStatus::SENDING;
    MessageContent m_content;
    QDateTime m_createdAt;
    QString m_threadUid;
    QString m_userUid;
    QString m_userName;
    QString m_userAvatar;
    QString m_extra;
};

// 使用智能指针的消息类型
using MessagePtr = QSharedPointer<Message>;

} // namespace Bytedesk

#endif // MESSAGE_H
