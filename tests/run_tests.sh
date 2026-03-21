#!/bin/bash

# 完整集成测试 - 修复版

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo "========================================"
echo "Scanner 一致性测试"
echo "========================================"

# 获取项目根目录的绝对路径
PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$PROJECT_ROOT"

# 编译 C++ 测试程序
echo -e "\n${YELLOW}步骤 1: 编译 C++ 测试程序${NC}"
g++ -std=c++17 -I./csrc/include \
    tests/scanner_test.cpp \
    csrc/scanner.cpp \
    csrc/token.cpp \
    -o tests/scanner_test

if [ $? -ne 0 ]; then
    echo -e "${RED}C++ 编译失败${NC}"
    exit 1
fi
echo -e "${GREEN}C++ 编译成功${NC}"

# 编译 Java 程序
echo -e "\n${YELLOW}步骤 2: 编译 Java 程序${NC}"
cd "$PROJECT_ROOT/lox"
javac *.java
if [ $? -ne 0 ]; then
    echo -e "${RED}Java 编译失败${NC}"
    exit 1
fi
echo -e "${GREEN}Java 编译成功${NC}"

# 运行测试
echo -e "\n${YELLOW}步骤 3: 运行测试${NC}"

cd "$PROJECT_ROOT"
TEST_FILE="tests/test_input.lox"

echo "运行 C++ Scanner..."
./tests/scanner_test "$TEST_FILE" > tests/cpp_output.txt 2>&1

echo "运行 Java Scanner..."
# 关键修复: 使用 -cp . 而不是 -cp lox
java -cp . lox.Lox "$TEST_FILE" > tests/java_output.txt 2>&1

# 比较输出
echo -e "\n${YELLOW}步骤 4: 比较输出${NC}"

if diff -u tests/java_output.txt tests/cpp_output.txt > tests/diff.txt; then
    echo -e "${GREEN}✓ 测试通过! C++ 和 Java 输出完全一致${NC}"
    rm tests/diff.txt
    exit 0
else
    echo -e "${RED}✗ 测试失败! 发现差异:${NC}"
    echo ""
    cat tests/diff.txt
    echo ""
    echo -e "${YELLOW}详细输出已保存到:${NC}"
    echo "  - tests/cpp_output.txt"
    echo "  - tests/java_output.txt"
    echo "  - tests/diff.txt"
    exit 1
fi
