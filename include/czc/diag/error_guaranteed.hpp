/**
 * @file error_guaranteed.hpp
 * @brief 类型安全错误保证。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-12-04
 *
 * @details
 *   借鉴 rustc ErrorGuaranteed 和 EmissionGuarantee trait 设计，
 *   在类型系统层面保证错误已被处理。
 */

#ifndef CZC_DIAG_ERROR_GUARANTEED_HPP
#define CZC_DIAG_ERROR_GUARANTEED_HPP

#include "czc/common/config.hpp"

#include <expected>

namespace czc::diag {

// 前向声明
class DiagContext;

/// 错误保证 - 证明至少发出了一个错误
/// 借鉴 rustc ErrorGuaranteed 设计
/// - 不可默认构造（只能由 DiagContext 创建）
/// - 可拷贝（传递保证）
/// - [[nodiscard]] 确保不被忽略
class [[nodiscard]] ErrorGuaranteed {
public:
  // 可拷贝
  ErrorGuaranteed(const ErrorGuaranteed &) = default;
  auto operator=(const ErrorGuaranteed &) -> ErrorGuaranteed & = default;

  // 可移动
  ErrorGuaranteed(ErrorGuaranteed &&) noexcept = default;
  auto operator=(ErrorGuaranteed &&) noexcept -> ErrorGuaranteed & = default;

  /// 默认析构
  ~ErrorGuaranteed() = default;

private:
  // 私有构造 - 只有 DiagContext 可以创建
  friend class DiagContext;
  ErrorGuaranteed() = default;
};

} // namespace czc::diag

namespace czc {

/// 诊断结果类型 - 成功返回 T，失败返回 ErrorGuaranteed
/// 使用 C++23 std::expected
template <typename T>
using DiagResult = std::expected<T, diag::ErrorGuaranteed>;

/// void 特化
using DiagVoidResult = std::expected<void, diag::ErrorGuaranteed>;

} // namespace czc

#endif // CZC_DIAG_ERROR_GUARANTEED_HPP
