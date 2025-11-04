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

namespace czc {
namespace lexer {

/**
 * @brief 定义了词法分析器可以产生的所有词法单元（Token）的类型。
 * @details
 *   此枚举是编译器语法分析的基础，它将源代码的字符流转换为
 *   具有明确语法含义的离散单元。
 */
enum class TokenType {
  // === 字面量 ===
  Integer,            ///< 整数字面量, e.g., `123`, `0xFF`
  Float,              ///< 浮点数字面量, e.g., `3.14`
  String,             ///< 字符串字面量, e.g., `"hello"`
  Identifier,         ///< 标识符, e.g., `my_var`
  ScientificExponent, ///< 科学计数法字面量, e.g., `1.23e-4`

  // === 关键字 ===
  Let,    ///< `let` 关键字
  Var,    ///< `var` 关键字
  Fn,     ///< `fn` 关键字
  Return, ///< `return` 关键字
  If,     ///< `if` 关键字
  Else,   ///< `else` 关键字
  While,  ///< `while` 关键字
  For,    ///< `for` 关键字
  In,     ///< `in` 关键字
  True,   ///< `true` 布尔字面量
  False,  ///< `false` 布尔字面量

  // === 运算符 ===
  Plus,    ///< `+`
  Minus,   ///< `-`
  Star,    ///< `*`
  Slash,   ///< `/`
  Percent, ///< `%`

  // === 赋值与比较运算符 ===
  Equal,        ///< `=`
  PlusEqual,    ///< `+=`
  MinusEqual,   ///< `-=`
  StarEqual,    ///< `*=`
  PercentEqual, ///< `%=`
  SlashEqual,   ///< `/=`
  EqualEqual,   ///< `==`
  BangEqual,    ///< `!=`
  Less,         ///< `<`
  LessEqual,    ///< `<=`
  Greater,      ///< `>`
  GreaterEqual, ///< `>=`

  // === 逻辑运算符 ===
  And,  ///< `&&`
  Or,   ///< `||`
  Bang, ///< `!`

  // === 分隔符 ===
  LeftParen,    ///< `(`
  RightParen,   ///< `)`
  LeftBrace,    ///< `{`
  RightBrace,   ///< `}`
  LeftBracket,  ///< `[`
  RightBracket, ///< `]`
  Comma,        ///< `,`
  Semicolon,    ///< `;`
  Colon,        ///< `:`
  Dot,          ///< `.`
  DotDot,       ///< `..`

  // === 特殊 ===
  EndOfFile, ///< 表示输入流已结束的特殊 Token
  Unknown,   ///< 表示无法识别的字符或序列
};

/**
 * @brief 代表源代码中的一个词法单元。
 * @details
 *   Token 是编译器进行语法分析的基本单位。每个 Token 都包含其类型、
 *   在源代码中的原始文本值，以及精确的源码位置，这对于错误报告至关重要。
 */
class Token {
public:
  // Token 的类型，定义了其语法含义。
  TokenType token_type;
  // Token 在源代码中的原始文本表示。
  std::string value;
  // Token 开始处的行号（从 1 开始计数）。
  size_t line;
  // Token 开始处的列号（从 1 开始计数）。
  size_t column;

  /**
   * @brief 构造一个新的 Token 对象。
   * @param[in] type   Token 的类型。
   * @param[in] val    Token 的文本值。
   * @param[in] line   Token 所在的行号。
   * @param[in] column Token 开始的列号。
   */
  Token(TokenType type, const std::string &val, size_t line = 0,
        size_t column = 0);
};

/**
 * @brief 检查一个字符串是否为关键字，并返回其对应的 TokenType。
 * @param[in] word 要检查的字符串。
 * @return 如果 `word` 是一个关键字，则返回对应的 `TokenType`；
 *         否则返回 `std::nullopt`。
 */
std::optional<TokenType> get_keyword(const std::string &word);

/**
 * @brief 将 TokenType 枚举转换为人类可读的字符串表示。
 * @details 用于调试和错误报告。
 * @param[in] type 要转换的 TokenType。
 * @return 该类型的字符串表示（例如，"TokenType::Integer"）。
 */
std::string token_type_to_string(TokenType type);

} // namespace lexer
} // namespace czc

#endif // CZC_LEXER_TOKEN_HPP
