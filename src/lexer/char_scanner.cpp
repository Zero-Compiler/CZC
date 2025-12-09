/**
 * @file char_scanner.cpp
 * @brief 字符/运算符/分隔符扫描器的实现。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-29
 *
 * 使用查表法进行字符扫描，支持单字符、双字符和三字符运算符。
 * 采用贪婪匹配策略，优先匹配最长的运算符。
 */

#include "czc/lexer/char_scanner.hpp"
#include <unordered_map>

namespace czc::lexer {

namespace {

/**
 * @brief 单字符运算符/分隔符查找表。
 */
const std::unordered_map<char, TokenType> kSingleCharTokens = {
    // 分隔符
    {'(', TokenType::DELIM_LPAREN},
    {')', TokenType::DELIM_RPAREN},
    {'{', TokenType::DELIM_LBRACE},
    {'}', TokenType::DELIM_RBRACE},
    {'[', TokenType::DELIM_LBRACKET},
    {']', TokenType::DELIM_RBRACKET},
    {',', TokenType::DELIM_COMMA},
    {';', TokenType::DELIM_SEMICOLON},
    {'_', TokenType::DELIM_UNDERSCORE},

    // 运算符
    {'@', TokenType::OP_AT},
    {'#', TokenType::OP_HASH},
    {'$', TokenType::OP_DOLLAR},
    {'\\', TokenType::OP_BACKSLASH},
};

/**
 * @brief 可能是多字符运算符起始的单字符运算符。
 * 这些字符在不构成多字符运算符时的默认类型。
 */
const std::unordered_map<char, TokenType> kPotentialMultiCharStart = {
    {'+', TokenType::OP_PLUS},        {'-', TokenType::OP_MINUS},
    {'*', TokenType::OP_STAR},        {'/', TokenType::OP_SLASH},
    {'%', TokenType::OP_PERCENT},     {'&', TokenType::OP_BIT_AND},
    {'|', TokenType::OP_BIT_OR},      {'^', TokenType::OP_BIT_XOR},
    {'~', TokenType::OP_BIT_NOT},     {'<', TokenType::OP_LT},
    {'>', TokenType::OP_GT},          {'=', TokenType::OP_ASSIGN},
    {'!', TokenType::OP_LOGICAL_NOT}, {'.', TokenType::OP_DOT},
    {':', TokenType::DELIM_COLON},
};

/**
 * @brief 双字符运算符查找表。
 * 使用两字符组合作为键。
 */
const std::unordered_map<std::string_view, TokenType> kDoubleCharTokens = {
    // 比较运算符
    {"==", TokenType::OP_EQ},
    {"!=", TokenType::OP_NE},
    {"<=", TokenType::OP_LE},
    {">=", TokenType::OP_GE},

    // 逻辑运算符
    {"&&", TokenType::OP_LOGICAL_AND},
    {"||", TokenType::OP_LOGICAL_OR},

    // 赋值运算符
    {"+=", TokenType::OP_PLUS_ASSIGN},
    {"-=", TokenType::OP_MINUS_ASSIGN},
    {"*=", TokenType::OP_STAR_ASSIGN},
    {"/=", TokenType::OP_SLASH_ASSIGN},
    {"%=", TokenType::OP_PERCENT_ASSIGN},
    {"&=", TokenType::OP_AND_ASSIGN},
    {"|=", TokenType::OP_OR_ASSIGN},
    {"^=", TokenType::OP_XOR_ASSIGN},

    // 位移运算符
    {"<<", TokenType::OP_BIT_SHL},
    {">>", TokenType::OP_BIT_SHR},

    // 箭头
    {"->", TokenType::OP_ARROW},
    {"=>", TokenType::OP_FAT_ARROW},

    // 范围运算符
    {"..", TokenType::OP_DOT_DOT},

    // 其他
    {"::", TokenType::OP_COLON_COLON},
};

/**
 * @brief 三字符运算符查找表。
 */
const std::unordered_map<std::string_view, TokenType> kTripleCharTokens = {
    // 位移赋值
    {"<<=", TokenType::OP_SHL_ASSIGN},
    {">>=", TokenType::OP_SHR_ASSIGN},

    // 范围运算符
    {"..=", TokenType::OP_DOT_DOT_EQ},
};

} // anonymous namespace

bool CharScanner::canScan(const ScanContext &ctx) const noexcept {
  auto ch = ctx.current();
  if (!ch.has_value()) {
    return false;
  }

  char c = ch.value();

  // 检查单字符表
  if (kSingleCharTokens.contains(c)) {
    return true;
  }

  // 检查多字符起始表
  if (kPotentialMultiCharStart.contains(c)) {
    return true;
  }

  return false;
}

Token CharScanner::scan(ScanContext &ctx) const {
  std::size_t startOffset = ctx.offset();
  SourceLocation startLoc = ctx.location();

  auto ch = ctx.current();
  if (!ch.has_value()) {
    return ctx.makeUnknown(startOffset, startLoc);
  }

  char first = ch.value();

  // 尝试三字符运算符
  auto second = ctx.peek(1);
  auto third = ctx.peek(2);

  if (second.has_value() && third.has_value()) {
    char chars[4] = {first, second.value(), third.value(), '\0'};
    std::string_view threeChar(chars, 3);

    auto it = kTripleCharTokens.find(threeChar);
    if (it != kTripleCharTokens.end()) {
      ctx.advance(3);
      return ctx.makeToken(it->second, startOffset, startLoc);
    }
  }

  // 尝试双字符运算符
  if (second.has_value()) {
    char chars[3] = {first, second.value(), '\0'};
    std::string_view twoChar(chars, 2);

    auto it = kDoubleCharTokens.find(twoChar);
    if (it != kDoubleCharTokens.end()) {
      ctx.advance(2);
      return ctx.makeToken(it->second, startOffset, startLoc);
    }
  }

  // 检查单字符表
  auto singleIt = kSingleCharTokens.find(first);
  if (singleIt != kSingleCharTokens.end()) {
    ctx.advance();
    return ctx.makeToken(singleIt->second, startOffset, startLoc);
  }

  // 检查多字符起始表（作为单字符使用）
  auto multiIt = kPotentialMultiCharStart.find(first);
  if (multiIt != kPotentialMultiCharStart.end()) {
    ctx.advance();
    return ctx.makeToken(multiIt->second, startOffset, startLoc);
  }

  // 未知字符
  ctx.advance();
  return ctx.makeUnknown(startOffset, startLoc);
}

} // namespace czc::lexer
