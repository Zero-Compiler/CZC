/**
 * @file token.hpp
 * @brief Token 类型和类定义
 * @author BegoniaHe
 * @date 2025-11-04
 */

#ifndef CZC_LEXER_TOKEN_HPP
#define CZC_LEXER_TOKEN_HPP

#include <optional>
#include <string>

/**
 * @brief Token 类型枚举
 */
enum class TokenType {
  // 字面量
  Integer,            // 整数字面量
  Float,              // 浮点数字面量
  String,             // 字符串字面量
  Identifier,         // 标识符
  ScientificExponent, // 科学计数法字面量

  // 关键字
  Let,    // let 关键字
  Var,    // var 关键字
  Fn,     // fn 关键字
  Return, // return 关键字
  If,     // if 关键字
  Else,   // else 关键字
  While,  // while 关键字
  For,    // for 关键字
  In,     // in 关键字
  True,   // true 关键字
  False,  // false 关键字

  // 运算符
  Plus,    // +
  Minus,   // -
  Star,    // *
  Slash,   // /
  Percent, // %

  // 赋值与比较运算符
  Equal,        // =
  PlusEqual,    // +=
  MinusEqual,   // -=
  StarEqual,    // *=
  PercentEqual, // %=
  SlashEqual,   // /=
  EqualEqual,   // ==
  BangEqual,    // !=
  Less,         // <
  LessEqual,    // <=
  Greater,      // >
  GreaterEqual, // >=

  // 逻辑运算符
  And,  // &&
  Or,   // ||
  Bang, // !

  // 分隔符
  LeftParen,    // (
  RightParen,   // )
  LeftBrace,    // {
  RightBrace,   // }
  LeftBracket,  // [
  RightBracket, // ]
  Comma,        // ,
  Semicolon,    // ;
  Colon,        // :
  Dot,          // .
  DotDot,       // ..

  // 特殊
  EndOfFile, // 文件结束
  Unknown,   // 未知 Token
};

/**
 * @brief 代表源代码中的一个词法单元。
 * @details
 *   Token 是编译器进行语法分析的基本单位。每个 Token 都包含其类型
 *   （例如标识符、整数、关键字）、其在源代码中的原始文本值，
 *   以及它在源文件中的位置（行号和列号），这对于错误报告至关重要。
 */
class Token {
public:
  /// @brief Token 的类型，定义了其语法含义。
  TokenType token_type;
  /// @brief Token 在源代码中的原始文本表示。
  std::string value;
  /// @brief Token 开始处的行号（从 1 开始）。
  size_t line;
  /// @brief Token 开始处的列号（从 1 开始）。
  size_t column;

  /**
   * @brief 构造一个新的 Token 对象。
   * @param[in] type Token 的类型。
   * @param[in] val Token 的文本值。
   * @param[in] line Token 所在的行号。
   * @param[in] column Token 开始的列号。
   */
  Token(TokenType type, const std::string &val, size_t line = 0,
        size_t column = 0);
};

/**
 * @brief 获取关键字对应的 Token 类型
 * @param word 关键字字符串
 * @return Token 类型的可选值
 */
std::optional<TokenType> get_keyword(const std::string &word);

/**
 * @brief 将 Token 类型转换为字符串表示
 * @param type Token 类型
 * @return Token 类型的字符串表示
 */
std::string token_type_to_string(TokenType type);

#endif // CZC_LEXER_TOKEN_HPP
