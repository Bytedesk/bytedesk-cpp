#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QPointer>
#include <QSharedPointer>

// 包含模型类头文件
#include "models/message.h"
#include "models/thread.h"
#include "models/user.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// 前置声明
namespace Bytedesk {
    class HttpClient;
    class AuthApi;
    class MessageApi;
    class ThreadApi;
    class MqttClient;
    class MqttMessageHandler;
    class AuthManager;
}

using namespace Bytedesk;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // 菜单动作
    void on_actionLogin_triggered();
    void on_actionLogout_triggered();
    void on_actionExit_triggered();
    void on_actionRefreshThreads_triggered();
    void on_actionAbout_triggered();

    // UI动作
    void onSendButtonClicked();
    void onThreadItemClicked(QListWidgetItem* item);
    void onMessageLineEditReturnPressed();

    // 业务逻辑回调
    void onLoginSuccess(const UserPtr& user);
    void onLoginFailed(const QString& error);
    void onThreadsLoaded(const QList<ThreadPtr>& threads);
    void onMessageReceived(const MessagePtr& message);
    void onMqttConnected();
    void onMqttDisconnected();

private:
    void setupConnections();
    void updateUIForLoginState(bool loggedIn);
    void appendMessageToChat(const MessagePtr& message);
    void loadThreads();
    void showLoginDialog();
    void updateStatusBar(const QString& message);

    Ui::MainWindow *ui;

    // 核心组件
    HttpClient* m_httpClient;
    AuthApi* m_authApi;
    MessageApi* m_messageApi;
    ThreadApi* m_threadApi;
    MqttClient* m_mqttClient;
    MqttMessageHandler* m_mqttHandler;
    AuthManager* m_authManager;

    // 数据
    QList<ThreadPtr> m_threads;
    ThreadPtr m_currentThread;
    UserPtr m_currentUser;

    // 标志
    bool m_isLoggedIn;
};

#endif // MAINWINDOW_H
