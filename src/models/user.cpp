#include "user.h"

namespace Bytedesk {

User::User()
    : m_createdAt(QDateTime::currentDateTime())
{
}

User::User(const QString& uid)
    : m_uid(uid)
    , m_createdAt(QDateTime::currentDateTime())
{
}

QString User::getTypeString() const
{
    return typeToString(m_type);
}

QString User::getStatusString() const
{
    return statusToString(m_status);
}

void User::setType(const QString& typeStr)
{
    m_type = stringToType(typeStr);
}

void User::setStatus(const QString& statusStr)
{
    m_status = stringToStatus(statusStr);
}

QJsonObject User::toJson() const
{
    QJsonObject obj;
    obj["uid"] = m_uid;
    obj["type"] = getTypeString();
    obj["status"] = getStatusString();
    obj["username"] = m_username;
    obj["nickname"] = m_nickname;
    obj["avatar"] = m_avatar;
    obj["email"] = m_email;
    obj["phone"] = m_phone;
    obj["description"] = m_description;
    obj["createdAt"] = m_createdAt.toString(Qt::ISODate);

    // 认证信息通常不序列化到JSON中，除非是登录响应
    // if (!m_accessToken.isEmpty()) {
    //     obj["accessToken"] = m_accessToken;
    //     obj["refreshToken"] = m_refreshToken;
    //     obj["tokenExpiresAt"] = m_tokenExpiresAt.toString(Qt::ISODate);
    // }

    return obj;
}

User User::fromJson(const QJsonObject& json)
{
    User user;
    user.setUid(json["uid"].toString());
    user.setType(json["type"].toString());
    user.setStatus(json["status"].toString());
    user.setUsername(json["username"].toString());
    user.setNickname(json["nickname"].toString());
    user.setAvatar(json["avatar"].toString());
    user.setEmail(json["email"].toString());
    user.setPhone(json["phone"].toString());
    user.setDescription(json["description"].toString());

    QString createdAtStr = json["createdAt"].toString();
    if (!createdAtStr.isEmpty()) {
        user.setCreatedAt(QDateTime::fromString(createdAtStr, Qt::ISODate));
    }

    // 处理认证信息（如果存在）
    if (json.contains("accessToken")) {
        user.setAccessToken(json["accessToken"].toString());
        user.setRefreshToken(json["refreshToken"].toString());

        QString expiresAtStr = json["tokenExpiresAt"].toString();
        if (!expiresAtStr.isEmpty()) {
            user.setTokenExpiresAt(QDateTime::fromString(expiresAtStr, Qt::ISODate));
        }
    }

    return user;
}

UserType User::stringToType(const QString& typeStr)
{
    static QHash<QString, UserType> typeMap = {
        {"AGENT", UserType::AGENT},
        {"USER", UserType::USER},
        {"MEMBER", UserType::MEMBER},
        {"ROBOT", UserType::ROBOT},
        {"SYSTEM", UserType::SYSTEM}
    };
    return typeMap.value(typeStr, UserType::USER);
}

QString User::typeToString(UserType type)
{
    switch (type) {
        case UserType::AGENT: return "AGENT";
        case UserType::USER: return "USER";
        case UserType::MEMBER: return "MEMBER";
        case UserType::ROBOT: return "ROBOT";
        case UserType::SYSTEM: return "SYSTEM";
        default: return "USER";
    }
}

UserStatus User::stringToStatus(const QString& statusStr)
{
    static QHash<QString, UserStatus> statusMap = {
        {"ONLINE", UserStatus::ONLINE},
        {"OFFLINE", UserStatus::OFFLINE},
        {"BUSY", UserStatus::BUSY},
        {"AWAY", UserStatus::AWAY},
        {"INVISIBLE", UserStatus::INVISIBLE}
    };
    return statusMap.value(statusStr, UserStatus::OFFLINE);
}

QString User::statusToString(UserStatus status)
{
    switch (status) {
        case UserStatus::ONLINE: return "ONLINE";
        case UserStatus::OFFLINE: return "OFFLINE";
        case UserStatus::BUSY: return "BUSY";
        case UserStatus::AWAY: return "AWAY";
        case UserStatus::INVISIBLE: return "INVISIBLE";
        default: return "OFFLINE";
    }
}

} // namespace Bytedesk
