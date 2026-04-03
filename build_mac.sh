#!/bin/bash

# 创建 build 目录（如果不存在）
mkdir -p build
cd build

# 设置使用GCC（修改版本号为你安装的版本）
export CC=/usr/local/bin/gcc-15
export CXX=/usr/local/bin/g++-15

# 配置 CMake（仅在未配置时）
if [ ! -f CMakeCache.txt ]; then
    cmake ..
fi

# 编译
cmake --build .

echo "编译完成! 可执行文件位于: build/craftinginterpreter"
echo "编译器: $(${CXX} --version | head -1)"
