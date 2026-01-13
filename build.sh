#!/bin/bash

# ByteDesk Qt Client 编译脚本

echo "==================================="
echo "ByteDesk Qt Client Build Script"
echo "==================================="
echo ""

# 检查Qt环境
echo "检查Qt环境..."
if [ -z "$QTDIR" ]; then
    echo "警告: QTDIR 环境变量未设置"
    echo "请设置Qt路径，例如: export QTDIR=/path/to/Qt/version/gcc_64"
fi

# 清理旧构建文件
echo ""
echo "清理旧构建文件..."
rm -rf build
mkdir -p build
cd build

# 使用qmake生成Makefile
echo ""
echo "生成Makefile..."
qmake ../bytedesk.pro

if [ $? -ne 0 ]; then
    echo "错误: qmake 失败"
    exit 1
fi

# 编译
echo ""
echo "开始编译..."
make -j4

if [ $? -eq 0 ]; then
    echo ""
    echo "==================================="
    echo "编译成功！"
    echo "可执行文件: build/qt"
    echo "==================================="
else
    echo ""
    echo "==================================="
    echo "编译失败！"
    echo "==================================="
    exit 1
fi
