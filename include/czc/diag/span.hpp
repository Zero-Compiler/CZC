/**
 * @file span.hpp
 * @brief 源码位置抽象。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-12-04
 *
 * @details
 *   借鉴 rustc 的 Span 和 MultiSpan 设计，提供源码位置的精确表示。
 *   使用偏移量而非行列号，避免重复计算。
 */

#ifndef CZC_DIAG_SPAN_HPP
#define CZC_DIAG_SPAN_HPP

#include "czc/common/config.hpp"

#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace czc::diag {

// 前向声明
class MessageRef;

/// 源码位置范围 - 不可变值类型
/// 使用偏移量而非行列号，避免重复计算
struct Span {
  uint32_t fileId{0};      ///< 文件标识符
  uint32_t startOffset{0}; ///< 起始偏移（字节）
  uint32_t endOffset{0};   ///< 结束偏移（字节，不含）

  /// 检查 Span 是否有效
  [[nodiscard]] constexpr auto isValid() const noexcept -> bool {
    return fileId != 0;
  }

  /// 获取 Span 长度
  [[nodiscard]] constexpr auto length() const noexcept -> uint32_t {
    return endOffset > startOffset ? endOffset - startOffset : 0;
  }

  /// 创建无效 Span
  [[nodiscard]] static constexpr auto invalid() noexcept -> Span {
    return Span{0, 0, 0};
  }

  /// 创建 Span
  [[nodiscard]] static constexpr auto create(uint32_t fileId, uint32_t start,
                                             uint32_t end) noexcept -> Span {
    return Span{fileId, start, end};
  }

  /// 合并两个 Span（取并集）
  [[nodiscard]] constexpr auto merge(const Span &other) const noexcept -> Span {
    if (!isValid())
      return other;
    if (!other.isValid())
      return *this;
    if (fileId != other.fileId)
      return *this;

    return Span{fileId, std::min(startOffset, other.startOffset),
                std::max(endOffset, other.endOffset)};
  }

  auto operator<=>(const Span &) const = default;
};

/// 带标签的位置 - 用于诊断标注
struct LabeledSpan {
  Span span;            ///< 位置范围
  std::string label;    ///< 标注文本
  bool isPrimary{true}; ///< 是否为主要位置

  /// 默认构造
  LabeledSpan() = default;

  /// 构造带标签的 Span
  LabeledSpan(Span s, std::string_view lbl, bool primary = true)
      : span(s), label(lbl), isPrimary(primary) {}
};

/// 多位置容器 - 支持主要和次要标注
/// 借鉴 rustc MultiSpan 设计
class MultiSpan {
public:
  MultiSpan() = default;
  ~MultiSpan() = default;

  // 可拷贝可移动
  MultiSpan(const MultiSpan &) = default;
  auto operator=(const MultiSpan &) -> MultiSpan & = default;
  MultiSpan(MultiSpan &&) noexcept = default;
  auto operator=(MultiSpan &&) noexcept -> MultiSpan & = default;

  /// 添加主要标注
  void addPrimary(Span span, std::string_view label = "");

  /// 添加次要标注
  void addSecondary(Span span, std::string_view label = "");

  /// 获取主要标注（第一个）
  [[nodiscard]] auto primary() const -> std::optional<LabeledSpan>;

  /// 获取所有标注
  [[nodiscard]] auto spans() const -> std::span<const LabeledSpan> {
    return spans_;
  }

  /// 获取所有次要标注
  [[nodiscard]] auto secondaries() const -> std::vector<LabeledSpan>;

  /// 检查是否为空
  [[nodiscard]] auto isEmpty() const noexcept -> bool { return spans_.empty(); }

  /// 获取标注数量
  [[nodiscard]] auto size() const noexcept -> size_t { return spans_.size(); }

private:
  std::vector<LabeledSpan> spans_;
};

} // namespace czc::diag

#endif // CZC_DIAG_SPAN_HPP
