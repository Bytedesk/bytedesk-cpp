#include "mqttclient.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QDebug>
#include <QHostAddress>
#include <QtGlobal>

namespace Bytedesk {

// MQTT协议常量
namespace MqttProtocol {
    const quint8 CONNECT = 0x10;
    const quint8 CONNACK = 0x20;
    const quint8 PUBLISH = 0x30;
    const quint8 PUBACK = 0x40;
    const quint8 SUBSCRIBE = 0x82;
    const quint8 SUBACK = 0x90;
    const quint8 UNSUBSCRIBE = 0xA2;
    const quint8 UNSUBACK = 0xB0;
    const quint8 PINGREQ = 0xC0;
    const quint8 PINGRESP = 0xD0;
    const quint8 DISCONNECT = 0xE0;
}

MqttClient::MqttClient(QObject* parent)
    : QObject(parent)
    , m_socket(new QTcpSocket(this))
    , m_state(MqttConnectionState::DISCONNECTED)
    , m_port(1883)
    , m_cleanSession(true)
    , m_keepAliveTimer(new QTimer(this))
    , m_reconnectTimer(new QTimer(this))
    , m_connectTimeoutTimer(new QTimer(this))
    , m_keepAliveInterval(30000)
    , m_reconnectInterval(3000)
    , m_connectTimeout(30000)
    , m_maxReconnectAttempts(5)
    , m_currentReconnectAttempt(0)
    , m_manualDisconnect(false)
    , m_messageId(1)
    , m_willQos(0)
{
    connect(m_socket, &QTcpSocket::connected, this, &MqttClient::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &MqttClient::onDisconnected);
    connect(m_socket, &QTcpSocket::errorOccurred, this, &MqttClient::onError);
    connect(m_socket, &QTcpSocket::readyRead, this, &MqttClient::onReadyRead);

    connect(m_keepAliveTimer, &QTimer::timeout, this, &MqttClient::onKeepAliveTimeout);
    connect(m_reconnectTimer, &QTimer::timeout, this, &MqttClient::reconnect);
    connect(m_connectTimeoutTimer, &QTimer::timeout, this, &MqttClient::onConnectTimeout);
}

MqttClient::~MqttClient()
{
    disconnectFromHost();
}

void MqttClient::connectToHost(const QString& host, int port, const QString& username,
                              const QString& password, const QString& clientId)
{
    m_host = host;
    m_port = port;
    m_username = username;
    m_password = password;
    m_clientId = clientId;
    m_manualDisconnect = false;
    m_currentReconnectAttempt = 0;

    qDebug() << "MQTT connecting to:" << host << ":" << port << "as" << clientId;

    setState(MqttConnectionState::CONNECTING);
    m_connectTimeoutTimer->start(m_connectTimeout);

    m_socket->connectToHost(host, port);
}

void MqttClient::disconnectFromHost()
{
    m_manualDisconnect = true;
    m_reconnectTimer->stop();
    m_connectTimeoutTimer->stop();
    m_keepAliveTimer->stop();

    if (m_socket->state() == QTcpSocket::ConnectedState) {
        // 发送DISCONNECT报文
        QByteArray packet;
        packet.append(static_cast<char>(MqttProtocol::DISCONNECT));
        packet.append(static_cast<char>(0x00)); // Remaining length
        m_socket->write(packet);

        m_socket->disconnectFromHost();
    }

    setState(MqttConnectionState::DISCONNECTED);
}

void MqttClient::reconnect()
{
    if (m_manualDisconnect) {
        return;
    }

    if (m_currentReconnectAttempt >= m_maxReconnectAttempts) {
        QString error = QString("Max reconnect attempts (%1) reached").arg(m_maxReconnectAttempts);
        qWarning() << "MQTT:" << error;
        setState(MqttConnectionState::ERROR);
        m_reconnectTimer->stop();
        emit errorOccurred(error);
        return;
    }

    m_currentReconnectAttempt++;
    qDebug() << "MQTT reconnect attempt" << m_currentReconnectAttempt << "of" << m_maxReconnectAttempts;

    disconnectFromHost();
    m_manualDisconnect = false;

    QTimer::singleShot(m_reconnectInterval, this, [this]() {
        setState(MqttConnectionState::RECONNECTING);
        m_socket->connectToHost(m_host, m_port);
    });
}

bool MqttClient::isConnected() const
{
    return m_state == MqttConnectionState::CONNECTED;
}

void MqttClient::subscribe(const QString& topic, quint8 qos)
{
    if (!isConnected()) {
        qWarning() << "Cannot subscribe, not connected:" << topic;
        return;
    }

    m_subscriptions[topic] = qos;
    sendMqttSubscribe(topic, qos);
    qDebug() << "Subscribed to topic:" << topic;
}

void MqttClient::unsubscribe(const QString& topic)
{
    if (m_subscriptions.contains(topic)) {
        m_subscriptions.remove(topic);
        // TODO: 实现UNSUBSCRIBE报文
        qDebug() << "Unsubscribed from topic:" << topic;
    }
}

void MqttClient::unsubscribeAll()
{
    for (const QString& topic : m_subscriptions.keys()) {
        unsubscribe(topic);
    }
}

void MqttClient::publish(const QString& topic, const QByteArray& payload, quint8 qos, bool retain)
{
    if (!isConnected()) {
        qWarning() << "Cannot publish, not connected:" << topic;
        return;
    }

    sendMqttPublish(topic, payload, qos, retain);
    qDebug() << "Published message to:" << topic << "QoS:" << qos;
}

void MqttClient::setMessageCallback(MqttMessageCallback callback)
{
    m_messageCallback = callback;
}

void MqttClient::setConnectedCallback(MqttConnectedCallback callback)
{
    m_connectedCallback = callback;
}

void MqttClient::setDisconnectedCallback(MqttDisconnectedCallback callback)
{
    m_disconnectedCallback = callback;
}

void MqttClient::setErrorCallback(MqttErrorCallback callback)
{
    m_errorCallback = callback;
}

void MqttClient::startKeepAlive(int intervalMs)
{
    m_keepAliveInterval = intervalMs;
    m_keepAliveTimer->start(intervalMs);
    qDebug() << "Keep alive timer started, interval:" << intervalMs << "ms";
}

void MqttClient::stopKeepAlive()
{
    m_keepAliveTimer->stop();
    qDebug() << "Keep alive timer stopped";
}

void MqttClient::setKeepAlive(int seconds)
{
    m_keepAliveInterval = seconds * 1000;
}

void MqttClient::setReconnectInterval(int milliseconds)
{
    m_reconnectInterval = milliseconds;
}

void MqttClient::setConnectTimeout(int milliseconds)
{
    m_connectTimeout = milliseconds;
}

void MqttClient::setCleanSession(bool clean)
{
    m_cleanSession = clean;
}

void MqttClient::setWillMessage(const QString& topic, const QByteArray& message, quint8 qos)
{
    m_willTopic = topic;
    m_willMessage = message;
    m_willQos = qos;
}

void MqttClient::setState(MqttConnectionState state)
{
    if (m_state != state) {
        m_state = state;
        emit connectionStateChanged(state);
    }
}

void MqttClient::scheduleReconnect()
{
    if (!m_manualDisconnect && m_maxReconnectAttempts > 0) {
        m_reconnectTimer->start(m_reconnectInterval);
    }
}

void MqttClient::updateLastMessageTime()
{
    m_lastMessageTime = QDateTime::currentDateTime();
}

void MqttClient::sendMqttConnect()
{
    // 构建CONNECT报文
    QByteArray packet;
    packet.append(static_cast<char>(MqttProtocol::CONNECT));

    QByteArray payload;
    // Protocol name
    payload.append(static_cast<char>(0x00));
    payload.append(static_cast<char>(0x04)); // Length
    payload.append('M'); payload.append('Q'); payload.append('T'); payload.append('T');
    // Protocol level
    payload.append(static_cast<char>(0x04)); // MQTT 3.1.1
    // Connect flags
    quint8 flags = 0x00;
    if (m_cleanSession) flags |= 0x02;
    if (!m_username.isEmpty()) flags |= 0x80;
    if (!m_password.isEmpty()) flags |= 0x40;
    payload.append(static_cast<char>(flags));
    // Keep alive
    payload.append(static_cast<char>((m_keepAliveInterval / 1000) >> 8));
    payload.append(static_cast<char>((m_keepAliveInterval / 1000) & 0xFF));
    // Client ID
    payload.append(encodeString(m_clientId));
    // Username
    if (!m_username.isEmpty()) {
        payload.append(encodeString(m_username));
    }
    // Password
    if (!m_password.isEmpty()) {
        payload.append(encodeString(m_password));
    }

    // Remaining length
    packet.append(static_cast<char>(payload.length()));
    packet.append(payload);

    m_socket->write(packet);
    qDebug() << "MQTT CONNECT sent";
}

void MqttClient::sendMqttSubscribe(const QString& topic, quint8 qos)
{
    QByteArray packet;
    packet.append(static_cast<char>(MqttProtocol::SUBSCRIBE));

    quint16 msgId = calculateMessageId();

    QByteArray payload;
    // Message ID
    payload.append(static_cast<char>(msgId >> 8));
    payload.append(static_cast<char>(msgId & 0xFF));
    // Topic filter
    payload.append(encodeString(topic));
    // QoS
    payload.append(static_cast<char>(qos));

    // Remaining length
    packet.append(static_cast<char>(payload.length()));
    packet.append(payload);

    m_socket->write(packet);
    qDebug() << "MQTT SUBSCRIBE sent for:" << topic;
}

void MqttClient::sendMqttPublish(const QString& topic, const QByteArray& payload, quint8 qos, bool retain)
{
    QByteArray packet;
    quint8 header = MqttProtocol::PUBLISH;
    if (retain) header |= 0x01;
    if (qos > 0) header |= (qos << 1);
    packet.append(static_cast<char>(header));

    QByteArray data;
    // Topic
    data.append(encodeString(topic));
    // Message ID (if QoS > 0)
    if (qos > 0) {
        quint16 msgId = calculateMessageId();
        data.append(static_cast<char>(msgId >> 8));
        data.append(static_cast<char>(msgId & 0xFF));
    }
    // Payload
    data.append(payload);

    // Remaining length (simplified, not handling multi-byte)
    packet.append(static_cast<char>(data.length() & 0xFF));
    packet.append(data);

    m_socket->write(packet);
}

QByteArray MqttClient::encodeString(const QString& str)
{
    QByteArray result;
    QByteArray utf8 = str.toUtf8();
    result.append(static_cast<char>(utf8.length() >> 8));
    result.append(static_cast<char>(utf8.length() & 0xFF));
    result.append(utf8);
    return result;
}

quint16 MqttClient::calculateMessageId()
{
    return m_messageId++;
}

void MqttClient::onConnected()
{
    qDebug() << "MQTT TCP connected";
    sendMqttConnect();
}

void MqttClient::onDisconnected()
{
    qDebug() << "MQTT disconnected";

    m_keepAliveTimer->stop();

    if (!m_manualDisconnect) {
        setState(MqttConnectionState::DISCONNECTED);
        scheduleReconnect();
    }

    emit disconnected();

    if (m_disconnectedCallback) {
        m_disconnectedCallback();
    }
}

void MqttClient::onError(QAbstractSocket::SocketError error)
{
    QString errorString = m_socket->errorString();
    qWarning() << "MQTT socket error:" << error << errorString;

    m_connectTimeoutTimer->stop();

    if (!m_manualDisconnect) {
        setState(MqttConnectionState::ERROR);
        scheduleReconnect();
    }

    emit errorOccurred(errorString);

    if (m_errorCallback) {
        m_errorCallback(errorString);
    }
}

void MqttClient::onReadyRead()
{
    m_readBuffer.append(m_socket->readAll());

    // 简化的MQTT报文解析
    while (m_readBuffer.size() >= 2) {
        quint8 messageType = static_cast<quint8>(m_readBuffer[0]);
        quint8 remainingLength = static_cast<quint8>(m_readBuffer[1]);

        if (m_readBuffer.size() < 2 + remainingLength) {
            break; // 等待更多数据
        }

        QByteArray payload = m_readBuffer.mid(2, remainingLength);
        m_readBuffer = m_readBuffer.mid(2 + remainingLength);

        // 处理不同类型的报文
        quint8 cmd = messageType & 0xF0;
        switch (cmd) {
            case MqttProtocol::CONNACK: {
                qDebug() << "MQTT CONNACK received";
                m_connectTimeoutTimer->stop();
                m_reconnectTimer->stop();
                m_currentReconnectAttempt = 0;
                setState(MqttConnectionState::CONNECTED);
                updateLastMessageTime();
                startKeepAlive();

                // 重新订阅所有主题
                for (auto it = m_subscriptions.begin(); it != m_subscriptions.end(); ++it) {
                    sendMqttSubscribe(it.key(), it.value());
                }

                emit connected();

                if (m_connectedCallback) {
                    m_connectedCallback();
                }
                break;
            }
            case MqttProtocol::PUBLISH: {
                // 简化的PUBLISH解析
                int topicLen = (static_cast<quint8>(payload[0]) << 8) | static_cast<quint8>(payload[1]);
                QString topic = QString::fromUtf8(payload.mid(2, topicLen));
                QByteArray messagePayload = payload.mid(2 + topicLen);

                qDebug() << "MQTT message received, topic:" << topic << "size:" << messagePayload.size();

                emit messageReceived(topic, messagePayload);

                if (m_messageCallback) {
                    m_messageCallback(topic, messagePayload);
                }
                break;
            }
            case MqttProtocol::PINGRESP: {
                qDebug() << "MQTT PINGRESP received";
                updateLastMessageTime();
                break;
            }
            default:
                qDebug() << "MQTT unhandled message type:" << Qt::hex << cmd;
                break;
        }
    }
}

void MqttClient::onKeepAliveTimeout()
{
    if (!isConnected()) {
        m_keepAliveTimer->stop();
        return;
    }

    // 发送PINGREQ
    QByteArray packet;
    packet.append(static_cast<char>(MqttProtocol::PINGREQ));
    packet.append(static_cast<char>(0x00));
    m_socket->write(packet);

    qDebug() << "MQTT PINGREQ sent";
}

void MqttClient::onConnectTimeout()
{
    if (m_state == MqttConnectionState::CONNECTING) {
        QString error = "Connection timeout";
        qWarning() << "MQTT connection timeout:" << error;

        setState(MqttConnectionState::ERROR);
        disconnectFromHost();
        scheduleReconnect();
    }
}

} // namespace Bytedesk
