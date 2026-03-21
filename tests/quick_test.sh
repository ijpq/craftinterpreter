#!/bin/bash

# 快速验证脚本 - 使用文件输入版本

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${YELLOW}快速验证测试${NC}\n"

# 保存项目根目录的绝对路径
PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$PROJECT_ROOT"

# 1. 编译 Java
echo -e "${YELLOW}[1/4] 编译 Java...${NC}"
cd "$PROJECT_ROOT/lox"
javac *.java 2>&1
if [ $? -ne 0 ]; then
    echo -e "${RED}✗ Java 编译失败${NC}"
    exit 1
fi
echo -e "${GREEN}✓ Java 编译成功${NC}\n"

# 2. 编译 C++
echo -e "${YELLOW}[2/4] 编译 C++...${NC}"
cd "$PROJECT_ROOT"
./build_mac.sh 2>&1 | tail -1
if [ $? -ne 0 ]; then
    echo -e "${RED}✗ C++ 编译失败${NC}"
    exit 1
fi
echo -e "${GREEN}✓ C++ 编译成功${NC}\n"

# 创建临时测试文件(使用文件而非管道,避免 runPrompt 的提示符问题)
TEST_FILE="/tmp/scanner_test_$$.lox"
echo "var a = 1;" > "$TEST_FILE"

# 3. 测试 Java
echo -e "${YELLOW}[3/4] 测试 Java Scanner...${NC}"
cd "$PROJECT_ROOT"
java_out=$(java -cp . lox.Lox "$TEST_FILE" 2>&1)
echo "$java_out"
if [[ "$java_out" == *"VAR"* ]]; then
    echo -e "${GREEN}✓ Java Scanner 正常工作${NC}\n"
else
    echo -e "${RED}✗ Java Scanner 输出异常${NC}"
    rm -f "$TEST_FILE"
    exit 1
fi

# 4. 测试 C++
echo -e "${YELLOW}[4/4] 测试 C++ Scanner...${NC}"
cpp_out=$(./build/craftinginterpreter "$TEST_FILE" 2>&1)
echo "$cpp_out"
if [[ "$cpp_out" == *"VAR"* ]]; then
    echo -e "${GREEN}✓ C++ Scanner 正常工作${NC}\n"
else
    echo -e "${RED}✗ C++ Scanner 输出异常${NC}"
    rm -f "$TEST_FILE"
    exit 1
fi

# 5. 对比输出
echo -e "${YELLOW}对比输出...${NC}"
if [ "$java_out" = "$cpp_out" ]; then
    echo -e "${GREEN}✓✓✓ 完美! Java 和 C++ 输出完全一致!${NC}"
    rm -f "$TEST_FILE"
    exit 0
else
    echo -e "${RED}✗ 输出不一致${NC}"
    echo -e "\nJava:\n$java_out"
    echo -e "\nC++:\n$cpp_out"
    rm -f "$TEST_FILE"
    exit 1
fi
