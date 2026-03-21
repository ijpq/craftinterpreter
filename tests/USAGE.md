# Scanner 测试套件使用指南

## 快速开始

### 运行快速测试

```bash
cd /Users/ketang/craftinginterpreter
chmod +x tests/*.sh
./tests/quick_test.sh
```

**预期输出**:
```
快速验证测试

[1/4] 编译 Java...
✓ Java 编译成功

[2/4] 编译 C++...
✓ C++ 编译成功

[3/4] 测试 Java Scanner...
VAR var null
IDENTIFIER a null
EQUAL = null
NUMBER 1 1.0
SEMICOLON ; null
EOF  null
✓ Java Scanner 正常工作

[4/4] 测试 C++ Scanner...
VAR var null
IDENTIFIER a null
EQUAL = null
NUMBER 1 1.0
SEMICOLON ; null
EOF  null
✓ C++ Scanner 正常工作

对比输出...
✓✓✓ 完美! Java 和 C++ 输出完全一致!
```

---

## Java 类路径说明

### 正确的运行方式

**从项目根目录运行**:
```bash
cd /Users/ketang/craftinginterpreter
java -cp lox lox.Lox
```

**错误示例** (会失败):
```bash
cd /Users/ketang/craftinginterpreter/lox
java lox.Lox  # ✗ ClassNotFoundException
```

**原因**: 
- Java package `lox` 要求 `.class` 文件在 `lox/` 子目录中
- 必须从包含 `lox/` 目录的父目录运行
- 使用 `-cp lox` 告诉 Java 在 `lox/` 目录中查找类

---

## 测试脚本说明

### 1. `quick_test.sh` - 快速验证 ⚡
```bash
./tests/quick_test.sh
```
- **用途**: 快速检查编译和基本功能
- **时间**: ~5 秒
- **特点**: 彩色输出,直观显示每个步骤

### 2. `unit_tests.sh` - 单元测试 🧪
```bash
./tests/unit_tests.sh
```
- **用途**: 24 个独立测试用例
- **时间**: ~30 秒
- **输出示例**:
```
测试: 整数
输入: 123;
✓ PASS

测试: 浮点数
输入: 123.456;
✓ PASS

========================================
测试总结
========================================
通过: 24
失败: 0
```

### 3. `run_tests.sh` - 完整集成测试 📋
```bash
./tests/run_tests.sh
```
- **用途**: 完整测试文件对比
- **时间**: ~10 秒
- **特点**: 生成详细 diff 报告

---

## 常见问题排查

### 问题 1: ClassNotFoundException

**错误**:
```
Error: Could not find or load main class lox.Lox
Caused by: java.lang.ClassNotFoundException: lox.Lox
```

**原因**: Java 类路径设置错误

**解决方案**:
```bash
# ✓ 正确 - 从项目根目录运行
cd /Users/ketang/craftinginterpreter
java -cp lox lox.Lox

# ✗ 错误 - 在 lox 目录内
cd /Users/ketang/craftinginterpreter/lox
java lox.Lox
```

### 问题 2: 编译错误 "cannot find symbol"

**原因**: 只编译了部分 Java 文件

**解决**:
```bash
cd /Users/ketang/craftinginterpreter/lox
javac *.java  # 编译所有文件
```

### 问题 3: C++ illegal instruction

**原因**: 旧版代码的悬空指针问题

**解决**: 使用最新代码(已修复)
```bash
./build_mac.sh  # 重新编译
```

---

## 手动测试

### 测试 Java:
```bash
cd /Users/ketang/craftinginterpreter
cd lox && javac *.java && cd ..
java -cp lox lox.Lox
> var a = 1;
VAR var null
IDENTIFIER a null
EQUAL = null
NUMBER 1 1.0
SEMICOLON ; null
EOF  null
```

### 测试 C++:
```bash
cd /Users/ketang/craftinginterpreter
./build_mac.sh
./build/craftinginterpreter
> var a = 1;
VAR var null
IDENTIFIER a null
EQUAL = null
NUMBER 1 1.0
SEMICOLON ; null
EOF  null
```

### 验证一致性:
输出应该**逐行完全相同**。

---

## 测试用例覆盖

| 类型 | 示例 | 说明 |
|------|------|------|
| 整数 | `123;` | NUMBER token |
| 浮点数 | `123.456;` | 小数点处理 |
| 字符串 | `"hello";` | 引号内容 |
| 标识符 | `myVar;` | 变量名 |
| 关键字 | `var if while` | 保留字 |
| 运算符 | `+ - * / != ==` | 单/双字符 |
| 注释 | `// comment` | 忽略直到行尾 |
| 复杂表达式 | `(a+b)*c` | 嵌套括号 |

---

## 开发工作流

### 修改 Scanner 后的测试流程:

1. **修改代码**
   ```bash
   vim csrc/scanner.cpp
   ```

2. **快速验证**
   ```bash
   ./tests/quick_test.sh
   ```

3. **完整测试** (如果快速测试通过)
   ```bash
   ./tests/unit_tests.sh
   ```

4. **调试失败** (如果测试失败)
   ```bash
   # 查看详细差异
   ./tests/run_tests.sh
   cat tests/diff.txt
   ```

---

## 技术细节

### Java Package 结构
```
craftinginterpreter/
├── lox/                  # Package 目录
│   ├── Lox.java         # package lox;
│   ├── Scanner.java     # package lox;
│   ├── Token.java       # package lox;
│   ├── Lox.class        # 编译后
│   └── Scanner.class    # 编译后
└── (从这里运行)          # java -cp lox lox.Lox
```

### C++ Include 结构
```
craftinginterpreter/
├── csrc/
│   ├── include/
│   │   ├── scanner.h
│   │   └── token.h
│   ├── scanner.cpp
│   └── token.cpp
└── build/
    └── craftinginterpreter
```

---

## 参考文档

- [Java Classpath](https://docs.oracle.com/javase/tutorial/essential/environment/paths.html) - Java 类路径详解
- [Crafting Interpreters - Scanning](https://craftinginterpreters.com/scanning.html) - 原书章节
- [GNU diff manual](https://www.gnu.org/software/diffutils/manual/html_node/index.html) - diff 命令用法
