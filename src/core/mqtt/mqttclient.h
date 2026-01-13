#ifndef MQTTCLIENT_H
#define MQTTCLIENT_H

#include <QObject>
#include <QString>
#include <QHash>
#include <QTimer>
#include <QMutex>
#include <QTcpSocket>
#include <functional>

#include "models/message.h"
#include "models/thread.h"
#include "models/user.h"

namespace Bytedesk {

// MQTT连接状态
enum class MqttConnectionState {
    DISCONNECTED = 0,
    CONNECTING = 1,
    CONNECTED = 2,
    RECONNECTING = 3,
    ERROR = 4
};

// MQTT消息回调类型
using MqttMessageCallback = std::function<void(const QString& topic, const QByteArray& message)>;
using MqttConnectedCallback = std::function<void()>;
using MqttDisconnectedCallback = std::function<void()>;
using MqttErrorCallback = std::function<void(const QString& error)>;

// MQTT客户端类 - 使用TCP socket的简化实现
// 注意：这是一个简化的MQTT实现，用于演示目的
// 生产环境建议使用完整的MQTT库或Qt MQTT模块
class MqttClient : public QObject
{
    Q_OBJECT

public:
    explicit MqttClient(QObject* parent = nullptr);
    ~MqttClient();

    // 连接管理
    void connectToHost(const QString& host, int port, const QString& username,
                      const QString& password, const QString& clientId);
    void disconnectFromHost();
    void reconnect();

    bool isConnected() const;
    MqttConnectionState getConnectionState() const { return m_state; }

    // 订阅管理
    void subscribe(const QString& topic, quint8 qos = 0);
    void unsubscribe(const QString& topic);
    void unsubscribeAll();

    // 消息发布
    void publish(const QString& topic, const QByteArray& payload, quint8 qos = 0, bool retain = false);

    // 回调设置
    void setMessageCallback(MqttMessageCallback callback);
    void setConnectedCallback(MqttConnectedCallback callback);
    void setDisconnectedCallback(MqttDisconnectedCallback callback);
    void setErrorCallback(MqttErrorCallback callback);

    // 心跳和重连
    void startKeepAlive(int intervalMs = 30000);
    void stopKeepAlive();

    // 配置
    void setKeepAlive(int seconds);
    void setReconnectInterval(int milliseconds);
    void setConnectTimeout(int milliseconds);
    void setCleanSession(bool clean);
    void setWillMessage(const QString& topic, const QByteArray& message, quint8 qos);

    QString getClientId() const { return m_clientId; }
    QString getUsername() const { return m_username; }

signals:
    void connected();
    void disconnected();
    void errorOccurred(const QString& error);
    void messageReceived(const QString& topic, const QByteArray& payload);
    void connectionStateChanged(MqttConnectionState state);

private slots:
    void onConnected();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError error);
    void onReadyRead();
    void onKeepAliveTimeout();
    void onConnectTimeout();

private:
    void setState(MqttConnectionState state);
    void scheduleReconnect();
    void updateLastMessageTime();
    void sendMqttConnect();
    void sendMqttSubscribe(const QString& topic, quint8 qos);
    void sendMqttPublish(const QString& topic, const QByteArray& payload, quint8 qos, bool retain);

    // MQTT协议相关方法
    QByteArray encodeString(const QString& str);
    quint16 calculateMessageId();

    QTcpSocket* m_socket;
    MqttConnectionState m_state;
    QString m_clientId;
    QString m_host;
    int m_port;
    QString m_username;
    QString m_password;
    bool m_cleanSession;

    // 订阅管理
    QHash<QString, quint8> m_subscriptions;

    // 回调函数
    MqttMessageCallback m_messageCallback;
    MqttConnectedCallback m_connectedCallback;
    MqttDisconnectedCallback m_disconnectedCallback;
    MqttErrorCallback m_errorCallback;

    // 心跳和重连
    QTimer* m_keepAliveTimer;
    QTimer* m_reconnectTimer;
    QTimer* m_connectTimeoutTimer;

    QDateTime m_lastMessageTime;
    int m_keepAliveInterval;
    int m_reconnectInterval;
    int m_connectTimeout;
    int m_maxReconnectAttempts;
    int m_currentReconnectAttempt;
    bool m_manualDisconnect;
    quint16 m_messageId;

    QString m_willTopic;
    QByteArray m_willMessage;
    quint8 m_willQos;

    QMutex m_mutex;

    // 读取缓冲区
    QByteArray m_readBuffer;
};

} // namespace Bytedesk

#endif // MQTTCLIENT_H
