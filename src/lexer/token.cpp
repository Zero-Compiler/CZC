/**
 * @file token.cpp
 * @brief `Token` 类及相关工具函数的实现。
 * @author BegoniaHe
 * @date 2025-11-05
 */

#include "czc/lexer/token.hpp"

namespace czc {
namespace lexer {

Token::Token(TokenType type, const std::string &val, size_t line, size_t column)
    : token_type(type), value(val), line(line), column(column) {}

std::optional<TokenType> get_keyword(const std::string &word) {
  // NOTE: 这是一个简单的线性搜索，用于将标识符字符串映射到其对应的关键字
  //       Token 类型。对于当前语言这种关键字数量较少（约10-20个）的情况，
  //       一连串的 `if` 比较通常比 `std::unordered_map` 更快，因为它避免了
  //       哈希计算和潜在的哈希冲突处理开销。如果未来关键字数量大幅增加，
  //       可以考虑使用完美的哈希函数生成器（如
  //       gperf）来生成一个更高效的查找表。
  if (word == "let")
    return TokenType::Let;
  if (word == "var")
    return TokenType::Var;
  if (word == "fn")
    return TokenType::Fn;
  if (word == "return")
    return TokenType::Return;
  if (word == "if")
    return TokenType::If;
  if (word == "else")
    return TokenType::Else;
  if (word == "while")
    return TokenType::While;
  if (word == "for")
    return TokenType::For;
  if (word == "in")
    return TokenType::In;
  if (word == "true")
    return TokenType::True;
  if (word == "false")
    return TokenType::False;

  // 如果字符串不匹配任何关键字，则返回空的可选值，
  // 表明它应该被视为一个普通标识符。
  return std::nullopt;
}

std::string token_type_to_string(TokenType type) {
  switch (type) {
  case TokenType::Integer:
    return "Integer";
  case TokenType::Float:
    return "Float";
  case TokenType::String:
    return "String";
  case TokenType::Identifier:
    return "Identifier";
  case TokenType::ScientificExponent:
    return "ScientificExponent";
  case TokenType::Let:
    return "Let";
  case TokenType::Var:
    return "Var";
  case TokenType::Fn:
    return "Fn";
  case TokenType::Return:
    return "Return";
  case TokenType::If:
    return "If";
  case TokenType::Else:
    return "Else";
  case TokenType::While:
    return "While";
  case TokenType::For:
    return "For";
  case TokenType::In:
    return "In";
  case TokenType::True:
    return "True";
  case TokenType::False:
    return "False";
  case TokenType::Plus:
    return "Plus";
  case TokenType::Minus:
    return "Minus";
  case TokenType::Star:
    return "Star";
  case TokenType::Slash:
    return "Slash";
  case TokenType::Percent:
    return "Percent";
  case TokenType::Equal:
    return "Equal";
  case TokenType::PlusEqual:
    return "PlusEqual";
  case TokenType::MinusEqual:
    return "MinusEqual";
  case TokenType::StarEqual:
    return "StarEqual";
  case TokenType::PercentEqual:
    return "PercentEqual";
  case TokenType::SlashEqual:
    return "SlashEqual";
  case TokenType::EqualEqual:
    return "EqualEqual";
  case TokenType::Bang:
    return "Bang";
  case TokenType::BangEqual:
    return "BangEqual";
  case TokenType::Less:
    return "Less";
  case TokenType::LessEqual:
    return "LessEqual";
  case TokenType::Greater:
    return "Greater";
  case TokenType::GreaterEqual:
    return "GreaterEqual";
  case TokenType::And:
    return "And";
  case TokenType::Or:
    return "Or";
  case TokenType::LeftParen:
    return "LeftParen";
  case TokenType::RightParen:
    return "RightParen";
  case TokenType::LeftBrace:
    return "LeftBrace";
  case TokenType::RightBrace:
    return "RightBrace";
  case TokenType::LeftBracket:
    return "LeftBracket";
  case TokenType::RightBracket:
    return "RightBracket";
  case TokenType::Comma:
    return "Comma";
  case TokenType::Semicolon:
    return "Semicolon";
  case TokenType::Colon:
    return "Colon";
  case TokenType::Dot:
    return "Dot";
  case TokenType::DotDot:
    return "DotDot";
  case TokenType::Arrow:
    return "Arrow";
  case TokenType::EndOfFile:
    return "EOF";
  case TokenType::Unknown:
    return "Unknown";
  default:
    return "Unknown";
  }
}

} // namespace lexer
} // namespace czc
