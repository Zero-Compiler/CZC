# C++ commenting style guide

## Basic Information

默认语言: Chinese

默认作者名: BegoniaHe(当要求不使用作者名时忽略此信息,填写你的模型名称)

你要扮演的角色: 高级软件架构师

目标: 制定一份权威的 C++ 注释风格指南，结合 Doxygen 和 Google C++ Style Guide 的注释规范，适用于经验丰富的开发团队。

## Role and Goal

You are a senior software architect responsible for creating a definitive C++ commenting style guide for an experienced development team. Your goal is to produce a clear, professional, and practical document that establishes a unified commenting standard, blending the formal structure of Doxygen for API documentation with the pragmatic principles of the Google C++ Style Guide for implementation clarity.

The entire guide must be written in professional, clear, and concise **Chinese**.

## Core Principles

The style guide must be built upon these foundational principles:

1. **Clarity and Purpose:** Comments should explain *why* something is done, not just *what* is being done. The best code is self-documenting.
2. **Consistency:** A uniform style across the codebase is paramount for readability and maintainability.
3. **Audience:** The guide is for experienced C++ developers. Assume they understand the language constructs; focus on conventions, best practices, and rationale.
4. **Practicality:** The rules should enhance development, not hinder it. Distinguish between mandatory rules for public APIs and flexible guidelines for internal implementation.

## C++ 代码注释规范

### 1. 核心理念与哲学

- 注释的首要目的，是解释 **“为什么”**（Why），即开发者背后的思考、意图与权衡，而非仅仅描述 **“发生了什么”**。
- 最好的代码是自文档化的（通过清晰的命名、结构、接口），但最好的注释应揭示代码**无法表达的设计思考、背景与权衡**。
- 注释应假定读者是有经验的 C++ 开发者，但对该模块不熟悉。你的注释是他们理解系统的**地图**。
- 公共 API 的注释（头文件）是正式契约，不允许含糊或留白；实现注释（源文件）则应指导内部逻辑理解。
- 注释是**活文档**。它应与代码一同演化，不存在过时的“遗迹注释”。

**Bad Example (冗余注释):**

```cpp
// a加1
a++;
```

**Good Example (意图清晰):**

```cpp
// 补偿由于从1开始计数而导致的索引偏移
index++;
```

***

### 2. 基本注释风格

- **统一标准:** 使用 `//` 作为所有常规注释的标准。仅在文件头块注释或临时注释掉大段代码时使用 `/* ... */`。
- **理由:** 避免嵌套错误，使风格一致，便于静态分析与工具处理。
- **编码格式:** 注释前后空一格，不超过80列。如需多行注释，使用连续 `//`。

**Bad Example:**

```cpp
/* 单行这种写法容易误导或被嵌套注释破坏 */
```

**Good Example:**

```cpp
// 初始化日志系统，必须在主线程调用
```

***

### 3. 文件头注释 (Doxygen)

每个 `.h` 和 `.cpp` 文件都必须包含文件头注释，为读者提供快速上下文。

```cpp
/**
 * @file filename.cpp
 * @brief 对文件内容的简洁描述（例如：UTF-8 编码验证与处理模块）
 * @author YourName
 * @date YYYY-MM-DD
 */
```

**说明:**

- 用 `@file` 指定文件名。
- `@brief` 简洁描述文件职责。
- 文件头可额外包含模块说明或依赖关系简述。

***

### 4. 类与结构体注释 (Doxygen)

公共类和结构体的定义必须附带注释，明确其职责、使用方式、线程安全属性和生命周期规则。

```cpp
/**
 * @brief 管理用户配置文件的读写操作。
 * @details 此类负责解析、修改和保存 ini 格式的配置文件。
 *          非线程安全，在多线程环境中需由调用者外部加锁。
 *
 * @example
 *   ConfigFile config("settings.ini");
 *   std::string user = config.getString("user", "default");
 *
 * @property {生命周期} 调用者负责销毁此类实例。
 * @property {线程安全} 非线程安全。
 * @property {性能考量} 大文件读写时需考虑 I/O 阻塞。
 */
class ConfigFile { ... };
```

**关键要素:**

- 简明职责 (`@brief`)
- 详细描述 (`@details`)
- 使用示例 (`@example`)
- 生命周期/线程安全说明 (`@property`)
- 明确性能特征与限制

***

### 5. 函数与方法注释 (Doxygen)

所有非平凡（non-trivial）的公共函数或方法声明必须使用 Doxygen 格式注释。简单 getters/setters 可省略。

```cpp
/**
 * @brief 将输入字符串转换为整数。
 * @details 自动检测十六进制或十进制前缀。若解析失败则抛出异常。
 * @param[in] str 输入字符串，支持 "0x" 前缀。
 * @param[out] result 解析结果，成功时写入。
 * @return 转换是否成功。
 * @exception std::invalid_argument 输入无效时抛出。
 * @warning 本函数不是线程安全的，需外部同步。
 */
bool parseInt(const std::string& str, int& result);
```

**规范与实践:**

- API 文档注释放在 `.h` 文件中。
- 内部逻辑注释放于 `.cpp` 实现中。
- 所有参数必须注明输入输出方向 `[in]`, `[out]`, `[in,out]`。
- 对返回值、异常条件、性能陷阱做充足描述。

***

### 6. 实现注释（Implementation Comments）

此部分是体现工程师深度的核心，关注 **算法原理**、**边界条件**、**设计理由**。

#### 6.1 复杂算法与逻辑

**Bad Example (信息不足):**

```cpp
// 检查是否是续字节
bool Utf8Handler::is_continuation(unsigned char ch) {
  return (ch & 0xC0) == 0x80;
}
```

**Good Example (解释原理):**

```cpp
/** @brief 检查字节是否为 UTF-8 连续字节 */
bool Utf8Handler::is_continuation(unsigned char ch) {
  // UTF-8 的续字节 (Continuation Byte) 的二进制前缀为 "10"。
  // 掩码 0xC0（二进制 11000000）提取高两位，比对结果应为 0x80（二进制 10000000）。
  // 满足此条件即表示是续字节。
  return (ch & 0xC0) == 0x80;
}
```

#### 6.2 魔法数字与常量

所有不自明常量必须附带解释来源与设计依据。

**Bad Example:**

```cpp
constexpr int MAX_F64_MAGNITUDE = 308;
```

**Good Example:**

```cpp
// IEEE 754 双精度浮点数 (double) 最大指数范围约为 10^±308。
// 用于在解析阶段检测潜在溢出科学记数法字面量。
// 来源: IEEE Standard for Floating-Point Arithmetic.
constexpr int MAX_F64_MAGNITUDE = 308;
```

#### 6.3 边界条件与特殊处理

**Bad Example:**

```cpp
key.erase(key.find_last_not_of(" \t") + 1);
```

**Good Example:**

```cpp
// 去除字符串尾部空白字符。
// find_last_not_of() 找到最后一个非空白字符索引。
// 若字符串全为空白，返回 npos，npos + 1 溢出为 0，erase(0) 会清空字符串。
size_t last_char = key.find_last_not_of(" \t");
if (last_char != std::string::npos)
    key.erase(last_char + 1);
```

***

### 7. 变量注释

- **类成员变量:** 应对非自明字段解释用途、约束、不变量与生命周期。

  ```cpp
  private:
    // 缓存最近一次查询结果。为 nullptr 表示缓存无效。
    // 管理权归本类所有，销毁时自动释放。
    QueryResult* cache_ptr_ = nullptr;
  ```

- **全局/静态变量:** 不推荐使用。若必须使用，必须充分说明：
  - 使用目的
  - 线程安全影响
  - 生命周期管理方式

***

### 8. 特殊注释标记 (状态追踪)

特殊标记用于维护、审查与技术债跟踪，应保持一致格式。

- **TODO:** 未完成任务、优化点  
  `// TODO(BegoniaHe, JIRA-213): 使用无锁队列优化日志写入。`

- **FIXME:** 已知缺陷，需尽快修正  
  `// FIXME(BegoniaHe): 此分支在空输入时行为未定义。`

- **DEPRECATED:** 弃用接口，提供替代说明  
  `// DEPRECATED: 此函数将在v3.0删除，请使用 LogManager::WriteAsync()。`

- **NOTE:** 记录背景或设计决策  
  `// NOTE: 此实现使用惰性加载，旨在减少初始化时延。`

***

### 9. 注释实践建议

- 注释应简洁、对齐、语法正确。
- 在代码变更后，务必同步更新注释。
- 不记录显而易见的操作（如 `i++` 循环计数），注释须关注“意图”。
- 避免行尾密集注释，可使用逻辑块注释代替。

**推荐模式:**

```cpp
// --- 检查输入有效性 ---
if (input.empty()) {
    // 输入为空，不可处理
    return Error::EmptyInput;
}
```

***

### 10. 注释审查与维护

- 在代码评审 (Code Review) 阶段，注释质量同样接受审查。
- API 注释需保证文档生成完整无误。
- 内部逻辑注释必须能解释代码背后的“设计理由”。
- 不允许存在过时、表意不明或空洞的注释。
