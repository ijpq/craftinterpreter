#!/bin/bash

# 最简单的测试 - 一步步验证

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${YELLOW}=== 简单测试 ===${NC}\n"

# 必须在项目根目录
cd /Users/ketang/craftinginterpreter

echo -e "${YELLOW}步骤 1: 编译 Java${NC}"
cd lox
javac *.java
cd ..
echo -e "${GREEN}完成${NC}\n"

echo -e "${YELLOW}步骤 2: 编译 C++${NC}"
./build_mac.sh 2>&1 | tail -1
echo ""

echo -e "${YELLOW}步骤 3: 测试 Java (直接在 lox 目录内)${NC}"
cd lox
echo "var a = 1;" > /tmp/test.lox
java Lox /tmp/test.lox 2>&1 | head -6
cd ..
echo ""

echo -e "${YELLOW}步骤 4: 测试 C++${NC}"
./build/craftinginterpreter /tmp/test.lox 2>&1 | head -6
echo ""

echo -e "${GREEN}测试完成${NC}"
