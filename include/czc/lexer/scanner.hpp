/**
 * @file scanner.hpp
 * @brief 扫描器接口和扫描上下文定义。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-29
 *
 * @details
 *   本文件定义了扫描器的核心组件：
 *   - Scanner concept: 扫描器接口约束
 *   - ScanContext: 扫描上下文，为扫描器提供统一的访问接口
 *
 *   采用 concepts 定义扫描器接口，提供编译期类型检查。
 */

#ifndef CZC_LEXER_SCANNER_HPP
#define CZC_LEXER_SCANNER_HPP

#include "czc/common/config.hpp"

#include "czc/lexer/lexer_error.hpp"
#include "czc/lexer/source_reader.hpp"
#include "czc/lexer/token.hpp"

#include <concepts>
#include <optional>

namespace czc::lexer {

// 前向声明
class ScanContext;

/**
 * @brief 扫描器概念。
 *
 * @details
 *   所有扫描器必须满足此概念，提供：
 *   - canScan(): 检查当前字符是否可由此扫描器处理
 *   - scan(): 执行扫描，返回 Token
 *
 * @tparam T 扫描器类型
 */
template <typename T>
concept Scanner = requires(T scanner, ScanContext &ctx) {
  { scanner.canScan(ctx) } -> std::convertible_to<bool>;
  { scanner.scan(ctx) } -> std::same_as<Token>;
};

/**
 * @brief 扫描上下文。
 *
 * @details
 *   为扫描器提供统一的访问接口，封装了：
 *   - SourceReader: 字符访问和位置跟踪
 *   - ErrorCollector: 错误报告
 *   - SourceManager: 源码管理
 *
 *   扫描器通过 ScanContext 访问源码和报告错误，
 *   避免直接依赖具体实现。
 */
class ScanContext {
public:
  /**
   * @brief 构造函数。
   *
   * @param reader SourceReader 引用
   * @param errors ErrorCollector 引用
   */
  ScanContext(SourceReader &reader, ErrorCollector &errors);

  // 不可拷贝，不可移动（引用语义）
  ScanContext(const ScanContext &) = delete;
  ScanContext &operator=(const ScanContext &) = delete;
  ScanContext(ScanContext &&) = delete;
  ScanContext &operator=(ScanContext &&) = delete;

  ~ScanContext() = default;

  /**
   * @brief 获取当前字符。
   *
   * @return 当前字符，若到达末尾返回 std::nullopt
   */
  [[nodiscard]] std::optional<char> current() const noexcept;

  /**
   * @brief 向前查看字符。
   *
   * @param offset 从当前位置的偏移量（默认为 1）
   * @return 偏移位置的字符，若越界返回 std::nullopt
   */
  [[nodiscard]] std::optional<char> peek(std::size_t offset = 1) const noexcept;

  /**
   * @brief 检查是否到达源码末尾。
   *
   * @return 若到达末尾返回 true
   */
  [[nodiscard]] bool isAtEnd() const noexcept;

  /**
   * @brief 获取当前源码位置。
   *
   * @return 当前的 SourceLocation
   */
  [[nodiscard]] SourceLocation location() const noexcept;

  /**
   * @brief 获取当前字节偏移。
   *
   * @return 字节偏移（0-based）
   */
  [[nodiscard]] std::size_t offset() const noexcept;

  /**
   * @brief 获取源码缓冲区 ID。
   *
   * @return BufferID
   */
  [[nodiscard]] BufferID buffer() const noexcept;

  /**
   * @brief 前进一个字符。
   */
  void advance();

  /**
   * @brief 前进指定数量的字符。
   *
   * @param count 前进的字符数
   */
  void advance(std::size_t count);

  /**
   * @brief 检查当前字符是否为指定字符。
   *
   * @param expected 期望的字符
   * @return 若匹配返回 true
   */
  [[nodiscard]] bool check(char expected) const noexcept;

  /**
   * @brief 匹配并消费指定字符。
   *
   * @param expected 期望的字符
   * @return 若匹配则前进并返回 true，否则返回 false
   */
  bool match(char expected);

  /**
   * @brief 匹配并消费指定字符串。
   *
   * @param expected 期望的字符串
   * @return 若匹配则前进并返回 true，否则返回 false
   */
  bool match(std::string_view expected);

  /**
   * @brief 提取从指定偏移到当前位置的切片。
   *
   * @param startOffset 起始偏移
   * @return 切片信息
   */
  [[nodiscard]] SourceReader::Slice sliceFrom(std::size_t startOffset) const;

  /**
   * @brief 获取从指定偏移到当前位置的文本。
   *
   * @param startOffset 起始偏移
   * @return 文本视图
   */
  [[nodiscard]] std::string_view textFrom(std::size_t startOffset) const;

  /**
   * @brief 获取 SourceManager 引用。
   *
   * @return SourceManager 引用
   */
  [[nodiscard]] SourceManager &sourceManager() noexcept;

  /**
   * @brief 获取 SourceManager 常量引用。
   *
   * @return SourceManager 常量引用
   */
  [[nodiscard]] const SourceManager &sourceManager() const noexcept;

  /**
   * @brief 报告错误。
   *
   * @param error 要报告的错误
   */
  void reportError(LexerError error);

  /**
   * @brief 检查是否有错误。
   *
   * @return 若有错误返回 true
   */
  [[nodiscard]] bool hasErrors() const noexcept;

  /**
   * @brief 创建 Token。
   *
   * @param type Token 类型
   * @param startOffset Token 起始偏移
   * @param startLoc Token 起始位置
   * @return 创建的 Token
   */
  [[nodiscard]] Token makeToken(TokenType type, std::size_t startOffset,
                                SourceLocation startLoc) const;

  /**
   * @brief 创建 Unknown Token。
   *
   * @param startOffset Token 起始偏移
   * @param startLoc Token 起始位置
   * @return Unknown Token
   */
  [[nodiscard]] Token makeUnknown(std::size_t startOffset,
                                  SourceLocation startLoc) const;

private:
  SourceReader &reader_;   ///< 源码读取器引用
  ErrorCollector &errors_; ///< 错误收集器引用
};

} // namespace czc::lexer

#endif // CZC_LEXER_SCANNER_HPP
