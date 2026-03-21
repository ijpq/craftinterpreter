#!/bin/bash

# 调试输出 - 查看原始输出

PROJECT_ROOT="/Users/ketang/craftinginterpreter"
cd "$PROJECT_ROOT"

echo "=== 原始输出对比 ==="
echo ""

echo "Java 原始输出(无过滤):"
echo "var a = 1;" | java -cp . lox.Lox 2>&1
echo ""

echo "Java 输出(grep -v '^>'):"
echo "var a = 1;" | java -cp . lox.Lox 2>&1 | grep -v "^>"
echo ""

echo "C++ 原始输出(无过滤):"
echo "var a = 1;" | ./build/craftinginterpreter 2>&1
echo ""

echo "C++ 输出(grep -v '^>'):"
echo "var a = 1;" | ./build/craftinginterpreter 2>&1 | grep -v "^>"
