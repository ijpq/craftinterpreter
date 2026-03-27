# 开发环境设置

## 初次clone后的设置

```bash
# 1. 安装Git hooks（自动格式化）
./scripts/install-hooks.sh

# 2. 安装格式化工具
brew install clang-format google-java-format

# 3. 编译项目
./build_mac.sh
```

## Git Hooks说明

项目使用pre-commit hook在提交时自动格式化代码：
- **C++文件**：使用`clang-format`（配置见`.clang-format`）
- **Java文件**：使用`google-java-format`

**工作流程**：
```bash
# 正常开发
vim csrc/lox.cpp

# 正常add和commit
git add csrc/lox.cpp
git commit -m "update"
# ↑ commit时自动格式化并重新add
```

## 目录结构

```
craftinginterpreter/
├── scripts/
│   ├── pre-commit           # Git hook脚本（模板）
│   └── install-hooks.sh     # Hook安装脚本
├── .clang-format            # C++格式化配置
├── build_mac.sh             # 编译脚本
└── tests/
    └── unit_tests.sh        # 测试脚本
```
