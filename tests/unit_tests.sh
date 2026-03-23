#!/bin/bash

# 统一单元测试脚本 - Scanner + ASTPrinter

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

TESTS_PASSED=0
TESTS_FAILED=0

# 获取项目根目录的绝对路径
PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"

# ========================================
# 通用测试函数
# ========================================

# Scanner 测试: 比较 Java Lox 和 C++ craftinginterpreter 的 scanner 输出
test_scanner() {
    local test_name="$1"
    local input="$2"
    
    echo -e "  ${BLUE}[Scanner] ${test_name}${NC}"
    
    local test_file="/tmp/scanner_test_$$.lox"
    echo "$input" > "$test_file"
    
    cd "$PROJECT_ROOT"
    java_output=$(java -cp . lox.Lox "$test_file" 2>&1)
    cpp_output=$("$PROJECT_ROOT/build/craftinginterpreter" "$test_file" 2>&1)
    
    rm -f "$test_file"
    
    if [ "$java_output" = "$cpp_output" ]; then
        echo -e "  ${GREEN}✓ PASS${NC}"
        ((TESTS_PASSED++))
    else
        echo -e "  ${RED}✗ FAIL${NC}"
        echo "    Java: $java_output"
        echo "    C++:  $cpp_output"
        ((TESTS_FAILED++))
    fi
}

# ASTPrinter 测试: 比较 Java AstPrinter 和 C++ test_ast_printer 的输出
test_ast_printer() {
    echo -e "  ${BLUE}[ASTPrinter] C++ vs Java 输出一致性${NC}"
    
    cd "$PROJECT_ROOT"
    java_output=$(java -cp . lox.AstPrinter 2>&1)
    cpp_output=$("$PROJECT_ROOT/build/test_ast_printer" 2>&1)
    
    if [ "$java_output" = "$cpp_output" ]; then
        echo -e "  ${GREEN}✓ PASS${NC}  output: $cpp_output"
        ((TESTS_PASSED++))
    else
        echo -e "  ${RED}✗ FAIL${NC}"
        echo "    Java: $java_output"
        echo "    C++:  $cpp_output"
        ((TESTS_FAILED++))
    fi
}

# ========================================
# 编译
# ========================================

echo -e "${YELLOW}编译程序...${NC}"
cd "$PROJECT_ROOT"

# 编译 C++ (cmake)
cd build && cmake --build . > /dev/null 2>&1
cmake_rc=$?
cd "$PROJECT_ROOT"
if [ $cmake_rc -ne 0 ]; then
    echo -e "${RED}C++ cmake 编译失败, 尝试重新配置...${NC}"
    ./build_mac.sh > /dev/null 2>&1
fi

# 编译 Java
cd "$PROJECT_ROOT/lox" && javac *.java 2>/dev/null
cd "$PROJECT_ROOT"

echo -e "${GREEN}编译完成${NC}"

# ========================================
# ASTPrinter 测试
# ========================================

echo -e "\n${YELLOW}========================================"
echo "ASTPrinter 测试"
echo "========================================${NC}"

test_ast_printer

# ========================================
# Scanner 测试
# ========================================

echo -e "\n${YELLOW}========================================"
echo "Scanner 测试"
echo "========================================${NC}"

# 基本类型
test_scanner "整数" "123;"
test_scanner "浮点数" "123.456;"
test_scanner "字符串" '"hello world";'

# 变量与表达式
test_scanner "变量声明" "var x = 10;"
test_scanner "算术表达式" "1 + 2 * 3;"
test_scanner "括号表达式" "(1 + 2) * 3;"
test_scanner "复杂表达式" "var result = (a + b) * c / d - e;"

# 运算符
test_scanner "比较运算符" "x > 5;"
test_scanner "相等运算符" "x == 10;"
test_scanner "不等运算符" "x != 5;"
test_scanner "逻辑运算符" "true and false;"

# 关键字
test_scanner "if" "if (x > 0) {}"
test_scanner "while" "while (true) {}"
test_scanner "for" "for (;;) {}"
test_scanner "fun" "fun add() {}"
test_scanner "class" "class MyClass {}"
test_scanner "return" "return 42;"
test_scanner "print" "print x;"

# 字面量
test_scanner "true" "true;"
test_scanner "false" "false;"
test_scanner "nil" "nil;"

# 多行与注释
test_scanner "注释" "// comment
var x = 1;"
test_scanner "多行" "var a = 1;
var b = 2;"

# ========================================
# 总结
# ========================================

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
