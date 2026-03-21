# 管道输入问题解决方案

## 问题症状

使用管道输入测试时,Java 输出缺少第一个 token:

```bash
echo "var a = 1;" | java -cp . lox.Lox 2>&1 | grep -v "^>"

# 输出:
IDENTIFIER a null    # ✗ 缺少 VAR token
EQUAL = null
NUMBER 1 1.0
SEMICOLON ; null
EOF  null
```

## 问题根源

### 1. Java 代码逻辑

`Lox.java` 的 `main` 方法:
```java
public static void main(String[] args) throws IOException {
    if (args.length > 1) {
        System.out.println("Usage: jlox [script]");
        System.exit(64);
    } else if (args.length == 1) {
        runFile(args[0]);    // 文件输入
    } else {
        runPrompt();          // 交互模式
    }
}
```

### 2. 管道触发交互模式

使用管道时 `args.length == 0`,进入 `runPrompt()`:

```java
private static void runPrompt() throws IOException {
    InputStreamReader input = new InputStreamReader(System.in);
    BufferedReader reader = new BufferedReader(input);

    for (;;) {
        System.out.print("> ");  // ← 输出提示符
        String line = reader.readLine();
        if (line == null) break;
        run(line);
        hadError = false;
    }
}
```

### 3. 提示符与第一个 token 同行

实际输出:
```
> VAR var null       ← 提示符和第一个 token 在同一行!
IDENTIFIER a null
EQUAL = null
...
```

### 4. grep 过滤掉整行

```bash
grep -v "^>"  # 过滤掉以 ">" 开头的行
```

结果:`> VAR var null` 整行被删除。

## 解决方案

### 方案 1: 使用文件输入(推荐) ✅

```bash
# 创建临时文件
echo "var a = 1;" > /tmp/test.lox

# 使用文件参数(触发 runFile 而非 runPrompt)
java -cp . lox.Lox /tmp/test.lox
```

**优点**:
- 不触发交互模式,无提示符
- 输出干净,无需过滤
- 与实际使用场景一致

### 方案 2: 修改 grep 模式(不推荐)

```bash
# 复杂且脆弱
echo "var a = 1;" | java -cp . lox.Lox | sed '1s/^> //'
```

**缺点**:
- 需要额外的文本处理
- 依赖输出格式假设
- 易出错

## 修复后的测试脚本

所有测试脚本已更新为使用文件输入:

```bash
# quick_test.sh
TEST_FILE="/tmp/scanner_test_$$.lox"
echo "var a = 1;" > "$TEST_FILE"
java -cp . lox.Lox "$TEST_FILE"
```

## 测试验证

```bash
cd /Users/ketang/craftinginterpreter
./tests/quick_test.sh
```

**预期输出**:
```
[3/4] 测试 Java Scanner...
VAR var null         # ✓ 完整输出
IDENTIFIER a null
EQUAL = null
NUMBER 1 1.0
SEMICOLON ; null
EOF  null
✓ Java Scanner 正常工作
```

## 技术要点

| 输入方式 | args.length | 调用方法 | 有提示符? |
|----------|-------------|----------|-----------|
| `java Lox file.lox` | 1 | `runFile()` | 否 |
| `echo "..." \| java Lox` | 0 | `runPrompt()` | 是 |

**关键**: 使用文件参数确保调用 `runFile()`,避免交互模式的提示符问题。

## 参考文档

- [Crafting Interpreters - REPL](https://craftinginterpreters.com/a-map-of-the-territory.html#the-parts-of-a-language) - 交互式解释器设计
- [Java BufferedReader](https://docs.oracle.com/javase/8/docs/api/java/io/BufferedReader.html) - 输入流处理
