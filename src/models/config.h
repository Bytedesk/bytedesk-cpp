#ifndef CONFIG_H
#define CONFIG_H

#include <QString>
#include <QSettings>
#include <QSharedPointer>

namespace Bytedesk {

// 应用配置类
class Config
{
public:
    static Config* instance();

    // 服务器配置
    QString getApiUrl() const;
    void setApiUrl(const QString& url);

    QString getMqttUrl() const;
    void setMqttUrl(const QString& url);

    QString getMqttHost() const;
    void setMqttHost(const QString& host);

    int getMqttPort() const;
    void setMqttPort(int port);

    bool getMqttUseSSL() const;
    void setMqttUseSSL(bool useSSL);

    QString getMqttPath() const;
    void setMqttPath(const QString& path);

    // 用户配置
    QString getAccessToken() const;
    void setAccessToken(const QString& token);

    QString getRefreshToken() const;
    void setRefreshToken(const QString& token);

    QString getUserUid() const;
    void setUserUid(const QString& uid);

    QString getUsername() const;
    void setUsername(const QString& username);

    bool getRememberPassword() const;
    void setRememberPassword(bool remember);

    bool getAutoLogin() const;
    void setAutoLogin(bool autoLogin);

    // 应用设置
    QString getLanguage() const;
    void setLanguage(const QString& language);

    QString getTheme() const;
    void setTheme(const QString& theme);

    bool getNotificationsEnabled() const;
    void setNotificationsEnabled(bool enabled);

    bool getSoundEnabled() const;
    void setSoundEnabled(bool enabled);

    // 聊天设置
    int getMaxThreadsInMemory() const;
    void setMaxThreadsInMemory(int max);

    int getMaxThreadsPersisted() const;
    void setMaxThreadsPersisted(int max);

    bool getShowTypingIndicator() const;
    void setShowTypingIndicator(bool show);

    // MQTT配置
    int getMqttKeepAlive() const;
    void setMqttKeepAlive(int seconds);

    int getMqttReconnectPeriod() const;
    void setMqttReconnectPeriod(int milliseconds);

    int getMqttConnectTimeout() const;
    void setMqttConnectTimeout(int milliseconds);

    bool getMqttCleanSession() const;
    void setMqttCleanSession(bool clean);

    // 工具方法
    void clearUserData();
    void clearAll();
    void save();
    void load();

    // 生成MQTT客户端ID
    QString generateMqttClientId(const QString& userUid, const QString& deviceUid) const;

private:
    Config();
    ~Config() = default;

    Q_DISABLE_COPY(Config)

    QSharedPointer<QSettings> m_settings;

    // 默认值
    static const QString DEFAULT_API_URL;
    static const QString DEFAULT_MQTT_URL;
    static const int DEFAULT_MQTT_PORT = 1883;
    static const bool DEFAULT_MQTT_USE_SSL = true;
    static const QString DEFAULT_MQTT_PATH;
    static const int DEFAULT_MQTT_KEEP_ALIVE = 60;
    static const int DEFAULT_MQTT_RECONNECT_PERIOD = 3000;
    static const int DEFAULT_MQTT_CONNECT_TIMEOUT = 30000;
    static const bool DEFAULT_MQTT_CLEAN_SESSION = false;
    static const int DEFAULT_MAX_THREADS_IN_MEMORY = 300;
    static const int DEFAULT_MAX_THREADS_PERSISTED = 200;
    static const QString DEFAULT_LANGUAGE;
    static const QString DEFAULT_THEME;
    static const bool DEFAULT_NOTIFICATIONS_ENABLED = true;
    static const bool DEFAULT_SOUND_ENABLED = true;
    static const bool DEFAULT_SHOW_TYPING_INDICATOR = true;
};

#define BYTDESK_CONFIG Config::instance()

} // namespace Bytedesk

#endif // CONFIG_H
