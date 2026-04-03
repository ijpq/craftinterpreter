#!/bin/bash

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
FAILED=0

# ========================================
# 构建
# ========================================

echo -e "${YELLOW}========================================
构建 C++
========================================${NC}"

cd "$PROJECT_ROOT"
if [[ "$(uname)" == "Darwin" ]]; then
    BUILD_SCRIPT="./build_mac.sh"
else
    BUILD_SCRIPT="./build_ubuntu.sh"
fi

if ! $BUILD_SCRIPT; then
    echo -e "${RED}C++ 编译失败${NC}"
    exit 1
fi

echo -e "\n${YELLOW}========================================
构建 Java
========================================${NC}"

rm -f "$PROJECT_ROOT/lox"/*.class
cd "$PROJECT_ROOT/lox"
if ! javac *.java; then
    echo -e "${RED}Java 编译失败${NC}"
    exit 1
fi
cd "$PROJECT_ROOT"

echo -e "\n${GREEN}编译完成${NC}"

# ========================================
# 运行 Java Ground Truth 测试
# ========================================

echo -e "\n${YELLOW}========================================
Java Ground Truth Tests
========================================${NC}"

cd "$PROJECT_ROOT"
if java -cp "$PROJECT_ROOT" lox.InterpreterTest; then
    echo -e "${GREEN}Java 测试通过 ✓${NC}"
else
    echo -e "${RED}Java 测试失败 ✗${NC}"
    ((FAILED++))
fi

# ========================================
# 运行 GTest
# ========================================

run_gtest() {
    local name="$1"
    local bin="$2"

    echo -e "\n${YELLOW}========================================
$name
========================================${NC}"

    if [ ! -f "$bin" ]; then
        echo -e "${RED}✗ $bin 未找到，构建可能失败${NC}"
        ((FAILED++))
        return
    fi

    if [[ "$(uname)" == "Darwin" ]]; then
        DYLD_LIBRARY_PATH="$PROJECT_ROOT/build" "$bin"
    else
        LD_LIBRARY_PATH="$PROJECT_ROOT/build" "$bin"
    fi

    if [ $? -ne 0 ]; then
        ((FAILED++))
    fi
}

run_gtest "Interpreter Unit Tests"  "$PROJECT_ROOT/build/interpreter_test"
run_gtest "Lox Integration Tests"   "$PROJECT_ROOT/build/lox_integration_test"

# ========================================
# 总结
# ========================================

echo -e "\n${YELLOW}========================================"
echo "总结"
echo "========================================${NC}"

if [ $FAILED -eq 0 ]; then
    echo -e "${GREEN}所有测试通过 ✓${NC}"
    exit 0
else
    echo -e "${RED}${FAILED} 个测试套件失败 ✗${NC}"
    exit 1
fi
