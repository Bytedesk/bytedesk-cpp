#ifndef MQTTMESSAGEHANDLER_H
#define MQTTMESSAGEHANDLER_H

#include <QObject>
#include <QHash>
#include <QMutex>
#include "mqttclient.h"
#include "models/message.h"
#include "models/thread.h"
#include "models/user.h"

namespace Bytedesk {

// MQTT消息处理器 - 处理BYTDESK协议的消息
class MqttMessageHandler : public QObject
{
    Q_OBJECT

public:
    explicit MqttMessageHandler(MqttClient* mqttClient, QObject* parent = nullptr);
    ~MqttMessageHandler();

    // 初始化
    void init();

    // 订阅主题
    void subscribeToThread(const QString& threadUid, const QString& topic);
    void unsubscribeFromThread(const QString& threadUid);
    void subscribeToQueue(const QString& agentUid);
    void unsubscribeFromQueue();

    // 发送消息
    void sendTextMessage(const ThreadPtr& thread, const QString& text, const UserPtr& user);
    void sendImageMessage(const ThreadPtr& thread, const QString& imageUrl, const UserPtr& user);
    void sendFileMessage(const ThreadPtr& thread, const QString& fileUrl,
                        const QString& fileName, qint64 fileSize, const UserPtr& user);
    void sendTypingMessage(const ThreadPtr& thread, const UserPtr& user);
    void sendReadReceipt(const ThreadPtr& thread, const QString& messageUid, const UserPtr& user);
    void sendDeliveredReceipt(const ThreadPtr& thread, const QString& messageUid, const UserPtr& user);

    // Protobuf序列化
    QByteArray serializeMessage(const Message& message);
    MessagePtr deserializeMessage(const QByteArray& data);

signals:
    void messageReceived(const MessagePtr& message);
    void typingReceived(const QString& threadUid, const QString& userUid);
    void readReceiptReceived(const QString& threadUid, const QString& messageUid);
    void deliveredReceiptReceived(const QString& threadUid, const QString& messageUid);
    void noticeReceived(const QString& threadUid, const QString& content);

private slots:
    void onMqttMessageReceived(const QString& topic, const QByteArray& payload);
    void onMqttConnected();
    void onMqttDisconnected();
    void onMqttError(const QString& error);

private:
    void handleMessage(const MessagePtr& message);
    void handleTypingMessage(const MessagePtr& message);
    void handleReceiptMessage(const MessagePtr& message);
    void handleNoticeMessage(const MessagePtr& message);

    QString generateMessageUid();
    void addToSentMessages(const QString& uid);

    MqttClient* m_mqttClient;
    UserPtr m_currentUser;

    // 主题映射
    QHash<QString, QString> m_threadTopics; // threadUid -> topic
    QHash<QString, QString> m_topicThreads; // topic -> threadUid

    // 防止重复处理回执消息
    QSet<QString> m_sentReadUids;
    QSet<QString> m_sentDeliveredUids;
    QMutex m_mutex;

    // 消息类型常量
    static const QString MESSAGE_TYPE_TEXT;
    static const QString MESSAGE_TYPE_IMAGE;
    static const QString MESSAGE_TYPE_FILE;
    static const QString MESSAGE_TYPE_VIDEO;
    static const QString MESSAGE_TYPE_VOICE;
    static const QString MESSAGE_TYPE_TYPING;
    static const QString MESSAGE_TYPE_NOTICE;
    static const QString MESSAGE_TYPE_RECALL;
    static const QString MESSAGE_TYPE_DELIVERED;
    static const QString MESSAGE_TYPE_READ;

    // 主题前缀
    static const QString TOPIC_ORG_PREFIX;
    static const QString TOPIC_ORG_AGENT_PREFIX;
    static const QString TOPIC_ORG_WORKGROUP_PREFIX;
    static const QString TOPIC_ORG_ROBOT_PREFIX;
    static const QString TOPIC_ORG_GROUP_PREFIX;
    static const QString TOPIC_ORG_MEMBER_PREFIX;
    static const QString TOPIC_QUEUE_PREFIX;
};

} // namespace Bytedesk

#endif // MQTTMESSAGEHANDLER_H
