cmake_minimum_required(VERSION 3.16)
project(craftinginterpreter LANGUAGES CXX)

# C++ 标准设置
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 收集源文件
set(SOURCES
    csrc/lox.cpp
    csrc/scanner.cpp
    csrc/token.cpp
)

# 创建主可执行文件
add_executable(${PROJECT_NAME} ${SOURCES})

# 设置 include 目录
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/csrc/include
)

# 编译选项
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# 测试可执行文件
add_executable(scanner_test
    tests/scanner_test.cpp
    csrc/scanner.cpp
    csrc/token.cpp
)

target_include_directories(scanner_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/csrc/include
)

if(NOT MSVC)
    target_compile_options(scanner_test PRIVATE -Wall -Wextra -Wpedantic)
endif()
