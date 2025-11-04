/**
 * @file lexer.cpp
 * @brief 词法分析器实现
 * @author BegoniaHe
 * @date 2025-11-04
 */

#include "czc/lexer/lexer.hpp"
#include "czc/lexer/utf8_handler.hpp"
#include <cctype>
#include <iomanip>
#include <sstream>

/**
 * @brief 报告一个词法分析错误
 * @param code 诊断代码
 * @param error_line 错误发生的行号
 * @param error_column 错误发生的列号
 * @param args 格式化参数
 */
void Lexer::report_error(DiagnosticCode code, size_t error_line,
                         size_t error_column,
                         const std::vector<std::string> &args) {
  auto loc = SourceLocation(tracker.get_filename(), error_line, error_column,
                            error_line, error_column);
  error_collector.add(code, loc, args);
}

/**
 * @brief 向前移动一个字符
 * @details 更新源码跟踪器的位置和当前字符
 */
void Lexer::advance() {
  // 如果已经到达源码末尾，则无需再前进。
  if (!current_char.has_value()) {
    return;
  }

  // 将当前字符传递给 tracker，以便它能正确更新行号和列号（特别是处理换行符）。
  char ch = current_char.value();
  tracker.advance(ch);

  size_t pos = tracker.get_position();
  const auto &input = tracker.get_input();

  // 更新 current_char 为 tracker 前进后的新位置上的字符。
  // 如果到达末尾，则设置为 std::nullopt。
  if (pos < input.size()) {
    current_char = input[pos];
  } else {
    current_char = std::nullopt;
  }
}

/**
 * @brief 查看未来位置的字符
 * @param offset 偏移量
 * @return 如果位置有效, 返回对应的字符, 否则返回 std::nullopt
 */
std::optional<char> Lexer::peek(size_t offset) const {
  size_t peek_pos = tracker.get_position() + offset;
  const auto &input = tracker.get_input();
  if (peek_pos < input.size()) {
    return input[peek_pos];
  }
  return std::nullopt;
}

/**
 * @brief 跳过空白字符
 */
void Lexer::skip_whitespace() {
  // 持续消耗字符，直到遇到非空白字符或源码结束。
  while (current_char.has_value() && std::isspace(current_char.value())) {
    advance();
  }
}

/**
 * @brief 跳过单行注释
 */
void Lexer::skip_comment() {
  // 检查是否是 `//` 开头的单行注释。
  if (current_char == '/' && peek(1) == '/') {
    // 如果是，则一直消耗字符直到行尾或文件末尾。
    while (current_char.has_value() && current_char != '\n') {
      advance();
    }
    // 如果是因为换行符而停止，则额外消耗掉这个换行符。
    if (current_char == '\n') {
      advance();
    }
  }
}

/**
 * @brief 读取带前缀的数字 (例如 0x, 0b, 0o)
 * @param valid_chars 允许的数字字符
 * @param prefix_str 前缀字符串 (用于报错)
 * @param error_code 缺少数字时报告的错误码
 * @return 构造的 Token
 */
Token Lexer::read_prefixed_number(const std::string &valid_chars,
                                  const std::string &prefix_str,
                                  DiagnosticCode error_code) {
  size_t start = tracker.get_position();
  size_t token_line = tracker.get_line();
  size_t token_column = tracker.get_column();

  // 消耗掉数字前缀，例如 "0x"。
  advance(); // '0'
  advance(); // 'x'/'b'/'o'

  // 记录数字部分的起始位置，用于后续检查是否为空。
  size_t digit_start = tracker.get_position();

  // 读取有效字符
  // 循环读取所有属于该数字进制的有效字符。
  while (current_char.has_value()) {
    char ch = current_char.value();
    if (valid_chars.find(ch) != std::string::npos) {
      advance();
    } else {
      // 遇到无效字符，数字部分结束。
      break;
    }
  }

  // 检查在前缀之后是否真的有数字。
  // 例如，"0x" 是无效的，必须是 "0x1" 这样的形式。
  size_t current_pos = tracker.get_position();
  if (current_pos == digit_start) {
    // 如果没有数字，则报告错误。
    report_error(error_code, token_line, token_column, {prefix_str});
    // 即使有错，也返回一个 Unknown 类型的 Token，包含了错误的文本，
    // 这样可以支持后续的错误恢复。
    const auto &input = tracker.get_input();
    return Token(TokenType::Unknown,
                 std::string(&input[start], current_pos - start), token_line,
                 token_column);
  }

  const auto &input = tracker.get_input();
  return Token(TokenType::Integer,
               std::string(&input[start], current_pos - start), token_line,
               token_column);
}

/**
 * @brief 读取数字 (整数, 浮点数, 科学计数法)
 * @return 构造的 Token
 */
Token Lexer::read_number() {
  size_t start = tracker.get_position();
  size_t token_line = tracker.get_line();
  size_t token_column = tracker.get_column();
  bool is_float = false;
  bool is_scientific = false;

  // 首先处理特殊数字前缀，如十六进制、二进制和八进制。
  // 检查当前字符是否为 '0' 并且后面有跟随字符。
  if (current_char == '0' && peek(1).has_value()) {
    char next_ch = peek(1).value();

    // 十六进制 (0x or 0X)
    if (next_ch == 'x' || next_ch == 'X') {
      return read_prefixed_number("0123456789abcdefABCDEF", "0x",
                                  DiagnosticCode::L0001_MissingHexDigits);
    }
    // 二进制 (0b or 0B)
    else if (next_ch == 'b' || next_ch == 'B') {
      return read_prefixed_number("01", "0b",
                                  DiagnosticCode::L0002_MissingBinaryDigits);
    }
    // 八进制 (0o or 0O)
    else if (next_ch == 'o' || next_ch == 'O') {
      return read_prefixed_number("01234567", "0o",
                                  DiagnosticCode::L0003_MissingOctalDigits);
    }
  }

  // 读取十进制数字的整数和可选的小数部分。
  while (current_char.has_value()) {
    char ch = current_char.value();
    if (std::isdigit(ch)) {
      advance();
    }
    // 处理小数点。只允许一个小数点。
    else if (ch == '.' && !is_float) {
      // 小数点后面必须跟一个数字，否则像 `1.` 这样的写法被视为整数 `1`
      // 和一个点号 `.`。
      auto next = peek(1);
      if (next.has_value() && std::isdigit(next.value())) {
        is_float = true;
        advance();
      } else {
        // 小数点后不是数字，数字部分结束。
        break;
      }
    } else {
      break;
    }
  }

  // 处理科学计数法部分 (e.g., e+10, E-5)。
  if (current_char.has_value() &&
      (current_char.value() == 'e' || current_char.value() == 'E')) {
    is_scientific = true;
    advance(); // 消耗 'e' 或 'E'

    // 消耗可选的指数符号 '+' 或 '-'。
    if (current_char.has_value() &&
        (current_char.value() == '+' || current_char.value() == '-')) {
      advance();
    }

    // 指数部分必须至少包含一个数字。
    size_t exp_start = tracker.get_position();
    while (current_char.has_value() && std::isdigit(current_char.value())) {
      advance();
    }

    // 如果在 'e' 或 'e-' 之后没有数字，这是一个词法错误。
    size_t current_pos = tracker.get_position();
    if (current_pos == exp_start) {
      const auto &input = tracker.get_input();
      report_error(DiagnosticCode::L0004_MissingExponentDigits, token_line,
                   token_column,
                   {std::string(&input[start], current_pos - start)});
      return Token(TokenType::Unknown,
                   std::string(&input[start], current_pos - start), token_line,
                   token_column);
    }
  }

  // 根据语言规范，数字字面量后面不能直接跟标识符字符（字母或下划线）。
  // 例如 `123a` 是无效的。
  if (current_char.has_value() &&
      (std::isalpha(current_char.value()) || current_char.value() == '_')) {
    std::string bad_suffix(1, current_char.value());
    report_error(DiagnosticCode::L0005_InvalidTrailingChar, token_line,
                 token_column, {bad_suffix});
    // 消耗掉这个无效字符，以避免词法分析器卡住。
    advance();
    size_t current_pos = tracker.get_position();
    const auto &input = tracker.get_input();
    return Token(TokenType::Unknown,
                 std::string(&input[start], current_pos - start), token_line,
                 token_column);
  }

  size_t current_pos = tracker.get_position();
  const auto &input = tracker.get_input();
  std::string value(&input[start], current_pos - start);

  // 根据解析过程中设置的标志，确定最终的 Token 类型。
  if (is_scientific) {
    // 科学计数法字面量需要后续处理（在 TokenPreprocessor
    // 中）来确定是整数还是浮点数。
    return Token(TokenType::ScientificExponent, value, token_line,
                 token_column);
  } else if (is_float) {
    return Token(TokenType::Float, value, token_line, token_column);
  } else {
    return Token(TokenType::Integer, value, token_line, token_column);
  }
}

/**
 * @brief 读取标识符或关键字
 * @return 构造的 Token
 */
Token Lexer::read_identifier() {
  size_t start = tracker.get_position();
  size_t token_line = tracker.get_line();
  size_t token_column = tracker.get_column();

  advance();

  while (current_char.has_value()) {
    char ch = current_char.value();
    unsigned char uch = static_cast<unsigned char>(ch);

    // 标识符可以包含字母、数字、下划线以及任何非 ASCII 的 UTF-8 字符。
    if (std::isalnum(ch) || ch == '_') {
      advance();
    }
    // 简单的检查，所有非 ASCII 字符（高位为1）都被视为标识符的一部分。
    // 更严格的检查（例如，只允许特定 Unicode 范围的字母）会更复杂。
    else if (uch >= 0x80) {
      advance();
    } else {
      break;
    }
  }

  size_t current_pos = tracker.get_position();
  const auto &input = tracker.get_input();
  std::string value(&input[start], current_pos - start);
  // 检查解析出的字符串是否是语言的关键字。
  auto keyword_type = get_keyword(value);
  // 如果是关键字，则使用关键字的 Token 类型；否则，它是一个普通的标识符。
  TokenType token_type = keyword_type.value_or(TokenType::Identifier);

  return Token(token_type, value, token_line, token_column);
}

// 解析 Unicode 转义序列
/**
 * @brief 解析 Unicode 转义序列
 * @param digit_count 需要解析的十六进制数字个数
 * @return 转换后的 UTF-8 字符串
 */
std::string Lexer::parse_unicode_escape(size_t digit_count) {
  std::string hex_digits;

  for (size_t i = 0; i < digit_count; ++i) {
    if (!current_char.has_value()) {
      report_error(DiagnosticCode::L0009_InvalidUnicodeEscape,
                   tracker.get_line(), tracker.get_column(), {"u"});
      return "";
    }

    char ch = current_char.value();
    if (!std::isxdigit(ch)) {
      report_error(DiagnosticCode::L0009_InvalidUnicodeEscape,
                   tracker.get_line(), tracker.get_column(), {"u"});
      return "";
    }

    hex_digits += ch;
    advance();
  }

  unsigned int codepoint;
  std::stringstream ss;
  ss << std::hex << hex_digits;
  ss >> codepoint;

  // 将解析出的 Unicode 码点转换为 UTF-8 编码的字符串。
  return Utf8Handler::codepoint_to_utf8(codepoint);
}

// 解析十六进制转义序列 \xHH
/**
 * @brief 解析十六进制转义序列 (\\xHH)
 * @return 转换后的字节字符串
 */
std::string Lexer::parse_hex_escape() {
  std::string hex_digits;

  // 读取最多 2 个十六进制数字
  for (size_t i = 0; i < 2; ++i) {
    if (!current_char.has_value()) {
      break;
    }

    char ch = current_char.value();
    if (!std::isxdigit(ch)) {
      break;
    }

    hex_digits += ch;
    advance();
  }

  if (hex_digits.empty()) {
    report_error(DiagnosticCode::L0008_InvalidHexEscape, tracker.get_line(),
                 tracker.get_column(), {"x"});
    return ""; // 返回空字符串
  }

  // 将十六进制字符串（例如 "FF"）转换为对应的字节值。
  unsigned int byte_value;
  std::stringstream ss;
  ss << std::hex << hex_digits;
  ss >> byte_value;

  std::string result;
  result += static_cast<char>(byte_value);
  return result;
}

/**
 * @brief 读取字符串字面量
 * @return 构造的 Token
 */
Token Lexer::read_string() {
  size_t token_line = tracker.get_line();
  size_t token_column = tracker.get_column();
  advance(); // 跳过开头的 "

  std::string value;
  // 为字符串值预分配一些空间，以减少在循环中因字符串增长而导致的内存重分配。
  value.reserve(64);
  bool terminated = false; // 标志位，用于检查字符串是否正常闭合。

  while (current_char.has_value()) {
    char ch = current_char.value();

    if (ch == '"') {
      terminated = true;
      break;
    }

    if (ch == '\n') {
      value += ch;
      advance();
      continue;
    }

    if (ch == '\\') {
      // 处理转义序列。
      advance(); // 消耗反斜杠 '\'。
      if (!current_char.has_value()) {
        // 如果反斜杠是字符串的最后一个字符，则字符串未闭合。
        report_error(DiagnosticCode::L0007_UnterminatedString, token_line,
                     token_column, {});
        return Token(TokenType::String, value, token_line, token_column);
      }

      char escaped = current_char.value();
      switch (escaped) {
      case 'n':
        value += '\n';
        advance();
        continue;
      case 't':
        value += '\t';
        advance();
        continue;
      case 'r':
        value += '\r';
        advance();
        continue;
      case '\\':
        value += '\\';
        advance();
        continue;
      case '"':
        value += '"';
        advance();
        continue;
      case '\'':
        value += '\'';
        advance();
        continue;
      case '0':
        value += '\0';
        advance();
        continue;
      case 'x':
        // 十六进制转义 \xHH
        advance();
        value += parse_hex_escape();
        continue;
      case 'u':
        // Unicode 转义
        advance();

        if (current_char.has_value() && current_char.value() == '{') {
          // \u{XXXXXX} 格式
          advance(); // 跳过 {

          std::string hex_digits;
          while (current_char.has_value() && current_char.value() != '}') {
            if (!std::isxdigit(current_char.value())) {
              report_error(DiagnosticCode::L0009_InvalidUnicodeEscape,
                           tracker.get_line(), tracker.get_column(), {"u"});
              // 跳过剩余内容直到 } 或字符串结束
              while (current_char.has_value() && current_char.value() != '}' &&
                     current_char.value() != '"') {
                advance();
              }
              break;
            }
            hex_digits += current_char.value();
            advance();
          }

          if (!current_char.has_value() || current_char.value() != '}') {
            report_error(DiagnosticCode::L0009_InvalidUnicodeEscape,
                         tracker.get_line(), tracker.get_column(), {"u"});
            continue;
          }

          advance(); // 跳过 }

          if (hex_digits.empty() || hex_digits.length() > 6) {
            report_error(DiagnosticCode::L0009_InvalidUnicodeEscape,
                         tracker.get_line(), tracker.get_column(), {"u"});
            continue;
          }

          unsigned int codepoint;
          std::stringstream ss;
          ss << std::hex << hex_digits;
          ss >> codepoint;

          value += Utf8Handler::codepoint_to_utf8(codepoint);

          continue;
        } else {
          // \uXXXX
          value += parse_unicode_escape(4);
          continue;
        }
      default:
        report_error(DiagnosticCode::L0006_InvalidEscapeSequence,
                     tracker.get_line(), tracker.get_column(),
                     {std::string(1, escaped)});
        // 跳过无效的转义字符,继续处理
        value += escaped;
        advance();
        continue;
      }
    } else {
      // 处理非转义的普通字符，支持多字节的 UTF-8 字符。
      if (!current_char.has_value())
        break;

      unsigned char byte = static_cast<unsigned char>(current_char.value());
      // 获取当前 UTF-8 字符所需的字节数。
      size_t char_length = Utf8Handler::get_char_length(byte);
      const auto &input = tracker.get_input();
      size_t pos = tracker.get_position();

      // 将组成单个 UTF-8 字符的所有字节都添加到值中。
      for (size_t i = 0; i < char_length && pos + i < input.size(); ++i) {
        value += input[pos + i];
      }
      // advance() 只移动一个字节，但对于多字节字符，我们需要确保 tracker
      // 的位置正确。
      // 这里的逻辑依赖于一个简化的假设：循环的下一次迭代将处理下一个字节。
      // 一个更健壮的实现会在这里一次性 advance `char_length` 次。
      // (当前实现依赖于循环的下一次迭代继续处理多字节字符的剩余部分，这是错误的，需要修正)
      // TODO(BegoniaHe): 修正此处对多字节字符的处理逻辑，应该一次性 advance
      // char_length。
      advance();
    }
  }

  // 如果循环结束时 `terminated` 标志仍为 false，说明字符串没有闭合引号。
  if (!terminated) {
    report_error(DiagnosticCode::L0007_UnterminatedString, token_line,
                 token_column, {});
    return Token(TokenType::String, value, token_line, token_column);
  }

  advance(); // 跳过结尾的 "

  return Token(TokenType::String, value, token_line, token_column);
}

/**
 * @brief 读取原始字符串字面量 (r"...")
 * @return 构造的 Token
 */
Token Lexer::read_raw_string() {
  size_t token_line = tracker.get_line();
  size_t token_column = tracker.get_column();

  // 跳过 'r'
  advance();

  if (!current_char.has_value() || current_char.value() != '"') {
    report_error(DiagnosticCode::L0010_InvalidCharacter, token_line,
                 token_column, {"r"});
    // 返回错误 token
    return Token(TokenType::Unknown, "r", token_line, token_column);
  }

  advance();

  std::string value;
  value.reserve(64); // 预留空间，减少重新分配
  bool terminated = false;

  while (current_char.has_value()) {
    char ch = current_char.value();

    if (ch == '"') {
      terminated = true;
      break;
    }

    // 在原始字符串中，所有字符（包括反斜杠）都按其字面意义处理，不进行任何转义。
    // 这里的逻辑与普通字符串中的 UTF-8 处理相同。
    unsigned char byte = static_cast<unsigned char>(ch);
    size_t char_length = Utf8Handler::get_char_length(byte);
    const auto &input = tracker.get_input();
    size_t pos = tracker.get_position();

    for (size_t i = 0; i < char_length && pos + i < input.size(); ++i) {
      value += input[pos + i];
    }
    // TODO(BegoniaHe): 同 read_string, 此处的 advance()
    // 逻辑需要修正为处理多字节。
    advance();
  }

  if (!terminated) {
    report_error(DiagnosticCode::L0007_UnterminatedString, token_line,
                 token_column, {});
    // 返回不完整的原始字符串 token
    return Token(TokenType::String, value, token_line, token_column);
  }

  advance();

  return Token(TokenType::String, value, token_line, token_column);
}

/**
 * @brief Lexer 构造函数
 * @param input_str 输入的源码字符串
 * @param fname 文件名
 */
Lexer::Lexer(const std::string &input_str, const std::string &fname)
    : tracker(input_str, fname) {
  const auto &input = tracker.get_input();
  if (!input.empty()) {
    current_char = input[0];
  } else {
    current_char = std::nullopt;
  }
}

/**
 * @brief 获取下一个 Token
 * @return 下一个 Token
 */
Token Lexer::next_token() {
  // 主循环，用于跳过所有不产生 Token 的内容（空白和注释）。
  // 这样可以确保每次进入后续的 Token 判断逻辑时，`current_char`
  // 都是一个有意义的字符。
  while (true) {
    skip_whitespace();

    if (current_char == '/' && peek(1) == '/') {
      skip_comment();
      continue; // 跳过注释后，可能还有更多空白，所以继续循环。
    }

    break; // 如果既不是空白也不是注释，则退出循环，开始解析 Token。
  }

  // 如果在跳过空白和注释后到达了文件末尾，则返回 EOF Token。
  if (!current_char.has_value()) {
    return Token(TokenType::EndOfFile, "", tracker.get_line(),
                 tracker.get_column());
  }

  char ch = current_char.value();
  unsigned char uch = static_cast<unsigned char>(ch);
  size_t token_line = tracker.get_line();
  size_t token_column = tracker.get_column();

  // 根据当前字符的类型，分派到不同的处理函数。
  // 这是一个典型的词法分析器状态机分派逻辑。

  // 如果是数字开头，则解析数字字面量。
  if (std::isdigit(ch)) {
    return read_number();
  }

  // 如果是字母、下划线或非 ASCII 字符开头，则解析标识符或关键字。
  if (std::isalpha(ch) || ch == '_' || uch >= 0x80) {
    // 特殊情况：`r"` 前缀表示原始字符串。
    if (ch == 'r' && peek(1) == '"') {
      return read_raw_string();
    }
    return read_identifier();
  }

  // 如果是双引号开头，则解析字符串字面量。
  if (ch == '"') {
    return read_string();
  }

  // 当前语言不支持单引号字符字面量，因此将其视为未知 Token。
  if (ch == '\'') {
    Token token(TokenType::Unknown, std::string(1, ch), token_line,
                token_column);
    advance();
    return token;
  }

  Token token(TokenType::Unknown, "", token_line, token_column);

  // 处理各种单字符和双字符运算符及分隔符。
  // 对于双字符运算符（如 `+=`, `==`），需要向前看（peek）一个字符来判断。
  switch (ch) {
  case '+':
    if (peek(1) == '=') {
      advance(); // 消耗 '='
      token = Token(TokenType::PlusEqual, "+=", token_line, token_column);
    } else {
      token =
          Token(TokenType::Plus, std::string(1, ch), token_line, token_column);
    }
    break;
  case '-':
    if (peek(1) == '=') {
      advance();
      token = Token(TokenType::MinusEqual, "-=", token_line, token_column);
    } else {
      token =
          Token(TokenType::Minus, std::string(1, ch), token_line, token_column);
    }
    break;
  case '*':
    if (peek(1) == '=') {
      advance();
      token = Token(TokenType::StarEqual, "*=", token_line, token_column);
    } else {
      token =
          Token(TokenType::Star, std::string(1, ch), token_line, token_column);
    }
    break;
  case '/':
    if (peek(1) == '=') {
      advance();
      token = Token(TokenType::SlashEqual, "/=", token_line, token_column);
    } else {
      token =
          Token(TokenType::Slash, std::string(1, ch), token_line, token_column);
    }
    break;
  case '%':
    if (peek(1) == '=') {
      advance();
      token = Token(TokenType::PercentEqual, "%=", token_line, token_column);
    } else {
      token = Token(TokenType::Percent, std::string(1, ch), token_line,
                    token_column);
    }
    break;
  case '=':
    if (peek(1) == '=') {
      advance();
      token = Token(TokenType::EqualEqual, "==", token_line, token_column);
    } else {
      token =
          Token(TokenType::Equal, std::string(1, ch), token_line, token_column);
    }
    break;
  case '!':
    if (peek(1) == '=') {
      advance();
      token = Token(TokenType::BangEqual, "!=", token_line, token_column);
    } else {
      token =
          Token(TokenType::Bang, std::string(1, ch), token_line, token_column);
    }
    break;
  case '<':
    if (peek(1) == '=') {
      advance();
      token = Token(TokenType::LessEqual, "<=", token_line, token_column);
    } else {
      token =
          Token(TokenType::Less, std::string(1, ch), token_line, token_column);
    }
    break;
  case '>':
    if (peek(1) == '=') {
      advance();
      token = Token(TokenType::GreaterEqual, ">=", token_line, token_column);
    } else {
      token = Token(TokenType::Greater, std::string(1, ch), token_line,
                    token_column);
    }
    break;
  case '&':
    if (peek(1) == '&') {
      advance();
      token = Token(TokenType::And, "&&", token_line, token_column);
    } else {
      // 单个 '&' 不是一个有效的运算符。
      token = Token(TokenType::Unknown, std::string(1, ch), token_line,
                    token_column);
    }
    break;
  case '|':
    if (peek(1) == '|') {
      advance();
      token = Token(TokenType::Or, "||", token_line, token_column);
    } else {
      // 单个 '|' 不是一个有效的运算符。
      token = Token(TokenType::Unknown, std::string(1, ch), token_line,
                    token_column);
    }
    break;
  case '(':
    token = Token(TokenType::LeftParen, std::string(1, ch), token_line,
                  token_column);
    break;
  case ')':
    token = Token(TokenType::RightParen, std::string(1, ch), token_line,
                  token_column);
    break;
  case '{':
    token = Token(TokenType::LeftBrace, std::string(1, ch), token_line,
                  token_column);
    break;
  case '}':
    token = Token(TokenType::RightBrace, std::string(1, ch), token_line,
                  token_column);
    break;
  case '[':
    token = Token(TokenType::LeftBracket, std::string(1, ch), token_line,
                  token_column);
    break;
  case ']':
    token = Token(TokenType::RightBracket, std::string(1, ch), token_line,
                  token_column);
    break;
  case ',':
    token =
        Token(TokenType::Comma, std::string(1, ch), token_line, token_column);
    break;
  case ';':
    token = Token(TokenType::Semicolon, std::string(1, ch), token_line,
                  token_column);
    break;
  case ':':
    token =
        Token(TokenType::Colon, std::string(1, ch), token_line, token_column);
    break;
  case '.':
    if (peek(1) == '.') {
      advance();
      token = Token(TokenType::DotDot, "..", token_line, token_column);
    } else {
      token =
          Token(TokenType::Dot, std::string(1, ch), token_line, token_column);
    }
    break;
  default:
    // 如果字符不匹配任何已知的 Token 模式，则将其标记为 Unknown。
    token =
        Token(TokenType::Unknown, std::string(1, ch), token_line, token_column);
    break;
  }

  // 消耗当前字符，为下一次调用 `next_token` 做准备。
  advance();
  return token;
}

/**
 * @brief 将整个输入源 tokenize
 * @return Token 向量
 */
std::vector<Token> Lexer::tokenize() {
  std::vector<Token> tokens;

  while (true) {
    Token token = next_token();
    bool is_eof = (token.token_type == TokenType::EndOfFile);
    tokens.push_back(token);

    // 持续获取 Token 直到文件结束。
    if (is_eof) {
      break;
    }
  }

  return tokens;
}
