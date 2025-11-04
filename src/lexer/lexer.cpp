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

namespace czc {
namespace lexer {

using namespace czc::diagnostics;
using namespace czc::utils;

void Lexer::report_error(DiagnosticCode code, size_t error_line,
                         size_t error_column,
                         const std::vector<std::string> &args) {
  // 创建一个只包含错误发生点的 SourceLocation。
  auto loc = SourceLocation(tracker.get_filename(), error_line, error_column,
                            error_line, error_column);
  error_collector.add(code, loc, args);
}

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
  // 如果到达末尾，则设置为 std::nullopt，这将作为后续处理的终止信号。
  if (pos < input.size()) {
    current_char = input[pos];
  } else {
    current_char = std::nullopt;
  }
}

std::optional<char> Lexer::peek(size_t offset) const {
  size_t peek_pos = tracker.get_position() + offset;
  const auto &input = tracker.get_input();
  if (peek_pos < input.size()) {
    return input[peek_pos];
  }
  return std::nullopt;
}

void Lexer::skip_whitespace() {
  // 持续消耗字符，直到遇到非空白字符或源码结束。
  while (current_char.has_value() && std::isspace(current_char.value())) {
    advance();
  }
}

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

  // --- 读取有效数字字符 ---
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

  // --- 验证前缀后是否有数字 ---
  // 例如，"0x" 是无效的，必须是 "0x1" 这样的形式。
  size_t current_pos = tracker.get_position();
  if (current_pos == digit_start) {
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

Token Lexer::read_number() {
  size_t start = tracker.get_position();
  size_t token_line = tracker.get_line();
  size_t token_column = tracker.get_column();
  bool is_float = false;
  bool is_scientific = false;

  // --- 处理特殊数字前缀 (0x, 0b, 0o) ---
  if (current_char == '0' && peek(1).has_value()) {
    char next_ch = peek(1).value();
    if (next_ch == 'x' || next_ch == 'X') {
      return read_prefixed_number("0123456789abcdefABCDEF", "0x",
                                  DiagnosticCode::L0001_MissingHexDigits);
    }
    if (next_ch == 'b' || next_ch == 'B') {
      return read_prefixed_number("01", "0b",
                                  DiagnosticCode::L0002_MissingBinaryDigits);
    }
    if (next_ch == 'o' || next_ch == 'O') {
      return read_prefixed_number("01234567", "0o",
                                  DiagnosticCode::L0003_MissingOctalDigits);
    }
  }

  // --- 读取十进制数字的整数和可选的小数部分 ---
  while (current_char.has_value()) {
    char ch = current_char.value();
    if (std::isdigit(ch)) {
      advance();
    }
    // 处理小数点。只允许一个小数点。
    else if (ch == '.' && !is_float) {
      // 小数点后面必须跟一个数字。像 `1.` 这样的写法被视为整数 `1`
      // 和一个单独的点号 `.` Token。这是为了简化语法。
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

  // --- 处理科学计数法部分 (e.g., e+10, E-5) ---
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

  // --- 验证数字字面量后的字符 ---
  // 根据语言规范，数字字面量后面不能直接跟标识符字符（字母或下划线）。
  // 例如 `123a` 是无效的，以避免歧义。
  if (current_char.has_value() &&
      (std::isalpha(current_char.value()) || current_char.value() == '_')) {
    std::string bad_suffix(1, current_char.value());
    report_error(DiagnosticCode::L0005_InvalidTrailingChar, token_line,
                 token_column, {bad_suffix});
    // 消耗掉这个无效字符，以避免词法分析器卡在同一个错误上。
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

  // --- 根据解析过程中设置的标志，确定最终的 Token 类型 ---
  if (is_scientific) {
    // 科学计数法字面量需要后续处理（在 TokenPreprocessor 中）
    // 来确定其最终类型是整数还是浮点数，并检查溢出。
    return Token(TokenType::ScientificExponent, value, token_line,
                 token_column);
  }
  if (is_float) {
    return Token(TokenType::Float, value, token_line, token_column);
  }
  return Token(TokenType::Integer, value, token_line, token_column);
}

Token Lexer::read_identifier() {
  size_t start = tracker.get_position();
  size_t token_line = tracker.get_line();
  size_t token_column = tracker.get_column();

  advance(); // 消耗第一个字符

  while (current_char.has_value()) {
    char ch = current_char.value();
    unsigned char uch = static_cast<unsigned char>(ch);

    // 标识符可以包含字母、数字、下划线以及任何非 ASCII 的 UTF-8 字符。
    if (std::isalnum(ch) || ch == '_') {
      advance();
    }
    // 简单的检查，所有非 ASCII 字符（高位为1）都被视为标识符的一部分。
    // NOTE: 这是一个宽松的规则。更严格的实现可能需要检查 Unicode
    // 属性，以只允许特定范围的字母或符号。
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

std::string Lexer::parse_unicode_escape(size_t digit_count) {
  std::string hex_digits;

  for (size_t i = 0; i < digit_count; ++i) {
    if (!current_char.has_value() || !std::isxdigit(current_char.value())) {
      report_error(DiagnosticCode::L0009_InvalidUnicodeEscape,
                   tracker.get_line(), tracker.get_column(), {"u"});
      return "";
    }
    hex_digits += current_char.value();
    advance();
  }

  unsigned int codepoint;
  std::stringstream ss;
  ss << std::hex << hex_digits;
  ss >> codepoint;

  // 将解析出的 Unicode 码点转换为 UTF-8 编码的字符串。
  return Utf8Handler::codepoint_to_utf8(codepoint);
}

std::string Lexer::parse_hex_escape() {
  std::string hex_digits;

  // 读取最多 2 个十六进制数字
  for (size_t i = 0; i < 2; ++i) {
    if (!current_char.has_value() || !std::isxdigit(current_char.value())) {
      break;
    }
    hex_digits += current_char.value();
    advance();
  }

  if (hex_digits.empty()) {
    report_error(DiagnosticCode::L0008_InvalidHexEscape, tracker.get_line(),
                 tracker.get_column(), {"x"});
    return "";
  }

  // 将十六进制字符串（例如 "FF"）转换为对应的字节值。
  unsigned int byte_value;
  std::stringstream ss;
  ss << std::hex << hex_digits;
  ss >> byte_value;

  return std::string(1, static_cast<char>(byte_value));
}

Token Lexer::read_string() {
  size_t token_line = tracker.get_line();
  size_t token_column = tracker.get_column();
  advance(); // 跳过开头的 "

  std::string value;
  value.reserve(64); // 预分配以提高性能
  bool terminated = false;

  while (current_char.has_value()) {
    char ch = current_char.value();

    if (ch == '"') {
      terminated = true;
      break;
    }

    if (ch == '\n') { // 允许字符串跨行
      value += ch;
      advance();
      continue;
    }

    if (ch == '\\') {
      // --- 处理转义序列 ---
      advance(); // 消耗反斜杠 '\'
      if (!current_char.has_value()) {
        // 如果反斜杠是文件的最后一个字符，则字符串未闭合。
        break;
      }

      char escaped = current_char.value();
      switch (escaped) {
      case 'n':
        value += '\n';
        advance();
        break;
      case 't':
        value += '\t';
        advance();
        break;
      case 'r':
        value += '\r';
        advance();
        break;
      case '\\':
        value += '\\';
        advance();
        break;
      case '"':
        value += '"';
        advance();
        break;
      case '\'':
        value += '\'';
        advance();
        break;
      case '0':
        value += '\0';
        advance();
        break;
      case 'x': // 十六进制转义 \xHH
        advance();
        value += parse_hex_escape();
        break;
      case 'u': // Unicode 转义 \uXXXX or \u{...}
        advance();
        if (current_char.has_value() && current_char.value() == '{') {
          // \u{XXXXXX} 格式
          advance(); // 跳过 '{'
          std::string hex_digits;
          while (current_char.has_value() && current_char.value() != '}') {
            if (!std::isxdigit(current_char.value())) {
              report_error(DiagnosticCode::L0009_InvalidUnicodeEscape,
                           tracker.get_line(), tracker.get_column(), {"u"});
              // 尝试恢复：跳过无效内容直到 '}' 或字符串结束
              while (current_char.has_value() && current_char.value() != '}' &&
                     current_char.value() != '"') {
                advance();
              }
              hex_digits.clear(); // 标记为失败
              break;
            }
            hex_digits += current_char.value();
            advance();
          }

          if (!current_char.has_value() || current_char.value() != '}') {
            report_error(DiagnosticCode::L0009_InvalidUnicodeEscape,
                         tracker.get_line(), tracker.get_column(), {"u"});
          } else {
            advance(); // 跳过 '}'
            if (!hex_digits.empty() && hex_digits.length() <= 6) {
              unsigned int codepoint = std::stoul(hex_digits, nullptr, 16);
              value += Utf8Handler::codepoint_to_utf8(codepoint);
            } else {
              report_error(DiagnosticCode::L0009_InvalidUnicodeEscape,
                           tracker.get_line(), tracker.get_column(), {"u"});
            }
          }
        } else {
          // \uXXXX 格式
          value += parse_unicode_escape(4);
        }
        break;
      default:
        report_error(DiagnosticCode::L0006_InvalidEscapeSequence,
                     tracker.get_line(), tracker.get_column(),
                     {std::string(1, escaped)});
        value += escaped; // 将无效的转义字符本身添加到值中
        advance();
        break;
      }
    } else {
      // --- 处理非转义的普通字符（可能为多字节 UTF-8） ---
      // NOTE: 这里的 UTF-8 处理逻辑存在缺陷。它一次只 advance() 一个字节，
      // 依赖于循环的下一次迭代来处理多字节字符的剩余部分。
      // TODO(BegoniaHe): 修正此处的 UTF-8 处理逻辑。应该使用
      // Utf8Handler::read_char 来完整读取一个字符，并一次性 advance
      // 对应的字节数。
      value += ch;
      advance();
    }
  }

  if (!terminated) {
    report_error(DiagnosticCode::L0007_UnterminatedString, token_line,
                 token_column, {});
    return Token(TokenType::String, value, token_line, token_column);
  }

  advance(); // 跳过结尾的 "
  return Token(TokenType::String, value, token_line, token_column);
}

Token Lexer::read_raw_string() {
  size_t token_line = tracker.get_line();
  size_t token_column = tracker.get_column();

  advance(); // 跳过 'r'

  if (!current_char.has_value() || current_char.value() != '"') {
    report_error(DiagnosticCode::L0010_InvalidCharacter, token_line,
                 token_column, {"r"});
    return Token(TokenType::Unknown, "r", token_line, token_column);
  }

  advance(); // 跳过 '"'

  std::string value;
  value.reserve(64);
  bool terminated = false;

  while (current_char.has_value()) {
    char ch = current_char.value();

    if (ch == '"') {
      terminated = true;
      break;
    }

    // 在原始字符串中，所有字符（包括反斜杠）都按其字面意义处理，不进行任何转义。
    // TODO(BegoniaHe): 同 read_string, 此处的 UTF-8 处理逻辑需要修正为
    // 一次性处理多字节字符。
    value += ch;
    advance();
  }

  if (!terminated) {
    report_error(DiagnosticCode::L0007_UnterminatedString, token_line,
                 token_column, {});
  } else {
    advance(); // 跳过结尾的 "
  }

  return Token(TokenType::String, value, token_line, token_column);
}

Lexer::Lexer(const std::string &input_str, const std::string &fname)
    : tracker(input_str, fname) {
  const auto &input = tracker.get_input();
  if (!input.empty()) {
    current_char = input[0];
  } else {
    current_char = std::nullopt;
  }
}

Token Lexer::next_token() {
  // --- 主循环：跳过空白和注释 ---
  // 确保每次进入 Token 解析逻辑时，`current_char` 都是一个有意义的字符。
  while (true) {
    skip_whitespace();

    if (current_char == '/' && peek(1) == '/') {
      skip_comment();
      continue; // 跳过注释后，可能还有更多空白，所以继续循环。
    }

    break; // 如果既不是空白也不是注释，则退出循环，开始解析 Token。
  }

  // 如果在跳过无关内容后到达了文件末尾，则返回 EOF Token。
  if (!current_char.has_value()) {
    return Token(TokenType::EndOfFile, "", tracker.get_line(),
                 tracker.get_column());
  }

  char ch = current_char.value();
  unsigned char uch = static_cast<unsigned char>(ch);
  size_t token_line = tracker.get_line();
  size_t token_column = tracker.get_column();

  // --- Token 解析分派 ---
  // 这是一个典型的词法分析器状态机分派逻辑。

  if (std::isdigit(ch)) {
    return read_number();
  }

  if (std::isalpha(ch) || ch == '_' || uch >= 0x80) {
    // 特殊情况：`r"` 前缀表示原始字符串。
    if (ch == 'r' && peek(1) == '"') {
      return read_raw_string();
    }
    return read_identifier();
  }

  if (ch == '"') {
    return read_string();
  }

  // 当前语言不支持单引号字符字面量，因此将其视为未知 Token。
  if (ch == '\'') {
    Token token(TokenType::Unknown, "'", token_line, token_column);
    advance();
    return token;
  }

  Token token(TokenType::Unknown, "", token_line, token_column);

  // --- 处理单字符和双字符运算符及分隔符 ---
  // 对于双字符运算符（如 `+=`, `==`），需要向前看（peek）一个字符来判断。
  switch (ch) {
  case '+':
    token = peek(1) == '='
                ? (advance(),
                   Token(TokenType::PlusEqual, "+=", token_line, token_column))
                : Token(TokenType::Plus, "+", token_line, token_column);
    break;
  case '-':
    token = peek(1) == '='
                ? (advance(),
                   Token(TokenType::MinusEqual, "-=", token_line, token_column))
                : Token(TokenType::Minus, "-", token_line, token_column);
    break;
  case '*':
    token = peek(1) == '='
                ? (advance(),
                   Token(TokenType::StarEqual, "*=", token_line, token_column))
                : Token(TokenType::Star, "*", token_line, token_column);
    break;
  case '/':
    token = peek(1) == '='
                ? (advance(),
                   Token(TokenType::SlashEqual, "/=", token_line, token_column))
                : Token(TokenType::Slash, "/", token_line, token_column);
    break;
  case '%':
    token = peek(1) == '='
                ? (advance(), Token(TokenType::PercentEqual, "%=", token_line,
                                    token_column))
                : Token(TokenType::Percent, "%", token_line, token_column);
    break;
  case '=':
    token = peek(1) == '='
                ? (advance(),
                   Token(TokenType::EqualEqual, "==", token_line, token_column))
                : Token(TokenType::Equal, "=", token_line, token_column);
    break;
  case '!':
    token = peek(1) == '='
                ? (advance(),
                   Token(TokenType::BangEqual, "!=", token_line, token_column))
                : Token(TokenType::Bang, "!", token_line, token_column);
    break;
  case '<':
    token = peek(1) == '='
                ? (advance(),
                   Token(TokenType::LessEqual, "<=", token_line, token_column))
                : Token(TokenType::Less, "<", token_line, token_column);
    break;
  case '>':
    token = peek(1) == '='
                ? (advance(), Token(TokenType::GreaterEqual, ">=", token_line,
                                    token_column))
                : Token(TokenType::Greater, ">", token_line, token_column);
    break;
  case '&':
    if (peek(1) == '&') {
      advance();
      token = Token(TokenType::And, "&&", token_line, token_column);
    } else {
      token = Token(TokenType::Unknown, "&", token_line, token_column);
    }
    break;
  case '|':
    if (peek(1) == '|') {
      advance();
      token = Token(TokenType::Or, "||", token_line, token_column);
    } else {
      token = Token(TokenType::Unknown, "|", token_line, token_column);
    }
    break;
  case '(':
    token = Token(TokenType::LeftParen, "(", token_line, token_column);
    break;
  case ')':
    token = Token(TokenType::RightParen, ")", token_line, token_column);
    break;
  case '{':
    token = Token(TokenType::LeftBrace, "{", token_line, token_column);
    break;
  case '}':
    token = Token(TokenType::RightBrace, "}", token_line, token_column);
    break;
  case '[':
    token = Token(TokenType::LeftBracket, "[", token_line, token_column);
    break;
  case ']':
    token = Token(TokenType::RightBracket, "]", token_line, token_column);
    break;
  case ',':
    token = Token(TokenType::Comma, ",", token_line, token_column);
    break;
  case ';':
    token = Token(TokenType::Semicolon, ";", token_line, token_column);
    break;
  case ':':
    token = Token(TokenType::Colon, ":", token_line, token_column);
    break;
  case '.':
    if (peek(1) == '.') {
      advance();
      token = Token(TokenType::DotDot, "..", token_line, token_column);
    } else {
      token = Token(TokenType::Dot, ".", token_line, token_column);
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

std::vector<Token> Lexer::tokenize() {
  std::vector<Token> tokens;
  tokens.reserve(256); // 预分配以提高性能

  while (true) {
    Token token = next_token();
    bool is_eof = (token.token_type == TokenType::EndOfFile);
    tokens.push_back(token);

    if (is_eof) {
      break;
    }
  }

  return tokens;
}

} // namespace lexer
} // namespace czc
