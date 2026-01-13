QT       += core gui network sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# WebSockets支持 - 可选模块
# 如果你的Qt版本支持websockets，取消下面一行的注释
# QT += websockets

# QML/Quick支持 - 可选模块
# QT += qml quick

CONFIG += c++17
# CONFIG += protobuf  # 如果需要使用Protobuf，取消注释

# 定义
DEFINES += QT_DEPRECATED_WARNINGS

# 源文件 - 只包含已实现的文件
SOURCES += \
    src/main.cpp \
    src/ui/mainwindow.cpp \
    src/models/message.cpp \
    src/models/thread.cpp \
    src/models/user.cpp \
    src/models/config.cpp \
    src/core/mqtt/mqttclient.cpp \
    src/core/mqtt/mqttmessagehandler.cpp \
    src/core/network/httpclient.cpp \
    src/core/network/apibase.cpp \
    src/core/network/authapi.cpp \
    src/core/network/messageapi.cpp \
    src/core/network/threadapi.cpp \
    src/core/auth/authmanager.cpp

# 头文件
HEADERS += \
    src/ui/mainwindow.h \
    src/models/message.h \
    src/models/thread.h \
    src/models/user.h \
    src/models/config.h \
    src/core/mqtt/mqttclient.h \
    src/core/mqtt/mqttmessagehandler.h \
    src/core/network/httpclient.h \
    src/core/network/apibase.h \
    src/core/network/authapi.h \
    src/core/network/messageapi.h \
    src/core/network/threadapi.h \
    src/core/auth/authmanager.h

# UI文件
FORMS += \
    src/ui/mainwindow.ui

# 包含路径
INCLUDEPATH += \
    src

# 资源文件 - 如果有资源文件，取消下面的注释
# RESOURCES += \
#     resources/icons.qrc \
#     resources/styles.qrc \
#     resources/i18n.qrc

# 翻译文件
TRANSLATIONS += \
    qt_zh_CN.ts

CONFIG += lrelease
CONFIG += embed_translations

# 默认规则用于部署
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
