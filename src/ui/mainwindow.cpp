#include "mainwindow.h"
#include "ui_mainwindow.h"

// 包含核心模块
#include "models/config.h"
#include "models/message.h"
#include "models/thread.h"
#include "models/user.h"
#include "core/network/httpclient.h"
#include "core/network/authapi.h"
#include "core/network/messageapi.h"
#include "core/network/threadapi.h"
#include "core/mqtt/mqttclient.h"
#include "core/mqtt/mqttmessagehandler.h"
#include "core/auth/authmanager.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_httpClient(nullptr)
    , m_authApi(nullptr)
    , m_messageApi(nullptr)
    , m_threadApi(nullptr)
    , m_mqttClient(nullptr)
    , m_mqttHandler(nullptr)
    , m_authManager(nullptr)
    , m_isLoggedIn(false)
{
    ui->setupUi(this);

    // 初始化核心组件
    m_httpClient = new HttpClient(this);
    m_httpClient->setBaseUrl(BYTDESK_CONFIG->getApiUrl());

    m_authApi = new AuthApi(m_httpClient, this);
    m_messageApi = new MessageApi(m_httpClient, this);
    m_threadApi = new ThreadApi(m_httpClient, this);

    m_mqttClient = new MqttClient(this);
    m_mqttHandler = new MqttMessageHandler(m_mqttClient, this);
    m_mqttHandler->init();

    m_authManager = new AuthManager(m_authApi, m_mqttClient, this);

    setupConnections();

    updateStatusBar("欢迎使用微语Qt客户端 - 请登录");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupConnections()
{
    // 菜单动作 - Qt 6的triggered信号带bool参数，使用lambda忽略
    connect(ui->actionLogin, &QAction::triggered, this, [this](bool) { on_actionLogin_triggered(); });
    connect(ui->actionLogout, &QAction::triggered, this, [this](bool) { on_actionLogout_triggered(); });
    connect(ui->actionExit, &QAction::triggered, this, [this](bool) { on_actionExit_triggered(); });
    connect(ui->actionRefreshThreads, &QAction::triggered, this, [this](bool) { on_actionRefreshThreads_triggered(); });
    connect(ui->actionAbout, &QAction::triggered, this, [this](bool) { on_actionAbout_triggered(); });

    // UI动作
    connect(ui->sendButton, &QPushButton::clicked, this, &MainWindow::onSendButtonClicked);
    connect(ui->threadListWidget, &QListWidget::itemClicked, this, &MainWindow::onThreadItemClicked);
    connect(ui->messageLineEdit, &QLineEdit::returnPressed, this, &MainWindow::onMessageLineEditReturnPressed);

    // 认证管理器信号
    connect(m_authManager, &AuthManager::loginSuccess, this, &MainWindow::onLoginSuccess);
    connect(m_authManager, &AuthManager::loginFailed, this, &MainWindow::onLoginFailed);
    connect(m_authManager, &AuthManager::logoutSuccess, [this]() {
        m_isLoggedIn = false;
        m_currentUser.reset();
        m_currentThread.reset();
        m_threads.clear();
        updateUIForLoginState(false);
        updateStatusBar("已登出");
    });

    // MQTT信号
    connect(m_mqttClient, &MqttClient::connected, this, &MainWindow::onMqttConnected);
    connect(m_mqttClient, &MqttClient::disconnected, this, &MainWindow::onMqttDisconnected);

    // 消息信号
    connect(m_mqttHandler, &MqttMessageHandler::messageReceived, this, &MainWindow::onMessageReceived);
}

void MainWindow::updateUIForLoginState(bool loggedIn)
{
    ui->actionLogin->setEnabled(!loggedIn);
    ui->actionLogout->setEnabled(loggedIn);
    ui->actionRefreshThreads->setEnabled(loggedIn);
    ui->sendButton->setEnabled(loggedIn && m_currentThread);
    ui->messageLineEdit->setEnabled(loggedIn && m_currentThread);

    if (!loggedIn) {
        ui->threadListWidget->clear();
        ui->chatTextBrowser->clear();
        ui->chatTitleLabel->setText("聊天窗口 - 请先登录");
    }
}

void MainWindow::appendMessageToChat(const MessagePtr& message)
{
    if (!message) return;

    QString sender = message->getUserName();
    if (sender.isEmpty()) {
        sender = message->getUserUid();
    }

    QString content = message->getContentString();
    QString time = message->getCreatedAt().toString("hh:mm:ss");

    QString html;
    if (message->isSelf(m_currentUser ? m_currentUser->getUid() : "")) {
        html = QString("<p><b style='color: blue;'>[%1] 我:</b> %2</p>").arg(time, content);
    } else {
        html = QString("<p><b style='color: green;'>[%1] %2:</b> %3</p>")
            .arg(time, sender, content);
    }

    ui->chatTextBrowser->append(html);
}

void MainWindow::loadThreads()
{
    if (!m_isLoggedIn) return;

    updateStatusBar("正在加载会话列表...");

    m_threadApi->getThreads(
        [this](const QList<ThreadPtr>& threads) {
            m_threads = threads;
            ui->threadListWidget->clear();

            for (const auto& thread : threads) {
                QString title = thread->getTitle();
                if (title.isEmpty()) {
                    title = thread->getUid();
                }

                QListWidgetItem* item = new QListWidgetItem(title);
                item->setData(Qt::UserRole, thread->getUid());
                ui->threadListWidget->addItem(item);
            }

            updateStatusBar(QString("加载了 %1 个会话").arg(threads.size()));
        },
        [this](const QString& error) {
            updateStatusBar("加载会话失败: " + error);
            QMessageBox::warning(this, "错误", "加载会话失败: " + error);
        }
    );
}

void MainWindow::showLoginDialog()
{
    bool ok;
    QString username = QInputDialog::getText(this, "登录",
        "用户名:", QLineEdit::Normal, "", &ok);

    if (!ok || username.isEmpty()) return;

    QString password = QInputDialog::getText(this, "登录",
        "密码:", QLineEdit::Password, "", &ok);

    if (!ok || password.isEmpty()) return;

    updateStatusBar("正在登录...");

    m_authManager->login(username, password);
}

void MainWindow::updateStatusBar(const QString& message)
{
    ui->statusbar->showMessage(message);
}

// 菜单动作实现
void MainWindow::on_actionLogin_triggered()
{
    showLoginDialog();
}

void MainWindow::on_actionLogout_triggered()
{
    int ret = QMessageBox::question(this, "确认登出",
        "确定要登出吗？");

    if (ret == QMessageBox::Yes) {
        m_authManager->logout();
    }
}

void MainWindow::on_actionExit_triggered()
{
    QMainWindow::close();
}

void MainWindow::on_actionRefreshThreads_triggered()
{
    loadThreads();
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, "关于微语Qt客户端",
        "<h3>微语 Qt Client v1.0</h3>"
        "<p>基于C++ + Qt + MQTT实现的跨平台即时通讯客户端</p>"
        "<p>核心功能：</p>"
        "<ul>"
        "<li>实时消息收发</li>"
        "<li>会话管理</li>"
        "<li>用户认证</li>"
        "<li>MQTT实时通信</li>"
        "</ul>"
        "<p><b>技术栈：</b>Qt 6, C++17, MQTT 3.1.1</p>"
        "<p><b>状态：</b>核心框架已完成，UI基础功能已实现</p>"
        );
}

// UI动作实现
void MainWindow::onSendButtonClicked()
{
    if (!m_isLoggedIn || !m_currentThread) {
        QMessageBox::warning(this, "提示", "请先登录并选择会话");
        return;
    }

    QString text = ui->messageLineEdit->text();
    if (text.isEmpty()) return;

    // 发送消息
    m_mqttHandler->sendTextMessage(m_currentThread, text, m_currentUser);

    ui->messageLineEdit->clear();

    // 在聊天窗口显示发送的消息
    MessagePtr msg = QSharedPointer<Message>::create();
    msg->setContent(text);
    msg->setUserUid(m_currentUser->getUid());
    msg->setUserName(m_currentUser->getNickname());
    msg->setCreatedAt(QDateTime::currentDateTime());
    appendMessageToChat(msg);
}

void MainWindow::onThreadItemClicked(QListWidgetItem* item)
{
    if (!item) return;

    QString threadUid = item->data(Qt::UserRole).toString();

    // 查找会话
    for (const auto& thread : m_threads) {
        if (thread->getUid() == threadUid) {
            m_currentThread = thread;

            QString title = thread->getTitle();
            if (title.isEmpty()) {
                title = thread->getUid();
            }

            ui->chatTitleLabel->setText("聊天 - " + title);
            ui->chatTextBrowser->clear();

            // 加载历史消息
            updateStatusBar("正在加载历史消息...");

            // TODO: 加载历史消息
            updateStatusBar("已切换到会话: " + title);

            ui->sendButton->setEnabled(true);
            ui->messageLineEdit->setEnabled(true);
            ui->messageLineEdit->setFocus();

            break;
        }
    }
}

void MainWindow::onMessageLineEditReturnPressed()
{
    onSendButtonClicked();
}

// 业务逻辑回调
void MainWindow::onLoginSuccess(const UserPtr& user)
{
    m_isLoggedIn = true;
    m_currentUser = user;

    updateUIForLoginState(true);

    QString username = user->getNickname();
    if (username.isEmpty()) {
        username = user->getUsername();
    }

    updateStatusBar("登录成功: " + username);

    // 加载会话列表
    loadThreads();

    QMessageBox::information(this, "登录成功",
        QString("欢迎, %1!\n\n已连接到服务器").arg(username));
}

void MainWindow::onLoginFailed(const QString& error)
{
    m_isLoggedIn = false;
    updateStatusBar("登录失败: " + error);
    QMessageBox::warning(this, "登录失败", error);
}

void MainWindow::onThreadsLoaded(const QList<ThreadPtr>& threads)
{
    m_threads = threads;
    ui->threadListWidget->clear();

    for (const auto& thread : threads) {
        QString title = thread->getTitle();
        if (title.isEmpty()) {
            title = thread->getUid();
        }

        QListWidgetItem* item = new QListWidgetItem(title);
        item->setData(Qt::UserRole, thread->getUid());
        ui->threadListWidget->addItem(item);
    }

    updateStatusBar(QString("已加载 %1 个会话").arg(threads.size()));
}

void MainWindow::onMessageReceived(const MessagePtr& message)
{
    // 如果消息来自当前会话，显示在聊天窗口
    if (m_currentThread && message->getThreadUid() == m_currentThread->getUid()) {
        appendMessageToChat(message);
    }

    // 更新会话列表的最后消息
    updateStatusBar("收到新消息");
}

void MainWindow::onMqttConnected()
{
    updateStatusBar("MQTT已连接");
}

void MainWindow::onMqttDisconnected()
{
    updateStatusBar("MQTT已断开");
}
