/**
 * @file diag_builder.hpp
 * @brief 诊断构建器。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-12-04
 *
 * @details
 *   流式 API 构建器，借鉴 rustc Diag 的链式调用模式。
 *   提供便捷的诊断创建接口。
 */

#ifndef CZC_DIAG_DIAG_BUILDER_HPP
#define CZC_DIAG_DIAG_BUILDER_HPP

#include "czc/common/config.hpp"
#include "czc/diag/diagnostic.hpp"
#include "czc/diag/error_guaranteed.hpp"

#include <string>
#include <string_view>

namespace czc::diag {

// 前向声明
class DiagContext;

/// 诊断构建器 - 提供流式 API
/// 借鉴 rustc Diag 智能指针设计，但使用值语义
class [[nodiscard]] DiagBuilder {
public:
  /// 构造诊断构建器
  explicit DiagBuilder(Level level, Message message);

  /// 带错误码构造
  DiagBuilder(Level level, Message message, ErrorCode code);

  /// 析构函数
  ~DiagBuilder() = default;

  // 链式方法 - 返回 *this 引用

  /// 设置错误码
  auto code(ErrorCode c) -> DiagBuilder &;

  /// 设置主要 Span
  auto span(Span s) -> DiagBuilder &;

  /// 设置带标签的 Span
  auto spanLabel(Span s, std::string_view label) -> DiagBuilder &;

  /// 添加次要 Span
  auto secondarySpan(Span s, std::string_view label = "") -> DiagBuilder &;

  /// 添加注释
  auto note(std::string_view message) -> DiagBuilder &;

  /// 添加带位置的注释
  auto note(Span s, std::string_view message) -> DiagBuilder &;

  /// 添加帮助信息
  auto help(std::string_view message) -> DiagBuilder &;

  /// 添加带位置的帮助信息
  auto help(Span s, std::string_view message) -> DiagBuilder &;

  /// 添加修复建议
  auto suggestion(Span s, std::string replacement, std::string_view message,
                  Applicability applicability = Applicability::Unspecified)
      -> DiagBuilder &;

  // 终结方法

  /// 构建诊断（消耗 builder）
  [[nodiscard]] auto build() && -> Diagnostic;

  /// 发射诊断到上下文
  void emit(DiagContext &dcx) &&;

  /// 发射错误诊断并返回保证
  [[nodiscard]] auto emitError(DiagContext &dcx) && -> ErrorGuaranteed;

  // 禁止拷贝，允许移动
  DiagBuilder(const DiagBuilder &) = delete;
  auto operator=(const DiagBuilder &) -> DiagBuilder & = delete;
  DiagBuilder(DiagBuilder &&) noexcept = default;
  auto operator=(DiagBuilder &&) noexcept -> DiagBuilder & = default;

private:
  Diagnostic diag_;
};

// ============================================================================
// 工厂函数
// ============================================================================

/// 创建错误诊断
[[nodiscard]] inline auto error(Message message) -> DiagBuilder {
  return DiagBuilder(Level::Error, std::move(message));
}

/// 创建带错误码的错误诊断
[[nodiscard]] inline auto error(ErrorCode code, Message message)
    -> DiagBuilder {
  return DiagBuilder(Level::Error, std::move(message), code);
}

/// 创建警告诊断
[[nodiscard]] inline auto warning(Message message) -> DiagBuilder {
  return DiagBuilder(Level::Warning, std::move(message));
}

/// 创建注释诊断
[[nodiscard]] inline auto note(Message message) -> DiagBuilder {
  return DiagBuilder(Level::Note, std::move(message));
}

/// 创建帮助诊断
[[nodiscard]] inline auto help(Message message) -> DiagBuilder {
  return DiagBuilder(Level::Help, std::move(message));
}

/// 创建内部错误诊断（编译器 bug）
[[nodiscard]] inline auto bug(Message message) -> DiagBuilder {
  return DiagBuilder(Level::Bug, std::move(message));
}

/// 创建致命错误诊断
[[nodiscard]] inline auto fatal(Message message) -> DiagBuilder {
  return DiagBuilder(Level::Fatal, std::move(message));
}

} // namespace czc::diag

#endif // CZC_DIAG_DIAG_BUILDER_HPP
