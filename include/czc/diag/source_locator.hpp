/**
 * @file source_locator.hpp
 * @brief 源码位置解析接口。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-12-04
 *
 * @details
 *   抽象接口，解耦诊断系统与具体源码管理实现。
 *   由各模块实现，提供 Span -> 文本的映射。
 */

#ifndef CZC_DIAG_SOURCE_LOCATOR_HPP
#define CZC_DIAG_SOURCE_LOCATOR_HPP

#include "czc/common/config.hpp"
#include "czc/diag/span.hpp"

#include <cstdint>
#include <string_view>

namespace czc::diag {

/// 行列位置
struct LineColumn {
  uint32_t line{0};   ///< 1-based 行号
  uint32_t column{0}; ///< 1-based 列号（UTF-8 字符）

  /// 检查是否有效
  [[nodiscard]] constexpr auto isValid() const noexcept -> bool {
    return line > 0 && column > 0;
  }
};

/// 源码定位器接口
/// 由各模块实现，提供 Span -> 文本的映射
class SourceLocator {
public:
  virtual ~SourceLocator() = default;

  /// 获取文件名
  [[nodiscard]] virtual auto getFilename(Span span) const
      -> std::string_view = 0;

  /// 偏移量转行列
  [[nodiscard]] virtual auto getLineColumn(uint32_t fileId,
                                           uint32_t offset) const
      -> LineColumn = 0;

  /// 获取某行内容
  [[nodiscard]] virtual auto getLineContent(uint32_t fileId,
                                            uint32_t line) const
      -> std::string_view = 0;

  /// 获取源码片段
  [[nodiscard]] virtual auto getSourceSlice(Span span) const
      -> std::string_view = 0;

protected:
  SourceLocator() = default;
  SourceLocator(const SourceLocator &) = default;
  auto operator=(const SourceLocator &) -> SourceLocator & = default;
  SourceLocator(SourceLocator &&) = default;
  auto operator=(SourceLocator &&) -> SourceLocator & = default;
};

} // namespace czc::diag

#endif // CZC_DIAG_SOURCE_LOCATOR_HPP
