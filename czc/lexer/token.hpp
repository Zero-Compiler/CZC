/**
 * @file token.hpp
 * @brief 定义了词法单元 `Token` 及其相关类型 `TokenType`。
 * @author BegoniaHe
 * @date 2025-11-11
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
  Integer,            // 整数字面量, e.g., `123`, `0xFF`
  Float,              // 浮点数字面量, e.g., `3.14`
  String,             // 字符串字面量, e.g., `"hello"`
  Identifier,         // 标识符, e.g., `my_var`
  ScientificExponent, // 科学计数法字面量, e.g., `1.23e-4`
  Comment,            // 注释, e.g., `// comment`

  // === 关键字 ===
  Let,    // `let` 关键字
  Var,    // `var` 关键字
  Fn,     // `fn` 关键字
  Return, // `return` 关键字
  If,     // `if` 关键字
  Else,   // `else` 关键字
  While,  // `while` 关键字
  For,    // `for` 关键字
  In,     // `in` 关键字
  Struct, // `struct` 关键字
  Enum,   // `enum` 关键字
  Type,   // `type` 关键字
  Trait,  // `trait` 关键字
  True,   // `true` 布尔字面量
  False,  // `false` 布尔字面量

  // === 运算符 ===
  Plus,    // `+`
  Minus,   // `-`
  Star,    // `*`
  Slash,   // `/`
  Percent, // `%`

  // === 赋值与比较运算符 ===
  Equal,        // `=`
  PlusEqual,    // `+=`
  MinusEqual,   // `-=`
  StarEqual,    // `*=`
  PercentEqual, // `%=`
  SlashEqual,   // `/=`
  EqualEqual,   // `==`
  BangEqual,    // `!=`
  Less,         // `<`
  LessEqual,    // `<=`
  Greater,      // `>`
  GreaterEqual, // `>=`

  // === 逻辑运算符 ===
  And,  // `&&`
  Or,   // `||`
  Bang, // `!`

  // === 分隔符 ===
  LeftParen,    // `(`
  RightParen,   // `)`
  LeftBrace,    // `{`
  RightBrace,   // `}`
  LeftBracket,  // `[`
  RightBracket, // `]`
  Comma,        // `,`
  Semicolon,    // `;`
  Colon,        // `:`
  Dot,          // `.`
  DotDot,       // `..`
  Arrow,        // `->`

  // === 特殊 ===
  EndOfFile, // 表示输入流已结束的特殊 Token
  Unknown,   // 表示无法识别的字符或序列
};

/**
 * @brief 代表源代码中的一个原子性语法单元（词法单元）。
 * @details
 *   Token 是词法分析阶段的输出，也是语法分析阶段的输入。它将无结构的字符流
 *   转换为结构化的、带有明确语法含义的单元。每个 Token
 * 实例都是一个不可变的数据容器，
 *   精确记录了其类型、原始文本和源码位置，这对于后续的语法分析、CST 构建和
 *   高质量的错误报告至关重要。
 *
 * @property {生命周期} 通常作为值对象被复制和传递。
 * @property {不可变性} Token 在构造后应被视为不可变的数据容器。
 */
class Token {
public:
  // Token 的语法类型，如 `Identifier`, `Integer`, `Plus` 等。
  TokenType token_type;

  // Token 在源代码中的原始文本表示，例如 "my_var", "42", "+"。
  std::string value;

  // 对于字符串字面量，保存原始的带引号的文本（如 "\"hello\""）。
  // 这用于在格式化时保持原始格式（包括转义序列和多行字符串）。
  // 对于非字符串 Token，此字段为空。
  std::string raw_literal;

  // Token 在源代码中起始位置的行号（从 1 开始计数）。
  size_t line;

  // Token 在源代码中起始位置的列号（UTF-8 字符计数，从 1 开始）。
  size_t column;

  // 标记这是否是一个由解析器插入的虚拟 Token（用于错误恢复）。
  // 虚拟 Token 不对应源码中的实际文本，不应被格式化器输出。
  bool is_synthetic;

  // 标记字符串字面量是否为原始字符串（r"..."）。
  // 仅对 TokenType::String 有意义，其他类型忽略此字段。
  bool is_raw_string;

  /**
   * @brief 构造一个新的 Token 对象。
   * @param[in] type   Token 的类型。
   * @param[in] val    Token 的文本值。
   * @param[in] line   Token 所在的行号。
   * @param[in] column Token 开始的列号。
   * @param[in] synthetic 是否为虚拟 Token（默认为 false）。
   */
  Token(TokenType type, const std::string& val, size_t line = 0,
        size_t column = 0, bool synthetic = false);
};

/**
 * @brief 检查一个字符串是否为关键字，并返回其对应的 TokenType。
 * @param[in] word 要检查的字符串。
 * @return 如果 `word` 是一个关键字，则返回对应的 `TokenType`；
 *         否则返回 `std::nullopt`。
 */
std::optional<TokenType> get_keyword(const std::string& word);

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
