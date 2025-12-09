/**
 * @file source_reader.hpp
 * @brief 源码读取器，管理源码扫描位置。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-29
 *
 * @details
 *   SourceReader 是对 SourceManager 中源码的包装，提供：
 *   - 字符级别的访问接口
 *   - 位置跟踪（行、列、偏移）
 *   - peek/advance 操作
 *
 *   不拥有源码，仅持有 SourceManager 的引用。
 */

#ifndef CZC_LEXER_SOURCE_READER_HPP
#define CZC_LEXER_SOURCE_READER_HPP

#include "czc/common/config.hpp"

#include "czc/lexer/source_manager.hpp"
#include "czc/lexer/token.hpp"

#include <cstdint>
#include <optional>
#include <string_view>

namespace czc::lexer {

/**
 * @brief 源码读取器。
 *
 * @details
 *   管理源码扫描位置，不拥有源码（源码由 SourceManager 持有）。
 *   提供 peek/advance 操作和位置跟踪。
 *
 * @note 不可拷贝，可移动
 */
class SourceReader {
public:
  /**
   * @brief 构造函数：引用 SourceManager 中的源码。
   *
   * @param sm SourceManager 引用
   * @param buffer 源码缓冲区 ID
   */
  explicit SourceReader(SourceManager &sm, BufferID buffer);

  // 不可拷贝
  SourceReader(const SourceReader &) = delete;
  SourceReader &operator=(const SourceReader &) = delete;

  // 可移动（移动构造可用，移动赋值因引用成员而删除）
  SourceReader(SourceReader &&) noexcept = default;
  SourceReader &operator=(SourceReader &&) = delete;

  ~SourceReader() = default;

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
   * @brief 前进一个字符。
   *
   * @details
   *   自动更新行号和列号。
   *   处理 \r\n 换行序列（视为单个换行）。
   */
  void advance();

  /**
   * @brief 前进指定数量的字符。
   *
   * @param count 前进的字符数
   */
  void advance(std::size_t count);

  /**
   * @brief 获取当前源码位置。
   *
   * @return 当前的 SourceLocation
   */
  [[nodiscard]] SourceLocation location() const noexcept;

  /**
   * @brief 获取源码缓冲区 ID。
   *
   * @return BufferID
   */
  [[nodiscard]] BufferID buffer() const noexcept { return buffer_; }

  /**
   * @brief 获取当前字节偏移。
   *
   * @return 字节偏移（0-based）
   */
  [[nodiscard]] std::size_t offset() const noexcept { return position_; }

  /**
   * @brief 获取当前行号。
   *
   * @return 行号（1-based）
   */
  [[nodiscard]] std::uint32_t line() const noexcept { return line_; }

  /**
   * @brief 获取当前列号。
   *
   * @return 列号（1-based，UTF-8 字符计数）
   */
  [[nodiscard]] std::uint32_t column() const noexcept { return column_; }

  /**
   * @brief 切片信息结构。
   */
  struct Slice {
    std::uint32_t offset; ///< 起始偏移
    std::uint16_t length; ///< 字节长度
  };

  /**
   * @brief 提取从指定偏移到当前位置的切片。
   *
   * @param startOffset 起始偏移
   * @return 切片信息
   */
  [[nodiscard]] Slice sliceFrom(std::size_t startOffset) const noexcept;

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
  [[nodiscard]] SourceManager &sourceManager() noexcept { return sm_; }

  /**
   * @brief 获取 SourceManager 常量引用。
   *
   * @return SourceManager 常量引用
   */
  [[nodiscard]] const SourceManager &sourceManager() const noexcept {
    return sm_;
  }

  /**
   * @brief 获取整个源码。
   *
   * @return 源码视图
   */
  [[nodiscard]] std::string_view source() const noexcept { return source_; }

private:
  SourceManager &sm_;       ///< 源码管理器引用
  BufferID buffer_;         ///< 源码缓冲区 ID
  std::string_view source_; ///< 缓存的源码视图
  std::size_t position_{0}; ///< 当前字节偏移
  std::uint32_t line_{1};   ///< 当前行号（1-based）
  std::uint32_t column_{1}; ///< 当前列号（1-based）
};

} // namespace czc::lexer

#endif // CZC_LEXER_SOURCE_READER_HPP
