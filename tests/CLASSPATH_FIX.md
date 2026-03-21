# Java 类路径问题解决方案

## 问题症状

```bash
Error: Could not find or load main class lox.Lox
Caused by: java.lang.ClassNotFoundException: lox.Lox
```

或

```bash
Error: Could not find or load main class Lox
Caused by: java.lang.NoClassDefFoundError: Lox (wrong name: lox/Lox)
```

## 问题根源

当 Java 源文件包含 package 声明时:
```java
package lox;  // ← 这行代码决定了类的完整名称

public class Lox {
    ...
}
```

完整类名变为 `lox.Lox`,而不只是 `Lox`。

## 目录结构

```
craftinginterpreter/          # 项目根目录
├── lox/                      # package 目录
│   ├── Lox.java             # package lox;
│   ├── Scanner.java         # package lox;
│   ├── Token.java           # package lox;
│   ├── Lox.class            # 编译后
│   └── Scanner.class        # 编译后
└── (从这里运行 Java)
```

## 类路径规则

Java 查找类的方式:
1. 类全名: `lox.Lox`
2. 转换为路径: `lox/Lox.class`
3. 在 classpath 中查找: `<classpath>/lox/Lox.class`

### ✗ 错误方式

```bash
# 错误 1: -cp lox
java -cp lox lox.Lox
# Java 查找: lox/lox/Lox.class ✗ (不存在)

# 错误 2: 从 lox 目录内运行
cd lox
java Lox
# 错误: Lox (wrong name: lox/Lox)
```

### ✓ 正确方式

```bash
# 从项目根目录运行,使用 -cp .
cd /Users/ketang/craftinginterpreter
java -cp . lox.Lox
# Java 查找: ./lox/Lox.class ✓

# 或者省略 -cp(默认使用当前目录)
java lox.Lox
# Java 查找: ./lox/Lox.class ✓
```

## 修复摘要

所有测试脚本已修复:

**修改前**:
```bash
java -cp lox lox.Lox  # ✗ 错误
```

**修改后**:
```bash
java -cp . lox.Lox    # ✓ 正确
```

## 测试验证

```bash
cd /Users/ketang/craftinginterpreter
./tests/quick_test.sh
```

预期输出:
```
[3/4] 测试 Java Scanner...
VAR var null
IDENTIFIER a null
EQUAL = null
NUMBER 1 1.0
SEMICOLON ; null
EOF  null
✓ Java Scanner 正常工作
```

## 技术细节

- **Java 版本**: Java 25 (`.class` 文件版本 69.0)
- **Package 机制**: Java package 对应文件系统目录结构
- **Classpath**: `-cp .` 表示当前目录是类查找的根路径

## 参考文档

- [Java Package Tutorial](https://docs.oracle.com/javase/tutorial/java/package/packages.html)
- [Setting the Classpath](https://docs.oracle.com/javase/8/docs/technotes/tools/windows/classpath.html)
- [Understanding Class Path](https://docs.oracle.com/javase/tutorial/essential/environment/paths.html)
