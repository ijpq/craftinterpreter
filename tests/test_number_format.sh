#!/bin/bash

# 测试数字格式化

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

PROJECT_ROOT="/Users/ketang/craftinginterpreter"
cd "$PROJECT_ROOT"

echo -e "${YELLOW}=== 数字格式化测试 ===${NC}\n"

# 重新编译
echo "编译 C++..."
./build_mac.sh > /dev/null 2>&1

echo "编译 Java..."
cd lox && javac *.java && cd ..
echo ""

TEST_FILE="tests/test_numbers.lox"

echo -e "${BLUE}Java 输出:${NC}"
java -cp . lox.Lox "$TEST_FILE" | grep "NUMBER"
echo ""

echo -e "${BLUE}C++ 输出:${NC}"
./build/craftinginterpreter "$TEST_FILE" | grep "NUMBER"
echo ""

echo -e "${YELLOW}=== 对比特定数字 ===${NC}\n"

test_number() {
    local num="$1"
    echo "var x = $num;" > /tmp/num_test.lox
    
    java_out=$(java -cp . lox.Lox /tmp/num_test.lox 2>&1 | grep "NUMBER")
    cpp_out=$(./build/craftinginterpreter /tmp/num_test.lox 2>&1 | grep "NUMBER")
    
    if [ "$java_out" = "$cpp_out" ]; then
        echo -e "${GREEN}✓${NC} $num"
    else
        echo -e "${RED}✗${NC} $num"
        echo "  Java: $java_out"
        echo "  C++:  $cpp_out"
    fi
}

test_number "1"
test_number "1.0"
test_number "123.456"
test_number "0.5"
test_number "3.14159"
test_number "42"

rm -f /tmp/num_test.lox
