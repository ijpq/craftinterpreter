#!/bin/bash

# 删除旧的 build 目录
rm -rf build

# 创建并进入 build 目录
mkdir build
cd build

# Ubuntu 使用系统默认的 GCC（或手动指定版本）
# 如需指定版本，取消注释并修改版本号：
# export CC=/usr/bin/gcc-13
# export CXX=/usr/bin/g++-13

# 配置 CMake（Linux rpath 设置）
cmake .. \
    -DCMAKE_BUILD_RPATH_USE_ORIGIN=ON \
    -DCMAKE_INSTALL_RPATH='$ORIGIN' \
    -DBUILD_SHARED_LIBS=ON \
    -DCMAKE_BUILD_TYPE=Debug

# 编译
cmake --build .

echo "编译完成! 可执行文件位于: build/craftinginterpreter"
echo "编译器: $(${CXX:-g++} --version | head -1)"
