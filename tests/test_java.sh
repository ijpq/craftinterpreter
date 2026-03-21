#!/bin/bash

# 简单的 Java 测试 - 调试 ClassNotFoundException

echo "=== Java 环境测试 ==="
echo ""

# 获取项目根目录
PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$PROJECT_ROOT"

echo "1. 当前目录: $(pwd)"
echo ""

echo "2. 检查 lox 目录:"
ls -la lox/*.class 2>/dev/null | head -5
echo ""

echo "3. 检查 package 声明:"
head -1 lox/Lox.java
echo ""

echo "4. 尝试运行 Java (方法 1 - 使用 -cp):"
echo "var x = 1;" | java -cp lox lox.Lox 2>&1 | head -3
echo ""

echo "5. 尝试运行 Java (方法 2 - 设置 CLASSPATH):"
export CLASSPATH=lox
echo "var x = 1;" | java lox.Lox 2>&1 | head -3
echo ""

echo "6. Java 版本:"
java -version 2>&1 | head -1
echo ""

echo "=== 测试完成 ==="
