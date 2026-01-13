#ifndef USER_H
#define USER_H

#include <QString>
#include <QDateTime>
#include <QJsonObject>

namespace Bytedesk {

// 用户类型枚举
enum class UserType {
    AGENT = 0,      // 客服
    USER = 1,       // 访客
    MEMBER = 2,     // 成员
    ROBOT = 3,      // 机器人
    SYSTEM = 4      // 系统
};

// 用户在线状态
enum class UserStatus {
    ONLINE = 0,
    OFFLINE = 1,
    BUSY = 2,
    AWAY = 3,
    INVISIBLE = 4
};

// 用户模型
class User
{
public:
    User();
    User(const QString& uid);

    // Getters
    QString getUid() const { return m_uid; }
    UserType getType() const { return m_type; }
    QString getTypeString() const;
    UserStatus getStatus() const { return m_status; }
    QString getStatusString() const;
    QString getUsername() const { return m_username; }
    QString getNickname() const { return m_nickname; }
    QString getAvatar() const { return m_avatar; }
    QString getEmail() const { return m_email; }
    QString getPhone() const { return m_phone; }
    QString getDescription() const { return m_description; }
    QDateTime getCreatedAt() const { return m_createdAt; }
    QString getAccessToken() const { return m_accessToken; }
    QString getRefreshToken() const { return m_refreshToken; }
    QDateTime getTokenExpiresAt() const { return m_tokenExpiresAt; }

    // Setters
    void setUid(const QString& uid) { m_uid = uid; }
    void setType(UserType type) { m_type = type; }
    void setType(const QString& typeStr);
    void setStatus(UserStatus status) { m_status = status; }
    void setStatus(const QString& statusStr);
    void setUsername(const QString& username) { m_username = username; }
    void setNickname(const QString& nickname) { m_nickname = nickname; }
    void setAvatar(const QString& avatar) { m_avatar = avatar; }
    void setEmail(const QString& email) { m_email = email; }
    void setPhone(const QString& phone) { m_phone = phone; }
    void setDescription(const QString& desc) { m_description = desc; }
    void setCreatedAt(const QDateTime& time) { m_createdAt = time; }
    void setAccessToken(const QString& token) { m_accessToken = token; }
    void setRefreshToken(const QString& token) { m_refreshToken = token; }
    void setTokenExpiresAt(const QDateTime& time) { m_tokenExpiresAt = time; }

    // 序列化
    QJsonObject toJson() const;
    static User fromJson(const QJsonObject& json);

    // 工具方法
    bool isNull() const { return m_uid.isEmpty(); }
    bool isAgent() const { return m_type == UserType::AGENT; }
    bool isMember() const { return m_type == UserType::MEMBER; }
    bool isRobot() const { return m_type == UserType::ROBOT; }
    bool isVisitor() const { return m_type == UserType::USER; }

    QString getDisplayName() const {
        return m_nickname.isEmpty() ? m_username : m_nickname;
    }

    bool isTokenValid() const {
        return !m_accessToken.isEmpty() &&
               (!m_tokenExpiresAt.isValid() ||
                m_tokenExpiresAt > QDateTime::currentDateTime());
    }

    void clearTokens() {
        m_accessToken.clear();
        m_refreshToken.clear();
        m_tokenExpiresAt = QDateTime();
    }

    // 静态工具方法
    static UserType stringToType(const QString& typeStr);
    static QString typeToString(UserType type);
    static UserStatus stringToStatus(const QString& statusStr);
    static QString statusToString(UserStatus status);

private:
    QString m_uid;
    UserType m_type = UserType::USER;
    UserStatus m_status = UserStatus::OFFLINE;
    QString m_username;
    QString m_nickname;
    QString m_avatar;
    QString m_email;
    QString m_phone;
    QString m_description;
    QDateTime m_createdAt;

    // 认证相关
    QString m_accessToken;
    QString m_refreshToken;
    QDateTime m_tokenExpiresAt;
};

using UserPtr = QSharedPointer<User>;

} // namespace Bytedesk

#endif // USER_H
