#!/bin/bash

# 诊断脚本 - 使用文件输入版本

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}测试环境诊断${NC}"
echo -e "${BLUE}========================================${NC}\n"

# 获取项目根目录的绝对路径
PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$PROJECT_ROOT"

ERRORS=0

# 检查当前目录
echo -e "${YELLOW}[1] 检查项目根目录${NC}"
echo "项目根目录: $PROJECT_ROOT"
if [[ "$PROJECT_ROOT" == *"craftinginterpreter"* ]]; then
    echo -e "${GREEN}✓ 目录正确${NC}\n"
else
    echo -e "${RED}✗ 警告: 不在 craftinginterpreter 目录中${NC}\n"
    ((ERRORS++))
fi

# 检查 Java
echo -e "${YELLOW}[2] 检查 Java 安装${NC}"
if command -v java &> /dev/null; then
    java -version 2>&1 | head -1
    echo -e "${GREEN}✓ Java 已安装${NC}\n"
else
    echo -e "${RED}✗ Java 未安装${NC}\n"
    ((ERRORS++))
fi

# 检查 javac
echo -e "${YELLOW}[3] 检查 Java 编译器${NC}"
if command -v javac &> /dev/null; then
    javac -version 2>&1
    echo -e "${GREEN}✓ javac 已安装${NC}\n"
else
    echo -e "${RED}✗ javac 未安装${NC}\n"
    ((ERRORS++))
fi

# 检查 g++
echo -e "${YELLOW}[4] 检查 C++ 编译器${NC}"
if command -v g++ &> /dev/null; then
    g++ --version | head -1
    echo -e "${GREEN}✓ g++ 已安装${NC}\n"
else
    echo -e "${RED}✗ g++ 未安装${NC}\n"
    ((ERRORS++))
fi

# 检查目录结构
echo -e "${YELLOW}[5] 检查项目目录结构${NC}"
DIRS=("lox" "csrc" "tests" "build")
for dir in "${DIRS[@]}"; do
    if [ -d "$PROJECT_ROOT/$dir" ]; then
        echo -e "${GREEN}✓${NC} $dir/ 存在"
    else
        echo -e "${RED}✗${NC} $dir/ 缺失"
        ((ERRORS++))
    fi
done
echo ""

# 检查 Java 源文件
echo -e "${YELLOW}[6] 检查 Java 源文件${NC}"
JAVA_FILES=("Lox.java" "Scanner.java" "Token.java" "TokenType.java")
for file in "${JAVA_FILES[@]}"; do
    if [ -f "$PROJECT_ROOT/lox/$file" ]; then
        echo -e "${GREEN}✓${NC} lox/$file"
    else
        echo -e "${RED}✗${NC} lox/$file 缺失"
        ((ERRORS++))
    fi
done
echo ""

# 检查 C++ 源文件
echo -e "${YELLOW}[7] 检查 C++ 源文件${NC}"
CPP_FILES=("csrc/scanner.cpp" "csrc/token.cpp" "csrc/include/scanner.h" "csrc/include/token.h")
for file in "${CPP_FILES[@]}"; do
    if [ -f "$PROJECT_ROOT/$file" ]; then
        echo -e "${GREEN}✓${NC} $file"
    else
        echo -e "${RED}✗${NC} $file 缺失"
        ((ERRORS++))
    fi
done
echo ""

# 检查 Java 编译
echo -e "${YELLOW}[8] 检查 Java 编译状态${NC}"
if [ -f "$PROJECT_ROOT/lox/Lox.class" ] && [ -f "$PROJECT_ROOT/lox/Scanner.class" ]; then
    echo -e "${GREEN}✓ Java 类已编译${NC}\n"
else
    echo -e "${YELLOW}! Java 类未编译或需要重新编译${NC}"
    echo "  运行: cd $PROJECT_ROOT/lox && javac *.java"
    echo ""
fi

# 检查 C++ 编译
echo -e "${YELLOW}[9] 检查 C++ 编译状态${NC}"
if [ -f "$PROJECT_ROOT/build/craftinginterpreter" ]; then
    echo -e "${GREEN}✓ C++ 程序已编译${NC}\n"
else
    echo -e "${YELLOW}! C++ 程序未编译或需要重新编译${NC}"
    echo "  运行: cd $PROJECT_ROOT && ./build_mac.sh"
    echo ""
fi

# 检查测试脚本权限
echo -e "${YELLOW}[10] 检查测试脚本权限${NC}"
TEST_SCRIPTS=("tests/quick_test.sh" "tests/unit_tests.sh" "tests/run_tests.sh")
for script in "${TEST_SCRIPTS[@]}"; do
    if [ -x "$PROJECT_ROOT/$script" ]; then
        echo -e "${GREEN}✓${NC} $script 可执行"
    else
        echo -e "${YELLOW}!${NC} $script 不可执行"
        echo "  运行: chmod +x $PROJECT_ROOT/$script"
    fi
done
echo ""

# 快速功能测试 - 使用文件输入
echo -e "${YELLOW}[11] 快速功能测试${NC}"
if [ -f "$PROJECT_ROOT/lox/Lox.class" ] && [ -f "$PROJECT_ROOT/build/craftinginterpreter" ]; then
    echo "测试输入: var x = 1;"
    
    # 创建临时测试文件
    TEST_FILE="/tmp/diagnose_test_$$.lox"
    echo "var x = 1;" > "$TEST_FILE"
    
    # 测试 Java
    java_result=$(cd "$PROJECT_ROOT" && java -cp . lox.Lox "$TEST_FILE" 2>&1 | head -1)
    if [[ "$java_result" == "VAR"* ]]; then
        echo -e "${GREEN}✓${NC} Java Scanner: $java_result"
    else
        echo -e "${RED}✗${NC} Java Scanner 异常: $java_result"
        ((ERRORS++))
    fi
    
    # 测试 C++
    cpp_result=$(cd "$PROJECT_ROOT" && ./build/craftinginterpreter "$TEST_FILE" 2>&1 | head -1)
    if [[ "$cpp_result" == "VAR"* ]]; then
        echo -e "${GREEN}✓${NC} C++ Scanner: $cpp_result"
    else
        echo -e "${RED}✗${NC} C++ Scanner 异常: $cpp_result"
        ((ERRORS++))
    fi
    
    # 清理
    rm -f "$TEST_FILE"
else
    echo -e "${YELLOW}! 跳过功能测试 (需要先编译)${NC}"
fi
echo ""

# 总结
echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}诊断总结${NC}"
echo -e "${BLUE}========================================${NC}"

if [ $ERRORS -eq 0 ]; then
    echo -e "${GREEN}✓ 所有检查通过! 环境配置正常${NC}"
    echo -e "\n可以运行测试:"
    echo "  cd $PROJECT_ROOT"
    echo "  ./tests/quick_test.sh"
    exit 0
else
    echo -e "${RED}✗ 发现 $ERRORS 个问题${NC}"
    echo -e "\n请根据上述提示修复问题后重试"
    exit 1
fi
