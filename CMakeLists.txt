cmake_minimum_required(VERSION 3.16)
project(craftinginterpreter LANGUAGES CXX)

# C++ 标准设置
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Shared library rpath 设置（macOS 开发阶段）
set(CMAKE_MACOSX_RPATH ON)
set(BUILD_SHARED_LIBS ON)
set(CMAKE_BUILD_RPATH_USE_ORIGIN TRUE)

# 全局 include 目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/csrc/include)

# ========================================
# 核心库：Scanner 和 Token
# ========================================
add_library(lox_core SHARED
    csrc/token.cpp
    csrc/scanner.cpp
)

# ========================================
# AST 库：Expr 和 Visitor（不含main函数）
# ========================================
add_library(interpreter_lib SHARED csrc/interpreter.cpp)
target_link_libraries(interpreter_lib lox_core expr_lib)

add_library(expr_lib SHARED
    csrc/expr.cpp
)
target_link_libraries(expr_lib lox_core)

# ========================================
# 主程序：craftinginterpreter
# ========================================
add_executable(craftinginterpreter
    csrc/lox.cpp
)
target_link_libraries(craftinginterpreter expr_lib lox_core interpreter_lib)

# ========================================
# 测试程序
# ========================================

# Scanner 测试
add_executable(scanner_test
    tests/scanner_test.cpp
)
target_link_libraries(scanner_test lox_core)

# AST Printer 测试（独立的main函数）
add_executable(test_ast_printer
    csrc/expr_test_main.cpp
)
target_link_libraries(test_ast_printer expr_lib lox_core)

# Parser 测试
add_executable(parser_test
    tests/parser_test.cpp
)
target_link_libraries(parser_test expr_lib lox_core)

add_executable(object_test
    tests/test_object.cpp)


# ========================================
# 编译选项
# ========================================
if(MSVC)
    target_compile_options(lox_core PRIVATE /W4)
else()
    target_compile_options(lox_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

# 为所有可执行文件设置相同的编译选项
set(EXECUTABLES 
    craftinginterpreter 
    scanner_test 
    test_ast_printer 
    parser_test
)

foreach(executable ${EXECUTABLES})
    if(MSVC)
        target_compile_options(${executable} PRIVATE /W4)
    else()
        target_compile_options(${executable} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endforeach()

# ========================================
# 输出信息
# ========================================
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
