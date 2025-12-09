/**
 * @file diagnostic.hpp
 * @brief 诊断类型定义。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-12-04
 *
 * @details
 *   核心诊断结构，借鉴 rustc DiagInner 设计。
 *   定义诊断级别、建议、子诊断等类型。
 */

#ifndef CZC_DIAG_DIAGNOSTIC_HPP
#define CZC_DIAG_DIAGNOSTIC_HPP

#include "czc/common/config.hpp"
#include "czc/diag/error_code.hpp"
#include "czc/diag/message.hpp"
#include "czc/diag/span.hpp"

#include <optional>
#include <string>
#include <vector>

namespace czc::diag {

/// 诊断级别 - 借鉴 rustc Level
enum class Level : uint8_t {
  Note = 0,    ///< 附加信息
  Help = 1,    ///< 帮助信息
  Warning = 2, ///< 警告
  Error = 3,   ///< 错误
  Fatal = 4,   ///< 致命错误（立即终止）
  Bug = 5,     ///< 内部编译器错误
};

/// 获取级别的字符串表示
[[nodiscard]] auto levelToString(Level level) -> std::string_view;

/// 建议适用性 - 借鉴 rustc Applicability
enum class Applicability : uint8_t {
  MachineApplicable, ///< 可自动应用
  HasPlaceholders,   ///< 需用户填充占位符
  MaybeIncorrect,    ///< 可能不正确
  Unspecified,       ///< 未指定
};

/// 代码修复建议
struct Suggestion {
  Span span;               ///< 替换位置
  std::string replacement; ///< 替换文本
  std::string message;     ///< 建议说明
  Applicability applicability{Applicability::Unspecified};

  /// 默认构造
  Suggestion() = default;

  /// 完整构造
  Suggestion(Span s, std::string repl, std::string msg,
             Applicability app = Applicability::Unspecified)
      : span(s), replacement(std::move(repl)), message(std::move(msg)),
        applicability(app) {}
};

/// 子诊断（注释、帮助）
struct SubDiagnostic {
  Level level{Level::Note}; ///< Note 或 Help
  std::string message;      ///< 消息内容
  std::optional<Span> span; ///< 可选位置

  /// 默认构造
  SubDiagnostic() = default;

  /// 完整构造
  SubDiagnostic(Level lvl, std::string msg,
                std::optional<Span> s = std::nullopt)
      : level(lvl), message(std::move(msg)), span(s) {}
};

/// 诊断 - 主要数据结构
/// 借鉴 rustc DiagInner，但简化为不可变值类型
struct Diagnostic {
  Level level{Level::Error};           ///< 诊断级别
  Message message;                     ///< 主要消息
  std::optional<ErrorCode> code;       ///< 错误码（可选）
  MultiSpan spans;                     ///< 位置信息
  std::vector<SubDiagnostic> children; ///< 子诊断
  std::vector<Suggestion> suggestions; ///< 修复建议

  /// 默认构造
  Diagnostic() = default;

  /// 基本构造
  Diagnostic(Level lvl, Message msg) : level(lvl), message(std::move(msg)) {}

  /// 带错误码构造
  Diagnostic(Level lvl, Message msg, ErrorCode c)
      : level(lvl), message(std::move(msg)), code(c) {}

  // 可拷贝可移动
  Diagnostic(const Diagnostic &) = default;
  auto operator=(const Diagnostic &) -> Diagnostic & = default;
  Diagnostic(Diagnostic &&) noexcept = default;
  auto operator=(Diagnostic &&) noexcept -> Diagnostic & = default;

  /// 检查是否有错误码
  [[nodiscard]] auto hasCode() const noexcept -> bool {
    return code.has_value();
  }

  /// 检查是否为错误级别
  [[nodiscard]] auto isError() const noexcept -> bool {
    return level >= Level::Error;
  }

  /// 检查是否为警告级别
  [[nodiscard]] auto isWarning() const noexcept -> bool {
    return level == Level::Warning;
  }

  /// 获取主要 Span
  [[nodiscard]] auto primarySpan() const -> std::optional<Span> {
    auto primary = spans.primary();
    if (primary) {
      return primary->span;
    }
    return std::nullopt;
  }
};

} // namespace czc::diag

#endif // CZC_DIAG_DIAGNOSTIC_HPP
