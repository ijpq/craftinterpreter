cmake_minimum_required(VERSION 3.16)
project(craftinginterpreter LANGUAGES CXX)

# C++ 标准设置
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(APPLE)
    # 确保最小部署版本支持 C++17 特性
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.14")
endif()

# 收集源文件
set(SOURCES
    csrc/lox.cpp
    csrc/scanner.cpp
    csrc/token.cpp
)

# 创建可执行文件
add_executable(${PROJECT_NAME} ${SOURCES})

# 设置 include 目录
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/csrc/include
)

# 编译选项(可选)
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()
