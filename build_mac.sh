#!/bin/bash

# 删除旧的 build 目录
rm -rf build

# 创建并进入 build 目录
mkdir build
cd build

# 配置 CMake
cmake ..

# 编译
cmake --build .

echo "编译完成! 可执行文件位于: build/craftinginterpreter"
