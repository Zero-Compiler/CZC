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

## Document Structure and Content Requirements

Generate the style guide using the following hierarchical structure. For each section, incorporate the philosophy and examples from the provided reference material where relevant.

---

### C++ 代码注释规范

#### 1. 核心理念

- 强调注释的目的是解释“为什么”和“意图”，而不是复述代码逻辑。
- 明确最好的代码是自文档化的（例如，通过有意义的变量名和函数名）。
- 注释的读者是未来的维护者（可能是自己），因此要清晰、完整。

#### 2. 基本注释风格

- **规范：** 统一使用 `//` 进行单行和多行注释。`/* ... */` 仅在特殊情况下（如临时注释掉代码块）允许使用。
- **理由：** `//` 更常用，可以避免嵌套块注释的意外错误，且易于统一管理。

#### 3. 文件头注释 (Doxygen 格式)

- **规范：** 每个 `.h` 和 `.cpp` 文件都必须包含文件头注释。
- **模板:**

  ```cpp
  /**
   * @file 文件名.cpp
   * @brief 对文件内容的简洁描述 (例如：实现一个配置解析器)
   * @author 作者名 (YourName)
   * @date YYYY-MM-DD
   */
  ```

- **说明：** 文件头提供了快速上下文，便于代码追溯和模块理解。

#### 4. 函数与方法注释 (Doxygen 格式)

- **规范：** 所有非平凡的公共函数/方法声明（在 `.h` 文件中）必须包含标准注释。简单的 getters/setters 可以省略。
- **模板与字段说明:**

  ```cpp
  /**
   * @brief 函数功能简述 (必须)
   * @details 对功能的详细描述、背景、算法或使用限制 (可选，用于复杂函数)
   * @param[in/out/in,out] param_name 参数描述 (必须，描述每个参数的用途、期望范围、是否可为nullptr等)
   * @return 返回值描述 (必须，如果函数非void)
   * @retval value1_description 特定返回值的含义 (可选，用于解释枚举或错误码)
   * @exception exception_type 异常抛出条件 (可选，但强烈推荐)
   * @note 补充说明或使用技巧 (可选)
   * @warning 需要特别注意的警告 (例如：线程安全问题、性能陷阱) (可选)
   */
  ```

- **关键实践：**
  - 注释应放在函数声明处（`.h` 文件），而不是定义处（`.cpp` 文件）。
  - 实现细节的注释（解释“如何做”）应放在 `.cpp` 的函数定义内部。
  - 对于参数，明确其是输入(`[in]`)、输出(`[out]`)还是输入输出(`[in,out]`)。

#### 5. 类与结构体注释

- **规范：** 每个公共类或结构体的定义都必须附有注释，说明其职责、用途和生命周期管理规则。
- **内容要点：**
  - **职责：** 这个类是用来做什么的？
  - **用法：** 提供一个简短的代码示例。
  - **线程安全：** 明确类的线程安全保证（或缺乏保证）。
- **示例：**

  ```cpp
  /**
   * @brief 管理用户配置文件的读写操作。
   * @details 此类负责解析、修改和保存ini格式的配置文件。
   *          注意：此类不是线程安全的，多线程环境需要外部加锁。
   * @example
   *   ConfigFile config("settings.ini");
   *   std::string user = config.getString("user", "default");
   */
  class ConfigFile { ... };
  ```

#### 6. 变量注释

- **类成员变量：** 对非明显的成员变量进行注释，解释其用途和任何不变量（如，`cache_ptr_` 可能为 `nullptr`）。

  ```cpp
  private:
    // 缓存最近一次查询的结果，若为 nullptr 则表示缓存无效。
    QueryResult* cache_ptr_;
  ```

- **全局/静态变量：** 不鼓励使用。如果必须使用，必须详细注释其用途、影响范围以及线程安全问题。

#### 7. 实现注释

- **目的：** 解释代码中复杂、晦涩或重要的部分。
- **代码块注释：** 在复杂的算法、循环或逻辑分支前，用 `//` 注释解释其目的或实现思路。
- **行尾注释：** 用于解释单行代码的微妙之处。与代码至少保持两个空格。

  ```cpp
  // 检查掩码位，判断是否为紧急消息。
  if (flags & URGENT_MASK) { ... }
  ```

- **禁止事项：** 不要写复述代码的“翻译式”注释。专注于“为什么”。

### 8. 特殊注释标记

- **TODO：** 用于标记待完成或需要改进的功能。必须包含责任人标识。
  `// TODO(YourName): 切换到更高效的序列化库 (参考 JIRA-123)`
- **DEPRECATED：** 用于标记已弃用的接口，并提供替代方案。
  `// DEPRECATED: 此函数将在 v2.1 中移除，请改用 NewApiFunction()`
