# ByteDesk Qt Client

> 基于 C++ + Qt + MQTT 实现的跨平台即时通讯客户端

[![Qt](https://img.shields.io/badge/Qt-6.10%2B-41CD52?logo=qt)](https://www.qt.io)
[![C++](https://img.shields.io/badge/C++-17-00599C?logo=c%2B%2B)](https://en.cppreference.com/w/C++17)
[![License](https://img.shields.io/badge/License-BSL--1.1-blue)](LICENSE)

---

## ✨ 特性

- ✅ **用户认证** - 登录/登出，Token自动刷新
- ✅ **实时通信** - MQTT长连接，消息即时送达
- ✅ **会话管理** - 加载、切换、刷新会话列表
- ✅ **消息收发** - 文本消息实时收发
- ✅ **跨平台** - 支持 Windows、macOS、Linux
- ✅ **轻量级** - 纯Qt实现，无第三方MQTT库依赖

---

## 🚀 快速开始

### 编译运行

```bash
# 方法1: Qt Creator（推荐）
1. 打开 bytedesk.pro
2. 点击运行按钮 (或按 Ctrl+R)

# 方法2: 命令行
qmake bytedesk.pro && make
./qt

# 方法3: 编译脚本
./build.sh
```

### 系统要求

- **Qt 6.10+**
- **C++17** 编译器
- **macOS**: 10.15+ (Catalina)
- **Windows**: 10+
- **Linux**: GCC 7+ 或 Clang 6+

---

## 📱 使用指南

### 1. 启动应用

运行程序后，您将看到主窗口：

```
┌──────────────────────────────────────────┐
│ 微语 - ByteDesk Qt Client               │
├──────────────────────────────────────────┤
│ 菜单: [登录] [登出] [刷新会话] [退出]    │
├──────────────┬───────────────────────────┤
│ 会话列表      │ 聊天窗口                  │
│              │                            │
│ □ 会话1     │ [10:30] 张三: 你好！      │
│ □ 会话2     │ [10:31] 我: 你好啊        │
│ □ 会话3     │                            │
│              │ [输入消息.......] [发送]  │
├──────────────┴───────────────────────────┤
│ 状态: 欢迎使用微语Qt客户端 - 请登录      │
└──────────────────────────────────────────┘
```

### 2. 登录

1. 点击菜单 **"菜单" → "登录"**
2. 输入用户名和密码
3. 点击确定

### 3. 发送消息

1. 点击左侧会话列表选择会话
2. 在输入框输入文字
3. 点击"发送"或按回车键

### 4. 接收消息

- 新消息会实时显示在聊天窗口
- 自己的消息显示为蓝色
- 他人的消息显示为绿色

---

## 🏗️ 项目结构

```
qt/
├── bytedesk.pro              # qmake项目文件
├── build.sh                  # 编译脚本
├── verify.sh                 # 验证脚本
└── src/
    ├── main.cpp              # 程序入口
    │
    ├── models/               # 数据模型 (8个文件)
    │   ├── message.cpp/h     # 消息模型
    │   ├── thread.cpp/h      # 会话模型
    │   ├── user.cpp/h        # 用户模型
    │   └── config.cpp/h      # 配置模型
    │
    ├── core/                 # 核心功能 (14个文件)
    │   ├── auth/            # 认证管理
    │   │   └── authmanager.cpp/h
    │   ├── mqtt/            # MQTT通信（TCP实现）
    │   │   ├── mqttclient.cpp/h
    │   │   └── mqttmessagehandler.cpp/h
    │   └── network/         # HTTP API
    │       ├── httpclient.cpp/h
    │       ├── apibase.cpp/h
    │       ├── authapi.cpp/h
    │       ├── messageapi.cpp/h
    │       └── threadapi.cpp/h
    │
    └── ui/                   # 用户界面 (3个文件)
        ├── mainwindow.cpp/h  # 主窗口
        └── mainwindow.ui     # UI设计
```

---

## 💡 代码示例

### 初始化组件

```cpp
#include "models/config.h"
#include "core/network/httpclient.h"
#include "core/network/authapi.h"
#include "core/mqtt/mqttclient.h"
#include "core/auth/authmanager.h"

// 配置服务器
BYTDESK_CONFIG->setApiUrl("https://api.bytedesk.com");

// 创建HTTP客户端
HttpClient* httpClient = new HttpClient(this);
httpClient->setBaseUrl(BYTDESK_CONFIG->getApiUrl());

// 创建认证管理器
AuthApi* authApi = new AuthApi(httpClient, this);
MqttClient* mqttClient = new MqttClient(this);
AuthManager* authManager = new AuthManager(authApi, mqttClient, this);
```

### 登录

```cpp
// 连接信号
connect(authManager, &AuthManager::loginSuccess, [](const UserPtr& user) {
    qDebug() << "登录成功:" << user->getUsername();
});

// 执行登录
authManager->login("username", "password");
```

### 发送消息

```cpp
#include "core/mqtt/mqttmessagehandler.h"

MqttMessageHandler* mqttHandler = new MqttMessageHandler(mqttClient, this);
mqttHandler->init();

// 发送文本消息
mqttHandler->sendTextMessage(thread, "Hello, World!", currentUser);

// 接收消息
connect(mqttHandler, &MqttMessageHandler::messageReceived,
        [](const MessagePtr& msg) {
    qDebug() << "收到消息:" << msg->getContentString();
});
```

---

## 🛠️ 技术栈

### 核心技术

- **Qt 6.10+** - 跨平台UI框架
- **C++17** - 现代C++标准
- **MQTT 3.1.1** - 实时消息协议（TCP实现）
- **HTTP/HTTPS** - REST API调用

### Qt模块

- `QtCore` - 核心功能
- `QtGui` - GUI基础
- `QtWidgets` - UI组件
- `QtNetwork` - 网络通信
- `QtSql` - 数据库（待集成）

---

## 🔧 配置说明

### 修改服务器地址

编辑 `src/models/config.cpp`：

```cpp
const QString Config::DEFAULT_API_URL = "https://your-server.com";
const QString Config::DEFAULT_MQTT_HOST = "mqtt.your-server.com";
const int Config::DEFAULT_MQTT_PORT = 1883;
```

### 配置文件位置

应用配置会保存到：

- **macOS**: `~/Library/Preferences/com.bytedesk.qt.plist`
- **Linux**: `~/.config/Bytedesk/bytedesk-qt.conf`
- **Windows**: `%APPDATA%/Bytedesk/bytedesk-qt.conf`

---

## 🎯 功能状态

### 已实现 ✅

- [x] 用户登录/登出
- [x] Token自动刷新
- [x] 会话列表管理
- [x] 文本消息收发
- [x] MQTT实时通信
- [x] HTTP API封装
- [x] 配置持久化
- [x] 完整UI界面

### 待开发 ⚠️

- [ ] 文件上传/下载
- [ ] 图片消息预览
- [ ] 历史消息加载
- [ ] 消息撤回
- [ ] 未读消息计数
- [ ] SQLite数据库
- [ ] 表情支持
- [ ] 搜索功能

---

## 🐛 常见问题

### 1. 编译错误：找不到Qt模块

**解决方案**：
- Qt Creator: 检查Kit配置
- 命令行: 设置正确的Qt路径
```bash
export PATH=$PATH:~/Qt/6.10.1/macos/bin
```

### 2. MQTT连接失败

**解决方案**：
- 检查服务器地址和端口
- 确认网络连接
- 验证Token是否有效
- 查看控制台日志

### 3. 登录失败

**解决方案**：
- 确认服务器地址正确
- 检查用户名密码
- 查看状态栏错误提示

### 4. 消息发送失败

**解决方案**：
- 确保已登录
- 确保已选择会话
- 检查MQTT连接状态

---

## 📊 项目完成度

| 模块 | 完成度 | 文件数 |
|------|--------|--------|
| 数据模型 | ✅ 100% | 8 |
| 核心功能 | ✅ 100% | 14 |
| UI界面 | ✅ 80% | 3 |
| 文档 | ✅ 100% | - |
| **总计** | **✅ 95%** | **25** |

---

## 🔗 相关资源

- **Desktop版本**: [ByteDesk Desktop](../../frontend/apps/desktop) - Electron + React 版本
- **后端API**: [ByteDesk Backend](../../starter) - Java Spring Boot
- **官网**: https://www.bytedesk.com
- **文档**: https://docs.bytedesk.com

---

## 📄 许可证

Business Source License 1.1

可免费用于内部使用和开发，禁止：
- 转售或SaaS托管
- 为非法业务部署
- 未经许可的商业分发

---

## 🤝 贡献

欢迎提交 Issue 和 Pull Request！

---

<div align="center">

**Made with ❤️ by ByteDesk**

[官网](https://www.bytedesk.com) • [文档](https://docs.bytedesk.com) • [GitHub](https://github.com/bytedesk)

</div>
