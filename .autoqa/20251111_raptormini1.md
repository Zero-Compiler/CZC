# CZC QA 报告 - 2025-11-11 (raptormini)

## 概要

### 高严重性问题（必须修复）

**1) UB 风险：使用有符号 char 值调用 C 风格 ctype 函数**

- **文件/位置**：`src/lexer/lexer.cpp`（多处 — `skip_whitespace()`、`read_number()`、`read_identifier()`）、`src/token_preprocessor/token_preprocessor.cpp`
- **证据**：`std::isspace(current_char.value())`、`std::isdigit(ch)` / `std::isalpha(ch)`、`std::isxdigit(current_char.value())`
- **问题原因**：在 char 为有符号类型的平台上，将带有负值（非 ASCII 字节）的有符号 char 传递给 ctype 函数会导致 C/C++ 中的未定义行为（UB）
- **修复方案**：在传递给 ctype 函数之前转换为无符号 char：`std::isspace(static_cast<unsigned char>(ch))`、`std::isdigit(static_cast<unsigned char>(ch))` 等。如有需要可添加小型内联辅助函数或宏

**2) UTF-8 处理不一致及潜在的无效连续字节**

- **文件/位置**：`src/lexer/lexer.cpp`、`src/lexer/utf8_handler.cpp`
- **证据**：`read_identifier()` 接受任何 >= 0x80 的字节作为标识符的一部分，并按单字节递增：`else if (uch >= 0x80) advance();` — 这允许混合连续字节和起始字节，并可能接受无效的 UTF-8
- **问题原因**：这可能导致标识符中包含无效的 UTF-8 字符，造成列/行跟踪不同步，或导致 token 只包含多字节码点的一半
- **修复方案**：在 `read_identifier()` 中使用现有的 `Utf8Handler::read_char()` 来消费完整的码点，或修改词法分析器使用 `get_char_length()` 和 `is_continuation()` 验证 UTF-8 序列。同时将无效序列视为错误处理

**3) 性能问题：UTF-8 处理中重复进行完整字符串复制**

- **文件/位置**：`src/lexer/lexer.cpp` → `read_string()` 和 `read_raw_string()` 使用 `std::string(tracker.get_input().begin(), tracker.get_input().end())` 调用 `Utf8Handler::read_char`（每次都复制完整输入）
- **证据**：在循环内重复使用 `std::string(tracker.get_input().begin(), tracker.get_input().end())`
- **问题原因**：每次调用都复制整个输入字符串；对于包含大量 UTF-8 字符的长输入，最坏情况为 O(N²) 行为
- **修复方案**：避免重复复制。让 `SourceTracker` 暴露 `const std::string` 或 `std::string_view`，或修改 `Utf8Handler::read_char` 接受 `const std::vector<char>&` 或 `const char*` 和长度（或 `absl::string_view`/`std::string_view`），并传递单个引用

**4) Token 位置不一致：解析器错误未存储文件名**

- **文件/位置**：`src/parser/parser.cpp` 的 `make_location()` 返回 `SourceLocation("", token.line, token.column);`
- **证据**：`make_location()` 创建的 SourceLocation 文件名为空。`Diagnostic::format()` 打印的诊断消息可能缺少文件名
- **问题原因**：解析器无法在错误中显示文件名。对于多文件构建，这是一个明显的用户体验退步
- **修复方案**：在 `Token` 中添加文件名字段，或为 `Parser` 提供文件上下文（例如在构造函数中传递文件名或提供 `SourceTracker` 或上下文）。然后在 `make_location()` 中使用文件名构造 SourceLocation

**5) 缺少块注释支持（/* */）**

- **文件/位置**：`tests/test_lexer_gtest.cpp` 包含 TODO；词法分析器仅支持 `//` 单行注释
- **证据**：`Lexer::read_comment()` 仅处理 `//`；不支持多行块注释
- **问题原因**：许多语言和测试中的示例都提到多行注释；缺少此功能限制了用户代码并造成测试空白
- **修复方案**：在 `read_comment()` 中实现 `/* ... */` 块注释解析，并稳健地处理嵌套或未闭合的情况（报告 L0007 或适当的诊断代码）

### 中等严重性问题（建议修复）

**1) 重复的双字符运算符解析逻辑**

- **文件/位置**：`src/lexer/lexer.cpp` 包含双字符运算符的重复逻辑，以及未使用的 `try_read_two_char_operator()` 辅助函数
- **证据**：存在一个独立的 `try_read_two_char_operator()` 未在 `next_token()` 中使用，而是用一个长 `switch` 语句内联处理
- **修复方案**：使用辅助函数整合双字符运算符逻辑，以减少重复和潜在的不一致性

**2) 从 `const vector<char>&` 使用 `std::string(&input[start], len)` 构造函数；建议使用 `std::string(input.data() + start, len)` 以提高清晰度**

- **文件/位置**：`src/lexer/lexer.cpp` 等
- **修复方案**：为了清晰和安全，使用 `std::string(input.data()+start, len)`，或如果支持 C++17 且需要字符串切片则使用 `std::string_view`，以防止混淆

**3) `SourceTracker::get_source_line()` 行为：精确输出边界**

- **文件/位置**：`src/utils/source_tracker.cpp` 计算 `line_end` 逻辑的行
- **证据**：对最后一行使用 `line_end = input.size()`，返回使用 `

**4) `I18nMessages::load_from_file()` 中的手动 TOML 解析可能在更复杂的输入上失败；缺少适当的转义/去引号/反转义和嵌套值处理**

- **文件/位置**：`src/diagnostics/diagnostic.cpp` 的 `I18nMessages::load_from_file`
- **证据**：简化的解析：`value.erase(0, value.find_first_not_of(...)`

### 构建与测试验证（本地运行）

- **`cmake`/`make` 成功**：构建成功完成
- **`make test` 成功**：执行 6 个测试并全部通过（0 个失败）
- **注意**：性能测试（例如生成 1000 万行）负载较重；考虑在 CI 中设置门控，并添加较小的冒烟测试变体
