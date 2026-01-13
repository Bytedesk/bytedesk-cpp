#!/bin/bash

# ByteDesk Qt Client 项目验证脚本

echo "==================================="
echo "ByteDesk Qt Client 项目验证"
echo "==================================="
echo ""

# 检查文件结构
echo "检查文件结构..."

required_files=(
    "bytedesk.pro"
    "src/main.cpp"
    "src/ui/mainwindow.cpp"
    "src/ui/mainwindow.h"
    "src/ui/mainwindow.ui"
    "src/models/message.cpp"
    "src/models/message.h"
    "src/models/thread.cpp"
    "src/models/thread.h"
    "src/models/user.cpp"
    "src/models/user.h"
    "src/models/config.cpp"
    "src/models/config.h"
    "src/core/mqtt/mqttclient.cpp"
    "src/core/mqtt/mqttclient.h"
    "src/core/mqtt/mqttmessagehandler.cpp"
    "src/core/mqtt/mqttmessagehandler.h"
    "src/core/network/httpclient.cpp"
    "src/core/network/httpclient.h"
    "src/core/network/apibase.cpp"
    "src/core/network/apibase.h"
    "src/core/network/authapi.cpp"
    "src/core/network/authapi.h"
    "src/core/network/messageapi.cpp"
    "src/core/network/messageapi.h"
    "src/core/network/threadapi.cpp"
    "src/core/network/threadapi.h"
    "src/core/auth/authmanager.cpp"
    "src/core/auth/authmanager.h"
)

missing_files=0

for file in "${required_files[@]}"; do
    if [ -f "$file" ]; then
        echo "✓ $file"
    else
        echo "✗ $file (缺失)"
        missing_files=$((missing_files + 1))
    fi
done

echo ""
echo "==================================="

if [ $missing_files -eq 0 ]; then
    echo "✓ 所有必需文件都存在"
    echo ""
    echo "文件统计:"
    echo "  - C++ 源文件: $(find src -name "*.cpp" | wc -l)"
    echo "  - C++ 头文件: $(find src -name "*.h" | wc -l)"
    echo "  - UI 文件: $(find src -name "*.ui" | wc -l)"
    echo "  - 总计: $(find src -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.ui" \) | wc -l) 个文件"
    echo ""
    echo "项目结构正确！可以开始编译。"
    echo ""
    echo "下一步:"
    echo "  1. 使用Qt Creator打开 bytedesk.pro"
    echo "  2. 或运行: qmake bytedesk.pro && make"
    echo "  3. 或运行: ./build.sh"
else
    echo "✗ 缺失 $missing_files 个文件"
    echo ""
    echo "请检查项目文件是否完整"
    exit 1
fi

echo "==================================="
