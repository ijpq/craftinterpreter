#!/bin/bash

# 使用文件输入而非管道的测试版本

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${YELLOW}文件输入测试${NC}\n"

PROJECT_ROOT="/Users/ketang/craftinginterpreter"
cd "$PROJECT_ROOT"

# 创建测试文件
echo "var a = 1;" > /tmp/test_scanner.lox

echo -e "${YELLOW}[1/2] 测试 Java Scanner (使用文件)...${NC}"
java -cp . lox.Lox /tmp/test_scanner.lox 2>&1 > /tmp/java_out.txt
cat /tmp/java_out.txt
echo ""

echo -e "${YELLOW}[2/2] 测试 C++ Scanner (使用文件)...${NC}"
./build/craftinginterpreter /tmp/test_scanner.lox 2>&1 > /tmp/cpp_out.txt
cat /tmp/cpp_out.txt
echo ""

echo -e "${YELLOW}对比:${NC}"
if diff /tmp/java_out.txt /tmp/cpp_out.txt > /dev/null; then
    echo -e "${GREEN}✓ 输出一致!${NC}"
else
    echo -e "${RED}✗ 输出不一致${NC}"
    echo ""
    echo "差异:"
    diff /tmp/java_out.txt /tmp/cpp_out.txt
fi

# 清理
rm -f /tmp/test_scanner.lox /tmp/java_out.txt /tmp/cpp_out.txt
