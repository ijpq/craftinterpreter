# Scanner 测试套件

C++ Scanner 与 Java 参考实现一致性测试。

## 一键测试 ⚡

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

## 测试脚本

| 脚本 | 用途 | 时间 |
|------|------|------|
| `quick_test.sh` | ⚡ 快速验证 | ~5秒 |
| `unit_tests.sh` | 🧪 24个单元测试 | ~30秒 |
| `run_tests.sh` | 📋 完整集成测试 | ~10秒 |
| `diagnose.sh` | 🔍 环境诊断 | ~2秒 |

---

## 关键技术修复

### ✅ 已解决的问题

1. **Java 类路径**: `java -cp . lox.Lox` (不是 `-cp lox`)
2. **管道输入**: 使用文件输入避免交互模式提示符干扰
3. **内存安全**: 修复 C++ 悬空指针问题
4. **Token 格式**: 输出 `VAR` 而非 `25`

详细说明:
- `tests/CLASSPATH_FIX.md` - Java 类路径问题
- `tests/PIPE_INPUT_FIX.md` - 管道输入问题

---

## 快速排错

运行诊断:
```bash
./tests/diagnose.sh
```

常见问题:
- **ClassNotFoundException**: 参见 `CLASSPATH_FIX.md`
- **输出缺少第一个 token**: 参见 `PIPE_INPUT_FIX.md`
- **illegal instruction**: 已修复(悬空指针)

---

## 测试覆盖

✅ 所有 TokenType (单/双字符运算符、关键字、字面量)  
✅ 数字(整数、浮点、小数点处理)  
✅ 字符串(包括多行)  
✅ 标识符与关键字区分  
✅ 注释处理  
✅ 错误处理  

---

## 文档

- `README.md` - 本文件(快速入门)
- `USAGE.md` - 详细使用指南
- `CLASSPATH_FIX.md` - Java 类路径问题解决
- `PIPE_INPUT_FIX.md` - 管道输入问题解决

---

**参考**: [Crafting Interpreters - Scanning](https://craftinginterpreters.com/scanning.html)
