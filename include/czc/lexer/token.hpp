/**
 * @file token.hpp
 * @brief Token definitions for the CZC lexer.
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-29
 *
 * @details
 *   本文件定义了 CZC 编译器词法分析器的核心类型：
 *   - TokenType: Token 类型枚举
 *   - SourceLocation: 源码位置信息
 *   - Trivia: 附加在 Token 上的空白和注释
 *   - Token: 词法单元类
 *
 *   Token 采用基于偏移量的存储设计，通过 SourceManager 获取实际文本。
 *   这种设计确保 Token 的生命周期安全——只要 SourceManager 存活，Token
 * 就永远有效。
 */

#ifndef CZC_LEXER_TOKEN_HPP
#define CZC_LEXER_TOKEN_HPP

#include "czc/common/config.hpp"
#include "czc/lexer/source_manager.hpp"

#include <bitset>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace czc::lexer {

/**
 * @brief Token 类型枚举。
 *
 * @details
 *   定义了词法分析器可以产生的所有词法单元类型。
 *   命名规范：
 *   - 关键字: KW_ 前缀
 *   - 字面量: LIT_ 前缀
 *   - 运算符: OP_ 前缀
 *   - 分隔符: DELIM_ 前缀
 *   - 注释: COMMENT_ 前缀
 *   - 特殊: TOKEN_ 前缀
 */
enum class TokenType {
  IDENTIFIER,

  // Keywords
  KW_LET,    // let
  KW_VAR,    // var
  KW_FN,     // fn
  KW_STRUCT, // struct
  KW_ENUM,   // enum
  KW_TYPE,   // type
  KW_IMPL,   // impl
  KW_TRAIT,  // trait
  KW_RETURN, // return

  KW_IF,       // if
  KW_ELSE,     // else
  KW_WHILE,    // while
  KW_FOR,      // for
  KW_IN,       // in
  KW_BREAK,    // break
  KW_CONTINUE, // continue
  KW_MATCH,    // match

  KW_IMPORT, // import
  KW_AS,     // as

  // Comments
  COMMENT_LINE,  // Single-line comment
  COMMENT_BLOCK, // Multi-line comment
  COMMENT_DOC,   // Documentation comment

  // Literals(except string, null and boolean literals)
  LIT_INT,     // Integer literal
  LIT_FLOAT,   // Floating-point literal
  LIT_DECIMAL, // Decimal literal
               // LIT_COMPLEX,  // Complex number literal

  // String literal
  LIT_STRING,     // String literal
  LIT_RAW_STRING, // Raw string literal
  LIT_TEX_STRING, // TeX string literal

  // Boolean literals
  LIT_TRUE,  // true
  LIT_FALSE, // false

  // Null literal
  LIT_NULL, // null

  // Type literals
  // TY_I8,    // i8
  // TY_I16,   // i16
  // TY_I32,   // i32
  // TY_I64,   // i64
  // TY_U8,    // u8
  // TY_U16,   // u16
  // TY_U32,   // u32
  // TY_U64,   // u64
  // TY_F32,   // f32
  // TY_F64,   // f64
  // TY_DEC64, // dec64
  // TY_CPX32, // cpx32
  // TY_CPX64, // cpx64
  // TY_BOOL,  // bool
  // TY_STRING,// string
  // TY_UNIT,  // unit
  // TY_NULLTYPE,// nulltype

  // Operators

  // Arithmetic Operators
  OP_PLUS,    // +
  OP_MINUS,   // -
  OP_STAR,    // *
  OP_SLASH,   // /
  OP_PERCENT, // %

  // Comparison Operators
  OP_EQ, // ==
  OP_NE, // !=
  OP_LT, // <
  OP_LE, // <=
  OP_GT, // >
  OP_GE, // >=

  // Logical Operators
  OP_LOGICAL_AND, // &&
  OP_LOGICAL_OR,  // ||
  OP_LOGICAL_NOT, // !

  // Bitwise Operators
  OP_BIT_AND, // &
  OP_BIT_OR,  // |
  OP_BIT_XOR, // ^
  OP_BIT_NOT, // ~
  OP_BIT_SHL, // <<
  OP_BIT_SHR, // >>

  // Assignment Operators
  OP_ASSIGN,         // =
  OP_PLUS_ASSIGN,    // +=
  OP_MINUS_ASSIGN,   // -=
  OP_STAR_ASSIGN,    // *=
  OP_SLASH_ASSIGN,   // /=
  OP_PERCENT_ASSIGN, // %=
  OP_AND_ASSIGN,     // &=
  OP_OR_ASSIGN,      // |=
  OP_XOR_ASSIGN,     // ^=
  OP_SHL_ASSIGN,     // <<=
  OP_SHR_ASSIGN,     // >>=

  // Type Operators
  // OP_TYPE_AND, // &
  // OP_TYPE_OR,  // |
  // OP_TYPE_NOT, // ~

  // Range Operators
  OP_DOT_DOT,    // ..
  OP_DOT_DOT_EQ, // ..=

  // Other Operators
  OP_ARROW,       // ->
  OP_FAT_ARROW,   // =>
  OP_DOT,         // .
  OP_AT,          // @
  OP_COLON_COLON, // ::

  // Delimiters
  DELIM_LPAREN,     // (
  DELIM_RPAREN,     // )
  DELIM_LBRACE,     // {
  DELIM_RBRACE,     // }
  DELIM_LBRACKET,   // [
  DELIM_RBRACKET,   // ]
  DELIM_COMMA,      // ,
  DELIM_COLON,      // :
  DELIM_SEMICOLON,  // ;
  DELIM_UNDERSCORE, // _

  // Reserved operators
  OP_HASH,      // #
  OP_DOLLAR,    // $
  OP_BACKSLASH, // backslash (\)

  // Special Tokens
  TOKEN_NEWLINE,    // New line
  TOKEN_EOF,        // End of file
  TOKEN_WHITESPACE, // Whitespace
  TOKEN_UNKNOWN     // Unknown token
};

/**
 * @brief 源码位置信息。
 *
 * @details
 *   记录 Token 在源码中的精确位置，用于错误报告和调试。
 *   所有计数均从 1 开始（1-based），除了 offset 从 0 开始。
 */
struct SourceLocation {
  BufferID buffer;         ///< 源码缓冲区 ID（4 bytes）
  std::uint32_t line{1};   ///< 行号，1-based（4 bytes）
  std::uint32_t column{1}; ///< 列号，1-based，UTF-8 字符计数（4 bytes）
  std::uint32_t offset{0}; ///< 字节偏移，0-based（4 bytes）
  // 总计：16 bytes

  /// 默认构造函数
  constexpr SourceLocation() noexcept = default;

  /// 完整构造函数
  constexpr SourceLocation(BufferID buf, std::uint32_t ln, std::uint32_t col,
                           std::uint32_t off) noexcept
      : buffer(buf), line(ln), column(col), offset(off) {}

  /// 检查位置是否有效
  [[nodiscard]] constexpr bool isValid() const noexcept {
    return buffer.isValid();
  }
};

/**
 * @brief Trivia: 附加在 Token 上的空白和注释。
 *
 * @details
 *   Trivia 用于保存 Token 之间的空白字符、换行符和注释。
 *   这对于代码格式化器、IDE 语义高亮等工具非常重要。
 *   存储偏移量而非实际文本，通过 SourceManager 获取内容。
 */
struct Trivia {
  /// Trivia 类型
  enum class Kind : std::uint8_t {
    kWhitespace, ///< 空白字符（空格、制表符等）
    kNewline,    ///< 换行符
    kComment     ///< 注释
  };

  Kind kind;            ///< Trivia 类型
  BufferID buffer;      ///< 源码缓冲区
  std::uint32_t offset; ///< 字节偏移
  std::uint16_t length; ///< 字节长度

  /**
   * @brief 获取 Trivia 的文本内容。
   *
   * @param sm SourceManager 引用
   * @return Trivia 的文本视图
   *
   * @warning 返回的 string_view 指向 SourceManager 内部缓冲区。
   *          只要 SourceManager 实例存活，返回值就有效。
   *          请勿在 SourceManager 析构后使用返回值。
   */
  [[nodiscard]] std::string_view text(const SourceManager &sm) const {
    return sm.slice(buffer, offset, length);
  }
};

/**
 * @brief 转义类型标记索引。
 *
 * @details
 *   用于快速判断字符串 Token 中包含哪些类型的转义序列。
 *   仅字符串 Token 使用此标记。
 */
enum EscapeFlagIndex : std::uint8_t {
  kHasNamed = 0,      ///< 包含 \n, \t, \r, \0, \\, \"
  kHasHex = 1,        ///< 包含 \xHH
  kHasUnicode = 2,    ///< 包含 \u{...}
  kHasLiteralCtrl = 3 ///< 包含直接嵌入的换行符（多行字符串）
};

/// 转义标记位集合
using EscapeFlags = std::bitset<4>;

/**
 * @brief Token 位置信息封装。
 *
 * @details
 *   封装 Token 在源码中的位置信息，符合 Clean Code 原则（≤ 3 个参数）。
 */
struct TokenSpan {
  BufferID buffer;         ///< 源码缓冲区 ID
  std::uint32_t offset{0}; ///< 字节偏移
  std::uint16_t length{0}; ///< 字节长度
  SourceLocation loc;      ///< 源码位置

  /// 默认构造函数
  constexpr TokenSpan() noexcept = default;

  /// 完整构造函数
  constexpr TokenSpan(BufferID buf, std::uint32_t off, std::uint16_t len,
                      SourceLocation location) noexcept
      : buffer(buf), offset(off), length(len), loc(location) {}
};

/**
 * @brief Token 类（基于偏移量存储）。
 *
 * @details
 *   Token 仅存储偏移量和长度，通过 SourceManager 获取实际文本。
 *   这种设计确保 Token 的生命周期安全——只要 SourceManager 存活，
 *   Token 就永远有效。
 *
 *   内存布局经过优化，基础模式下无堆分配（空 vector 不分配）。
 */
class Token {
public:
  /**
   * @brief 构造函数（使用 TokenSpan 封装）。
   *
   * @param type Token 类型
   * @param span 位置信息
   */
  Token(TokenType type, TokenSpan span) noexcept
      : type_(type), buffer_(span.buffer), offset_(span.offset),
        rawOffset_(span.offset), loc_(span.loc), length_(span.length),
        rawLength_(span.length), escapeFlags_(), padding_{},
        expansionId_(ExpansionID::invalid()) {}

  /**
   * @brief 构造函数：显式初始化所有字段（兼容旧代码）。
   *
   * @param type Token 类型
   * @param buffer 源码缓冲区 ID
   * @param offset value 的字节偏移
   * @param length value 的字节长度
   * @param loc 源码位置
   * @deprecated 推荐使用 Token(TokenType, TokenSpan) 构造函数
   */
  Token(TokenType type, BufferID buffer, std::uint32_t offset,
        std::uint16_t length, SourceLocation loc) noexcept
      : Token(type, TokenSpan{buffer, offset, length, loc}) {}

  /// 获取 Token 类型
  [[nodiscard]] TokenType type() const noexcept { return type_; }

  /// 获取源码缓冲区 ID
  [[nodiscard]] BufferID buffer() const noexcept { return buffer_; }

  /// 获取 value 的字节偏移
  [[nodiscard]] std::uint32_t offset() const noexcept { return offset_; }

  /// 获取 value 的字节长度
  [[nodiscard]] std::uint16_t length() const noexcept { return length_; }

  /// 获取源码位置
  [[nodiscard]] const SourceLocation &location() const noexcept { return loc_; }

  /**
   * @brief 获取 Token 的语义值（需要 SourceManager）。
   *
   * @details
   *   对于字符串字面量，返回处理转义后的内容。
   *   对于其他 Token，返回原始文本。
   *
   * @param sm SourceManager 引用
   * @return Token 的语义值
   *
   * @warning 返回的 string_view 指向 SourceManager 内部缓冲区。
   *          只要 SourceManager 实例存活，返回值就有效。
   *          请勿在 SourceManager 析构后使用返回值。
   */
  [[nodiscard]] std::string_view value(const SourceManager &sm) const {
    return sm.slice(buffer_, offset_, length_);
  }

  /**
   * @brief 获取原始文本（含引号等，需要 SourceManager）。
   *
   * @details
   *   对于字符串字面量，返回包含引号的原始文本。
   *   对于其他 Token，与 value() 相同。
   *
   * @param sm SourceManager 引用
   * @return Token 的原始文本
   *
   * @warning 返回的 string_view 指向 SourceManager 内部缓冲区。
   *          只要 SourceManager 实例存活，返回值就有效。
   *          请勿在 SourceManager 析构后使用返回值。
   */
  [[nodiscard]] std::string_view rawLiteral(const SourceManager &sm) const {
    return sm.slice(buffer_, rawOffset_, rawLength_);
  }

  /**
   * @brief 设置原始文本的偏移量和长度。
   *
   * @details
   *   仅用于字符串 Token，记录包含引号的原始文本位置。
   *
   * @param offset 原始文本的字节偏移
   * @param length 原始文本的字节长度
   */
  void setRawLiteral(std::uint32_t offset, std::uint16_t length) noexcept {
    rawOffset_ = offset;
    rawLength_ = length;
  }

  /// 检查是否有 Trivia
  [[nodiscard]] bool hasTrivia() const noexcept {
    return !leadingTrivia_.empty() || !trailingTrivia_.empty();
  }

  /// 获取前置 Trivia
  [[nodiscard]] std::span<const Trivia> leadingTrivia() const noexcept {
    return leadingTrivia_;
  }

  /// 获取后置 Trivia
  [[nodiscard]] std::span<const Trivia> trailingTrivia() const noexcept {
    return trailingTrivia_;
  }

  /// 添加前置 Trivia
  void addLeadingTrivia(Trivia trivia) { leadingTrivia_.push_back(trivia); }

  /// 添加后置 Trivia
  void addTrailingTrivia(Trivia trivia) { trailingTrivia_.push_back(trivia); }

  /// 设置前置 Trivia（移动语义）
  void setLeadingTrivia(std::vector<Trivia> trivia) {
    leadingTrivia_ = std::move(trivia);
  }

  /// 设置后置 Trivia（移动语义）
  void setTrailingTrivia(std::vector<Trivia> trivia) {
    trailingTrivia_ = std::move(trivia);
  }

  /// 获取转义标记
  [[nodiscard]] EscapeFlags escapeFlags() const noexcept {
    return escapeFlags_;
  }

  /// 设置转义标记
  void setEscapeFlags(EscapeFlags flags) noexcept { escapeFlags_ = flags; }

  /// 检查是否包含命名转义（\n, \t 等）
  [[nodiscard]] bool hasNamedEscape() const noexcept {
    return escapeFlags_[kHasNamed];
  }

  /// 检查是否包含十六进制转义（\xHH）
  [[nodiscard]] bool hasHexEscape() const noexcept {
    return escapeFlags_[kHasHex];
  }

  /// 检查是否包含 Unicode 转义（\u{...}）
  [[nodiscard]] bool hasUnicodeEscape() const noexcept {
    return escapeFlags_[kHasUnicode];
  }

  /// 检查是否包含直接嵌入的控制字符
  [[nodiscard]] bool hasLiteralCtrl() const noexcept {
    return escapeFlags_[kHasLiteralCtrl];
  }

  /// 检查 Token 是否来自宏展开
  [[nodiscard]] bool isFromMacroExpansion() const noexcept {
    return expansionId_.isValid();
  }

  /// 获取宏展开 ID
  [[nodiscard]] ExpansionID expansionId() const noexcept {
    return expansionId_;
  }

  /// 设置宏展开 ID
  void setExpansionId(ExpansionID id) noexcept { expansionId_ = id; }

  /**
   * @brief 创建 EOF Token。
   *
   * @param loc 源码位置
   * @return EOF Token
   */
  [[nodiscard]] static Token makeEof(SourceLocation loc) {
    return Token(TokenType::TOKEN_EOF,
                 TokenSpan{loc.buffer, loc.offset, 0, loc});
  }

  /**
   * @brief 创建 Unknown Token。
   *
   * @param span Token 位置信息
   * @return Unknown Token
   */
  [[nodiscard]] static Token makeUnknown(TokenSpan span) {
    return Token(TokenType::TOKEN_UNKNOWN, span);
  }

private:
  // 目标：减少 padding，优化缓存访问

  TokenType type_;          // 4 bytes
  BufferID buffer_;         // 4 bytes
  std::uint32_t offset_;    // 4 bytes - value 的字节偏移
  std::uint32_t rawOffset_; // 4 bytes - rawLiteral 的字节偏移

  SourceLocation loc_; // 16 bytes

  std::uint16_t length_;    // 2 bytes - value 的字节长度
  std::uint16_t rawLength_; // 2 bytes - rawLiteral 的字节长度
  EscapeFlags escapeFlags_; // 1 byte  - 仅字符串 Token 使用
  [[maybe_unused]] std::uint8_t
      padding_[3]{}; // 3 bytes - 显式 padding，预留未来扩展
                     // 用途说明：此字段用于未来在不破坏 ABI
  // 的情况下添加小型字段（如新标志位、状态字节等）。
  // 若需访问或扩展此区域，请使用下方的 accessor。

  /// @brief 访问预留的 padding 字节（仅供未来扩展使用）
  /// @return 指向 padding_ 数组的指针
  [[nodiscard]] constexpr std::uint8_t *reservedBytes() noexcept {
    return padding_;
  }
  /// @brief 只读访问预留的 padding 字节
  [[nodiscard]] constexpr const std::uint8_t *reservedBytes() const noexcept {
    return padding_;
    ;
  }
  ExpansionID expansionId_; // 4 bytes - 宏展开 ID（预留）
  // 4 bytes implicit padding（对齐到 8 字节边界）

  // Trivia 直接存储（空 vector 不分配堆内存）
  std::vector<Trivia> leadingTrivia_;  // 24 bytes
  std::vector<Trivia> trailingTrivia_; // 24 bytes
};

/**
 * @brief 查找关键字。
 *
 * @param word 待查找的单词
 * @return 若为关键字则返回对应的 TokenType，否则返回 std::nullopt
 */
[[nodiscard]] std::optional<TokenType> lookupKeyword(std::string_view word);

/**
 * @brief 获取 TokenType 的名称字符串。
 *
 * @param type Token 类型
 * @return TokenType 的名称
 */
[[nodiscard]] std::string_view tokenTypeName(TokenType type);

} // namespace czc::lexer

#endif // CZC_LEXER_TOKEN_HPP