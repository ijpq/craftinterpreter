#!/bin/bash

# 调试脚本 - 详细检查 Java 类路径问题

echo "=== Java 类路径调试 ==="
echo ""

PROJECT_ROOT="/Users/ketang/craftinginterpreter"
cd "$PROJECT_ROOT"

echo "1. 当前工作目录:"
pwd
echo ""

echo "2. 列出 lox 目录内容:"
ls -la lox/*.class 2>/dev/null | grep -E "(Lox|Scanner|Token)" | head -5
echo ""

echo "3. 检查 Lox.class 文件:"
file lox/Lox.class 2>&1
echo ""

echo "4. 使用 javap 检查类信息:"
javap -cp lox lox.Lox 2>&1 | head -3
echo ""

echo "5. 尝试不同的运行方式:"

echo "  方式 1: java -cp lox lox.Lox"
echo "var x=1;" | java -cp lox lox.Lox 2>&1 | head -3
echo ""

echo "  方式 2: java -classpath lox lox.Lox"  
echo "var x=1;" | java -classpath lox lox.Lox 2>&1 | head -3
echo ""

echo "  方式 3: CLASSPATH=lox java lox.Lox"
export CLASSPATH=lox
echo "var x=1;" | java lox.Lox 2>&1 | head -3
unset CLASSPATH
echo ""

echo "  方式 4: 从 lox 目录内运行(错误示范)"
cd lox
echo "var x=1;" | java Lox 2>&1 | head -3
cd ..
echo ""

echo "6. 检查 JAVA_HOME:"
echo "JAVA_HOME=$JAVA_HOME"
echo ""

echo "7. Java 版本详细信息:"
java -version 2>&1
echo ""

echo "=== 调试完成 ==="
