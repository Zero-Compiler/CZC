/**
 * @file token.cpp
 * @brief Token 类实现
 * @author BegoniaHe
 * @date 2025-11-04
 */

#include "czc/lexer/token.hpp"

/**
 * @brief Token 构造函数
 * @param type Token 类型
 * @param val Token 的值
 * @param line 行号
 * @param column 列号
 */
Token::Token(TokenType type, const std::string &val, size_t line, size_t column)
    : token_type(type), value(val), line(line), column(column) {}

/**
 * @brief 根据字符串获取关键字类型
 * @param word 待检查的字符串
 * @return 如果是关键字, 返回对应的 TokenType, 否则返回 std::nullopt
 */
std::optional<TokenType> get_keyword(const std::string &word) {
  // 这是一个简单的线性搜索，用于将字符串映射到关键字 Token 类型。
  // 对于少量关键字，这种方法足够高效。如果关键字数量庞大，
  // 可以考虑使用 std::unordered_map 或完美的哈希函数来优化性能。
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
  // 表明它是一个标识符。
  return std::nullopt;
}

/**
 * @brief 将 TokenType 转换为字符串表示
 * @param type Token 类型
 * @return 类型的字符串表示
 */
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
  case TokenType::EndOfFile:
    return "EOF";
  case TokenType::Unknown:
    return "Unknown";
  default:
    return "Unknown";
  }
}
