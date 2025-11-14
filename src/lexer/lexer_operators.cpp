/**
 * @file lexer_operators.cpp
 * @brief 运算符和分隔符解析实现
 * @details 实现所有运算符和分隔符的词法分析，包括单字符和双字符运算符、
 *          赋值运算符、比较运算符和逻辑运算符。
 * @author BegoniaHe
 * @date 2025-11-14
 */

#include "czc/diagnostics/diagnostic_code.hpp"
#include "czc/lexer/lexer.hpp"

namespace czc::lexer {

std::optional<Token> Lexer::try_read_two_char_operator(char first_char,
                                                       size_t token_line,
                                                       size_t token_column) {
  std::optional<char> next = peek(1);
  if (!next.has_value()) {
    return std::nullopt;
  }

  char second_char = next.value();

  // Helper lambda to advance and create token
  auto make_two_char_token = [&](TokenType type,
                                 const std::string& value) -> Token {
    advance();
    return Token(type, value, token_line, token_column);
  };

  switch (first_char) {
  case '+':
    if (second_char == '=')
      return make_two_char_token(TokenType::PlusEqual, "+=");
    break;
  case '-':
    if (second_char == '=')
      return make_two_char_token(TokenType::MinusEqual, "-=");
    if (second_char == '>')
      return make_two_char_token(TokenType::Arrow, "->");
    break;
  case '*':
    if (second_char == '=')
      return make_two_char_token(TokenType::StarEqual, "*=");
    break;
  case '/':
    if (second_char == '=')
      return make_two_char_token(TokenType::SlashEqual, "/=");
    break;
  case '%':
    if (second_char == '=')
      return make_two_char_token(TokenType::PercentEqual, "%=");
    break;
  case '=':
    if (second_char == '=')
      return make_two_char_token(TokenType::EqualEqual, "==");
    break;
  case '!':
    if (second_char == '=')
      return make_two_char_token(TokenType::BangEqual, "!=");
    break;
  case '<':
    if (second_char == '=')
      return make_two_char_token(TokenType::LessEqual, "<=");
    break;
  case '>':
    if (second_char == '=')
      return make_two_char_token(TokenType::GreaterEqual, ">=");
    break;
  case '&':
    if (second_char == '&')
      return make_two_char_token(TokenType::AndAnd, "&&");
    break;
  case '|':
    if (second_char == '|')
      return make_two_char_token(TokenType::OrOr, "||");
    break;
  case '.':
    if (second_char == '.')
      return make_two_char_token(TokenType::DotDot, "..");
    break;
  default:
    break;
  }

  return std::nullopt;
}

Token Lexer::read_single_char_token(char ch, size_t token_line,
                                    size_t token_column) {
  switch (ch) {
  case '+':
    return Token(TokenType::Plus, "+", token_line, token_column);
  case '-':
    return Token(TokenType::Minus, "-", token_line, token_column);
  case '*':
    return Token(TokenType::Star, "*", token_line, token_column);
  case '/':
    return Token(TokenType::Slash, "/", token_line, token_column);
  case '%':
    return Token(TokenType::Percent, "%", token_line, token_column);
  case '=':
    return Token(TokenType::Equal, "=", token_line, token_column);
  case '!':
    return Token(TokenType::Bang, "!", token_line, token_column);
  case '~':
    return Token(TokenType::Tilde, "~", token_line, token_column);
  case '<':
    return Token(TokenType::Less, "<", token_line, token_column);
  case '>':
    return Token(TokenType::Greater, ">", token_line, token_column);
  case '&':
    return Token(TokenType::And, "&", token_line, token_column);
  case '|':
    return Token(TokenType::Or, "|", token_line, token_column);
  case '(':
    return Token(TokenType::LeftParen, "(", token_line, token_column);
  case ')':
    return Token(TokenType::RightParen, ")", token_line, token_column);
  case '{':
    return Token(TokenType::LeftBrace, "{", token_line, token_column);
  case '}':
    return Token(TokenType::RightBrace, "}", token_line, token_column);
  case '[':
    return Token(TokenType::LeftBracket, "[", token_line, token_column);
  case ']':
    return Token(TokenType::RightBracket, "]", token_line, token_column);
  case ',':
    return Token(TokenType::Comma, ",", token_line, token_column);
  case ';':
    return Token(TokenType::Semicolon, ";", token_line, token_column);
  case ':':
    return Token(TokenType::Colon, ":", token_line, token_column);
  case '.':
    return Token(TokenType::Dot, ".", token_line, token_column);
  default:
    return Token(TokenType::Unknown, std::string(1, ch), token_line,
                 token_column);
  }
}

} // namespace czc::lexer