#!/bin/bash

# 统一单元测试脚本 - Scanner + ASTPrinter + Parser

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

# Parser 测试: 测试 C++ parser 能否正确解析表达式
test_parser() {
    local test_name="$1"
    local test_file="$2"
    
    echo -e "  ${BLUE}[Parser] ${test_name}${NC}"
    
    cd "$PROJECT_ROOT"
    parser_output=$("$PROJECT_ROOT/build/parser_test" "$test_file" 2>&1)
    parser_rc=$?
    
    if [ $parser_rc -eq 0 ]; then
        echo -e "  ${GREEN}✓ PASS${NC}"
        local ast_line=$(echo "$parser_output" | grep -A 1 "=== AST ===" | tail -1)
        echo "    AST: $ast_line"
        ((TESTS_PASSED++))
    else
        echo -e "  ${RED}✗ FAIL${NC}"
        echo "$parser_output" | sed 's/^/    /'
        ((TESTS_FAILED++))
    fi
}

# ========================================
# 清理和编译
# ========================================

echo -e "${YELLOW}========================================"
echo "清理和编译"
echo "========================================${NC}"

# 清理和编译 C++
echo -e "${BLUE}清理C++旧编译文件...${NC}"
rm -rf "$PROJECT_ROOT/build"

echo -e "${BLUE}编译C++代码...${NC}"
cd "$PROJECT_ROOT"
if ! ./build_mac.sh; then
    echo -e "${RED}C++编译失败${NC}"
    exit 1
fi

# 清理和编译 Java
echo -e "\n${BLUE}清理Java旧编译文件...${NC}"
rm -f "$PROJECT_ROOT/lox"/*.class

echo -e "${BLUE}编译Java代码...${NC}"
cd "$PROJECT_ROOT/lox"
if ! javac *.java; then
    echo -e "${RED}Java编译失败${NC}"
    cd "$PROJECT_ROOT"
    exit 1
fi
cd "$PROJECT_ROOT"

echo -e "\n${GREEN}编译完成${NC}"

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
# Parser 测试
# ========================================

echo -e "\n${YELLOW}========================================"
echo "Parser 测试"
echo "========================================${NC}"

# 测试各种表达式解析
test_parser "算术表达式" "$PROJECT_ROOT/tests/test_arithmetic.lox"
test_parser "比较和相等" "$PROJECT_ROOT/tests/test_comparison.lox"
test_parser "一元运算符" "$PROJECT_ROOT/tests/test_unary.lox"
test_parser "分组括号" "$PROJECT_ROOT/tests/test_grouping.lox"
test_parser "字面量" "$PROJECT_ROOT/tests/test_literals.lox"

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
