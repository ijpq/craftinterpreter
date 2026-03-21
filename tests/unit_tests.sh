#!/bin/bash

# 单元测试脚本 - 使用文件输入版本

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

TESTS_PASSED=0
TESTS_FAILED=0

# 获取项目根目录的绝对路径
PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"

# 测试函数 - 使用文件输入
test_case() {
    local test_name="$1"
    local input="$2"
    
    echo -e "\n${BLUE}测试: ${test_name}${NC}"
    echo "输入: $input"
    
    # 创建临时测试文件
    local test_file="/tmp/scanner_test_$$.lox"
    echo "$input" > "$test_file"
    
    # 运行 Java
    cd "$PROJECT_ROOT"
    java_output=$(java -cp . lox.Lox "$test_file" 2>&1)
    
    # 运行 C++
    cpp_output=$("$PROJECT_ROOT/build/craftinginterpreter" "$test_file" 2>&1)
    
    # 清理
    rm -f "$test_file"
    
    # 比较
    if [ "$java_output" = "$cpp_output" ]; then
        echo -e "${GREEN}✓ PASS${NC}"
        ((TESTS_PASSED++))
    else
        echo -e "${RED}✗ FAIL${NC}"
        echo "Java 输出:"
        echo "$java_output"
        echo ""
        echo "C++ 输出:"
        echo "$cpp_output"
        ((TESTS_FAILED++))
    fi
}

# 编译
echo -e "${YELLOW}编译程序...${NC}"
cd "$PROJECT_ROOT"
./build_mac.sh > /dev/null 2>&1
cd "$PROJECT_ROOT/lox" && javac *.java && cd "$PROJECT_ROOT"

echo -e "\n${YELLOW}========================================"
echo "开始单元测试"
echo "========================================${NC}"

# 测试用例
test_case "整数" "123;"
test_case "浮点数" "123.456;"
test_case "字符串" '"hello world";'
test_case "变量声明" "var x = 10;"
test_case "算术表达式" "1 + 2 * 3;"
test_case "比较运算符" "x > 5;"
test_case "相等运算符" "x == 10;"
test_case "不等运算符" "x != 5;"
test_case "逻辑运算符" "true and false;"
test_case "括号表达式" "(1 + 2) * 3;"
test_case "标识符" "myVariable;"
test_case "关键字 if" "if (x > 0) {}"
test_case "关键字 while" "while (true) {}"
test_case "关键字 for" "for (;;) {}"
test_case "关键字 fun" "fun add() {}"
test_case "关键字 class" "class MyClass {}"
test_case "关键字 return" "return 42;"
test_case "关键字 print" "print x;"
test_case "布尔值 true" "true;"
test_case "布尔值 false" "false;"
test_case "nil" "nil;"
test_case "注释" "// comment
var x = 1;"
test_case "多行" "var a = 1;
var b = 2;"
test_case "复杂表达式" "var result = (a + b) * c / d - e;"

# 总结
echo -e "\n${YELLOW}========================================"
echo "测试总结"
echo "========================================${NC}"
echo -e "${GREEN}通过: $TESTS_PASSED${NC}"
echo -e "${RED}失败: $TESTS_FAILED${NC}"

if [ $TESTS_FAILED -eq 0 ]; then
    echo -e "\n${GREEN}所有测试通过!${NC}"
    exit 0
else
    echo -e "\n${RED}部分测试失败${NC}"
    exit 1
fi
