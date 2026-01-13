#include "authmanager.h"
#include "models/config.h"
#include <QTimer>
#include <QDateTime>

namespace Bytedesk {

AuthManager::AuthManager(AuthApi* authApi, MqttClient* mqttClient, QObject* parent)
    : QObject(parent)
    , m_authApi(authApi)
    , m_mqttClient(mqttClient)
    , m_state(AuthState::LOGGED_OUT)
    , m_tokenRefreshTimer(new QTimer(this))
{
    Q_ASSERT(authApi);
    Q_ASSERT(mqttClient);

    // 连接信号
    connect(m_authApi, &AuthApi::loginCompleted, this, &AuthManager::onLoginSuccess);
    connect(m_authApi, &AuthApi::logoutCompleted, this, &AuthManager::onLogoutCompleted);

    connect(m_mqttClient, &MqttClient::connected, this, &AuthManager::onMqttConnected);
    connect(m_mqttClient, &MqttClient::disconnected, this, &AuthManager::onMqttDisconnected);
    connect(m_mqttClient, &MqttClient::errorOccurred, this, &AuthManager::onMqttError);

    // Token自动刷新
    connect(m_tokenRefreshTimer, &QTimer::timeout, this, &AuthManager::refreshAccessToken);

    // 加载保存的凭据
    loadSavedCredentials();
}

AuthManager::~AuthManager()
{
}

void AuthManager::login(const QString& username, const QString& password)
{
    if (m_state == AuthState::LOGGING_IN) {
        qWarning() << "Login already in progress";
        return;
    }

    qDebug() << "Login request for user:" << username;

    setState(AuthState::LOGGING_IN);

    LoginRequest request;
    request.username = username;
    request.password = password;

    m_authApi->login(request, [this](const LoginResult& result) {
        if (result.success) {
            onLoginSuccess(result);
        } else {
            qDebug() << "Login failed:" << result.message;
            setState(AuthState::ERROR);
            emit loginFailed(result.message);
        }
    });
}

void AuthManager::logout()
{
    qDebug() << "Logout request";

    m_authApi->logout([this](bool success) {
        Q_UNUSED(success);
        onLogoutCompleted();
    });
}

void AuthManager::registerUser(const QString& username, const QString& password,
                              const QString& email, const QString& nickname)
{
    qDebug() << "Register request for user:" << username;

    RegisterRequest request;
    request.username = username;
    request.password = password;
    request.email = email;
    request.nickname = nickname;

    m_authApi->registerUser(request,
        [this](bool success, const QString& message) {
            if (success) {
                qDebug() << "Registration successful";
                emit registerSuccess();
            } else {
                qWarning() << "Registration failed:" << message;
                emit registerFailed(message);
            }
        }
    );
}

void AuthManager::refreshAccessToken()
{
    if (m_refreshToken.isEmpty()) {
        qWarning() << "No refresh token available";
        return;
    }

    qDebug() << "Refreshing access token";

    m_authApi->refreshAccessToken(m_refreshToken,
        [this](const QString& accessToken) {
            if (!accessToken.isEmpty()) {
                qDebug() << "Token refreshed successfully";
                m_accessToken = accessToken;
                m_tokenExpiryTime = QDateTime::currentDateTime().addSecs(3600); // 假设1小时后过期
                emit tokenRefreshed(accessToken);
                scheduleTokenRefresh();
            } else {
                qWarning() << "Failed to refresh token";
                setState(AuthState::TOKEN_EXPIRED);
                emit tokenExpired();
            }
        }
    );
}

void AuthManager::tryAutoLogin()
{
    if (m_autoLogin && !m_accessToken.isEmpty() && !m_userUid.isEmpty()) {
        qDebug() << "Attempting auto login";

        setState(AuthState::LOGGING_IN);

        // 验证token是否仍然有效
        m_authApi->getCurrentUser(
            [this](const User& user) {
                qDebug() << "Auto login successful:" << user.getUid();
                m_currentUser = QSharedPointer<User>::create(user);
                setState(AuthState::LOGGED_IN);
                emit loginSuccess(m_currentUser);

                // 连接MQTT
                connectMqttWithToken();
            },
            [this](const QString& error) {
                qWarning() << "Auto login failed:" << error;
                clearCredentials();
                setState(AuthState::LOGGED_OUT);
                emit loginFailed("Auto login failed: " + error);
            }
        );
    } else {
        qDebug() << "Auto login not available";
    }
}

void AuthManager::onLoginSuccess(const LoginResult& result)
{
    qDebug() << "Login successful:" << result.user.getUid();

    m_accessToken = result.accessToken;
    m_refreshToken = result.refreshToken;
    m_currentUser = QSharedPointer<User>::create(result.user);
    m_userUid = result.user.getUid();
    m_tokenExpiryTime = QDateTime::currentDateTime().addSecs(3600); // 假设1小时后过期

    // 保存凭据
    if (m_rememberPassword) {
        BYTDESK_CONFIG->setAccessToken(m_accessToken);
        BYTDESK_CONFIG->setRefreshToken(m_refreshToken);
        BYTDESK_CONFIG->setUserUid(m_userUid);
        BYTDESK_CONFIG->save();
    }

    setState(AuthState::LOGGED_IN);
    emit loginSuccess(m_currentUser);

    // 连接MQTT
    connectMqttWithToken();

    // 安排token自动刷新
    scheduleTokenRefresh();
}

void AuthManager::onLogoutCompleted()
{
    qDebug() << "Logout completed";

    // 断开MQTT连接
    m_mqttClient->disconnectFromHost();

    // 清除状态
    m_accessToken.clear();
    m_refreshToken.clear();
    m_userUid.clear();
    m_currentUser.reset();
    m_tokenExpiryTime = QDateTime();

    // 清除保存的凭据
    clearCredentials();

    setState(AuthState::LOGGED_OUT);
    emit logoutSuccess();
}

void AuthManager::onMqttConnected()
{
    qDebug() << "MQTT connected, auth completed";
}

void AuthManager::onMqttDisconnected()
{
    qDebug() << "MQTT disconnected";
    // 这里可以添加重连逻辑
}

void AuthManager::onMqttError(const QString& error)
{
    qWarning() << "MQTT error during auth:" << error;
}

void AuthManager::setState(AuthState state)
{
    if (m_state != state) {
        m_state = state;
        emit authStateChanged(state);
    }
}

void AuthManager::loadSavedCredentials()
{
    m_accessToken = BYTDESK_CONFIG->getAccessToken();
    m_refreshToken = BYTDESK_CONFIG->getRefreshToken();
    m_userUid = BYTDESK_CONFIG->getUserUid();
    m_rememberPassword = BYTDESK_CONFIG->getRememberPassword();
    m_autoLogin = BYTDESK_CONFIG->getAutoLogin();

    qDebug() << "Loaded saved credentials, remember password:" << m_rememberPassword
             << "auto login:" << m_autoLogin;
}

void AuthManager::clearCredentials()
{
    BYTDESK_CONFIG->setAccessToken(QString());
    BYTDESK_CONFIG->setRefreshToken(QString());
    BYTDESK_CONFIG->setUserUid(QString());
    BYTDESK_CONFIG->save();
}

void AuthManager::connectMqttWithToken()
{
    QString mqttHost = BYTDESK_CONFIG->getMqttHost();
    int mqttPort = BYTDESK_CONFIG->getMqttPort();
    QString username = m_currentUser->getUsername();
    QString password = m_accessToken;

    // 生成客户端ID
    QString deviceUid = QUuid::createUuid().toString(QUuid::WithoutBraces);
    QString clientId = BYTDESK_CONFIG->generateMqttClientId(m_userUid, deviceUid);

    m_mqttClient->connectToHost(mqttHost, mqttPort, username, password, clientId);
}

void AuthManager::scheduleTokenRefresh()
{
    if (!m_tokenExpiryTime.isValid()) {
        return;
    }

    QDateTime now = QDateTime::currentDateTime();
    qint64 millisecondsUntilExpiry = now.msecsTo(m_tokenExpiryTime);

    // 在token过期前5分钟刷新
    qint64 refreshInterval = qMax(qint64(0), millisecondsUntilExpiry - 5 * 60 * 1000);

    qDebug() << "Scheduling token refresh in" << refreshInterval / 1000 << "seconds";

    m_tokenRefreshTimer->start(static_cast<int>(refreshInterval));
}

} // namespace Bytedesk
