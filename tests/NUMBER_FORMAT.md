# 数字格式化实现

## 问题

Java 和 C++ 的数字格式化不一致:

```
Java:  NUMBER 1 1.0
C++:   NUMBER 1 1.000000  ✗
```

## Java 行为

Java 的 `Double.toString()` 实现:
- 整数值: `1.0` → `"1.0"`, `42.0` → `"42.0"`
- 小数值: `3.14` → `"3.14"`, `0.5` → `"0.5"`
- 去除无意义的尾随零: `1.5000` → `"1.5"`

**示例**:
```java
Double.toString(1.0)      // "1.0"
Double.toString(123.456)  // "123.456"
Double.toString(0.5)      // "0.5"
Double.toString(42.0)     // "42.0"
```

## C++ 实现

### 方案 1: 使用 std::to_string (简单但不精确)

```cpp
std::to_string(1.0)      // "1.000000"  ✗
std::to_string(123.456)  // "123.456000" ✗
```

**缺点**: 总是输出 6 位小数,产生无意义的尾随零

### 方案 2: 智能格式化 (推荐) ✅

```cpp
std::string double_to_string(double value) {
  // 1. 检查是否是整数值
  if (std::floor(value) == value && std::isfinite(value)) {
    // 整数: 格式化为 "x.0"
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << value;
    return oss.str();
  }
  
  // 2. 小数: 使用默认格式,然后去除尾随零
  std::ostringstream oss;
  oss << value;
  std::string result = oss.str();
  
  // 3. 去除尾随零(但至少保留一位小数)
  if (result.find('.') != std::string::npos) {
    while (result.length() > 1 && result.back() == '0' && 
           result[result.length() - 2] != '.') {
      result.pop_back();
    }
  }
  
  return result;
}
```

## 实现细节

### 1. 整数检测

```cpp
std::floor(value) == value && std::isfinite(value)
```

- `std::floor(value) == value`: 检查是否等于向下取整
- `std::isfinite(value)`: 排除 NaN 和 Infinity

**测试**:
```cpp
1.0   → true   // 整数
1.5   → false  // 小数
42.0  → true   // 整数
3.14  → false  // 小数
```

### 2. 整数格式化

```cpp
oss << std::fixed << std::setprecision(1) << value;
```

- `std::fixed`: 固定小数点格式(非科学计数法)
- `std::setprecision(1)`: 1 位小数

**输出**: `1.0`, `42.0`, `100.0`

### 3. 小数格式化

```cpp
oss << value;  // 使用默认格式
```

默认格式会输出有意义的位数,但可能包含尾随零。

### 4. 去除尾随零

```cpp
while (result.length() > 1 && result.back() == '0' && 
       result[result.length() - 2] != '.') {
  result.pop_back();
}
```

**逻辑**:
- `result.back() == '0'`: 最后一个字符是 '0'
- `result[result.length() - 2] != '.'`: 倒数第二个不是小数点
- 保证至少保留 "x.0" 的格式

**示例**:
```
"1.50000" → 去除 '0' → "1.5"
"3.14000" → 去除 '0' → "3.14"
"1.0"     → 不去除(倒数第二个是 '.') → "1.0"
```

## 测试用例

| 输入 | Java 输出 | C++ 输出 | 匹配? |
|------|-----------|----------|-------|
| `1.0` | `1.0` | `1.0` | ✅ |
| `42.0` | `42.0` | `42.0` | ✅ |
| `0.5` | `0.5` | `0.5` | ✅ |
| `3.14` | `3.14` | `3.14` | ✅ |
| `123.456` | `123.456` | `123.456` | ✅ |

## 边界情况

### NaN 和 Infinity

```cpp
std::isfinite(value)  // 排除 NaN 和 Infinity
```

Java 行为:
```java
Double.toString(Double.NaN)       // "NaN"
Double.toString(Double.POSITIVE_INFINITY)  // "Infinity"
```

C++ 默认格式会自动处理:
```cpp
std::ostringstream oss;
oss << std::numeric_limits<double>::quiet_NaN();  // "nan"
oss << std::numeric_limits<double>::infinity();    // "inf"
```

### 科学计数法

极大或极小的数字:
```
1e20  → Java: "1.0E20"
1e-20 → Java: "1.0E-20"
```

C++ 默认 ostringstream 会自动切换到科学计数法。

## 性能考虑

- `std::ostringstream`: 相对较慢,但对于 Scanner(运行一次)可接受
- 可选优化: 使用 `snprintf` + 手动解析,但增加复杂度

## 参考文档

- [Java Double.toString()](https://docs.oracle.com/javase/8/docs/api/java/lang/Double.html#toString-double-) - Java 官方文档
- [std::fixed](https://en.cppreference.com/w/cpp/io/manip/fixed) - 固定小数点格式
- [std::setprecision](https://en.cppreference.com/w/cpp/io/manip/setprecision) - 精度控制
- [std::floor](https://en.cppreference.com/w/cpp/numeric/math/floor) - 向下取整
