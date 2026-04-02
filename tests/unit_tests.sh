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
    
    echo "    --- Java ---"
    echo "$java_output" | sed 's/^/      /'
    echo "    --- C++ ---"
    echo "$cpp_output" | sed 's/^/      /'
    if [ "$java_output" = "$cpp_output" ]; then
        echo -e "  ${GREEN}✓ PASS${NC}"
        ((TESTS_PASSED++))
    else
        echo -e "  ${RED}✗ FAIL${NC}"
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

# Parser 测试: 打印 C++ parser_test 输出
test_parser() {
    local test_name="$1"
    local test_file="$2"

    echo -e "  ${BLUE}[Parser] ${test_name}${NC}"

    cd "$PROJECT_ROOT"
    cpp_output=$("$PROJECT_ROOT/build/parser_test" "$test_file" 2>&1)
    cpp_rc=$?

    echo "    --- C++ ---"
    echo "$cpp_output" | sed 's/^/      /'

    if [ $cpp_rc -eq 0 ]; then
        echo -e "  ${GREEN}✓ PASS${NC}"
        ((TESTS_PASSED++))
    else
        echo -e "  ${RED}✗ FAIL${NC}"
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

# 字面量
test_scanner "整数" "123;"
test_scanner "浮点数" "123.456;"
test_scanner "字符串" '"hello world";'
test_scanner "true" "true;"
test_scanner "false" "false;"
test_scanner "nil" "nil;"

# expression statement
test_scanner "算术表达式" "1 + 2 * 3;"
test_scanner "括号表达式" "(1 + 2) * 3;"
test_scanner "比较运算符" "1 > 5;"
test_scanner "相等运算符" "1 == 2;"
test_scanner "不等运算符" "1 != 2;"
test_scanner "一元运算符" "!true;"

# print statement
test_scanner "print 数字" "print 42;"
test_scanner "print 字符串" 'print "hello";'
test_scanner "print 表达式" "print 1 + 2;"
test_scanner "print bool" "print true;"
test_scanner "print nil" "print nil;"

# 多条语句
test_scanner "多条语句" "print 1;
print 2;"
test_scanner "注释" "// comment
print 1;"

# ========================================
# Parser 测试
# ========================================

echo -e "\n${YELLOW}========================================"
echo "Parser 测试"
echo "========================================${NC}"

# 测试各种表达式解析
# test_parser "算术表达式" "$PROJECT_ROOT/tests/test_arithmetic.lox"
# test_parser "比较和相等" "$PROJECT_ROOT/tests/test_comparison.lox"
# test_parser "一元运算符" "$PROJECT_ROOT/tests/test_unary.lox"
# test_parser "分组括号" "$PROJECT_ROOT/tests/test_grouping.lox"
# test_parser "字面量" "$PROJECT_ROOT/tests/test_literals.lox"

# ========================================
# Chapter 7 Interpreter 正确性测试（Java vs C++ 输出比较）
# ========================================

echo -e "\n${YELLOW}========================================"
echo "Chapter 7 Interpreter 正确性测试"
echo "========================================${NC}"

test_interpreter() {
    local test_name="$1"
    local test_file="$2"

    echo -e "  ${BLUE}[Interpreter] ${test_name}${NC}"

    cd "$PROJECT_ROOT"
    java_output=$(java -cp "$PROJECT_ROOT" lox.Lox "$test_file" 2>&1)
    cpp_output=$(DYLD_LIBRARY_PATH="$PROJECT_ROOT/build" "$PROJECT_ROOT/build/craftinginterpreter" "$test_file" 2>&1)

    echo "    --- Java ---"
    echo "$java_output" | sed 's/^/      /'
    echo "    --- C++ ---"
    echo "$cpp_output" | sed 's/^/      /'
    if [ "$java_output" = "$cpp_output" ]; then
        echo -e "  ${GREEN}✓ PASS${NC}"
        ((TESTS_PASSED++))
    else
        echo -e "  ${RED}✗ FAIL${NC}"
        ((TESTS_FAILED++))
    fi
}

test_interpreter "算术" "$PROJECT_ROOT/tests/ch7_arithmetic.lox"
test_interpreter "字符串拼接" "$PROJECT_ROOT/tests/ch7_strings.lox"
test_interpreter "比较运算" "$PROJECT_ROOT/tests/ch7_comparison.lox"
test_interpreter "相等判断" "$PROJECT_ROOT/tests/ch7_equality.lox"
test_interpreter "一元运算" "$PROJECT_ROOT/tests/ch7_unary.lox"
test_interpreter "数字格式化" "$PROJECT_ROOT/tests/ch7_numbers.lox"

# ========================================
# Chapter 8 正确性测试（Java vs C++ 输出比较）
# ========================================

echo -e "\n${YELLOW}========================================"
echo "Chapter 8 语句正确性测试"
echo "========================================${NC}"

test_interpreter "print 语句" "$PROJECT_ROOT/tests/ch8_print.lox"

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
