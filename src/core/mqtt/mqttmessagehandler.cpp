#include "mqttmessagehandler.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QUuid>
#include <QDebug>

namespace Bytedesk {

// 静态常量定义
const QString MqttMessageHandler::MESSAGE_TYPE_TEXT = "TEXT";
const QString MqttMessageHandler::MESSAGE_TYPE_IMAGE = "IMAGE";
const QString MqttMessageHandler::MESSAGE_TYPE_FILE = "FILE";
const QString MqttMessageHandler::MESSAGE_TYPE_VIDEO = "VIDEO";
const QString MqttMessageHandler::MESSAGE_TYPE_VOICE = "VOICE";
const QString MqttMessageHandler::MESSAGE_TYPE_TYPING = "TYPING";
const QString MqttMessageHandler::MESSAGE_TYPE_NOTICE = "NOTICE";
const QString MqttMessageHandler::MESSAGE_TYPE_RECALL = "RECALL";
const QString MqttMessageHandler::MESSAGE_TYPE_DELIVERED = "DELIVERED";
const QString MqttMessageHandler::MESSAGE_TYPE_READ = "READ";

const QString MqttMessageHandler::TOPIC_ORG_PREFIX = "org/";
const QString MqttMessageHandler::TOPIC_ORG_AGENT_PREFIX = "org/agent/";
const QString MqttMessageHandler::TOPIC_ORG_WORKGROUP_PREFIX = "org/workgroup/";
const QString MqttMessageHandler::TOPIC_ORG_ROBOT_PREFIX = "org/robot/";
const QString MqttMessageHandler::TOPIC_ORG_GROUP_PREFIX = "org/group/";
const QString MqttMessageHandler::TOPIC_ORG_MEMBER_PREFIX = "org/member/";
const QString MqttMessageHandler::TOPIC_QUEUE_PREFIX = "org/queue/";

MqttMessageHandler::MqttMessageHandler(MqttClient* mqttClient, QObject* parent)
    : QObject(parent)
    , m_mqttClient(mqttClient)
{
    Q_ASSERT(mqttClient);

    // 连接MQTT客户端信号
    connect(mqttClient, &MqttClient::messageReceived,
            this, &MqttMessageHandler::onMqttMessageReceived);
    connect(mqttClient, &MqttClient::connected,
            this, &MqttMessageHandler::onMqttConnected);
    connect(mqttClient, &MqttClient::disconnected,
            this, &MqttMessageHandler::onMqttDisconnected);
    connect(mqttClient, &MqttClient::errorOccurred,
            this, &MqttMessageHandler::onMqttError);
}

MqttMessageHandler::~MqttMessageHandler()
{
}

void MqttMessageHandler::init()
{
    // 初始化消息处理器
    qDebug() << "MqttMessageHandler initialized";
}

void MqttMessageHandler::subscribeToThread(const QString& threadUid, const QString& topic)
{
    QMutexLocker locker(&m_mutex);

    m_threadTopics[threadUid] = topic;
    m_topicThreads[topic] = threadUid;

    if (m_mqttClient->isConnected()) {
        m_mqttClient->subscribe(topic, 0);
        qDebug() << "Subscribed to thread:" << threadUid << "topic:" << topic;
    }
}

void MqttMessageHandler::unsubscribeFromThread(const QString& threadUid)
{
    QMutexLocker locker(&m_mutex);

    if (m_threadTopics.contains(threadUid)) {
        QString topic = m_threadTopics.take(threadUid);
        m_topicThreads.remove(topic);

        if (m_mqttClient->isConnected()) {
            m_mqttClient->unsubscribe(topic);
            qDebug() << "Unsubscribed from thread:" << threadUid;
        }
    }
}

void MqttMessageHandler::subscribeToQueue(const QString& agentUid)
{
    QString topic = TOPIC_QUEUE_PREFIX + agentUid;

    if (m_mqttClient->isConnected()) {
        m_mqttClient->subscribe(topic, 0);
        qDebug() << "Subscribed to queue:" << topic;
    }
}

void MqttMessageHandler::unsubscribeFromQueue()
{
    // TODO: 实现取消订阅队列
}

void MqttMessageHandler::sendTextMessage(const ThreadPtr& thread, const QString& text, const UserPtr& user)
{
    if (!thread || !user) {
        qWarning() << "Invalid thread or user for text message";
        return;
    }

    MessagePtr message = QSharedPointer<Message>::create();
    message->setUid(generateMessageUid());
    message->setType(MessageType::TEXT);
    message->setContent(text);
    message->setThreadUid(thread->getUid());
    message->setUserUid(user->getUid());
    message->setUserName(user->getNickname());
    message->setUserAvatar(user->getAvatar());
    message->setStatus(MessageStatus::SENDING);

    QByteArray data = serializeMessage(*message);

    if (m_mqttClient->isConnected() && !thread->getTopic().isEmpty()) {
        m_mqttClient->publish(thread->getTopic(), data, 0, false);
        message->setStatus(MessageStatus::SENT);
        qDebug() << "Sent text message:" << message->getUid();
    } else {
        qWarning() << "MQTT not connected or topic is empty";
        message->setStatus(MessageStatus::FAILED);
    }

    emit messageReceived(message);
}

void MqttMessageHandler::sendImageMessage(const ThreadPtr& thread, const QString& imageUrl, const UserPtr& user)
{
    if (!thread || !user) {
        return;
    }

    MessagePtr message = QSharedPointer<Message>::create();
    message->setUid(generateMessageUid());
    message->setType(MessageType::IMAGE);

    MessageContent content;
    content.imageUrl = imageUrl;
    message->setContent(content);

    message->setThreadUid(thread->getUid());
    message->setUserUid(user->getUid());
    message->setUserName(user->getNickname());
    message->setUserAvatar(user->getAvatar());
    message->setStatus(MessageStatus::SENDING);

    QByteArray data = serializeMessage(*message);

    if (m_mqttClient->isConnected() && !thread->getTopic().isEmpty()) {
        m_mqttClient->publish(thread->getTopic(), data, 0, false);
        message->setStatus(MessageStatus::SENT);
        qDebug() << "Sent image message:" << message->getUid();
    }

    emit messageReceived(message);
}

void MqttMessageHandler::sendFileMessage(const ThreadPtr& thread, const QString& fileUrl,
                                        const QString& fileName, qint64 fileSize, const UserPtr& user)
{
    if (!thread || !user) {
        return;
    }

    MessagePtr message = QSharedPointer<Message>::create();
    message->setUid(generateMessageUid());
    message->setType(MessageType::FILE);

    MessageContent content;
    content.fileUrl = fileUrl;
    content.fileName = fileName;
    content.fileSize = fileSize;
    message->setContent(content);

    message->setThreadUid(thread->getUid());
    message->setUserUid(user->getUid());
    message->setUserName(user->getNickname());
    message->setUserAvatar(user->getAvatar());
    message->setStatus(MessageStatus::SENDING);

    QByteArray data = serializeMessage(*message);

    if (m_mqttClient->isConnected() && !thread->getTopic().isEmpty()) {
        m_mqttClient->publish(thread->getTopic(), data, 0, false);
        message->setStatus(MessageStatus::SENT);
    }

    emit messageReceived(message);
}

void MqttMessageHandler::sendTypingMessage(const ThreadPtr& thread, const UserPtr& user)
{
    if (!thread || !user) {
        return;
    }

    MessagePtr message = QSharedPointer<Message>::create();
    message->setUid(generateMessageUid());
    message->setType(MessageType::TYPING);
    message->setThreadUid(thread->getUid());
    message->setUserUid(user->getUid());

    QByteArray data = serializeMessage(*message);

    if (m_mqttClient->isConnected() && !thread->getTopic().isEmpty()) {
        m_mqttClient->publish(thread->getTopic(), data, 0, false);
    }
}

void MqttMessageHandler::sendReadReceipt(const ThreadPtr& thread, const QString& messageUid, const UserPtr& user)
{
    if (!thread || !user) {
        return;
    }

    // 防止重复发送
    if (m_sentReadUids.contains(messageUid)) {
        return;
    }

    addToSentMessages(messageUid);

    MessagePtr message = QSharedPointer<Message>::create();
    message->setUid(generateMessageUid());
    message->setType(MessageType::READ);
    message->setContent(messageUid); // 读取的消息UID
    message->setThreadUid(thread->getUid());
    message->setUserUid(user->getUid());

    QByteArray data = serializeMessage(*message);

    if (m_mqttClient->isConnected() && !thread->getTopic().isEmpty()) {
        m_mqttClient->publish(thread->getTopic(), data, 0, false);
        qDebug() << "Sent read receipt for:" << messageUid;
    }
}

void MqttMessageHandler::sendDeliveredReceipt(const ThreadPtr& thread, const QString& messageUid, const UserPtr& user)
{
    if (!thread || !user) {
        return;
    }

    if (m_sentDeliveredUids.contains(messageUid)) {
        return;
    }

    addToSentMessages(messageUid);

    MessagePtr message = QSharedPointer<Message>::create();
    message->setUid(generateMessageUid());
    message->setType(MessageType::DELIVERED);
    message->setContent(messageUid);
    message->setThreadUid(thread->getUid());
    message->setUserUid(user->getUid());

    QByteArray data = serializeMessage(*message);

    if (m_mqttClient->isConnected() && !thread->getTopic().isEmpty()) {
        m_mqttClient->publish(thread->getTopic(), data, 0, false);
    }
}

QByteArray MqttMessageHandler::serializeMessage(const Message& message)
{
    // 如果使用Protobuf，这里调用Protobuf序列化
    // 目前使用JSON作为示例
    QJsonObject json = message.toJson();
    return QJsonDocument(json).toJson(QJsonDocument::Compact);
}

MessagePtr MqttMessageHandler::deserializeMessage(const QByteArray& data)
{
    // 如果使用Protobuf，这里调用Protobuf反序列化
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);

    if (error.error != QJsonParseError::NoError) {
        qWarning() << "Failed to parse message:" << error.errorString();
        return QSharedPointer<Message>::create();
    }

    return QSharedPointer<Message>::create(
        Message::fromJson(doc.object())
    );
}

void MqttMessageHandler::onMqttMessageReceived(const QString& topic, const QByteArray& payload)
{
    qDebug() << "MQTT message received, topic:" << topic << "size:" << payload.size();

    MessagePtr message = deserializeMessage(payload);
    if (message->isNull()) {
        qWarning() << "Failed to deserialize message";
        return;
    }

    // 根据消息类型处理
    switch (message->getType()) {
        case MessageType::TYPING:
            handleTypingMessage(message);
            break;
        case MessageType::READ:
        case MessageType::DELIVERED:
            handleReceiptMessage(message);
            break;
        case MessageType::NOTICE:
            handleNoticeMessage(message);
            break;
        default:
            handleMessage(message);
            break;
    }
}

void MqttMessageHandler::onMqttConnected()
{
    qDebug() << "MQTT connected, resubscribing to threads";

    // 重新订阅所有主题
    QMutexLocker locker(&m_mutex);
    for (const QString& topic : m_threadTopics) {
        m_mqttClient->subscribe(topic, 0);
        qDebug() << "Resubscribed to:" << topic;
    }
}

void MqttMessageHandler::onMqttDisconnected()
{
    qDebug() << "MQTT disconnected";
}

void MqttMessageHandler::onMqttError(const QString& error)
{
    qWarning() << "MQTT error:" << error;
}

void MqttMessageHandler::handleMessage(const MessagePtr& message)
{
    qDebug() << "Handling message:" << message->getUid() << "type:" << message->getTypeString();
    emit messageReceived(message);
}

void MqttMessageHandler::handleTypingMessage(const MessagePtr& message)
{
    QString threadUid = message->getThreadUid();
    QString userUid = message->getUserUid();
    emit typingReceived(threadUid, userUid);
}

void MqttMessageHandler::handleReceiptMessage(const MessagePtr& message)
{
    QString threadUid = message->getThreadUid();
    QString messageUid = message->getContentString();

    if (message->getType() == MessageType::READ) {
        emit readReceiptReceived(threadUid, messageUid);
    } else if (message->getType() == MessageType::DELIVERED) {
        emit deliveredReceiptReceived(threadUid, messageUid);
    }
}

void MqttMessageHandler::handleNoticeMessage(const MessagePtr& message)
{
    QString threadUid = message->getThreadUid();
    QString content = message->getContentString();
    emit noticeReceived(threadUid, content);
}

QString MqttMessageHandler::generateMessageUid()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

void MqttMessageHandler::addToSentMessages(const QString& uid)
{
    QMutexLocker locker(&m_mutex);
    // 限制集合大小，避免内存无限增长
    if (m_sentReadUids.size() > 1000) {
        m_sentReadUids.clear();
    }
    if (m_sentDeliveredUids.size() > 1000) {
        m_sentDeliveredUids.clear();
    }
    m_sentReadUids.insert(uid);
    m_sentDeliveredUids.insert(uid);
}

} // namespace Bytedesk
