#ifndef AUTHAPI_H
#define AUTHAPI_H

#include "apibase.h"
#include "models/user.h"
#include <QJsonObject>
#include <functional>

namespace Bytedesk {

// 登录请求结构
struct LoginRequest {
    QString username;
    QString password;
    QString clientType = "HTTP"; // 默认HTTP客户端类型

    QJsonObject toJson() const {
        QJsonObject obj;
        obj["username"] = username;
        obj["password"] = password;
        obj["clientType"] = clientType;
        return obj;
    }
};

// 登录响应结构
struct LoginResult {
    bool success = false;
    QString message;
    QString accessToken;
    QString refreshToken;
    User user;

    static LoginResult fromJson(const QJsonObject& json) {
        LoginResult result;
        result.success = json["statusCode"].toInt() == 200;
        result.message = json["message"].toString();

        if (json.contains("data")) {
            QJsonObject data = json["data"].toObject();
            result.accessToken = data["accessToken"].toString();
            result.refreshToken = data["refreshToken"].toString();

            if (data.contains("user")) {
                result.user = User::fromJson(data["user"].toObject());
            }
        }

        return result;
    }
};

// 注册请求结构
struct RegisterRequest {
    QString username;
    QString password;
    QString email;
    QString nickname;
    QString phone;

    QJsonObject toJson() const {
        QJsonObject obj;
        obj["username"] = username;
        obj["password"] = password;
        if (!email.isEmpty()) obj["email"] = email;
        if (!nickname.isEmpty()) obj["nickname"] = nickname;
        if (!phone.isEmpty()) obj["phone"] = phone;
        return obj;
    }
};

// 认证API回调
using LoginCallback = std::function<void(const LoginResult& result)>;
using RegisterCallback = std::function<void(bool success, const QString& message)>;
using LogoutCallback = std::function<void(bool success)>;
using RefreshTokenCallback = std::function<void(const QString& accessToken)>;

// 认证API类
class AuthApi : public ApiBase
{
    Q_OBJECT

public:
    explicit AuthApi(HttpClient* httpClient, QObject* parent = nullptr);
    ~AuthApi();

    // 登录
    void login(const LoginRequest& request, LoginCallback callback);

    // 注册
    void registerUser(const RegisterRequest& request, RegisterCallback callback);

    // 登出
    void logout(LogoutCallback callback);

    // 刷新Token
    void refreshAccessToken(const QString& refreshToken, RefreshTokenCallback callback);

    // 获取当前用户信息
    void getCurrentUser(std::function<void(const User& user)> callback,
                       std::function<void(const QString& error)> onError = nullptr);

    // 更新用户信息
    void updateProfile(const QString& nickname, const QString& avatar,
                      std::function<void(bool success)> callback,
                      std::function<void(const QString& error)> onError = nullptr);

signals:
    void loginCompleted(const LoginResult& result);
    void logoutCompleted();

private:
    QString m_authPath = "/auth/v1";
    QString m_userPath = "/api/v1/user";
};

} // namespace Bytedesk

#endif // AUTHAPI_H
