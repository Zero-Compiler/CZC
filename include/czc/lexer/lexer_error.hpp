/**
 * @file lexer_error.hpp
 * @brief 词法分析器错误定义。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-29
 *
 * @details
 *   本文件定义了词法分析器的错误类型和错误收集器：
 *   - LexerErrorCode: 词法错误码枚举
 *   - LexerError: 词法错误结构
 *   - ErrorCollector: 错误收集器类
 *
 *   采用预格式化存储，避免运行时字符串拼接。
 *   错误码采用显式数值，便于错误消息映射。
 */

#ifndef CZC_LEXER_LEXER_ERROR_HPP
#define CZC_LEXER_LEXER_ERROR_HPP

#include "czc/common/config.hpp"

#include "czc/lexer/token.hpp"

#include <cstdint>
#include <format>
#include <span>
#include <string>
#include <vector>

namespace czc::lexer {

/**
 * @brief 词法错误码（使用显式数值以便错误消息映射）。
 *
 * @details
 *   错误码分组：
 *   - 1001-1010: 数字相关
 *   - 1011-1020: 字符串相关
 *   - 1021-1030: 字符相关
 *   - 1031-1040: 注释相关
 */
enum class LexerErrorCode : std::uint16_t {
  // ========== 数字相关 (1001-1010) ==========

  /// "0x" 后缺少十六进制数字
  MissingHexDigits = 1001,

  /// "0b" 后缺少二进制数字
  MissingBinaryDigits = 1002,

  /// "0o" 后缺少八进制数字
  MissingOctalDigits = 1003,

  /// 科学计数法指数部分缺少数字
  MissingExponentDigits = 1004,

  /// 数字字面量后跟随无效字符
  InvalidTrailingChar = 1005,

  /// 无效的数字后缀
  InvalidNumberSuffix = 1006,

  // ========== 字符串相关 (1011-1020) ==========

  /// 无效的转义序列
  InvalidEscapeSequence = 1011,

  /// 字符串未闭合
  UnterminatedString = 1012,

  /// 无效的十六进制转义
  InvalidHexEscape = 1013,

  /// 无效的 Unicode 转义
  InvalidUnicodeEscape = 1014,

  /// 原始字符串未闭合
  UnterminatedRawString = 1015,

  // ========== 字符相关 (1021-1030) ==========

  /// 无效字符
  InvalidCharacter = 1021,

  /// 无效的 UTF-8 序列
  InvalidUtf8Sequence = 1022,

  // ========== 注释相关 (1031-1040) ==========

  /// 块注释未闭合
  UnterminatedBlockComment = 1031,

  // ========== 通用错误 (1041-1050) ==========

  /// Token 长度超过限制（65535 字节）
  TokenTooLong = 1041,
};

/**
 * @brief 词法错误（预格式化存储）。
 *
 * @details
 *   存储错误的完整信息，包括错误码、位置、长度和格式化后的消息。
 *   采用工厂方法创建，确保类型安全。
 */
struct LexerError {
  LexerErrorCode code;          ///< 错误码
  SourceLocation location;      ///< 错误位置
  uint32_t length{1};           ///< 错误跨越的字符数（用于显示标注）
  std::string formattedMessage; ///< 预格式化的错误消息

  /**
   * @brief 获取错误码字符串（如 "L1001"）。
   *
   * @return 错误码字符串
   */
  [[nodiscard]] std::string codeString() const {
    return std::format("L{:04d}", static_cast<int>(code));
  }

  /**
   * @brief 类型安全的工厂方法（编译期检查参数类型和数量）。
   *
   * @tparam Args 格式化参数类型
   * @param code 错误码
   * @param loc 错误位置
   * @param len 错误跨越的字符数
   * @param fmt 格式字符串
   * @param args 格式化参数
   * @return 构造好的 LexerError
   */
  template <typename... Args>
  [[nodiscard]] static LexerError
  make(LexerErrorCode code, SourceLocation loc, uint32_t len,
       std::format_string<Args...> fmt, Args &&...args) {
    return {code, loc, len, std::format(fmt, std::forward<Args>(args)...)};
  }

  /**
   * @brief 创建简单错误（无格式化参数）。
   *
   * @param code 错误码
   * @param loc 错误位置
   * @param len 错误跨越的字符数
   * @param message 错误消息
   * @return 构造好的 LexerError
   */
  [[nodiscard]] static LexerError simple(LexerErrorCode code,
                                         SourceLocation loc, uint32_t len,
                                         std::string message) {
    return {code, loc, len, std::move(message)};
  }

  /**
   * @brief 创建简单错误（默认长度为 1）。
   *
   * @param code 错误码
   * @param loc 错误位置
   * @param message 错误消息
   * @return 构造好的 LexerError
   */
  [[nodiscard]] static LexerError
  simple(LexerErrorCode code, SourceLocation loc, std::string message) {
    return {code, loc, 1, std::move(message)};
  }
};

/**
 * @brief 错误收集器。
 *
 * @details
 *   收集词法分析过程中产生的所有错误。
 *   允许一次扫描报告所有错误，提升用户体验。
 */
class ErrorCollector {
public:
  ErrorCollector() = default;

  // 可拷贝可移动
  ErrorCollector(const ErrorCollector &) = default;
  ErrorCollector &operator=(const ErrorCollector &) = default;
  ErrorCollector(ErrorCollector &&) noexcept = default;
  ErrorCollector &operator=(ErrorCollector &&) noexcept = default;

  ~ErrorCollector() = default;

  /**
   * @brief 添加错误。
   *
   * @param error 要添加的错误
   */
  void add(LexerError error) { errors_.push_back(std::move(error)); }

  /**
   * @brief 获取所有错误。
   *
   * @return 错误列表的 span 视图
   */
  [[nodiscard]] std::span<const LexerError> errors() const noexcept {
    return errors_;
  }

  /**
   * @brief 检查是否有错误。
   *
   * @return 若有错误返回 true
   */
  [[nodiscard]] bool hasErrors() const noexcept { return !errors_.empty(); }

  /**
   * @brief 获取错误数量。
   *
   * @return 错误数量
   */
  [[nodiscard]] std::size_t count() const noexcept { return errors_.size(); }

  /**
   * @brief 清空所有错误。
   */
  void clear() { errors_.clear(); }

private:
  std::vector<LexerError> errors_; ///< 错误列表
};

/**
 * @brief 获取错误的宏展开链（按需查询）。
 *
 * @details
 *   如果错误发生在宏展开的代码中，此函数返回完整的展开链，
 *   从最内层（错误发生位置）到最外层（原始宏调用位置）。
 *
 * @param error 词法错误
 * @param sm SourceManager 引用
 * @return 展开链，若非宏展开则返回空向量
 */
[[nodiscard]] std::vector<SourceLocation>
getExpansionChain(const LexerError &error, const SourceManager &sm);

/**
 * @brief 格式化错误消息（含宏展开上下文）。
 *
 * @details
 *   生成完整的多行错误报告，包含宏展开链信息。
 *
 * @param error 词法错误
 * @param sm SourceManager 引用
 * @return 格式化后的错误消息
 */
[[nodiscard]] std::string formatError(const LexerError &error,
                                      const SourceManager &sm);

} // namespace czc::lexer

#endif // CZC_LEXER_LEXER_ERROR_HPP
