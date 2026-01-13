#ifndef AUTHMANAGER_H
#define AUTHMANAGER_H

#include <QObject>
#include <QString>
#include "models/user.h"
#include "core/network/authapi.h"
#include "core/mqtt/mqttclient.h"
#include <functional>

namespace Bytedesk {

// 认证状态
enum class AuthState {
    LOGGED_OUT = 0,
    LOGGING_IN = 1,
    LOGGED_IN = 2,
    TOKEN_EXPIRED = 3,
    ERROR = 4
};

// 认证管理器 - 统一管理用户认证状态
class AuthManager : public QObject
{
    Q_OBJECT

public:
    explicit AuthManager(AuthApi* authApi, MqttClient* mqttClient, QObject* parent = nullptr);
    ~AuthManager();

    // 认证操作
    void login(const QString& username, const QString& password);
    void logout();
    void registerUser(const QString& username, const QString& password,
                     const QString& email, const QString& nickname);

    // Token刷新
    void refreshAccessToken();

    // 获取状态
    AuthState getAuthState() const { return m_state; }
    bool isLoggedIn() const { return m_state == AuthState::LOGGED_IN; }
    bool isLoggingIn() const { return m_state == AuthState::LOGGING_IN; }

    // 获取当前用户
    UserPtr getCurrentUser() const { return m_currentUser; }
    QString getAccessToken() const { return m_accessToken; }
    QString getUserUid() const { return m_userUid; }

    // 自动登录
    void tryAutoLogin();

signals:
    void authStateChanged(AuthState state);
    void loginSuccess(const UserPtr& user);
    void loginFailed(const QString& error);
    void logoutSuccess();
    void registerSuccess();
    void registerFailed(const QString& error);
    void tokenRefreshed(const QString& accessToken);
    void tokenExpired();

private slots:
    void onLoginSuccess(const LoginResult& result);
    void onLogoutCompleted();
    void onMqttConnected();
    void onMqttDisconnected();
    void onMqttError(const QString& error);

private:
    void setState(AuthState state);
    void loadSavedCredentials();
    void clearCredentials();
    void connectMqttWithToken();
    void scheduleTokenRefresh();

    AuthApi* m_authApi;
    MqttClient* m_mqttClient;

    AuthState m_state;
    UserPtr m_currentUser;
    QString m_accessToken;
    QString m_refreshToken;
    QString m_userUid;
    QDateTime m_tokenExpiryTime;

    QTimer* m_tokenRefreshTimer;
    bool m_rememberPassword;
    bool m_autoLogin;
};

} // namespace Bytedesk

#endif // AUTHMANAGER_H
