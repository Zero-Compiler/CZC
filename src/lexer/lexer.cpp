/**
 * @file lexer.cpp
 * @brief `Lexer` 类的功能实现。
 * @author BegoniaHe
 * @date 2025-11-11
 */

#include "czc/lexer/lexer.hpp"

#include "czc/lexer/utf8_handler.hpp"

#include <cctype>
#include <iomanip>
#include <sstream>

namespace czc::lexer {

using namespace czc::diagnostics;
using namespace czc::utils;

void Lexer::report_error(DiagnosticCode code, size_t error_line,
                         size_t error_column,
                         const std::vector<std::string>& args) {
  // NOTE: 创建一个只包含错误发生点的 SourceLocation。对于词法错误，
  //       通常我们只关心单个字符或符号的位置，因此起始和结束位置是相同的。
  auto loc = SourceLocation(tracker.get_filename(), error_line, error_column,
                            error_line, error_column);
  LexerError error(code, loc, args);
  error_collector.add(error);
}

void Lexer::advance() {
  // --- 防御性检查 ---
  // 如果已经到达源码末尾，则无需再前进，防止 tracker 越界。
  if (!current_char.has_value()) {
    return;
  }

  // NOTE: 将当前字符传递给 tracker 是 advance 逻辑的核心。tracker 内部会
  //       检查该字符是否为换行符，并据此更新其内部的行号和列号状态。
  //       这是将位置跟踪逻辑与词法分析规则逻辑解耦的关键。
  char ch = current_char.value();
  tracker.advance(ch);

  size_t pos = tracker.get_position();
  const auto& input = tracker.get_input();

  // --- 更新当前字符 ---
  // 更新 current_char 为 tracker 前进后的新位置上的字符。
  // 如果到达末尾，则设置为 std::nullopt，这将作为后续所有处理的终止信号。
  if (pos < input.size()) {
    current_char = input[pos];
  } else {
    current_char = std::nullopt;
  }
}

std::optional<char> Lexer::peek(size_t offset) const {
  size_t peek_pos = tracker.get_position() + offset;
  const auto& input = tracker.get_input();
  if (peek_pos < input.size()) {
    return input[peek_pos];
  }
  return std::nullopt;
}

void Lexer::skip_whitespace() {
  // NOTE: 持续消耗字符，直到遇到非空白字符或源码结束。
  //       `isspace` 可以正确处理空格、制表符、换行符等多种空白字符。
  //       转换为 unsigned char 以避免在 char 为有符号类型的平台上出现 UB。
  while (current_char.has_value() &&
         std::isspace(static_cast<unsigned char>(current_char.value()))) {
    advance();
  }
}

Token Lexer::read_comment() {
  // --- 单行注释处理 ---
  // 检查是否是 `//` 开头的单行注释。
  size_t token_line = tracker.get_line();
  size_t token_column = tracker.get_column();
  std::string comment_text;

  if (current_char == '/' && peek(1) == '/') {
    // 记录 "//"
    comment_text += "//";
    advance(); // 跳过第一个 '/'
    advance(); // 跳过第二个 '/'

    // 读取注释内容直到行尾（`\n`）或文件末尾。
    while (current_char.has_value() && current_char != '\n') {
      comment_text += current_char.value();
      advance();
    }

    // NOTE: 如果是因为换行符而停止，则需要额外调用一次 advance() 来消耗掉
    //       这个换行符本身，以便下一次 next_token() 从新的一行开始。
    if (current_char == '\n') {
      advance();
    }

    return Token(TokenType::Comment, comment_text, token_line, token_column);
  }

  // 如果不是注释，返回一个 Unknown token。
  return Token(TokenType::Unknown, "", token_line, token_column);
}

Token Lexer::read_identifier() {
  size_t start = tracker.get_position();
  size_t token_line = tracker.get_line();
  size_t token_column = tracker.get_column();

  // 处理标识符的第一个字符
  if (current_char.has_value()) {
    unsigned char uch = static_cast<unsigned char>(current_char.value());

    // 如果第一个字符是UTF-8起始字节,使用Utf8Handler读取完整字符
    if (uch >= 0x80) {
      size_t current_pos = tracker.get_position();
      std::string utf8_char;
      const auto& input = tracker.get_input();

      if (Utf8Handler::read_char(input, current_pos, utf8_char)) {
        // UTF-8 字符有效，同步 tracker 的位置
        while (tracker.get_position() < current_pos) {
          advance();
        }
      } else {
        // 第一个字符就是无效的UTF-8序列,返回错误token
        advance();
        return Token(TokenType::Unknown, std::string(input.data() + start, 1),
                     token_line, token_column);
      }
    } else {
      // ASCII字符,直接消耗
      advance();
    }
  }

  // 继续读取后续字符
  while (current_char.has_value()) {
    char ch = current_char.value();
    unsigned char uch = static_cast<unsigned char>(ch);

    // 标识符可以包含字母、数字、下划线
    if (std::isalnum(static_cast<unsigned char>(ch)) || ch == '_') {
      advance();
    }
    // NOTE: 对于非 ASCII 字符（UTF-8 起始字节），使用 Utf8Handler
    //       来正确读取完整的多字节字符序列，并验证其有效性。
    else if (uch >= 0x80) {
      size_t current_pos = tracker.get_position();
      std::string utf8_char;
      const auto& input = tracker.get_input();

      if (Utf8Handler::read_char(input, current_pos, utf8_char)) {
        // UTF-8 字符有效，同步 tracker 的位置
        while (tracker.get_position() < current_pos) {
          advance();
        }
      } else {
        // 遇到无效的 UTF-8 序列，标识符在此处结束
        break;
      }
    } else {
      break;
    }
  }

  size_t current_pos = tracker.get_position();
  const auto& input = tracker.get_input();
  std::string value(input.data() + start, current_pos - start);

  // 检查解析出的字符串是否是语言的关键字。
  auto keyword_type = get_keyword(value);
  // 如果是关键字，则使用关键字的 Token 类型；否则，它是一个普通的标识符。
  TokenType token_type = keyword_type.value_or(TokenType::Identifier);

  return Token(token_type, value, token_line, token_column);
}

Lexer::Lexer(const std::string& input_str, const std::string& fname)
    : tracker(input_str, fname) {
  const auto& input = tracker.get_input();
  if (!input.empty()) {
    current_char = input[0];
  } else {
    current_char = std::nullopt;
  }
}

Token Lexer::next_token() {
  // --- 主循环：跳过空白 ---
  // NOTE: 这个循环是词法分析器的"空转"阶段。它的任务是不断地跳过
  //       所有空白字符，直到 `current_char` 指向一个潜在 Token 的起始字符，
  //       或者到达文件末尾。注释现在被视为有效的 Token。
  skip_whitespace();

  // 如果在跳过空白后到达了文件末尾，则返回 EOF Token。
  if (!current_char.has_value()) {
    return Token::makeEOF();
  }

  // 检查是否是注释
  if (current_char == '/' && peek(1) == '/') {
    return read_comment();
  }

  char ch = current_char.value();
  unsigned char uch = static_cast<unsigned char>(ch);
  size_t token_line = tracker.get_line();
  size_t token_column = tracker.get_column();

  // --- Token 解析分派 ---
  // 这是一个典型的词法分析器状态机分派逻辑。

  if (std::isdigit(static_cast<unsigned char>(ch))) {
    return read_number();
  }

  if (std::isalpha(static_cast<unsigned char>(ch)) || ch == '_' ||
      uch >= 0x80) {
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
  // NOTE: 对于可能构成双字符运算符（如 `+=`, `==`）的字符，词法分析器
  //       必须向前“看”一个字符（peek）来做出决定。如果匹配，则消耗两个
  //       字符并返回双字符 Token；否则，只消耗当前字符并返回单字符 Token。
  //       这种策略被称为“最大匹配原则”（Maximal Munch Principle）。
  switch (ch) {
  case '+':
    token = peek(1) == '='
                ? (advance(),
                   Token(TokenType::PlusEqual, "+=", token_line, token_column))
                : Token(TokenType::Plus, "+", token_line, token_column);
    break;
  case '-':
    if (peek(1) == '=') {
      advance();
      token = Token(TokenType::MinusEqual, "-=", token_line, token_column);
    } else if (peek(1) == '>') {
      advance();
      token = Token(TokenType::Arrow, "->", token_line, token_column);
    } else {
      token = Token(TokenType::Minus, "-", token_line, token_column);
    }
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
  case '~':
    token = Token(TokenType::Tilde, "~", token_line, token_column);
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
      token = Token(TokenType::AndAnd, "&&", token_line, token_column);
    } else {
      token = Token(TokenType::And, "&", token_line, token_column);
    }
    break;
  case '|':
    if (peek(1) == '|') {
      advance();
      token = Token(TokenType::OrOr, "||", token_line, token_column);
    } else {
      token = Token(TokenType::Or, "|", token_line, token_column);
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

} // namespace czc::lexer
