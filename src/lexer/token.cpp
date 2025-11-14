/**
 * @file token.cpp
 * @brief `Token` 类及相关工具函数的实现。
 * @author BegoniaHe
 * @date 2025-11-11
 */

#include "czc/lexer/token.hpp"

#include <unordered_map>

namespace czc::lexer {

Token::Token(TokenType type, const std::string& val, size_t line, size_t column,
             bool synthetic)
    : token_type(type), value(val), line(line), column(column),
      is_synthetic(synthetic) {}

std::optional<TokenType> get_keyword(const std::string& word) {
  // NOTE: 使用静态哈希表优化关键字查找性能。
  //       相比线性搜索，哈希表查找的时间复杂度从 O(n) 降低到 O(1)。
  //       对于 15+ 个关键字的场景，这能带来明显的性能提升。
  //       静态局部变量确保哈希表只初始化一次，避免重复构建开销。
  static const std::unordered_map<std::string, TokenType> KEYWORDS = {
      {"let", TokenType::Let},     {"var", TokenType::Var},
      {"fn", TokenType::Fn},       {"return", TokenType::Return},
      {"if", TokenType::If},       {"else", TokenType::Else},
      {"while", TokenType::While}, {"for", TokenType::For},
      {"in", TokenType::In},       {"struct", TokenType::Struct},
      {"enum", TokenType::Enum},   {"type", TokenType::Type},
      {"trait", TokenType::Trait}, {"true", TokenType::True},
      {"false", TokenType::False}};

  auto it = KEYWORDS.find(word);
  if (it != KEYWORDS.end()) {
    return it->second;
  }

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
  case TokenType::Comment:
    return "Comment";
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
  case TokenType::Struct:
    return "Struct";
  case TokenType::Enum:
    return "Enum";
  case TokenType::Type:
    return "Type";
  case TokenType::Trait:
    return "Trait";
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
  case TokenType::AndAnd:
    return "AndAnd";
  case TokenType::OrOr:
    return "OrOr";
  case TokenType::Tilde:
    return "Tilde";
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
  default:
    return "Unknown";
  }
}

} // namespace czc::lexer
