#include "config.h"
#include <QDir>
#include <QStandardPaths>
#include <QUuid>
#include <QRegularExpression>
#include <QUrl>

namespace Bytedesk {

// 静态成员初始化
const QString Config::DEFAULT_API_URL = "https://api.bytedesk.com";
const QString Config::DEFAULT_MQTT_URL = "wss://mqtt.bytedesk.com";
const QString Config::DEFAULT_MQTT_PATH = "/websocket";
const QString Config::DEFAULT_LANGUAGE = "zh_CN";
const QString Config::DEFAULT_THEME = "light";

Config* Config::instance()
{
    static Config config;
    return &config;
}

Config::Config()
{
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(configPath);

    QString config_file = configPath + "/bytedesk-qt.conf";
    m_settings = QSharedPointer<QSettings>::create(config_file, QSettings::IniFormat);
    // Qt 6 默认使用UTF-8，无需设置codec

    load();
}

QString Config::getApiUrl() const
{
    return m_settings->value("server/apiUrl", DEFAULT_API_URL).toString();
}

void Config::setApiUrl(const QString& url)
{
    m_settings->setValue("server/apiUrl", url);
}

QString Config::getMqttUrl() const
{
    return m_settings->value("server/mqttUrl", DEFAULT_MQTT_URL).toString();
}

void Config::setMqttUrl(const QString& url)
{
    m_settings->setValue("server/mqttUrl", url);
}

QString Config::getMqttHost() const
{
    QString url = getMqttUrl();
    // 从 wss://mqtt.example.com:443/path 提取主机
    QString host = url;
    host.remove("ws://");
    host.remove("wss://");

    int slashIndex = host.indexOf('/');
    if (slashIndex > 0) {
        host = host.left(slashIndex);
    }

    int colonIndex = host.indexOf(':');
    if (colonIndex > 0) {
        host = host.left(colonIndex);
    }

    return host;
}

void Config::setMqttHost(const QString& host)
{
    Q_UNUSED(host);
    // Host从URL解析，不单独存储
}

int Config::getMqttPort() const
{
    QString url = getMqttUrl();
    // 从URL中提取端口
    QRegularExpression portRegex(":(\\d+)");
    QRegularExpressionMatch match = portRegex.match(url);
    if (match.hasMatch()) {
        return match.captured(1).toInt();
    }

    // 默认端口
    return getMqttUseSSL() ? 443 : 1883;
}

void Config::setMqttPort(int port)
{
    Q_UNUSED(port);
    // Port从URL解析，不单独存储
}

bool Config::getMqttUseSSL() const
{
    QString url = getMqttUrl();
    return url.startsWith("wss://");
}

void Config::setMqttUseSSL(bool useSSL)
{
    QString currentUrl = getMqttUrl();
    QString newUrl = currentUrl;

    if (useSSL && !currentUrl.startsWith("wss://")) {
        newUrl.replace("ws://", "wss://");
    } else if (!useSSL && currentUrl.startsWith("wss://")) {
        newUrl.replace("wss://", "ws://");
    }

    if (newUrl != currentUrl) {
        setMqttUrl(newUrl);
    }
}

QString Config::getMqttPath() const
{
    return m_settings->value("server/mqttPath", DEFAULT_MQTT_PATH).toString();
}

void Config::setMqttPath(const QString& path)
{
    m_settings->setValue("server/mqttPath", path);
}

QString Config::getAccessToken() const
{
    return m_settings->value("user/accessToken").toString();
}

void Config::setAccessToken(const QString& token)
{
    m_settings->setValue("user/accessToken", token);
}

QString Config::getRefreshToken() const
{
    return m_settings->value("user/refreshToken").toString();
}

void Config::setRefreshToken(const QString& token)
{
    m_settings->setValue("user/refreshToken", token);
}

QString Config::getUserUid() const
{
    return m_settings->value("user/uid").toString();
}

void Config::setUserUid(const QString& uid)
{
    m_settings->setValue("user/uid", uid);
}

QString Config::getUsername() const
{
    return m_settings->value("user/username").toString();
}

void Config::setUsername(const QString& username)
{
    m_settings->setValue("user/username", username);
}

bool Config::getRememberPassword() const
{
    return m_settings->value("user/rememberPassword", false).toBool();
}

void Config::setRememberPassword(bool remember)
{
    m_settings->setValue("user/rememberPassword", remember);
}

bool Config::getAutoLogin() const
{
    return m_settings->value("user/autoLogin", false).toBool();
}

void Config::setAutoLogin(bool autoLogin)
{
    m_settings->setValue("user/autoLogin", autoLogin);
}

QString Config::getLanguage() const
{
    return m_settings->value("app/language", DEFAULT_LANGUAGE).toString();
}

void Config::setLanguage(const QString& language)
{
    m_settings->setValue("app/language", language);
}

QString Config::getTheme() const
{
    return m_settings->value("app/theme", DEFAULT_THEME).toString();
}

void Config::setTheme(const QString& theme)
{
    m_settings->setValue("app/theme", theme);
}

bool Config::getNotificationsEnabled() const
{
    return m_settings->value("app/notificationsEnabled", DEFAULT_NOTIFICATIONS_ENABLED).toBool();
}

void Config::setNotificationsEnabled(bool enabled)
{
    m_settings->setValue("app/notificationsEnabled", enabled);
}

bool Config::getSoundEnabled() const
{
    return m_settings->value("app/soundEnabled", DEFAULT_SOUND_ENABLED).toBool();
}

void Config::setSoundEnabled(bool enabled)
{
    m_settings->setValue("app/soundEnabled", enabled);
}

int Config::getMaxThreadsInMemory() const
{
    return m_settings->value("chat/maxThreadsInMemory", DEFAULT_MAX_THREADS_IN_MEMORY).toInt();
}

void Config::setMaxThreadsInMemory(int max)
{
    m_settings->setValue("chat/maxThreadsInMemory", max);
}

int Config::getMaxThreadsPersisted() const
{
    return m_settings->value("chat/maxThreadsPersisted", DEFAULT_MAX_THREADS_PERSISTED).toInt();
}

void Config::setMaxThreadsPersisted(int max)
{
    m_settings->setValue("chat/maxThreadsPersisted", max);
}

bool Config::getShowTypingIndicator() const
{
    return m_settings->value("chat/showTypingIndicator", DEFAULT_SHOW_TYPING_INDICATOR).toBool();
}

void Config::setShowTypingIndicator(bool show)
{
    m_settings->setValue("chat/showTypingIndicator", show);
}

int Config::getMqttKeepAlive() const
{
    return m_settings->value("mqtt/keepAlive", DEFAULT_MQTT_KEEP_ALIVE).toInt();
}

void Config::setMqttKeepAlive(int seconds)
{
    m_settings->setValue("mqtt/keepAlive", seconds);
}

int Config::getMqttReconnectPeriod() const
{
    return m_settings->value("mqtt/reconnectPeriod", DEFAULT_MQTT_RECONNECT_PERIOD).toInt();
}

void Config::setMqttReconnectPeriod(int milliseconds)
{
    m_settings->setValue("mqtt/reconnectPeriod", milliseconds);
}

int Config::getMqttConnectTimeout() const
{
    return m_settings->value("mqtt/connectTimeout", DEFAULT_MQTT_CONNECT_TIMEOUT).toInt();
}

void Config::setMqttConnectTimeout(int milliseconds)
{
    m_settings->setValue("mqtt/connectTimeout", milliseconds);
}

bool Config::getMqttCleanSession() const
{
    return m_settings->value("mqtt/cleanSession", DEFAULT_MQTT_CLEAN_SESSION).toBool();
}

void Config::setMqttCleanSession(bool clean)
{
    m_settings->setValue("mqtt/cleanSession", clean);
}

void Config::clearUserData()
{
    m_settings->remove("user");
}

void Config::clearAll()
{
    m_settings->clear();
}

void Config::save()
{
    m_settings->sync();
}

void Config::load()
{
    // QSettings自动加载，这里可以做一些额外的初始化
}

QString Config::generateMqttClientId(const QString& userUid, const QString& deviceUid) const
{
    if (deviceUid.isEmpty()) {
        return QString("%1/HTTP/%2").arg(userUid, QUuid::createUuid().toString(QUuid::WithoutBraces));
    }
    return QString("%1/HTTP/%2").arg(userUid, deviceUid);
}

} // namespace Bytedesk
