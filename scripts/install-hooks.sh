#!/bin/bash

# 安装Git hooks到.git/hooks/目录

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
HOOKS_DIR="$PROJECT_ROOT/.git/hooks"

echo "安装Git hooks..."

# 确保.git/hooks目录存在
if [ ! -d "$HOOKS_DIR" ]; then
    echo "错误: .git/hooks目录不存在。请确保在git仓库中运行此脚本。"
    exit 1
fi

# 安装pre-commit hook
echo "  - 安装pre-commit hook"
cp "$SCRIPT_DIR/pre-commit" "$HOOKS_DIR/pre-commit"
chmod +x "$HOOKS_DIR/pre-commit"

echo "Git hooks安装完成！"
echo ""
echo "注意: 需要安装格式化工具："
echo "  brew install clang-format google-java-format"
