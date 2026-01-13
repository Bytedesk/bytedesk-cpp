cmake_minimum_required(VERSION 3.16)
project(bytedesk-qt VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Qt配置
set(CMAKE_PREFIX_PATH "$ENV{HOME}/Qt/6.5.0/gcc_64")
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network WebSockets Sql Qml Quick)

# Protobuf配置
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})

# MQTT库配置 (使用Qt的QMqttClient或第三方库)
# 如果使用Qt MQTT，需要Qt6Components OPTIONAL
# 这里我们使用Qt自带的QMqttClient (Qt 5.12+ 或 Qt 6.2+)

# 源文件
set(PROJECT_SOURCES
    src/main.cpp
    src/ui/mainwindow.cpp
    src/ui/mainwindow.h
    src/ui/mainwindow.ui

    # Models
    src/models/message.cpp
    src/models/message.h
    src/models/thread.cpp
    src/models/thread.h
    src/models/user.cpp
    src/models/user.h
    src/models/config.cpp
    src/models/config.h

    # Core - MQTT
    src/core/mqtt/mqttclient.cpp
    src/core/mqtt/mqttclient.h
    src/core/mqtt/mqttmessagehandler.cpp
    src/core/mqtt/mqttmessagehandler.h

    # Core - Network
    src/core/network/httpclient.cpp
    src/core/network/httpclient.h
    src/core/network/apibase.cpp
    src/core/network/apibase.h
    src/core/network/authapi.cpp
    src/core/network/authapi.h
    src/core/network/messageapi.cpp
    src/core/network/messageapi.h
    src/core/network/threadapi.cpp
    src/core/network/threadapi.h

    # Core - Auth
    src/core/auth/authmanager.cpp
    src/core/auth/authmanager.h

    # Core - Protobuf
    src/core/protobuf/protobufwrapper.cpp
    src/core/protobuf/protobufwrapper.h

    # UI - Pages
    src/ui/pages/login/loginpage.cpp
    src/ui/pages/login/loginpage.h
    src/ui/pages/login/loginpage.ui

    src/ui/pages/chat/chatpage.cpp
    src/ui/pages/chat/chatpage.h
    src/ui/pages/chat/chatpage.ui

    src/ui/pages/contact/contactpage.cpp
    src/ui/pages/contact/contactpage.h
    src/ui/pages/contact/contactpage.ui

    src/ui/pages/settings/settingspage.cpp
    src/ui/pages/settings/settingspage.h
    src/ui/pages/settings/settingspage.ui

    # UI - Widgets
    src/ui/widgets/messagebubble.cpp
    src/ui/widgets/messagebubble.h
    src/ui/widgets/messageeditor.cpp
    src/ui/widgets/messageeditor.h
    src/ui/widgets/threadlistitem.cpp
    src/ui/widgets/threadlistitem.h
    src/ui/widgets/chatview.cpp
    src/ui/widgets/chatview.h

    # Stores
    src/stores/authstore.cpp
    src/stores/authstore.h
    src/stores/messagestore.cpp
    src/stores/messagestore.h
    src/stores/threadstore.cpp
    src/stores/threadstore.h

    # Database
    src/database/database.cpp
    src/database/database.h
    src/database/messagedao.cpp
    src/database/messagedao.h
    src/database/threaddao.cpp
    src/database/threaddao.h

    # Utils
    src/utils/configutils.cpp
    src/utils/configutils.h
    src/utils/messageutils.cpp
    src/utils/messageutils.h
    src/utils/datetimeserializer.cpp
    src/utils/datetimeserializer.h
)

# Protobuf生成的源文件
set(PROTO_FILES
    proto/message.proto
    proto/thread.proto
    proto/user.proto
)

# 生成Protobuf头文件和源文件
foreach(PROTO_FILE ${PROTO_FILES})
    get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)

    set(PROTO_SRC "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.pb.cc")
    set(PROTO_HDR "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.pb.h")

    add_custom_command(
        OUTPUT ${PROTO_SRC} ${PROTO_HDR}
        COMMAND protobuf::protoc
        ARGS --proto_path=${CMAKE_CURRENT_SOURCE_DIR}/proto
             --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
             ${CMAKE_CURRENT_SOURCE_DIR}/${PROTO_FILE}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${PROTO_FILE}
        COMMENT "Generating Protobuf files for ${PROTO_FILE}"
    )

    list(APPEND PROTOBUF_SRCS ${PROTO_SRC})
    list(APPEND PROTOBUF_HDRS ${PROTO_HDR})
endforeach()

# 资源文件
set(PROJECT_RESOURCES
    resources/icons.qrc
    resources/styles.qrc
    resources/i18n.qrc
)

# 创建可执行文件
add_executable(${PROJECT_NAME}
    ${PROJECT_SOURCES}
    ${PROTOBUF_SRCS}
    ${PROJECT_RESOURCES}
)

# 链接库
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::WebSockets
    Qt6::Sql
    Qt6::Qml
    Qt6::Quick
    protobuf::libprotobuf
)

# 包含目录
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
    ${PROTOBUF_INCLUDE_DIRS}
)

# 编译选项
target_compile_definitions(${PROJECT_NAME} PRIVATE
    QT_DEPRECATED_WARNINGS
    $<$<CONFIG:Debug>:DEBUG_MODE>
)

# 安装配置
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

# 如果需要使用Qt MQTT (Qt 6.2+)
# find_package(Qt6Mqtt)
# if(Qt6Mqtt_FOUND)
#     target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Mqtt)
# else()
#     # 使用第三方MQTT库
#     # 例如: qtadvancedmqtt或QMqttClient
# endif()

# 调试信息
message(STATUS "Project: ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "Qt Prefix: ${CMAKE_PREFIX_PATH}")
message(STATUS "Protobuf: ${PROTOBUF_INCLUDE_DIRS}")
