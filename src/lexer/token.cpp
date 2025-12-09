/**
 * @file token.cpp
 * @brief Token 相关实现。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-29
 *
 * @details
 *   使用 constexpr switch 实现 TokenType 到名称的映射，
 *   保证编译时安全性，避免枚举顺序依赖的问题。
 */

#include "czc/lexer/token.hpp"

#include <unordered_map>

namespace czc::lexer {

namespace {

/// 关键字到 TokenType 的映射表
const std::unordered_map<std::string_view, TokenType> kKeywordMap = {
    // 声明关键字
    {"let", TokenType::KW_LET},
    {"var", TokenType::KW_VAR},
    {"fn", TokenType::KW_FN},
    {"struct", TokenType::KW_STRUCT},
    {"enum", TokenType::KW_ENUM},
    {"type", TokenType::KW_TYPE},
    {"impl", TokenType::KW_IMPL},
    {"trait", TokenType::KW_TRAIT},
    {"return", TokenType::KW_RETURN},

    // 控制流关键字
    {"if", TokenType::KW_IF},
    {"else", TokenType::KW_ELSE},
    {"while", TokenType::KW_WHILE},
    {"for", TokenType::KW_FOR},
    {"in", TokenType::KW_IN},
    {"break", TokenType::KW_BREAK},
    {"continue", TokenType::KW_CONTINUE},
    {"match", TokenType::KW_MATCH},

    // 模块关键字
    {"import", TokenType::KW_IMPORT},
    {"as", TokenType::KW_AS},

    // 字面量关键字
    {"true", TokenType::LIT_TRUE},
    {"false", TokenType::LIT_FALSE},
    {"null", TokenType::LIT_NULL},
};

} // anonymous namespace

std::optional<TokenType> lookupKeyword(std::string_view word) {
  auto it = kKeywordMap.find(word);
  if (it != kKeywordMap.end()) {
    return it->second;
  }
  return std::nullopt;
}

/**
 * @brief 获取 TokenType 的名称字符串（编译时安全）。
 *
 * @details
 *   使用 switch 语句替代数组映射，保证：
 *   1. 枚举值与名称的对应关系在编译时检查
 *   2. 新增枚举值时编译器会警告未处理的 case
 *   3. 不依赖枚举值的顺序
 *
 * @param type Token 类型
 * @return TokenType 的名称
 */
std::string_view tokenTypeName(TokenType type) {
  // NOLINTBEGIN(bugprone-branch-clone)
  switch (type) {
  // Identifier
  case TokenType::IDENTIFIER:
    return "IDENTIFIER";

  // Keywords - Declaration
  case TokenType::KW_LET:
    return "KW_LET";
  case TokenType::KW_VAR:
    return "KW_VAR";
  case TokenType::KW_FN:
    return "KW_FN";
  case TokenType::KW_STRUCT:
    return "KW_STRUCT";
  case TokenType::KW_ENUM:
    return "KW_ENUM";
  case TokenType::KW_TYPE:
    return "KW_TYPE";
  case TokenType::KW_IMPL:
    return "KW_IMPL";
  case TokenType::KW_TRAIT:
    return "KW_TRAIT";
  case TokenType::KW_RETURN:
    return "KW_RETURN";

  // Keywords - Control Flow
  case TokenType::KW_IF:
    return "KW_IF";
  case TokenType::KW_ELSE:
    return "KW_ELSE";
  case TokenType::KW_WHILE:
    return "KW_WHILE";
  case TokenType::KW_FOR:
    return "KW_FOR";
  case TokenType::KW_IN:
    return "KW_IN";
  case TokenType::KW_BREAK:
    return "KW_BREAK";
  case TokenType::KW_CONTINUE:
    return "KW_CONTINUE";
  case TokenType::KW_MATCH:
    return "KW_MATCH";

  // Keywords - Module
  case TokenType::KW_IMPORT:
    return "KW_IMPORT";
  case TokenType::KW_AS:
    return "KW_AS";

  // Comments
  case TokenType::COMMENT_LINE:
    return "COMMENT_LINE";
  case TokenType::COMMENT_BLOCK:
    return "COMMENT_BLOCK";
  case TokenType::COMMENT_DOC:
    return "COMMENT_DOC";

  // Literals - Numeric
  case TokenType::LIT_INT:
    return "LIT_INT";
  case TokenType::LIT_FLOAT:
    return "LIT_FLOAT";
  case TokenType::LIT_DECIMAL:
    return "LIT_DECIMAL";

  // Literals - String
  case TokenType::LIT_STRING:
    return "LIT_STRING";
  case TokenType::LIT_RAW_STRING:
    return "LIT_RAW_STRING";
  case TokenType::LIT_TEX_STRING:
    return "LIT_TEX_STRING";

  // Literals - Boolean
  case TokenType::LIT_TRUE:
    return "LIT_TRUE";
  case TokenType::LIT_FALSE:
    return "LIT_FALSE";

  // Literals - Null
  case TokenType::LIT_NULL:
    return "LIT_NULL";

  // Operators - Arithmetic
  case TokenType::OP_PLUS:
    return "OP_PLUS";
  case TokenType::OP_MINUS:
    return "OP_MINUS";
  case TokenType::OP_STAR:
    return "OP_STAR";
  case TokenType::OP_SLASH:
    return "OP_SLASH";
  case TokenType::OP_PERCENT:
    return "OP_PERCENT";

  // Operators - Comparison
  case TokenType::OP_EQ:
    return "OP_EQ";
  case TokenType::OP_NE:
    return "OP_NE";
  case TokenType::OP_LT:
    return "OP_LT";
  case TokenType::OP_LE:
    return "OP_LE";
  case TokenType::OP_GT:
    return "OP_GT";
  case TokenType::OP_GE:
    return "OP_GE";

  // Operators - Logical
  case TokenType::OP_LOGICAL_AND:
    return "OP_LOGICAL_AND";
  case TokenType::OP_LOGICAL_OR:
    return "OP_LOGICAL_OR";
  case TokenType::OP_LOGICAL_NOT:
    return "OP_LOGICAL_NOT";

  // Operators - Bitwise
  case TokenType::OP_BIT_AND:
    return "OP_BIT_AND";
  case TokenType::OP_BIT_OR:
    return "OP_BIT_OR";
  case TokenType::OP_BIT_XOR:
    return "OP_BIT_XOR";
  case TokenType::OP_BIT_NOT:
    return "OP_BIT_NOT";
  case TokenType::OP_BIT_SHL:
    return "OP_BIT_SHL";
  case TokenType::OP_BIT_SHR:
    return "OP_BIT_SHR";

  // Operators - Assignment
  case TokenType::OP_ASSIGN:
    return "OP_ASSIGN";
  case TokenType::OP_PLUS_ASSIGN:
    return "OP_PLUS_ASSIGN";
  case TokenType::OP_MINUS_ASSIGN:
    return "OP_MINUS_ASSIGN";
  case TokenType::OP_STAR_ASSIGN:
    return "OP_STAR_ASSIGN";
  case TokenType::OP_SLASH_ASSIGN:
    return "OP_SLASH_ASSIGN";
  case TokenType::OP_PERCENT_ASSIGN:
    return "OP_PERCENT_ASSIGN";
  case TokenType::OP_AND_ASSIGN:
    return "OP_AND_ASSIGN";
  case TokenType::OP_OR_ASSIGN:
    return "OP_OR_ASSIGN";
  case TokenType::OP_XOR_ASSIGN:
    return "OP_XOR_ASSIGN";
  case TokenType::OP_SHL_ASSIGN:
    return "OP_SHL_ASSIGN";
  case TokenType::OP_SHR_ASSIGN:
    return "OP_SHR_ASSIGN";

  // Operators - Range
  case TokenType::OP_DOT_DOT:
    return "OP_DOT_DOT";
  case TokenType::OP_DOT_DOT_EQ:
    return "OP_DOT_DOT_EQ";

  // Operators - Other
  case TokenType::OP_ARROW:
    return "OP_ARROW";
  case TokenType::OP_FAT_ARROW:
    return "OP_FAT_ARROW";
  case TokenType::OP_DOT:
    return "OP_DOT";
  case TokenType::OP_AT:
    return "OP_AT";
  case TokenType::OP_COLON_COLON:
    return "OP_COLON_COLON";

  // Delimiters
  case TokenType::DELIM_LPAREN:
    return "DELIM_LPAREN";
  case TokenType::DELIM_RPAREN:
    return "DELIM_RPAREN";
  case TokenType::DELIM_LBRACE:
    return "DELIM_LBRACE";
  case TokenType::DELIM_RBRACE:
    return "DELIM_RBRACE";
  case TokenType::DELIM_LBRACKET:
    return "DELIM_LBRACKET";
  case TokenType::DELIM_RBRACKET:
    return "DELIM_RBRACKET";
  case TokenType::DELIM_COMMA:
    return "DELIM_COMMA";
  case TokenType::DELIM_COLON:
    return "DELIM_COLON";
  case TokenType::DELIM_SEMICOLON:
    return "DELIM_SEMICOLON";
  case TokenType::DELIM_UNDERSCORE:
    return "DELIM_UNDERSCORE";

  // Reserved Operators
  case TokenType::OP_HASH:
    return "OP_HASH";
  case TokenType::OP_DOLLAR:
    return "OP_DOLLAR";
  case TokenType::OP_BACKSLASH:
    return "OP_BACKSLASH";

  // Special Tokens
  case TokenType::TOKEN_NEWLINE:
    return "TOKEN_NEWLINE";
  case TokenType::TOKEN_EOF:
    return "TOKEN_EOF";
  case TokenType::TOKEN_WHITESPACE:
    return "TOKEN_WHITESPACE";
  case TokenType::TOKEN_UNKNOWN:
    return "TOKEN_UNKNOWN";
  }
  // NOLINTEND(bugprone-branch-clone)

  // 使用 CZC_UNREACHABLE() 标记不可达代码
  // 如果到达这里，说明枚举值未在 switch 中处理
  CZC_UNREACHABLE();
}

} // namespace czc::lexer
