/**
 * @file diag_context.hpp
 * @brief 诊断上下文。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-12-04
 *
 * @details
 *   核心上下文类，借鉴 rustc DiagCtxt 设计，管理诊断发射和统计。
 */

#ifndef CZC_DIAG_DIAG_CONTEXT_HPP
#define CZC_DIAG_DIAG_CONTEXT_HPP

#include "czc/common/config.hpp"
#include "czc/diag/diagnostic.hpp"
#include "czc/diag/emitter.hpp"
#include "czc/diag/error_guaranteed.hpp"
#include "czc/diag/i18n.hpp"
#include "czc/diag/source_locator.hpp"

#include <functional>
#include <memory>
#include <mutex>

namespace czc::diag {

class Emitter;

/// 诊断配置
struct DiagConfig {
  bool deduplicate{true};            ///< 去重相同诊断
  size_t maxErrors{0};               ///< 最大错误数（0=无限）
  bool treatWarningsAsErrors{false}; ///< -Werror
  bool colorOutput{true};            ///< 彩色输出
};

// DiagnosticStats 定义在 emitter.hpp 中

/// 诊断上下文 - 线程安全
/// 借鉴 rustc DiagCtxt 设计
class DiagContext {
public:
  /// 构造诊断上下文
  /// @param emitter 诊断发射器
  /// @param locator 源码定位器（可选）
  /// @param config 诊断配置
  /// @param translator 翻译器（可选，默认创建新实例）
  explicit DiagContext(std::unique_ptr<Emitter> emitter,
                       const SourceLocator *locator = nullptr,
                       DiagConfig config = {},
                       std::unique_ptr<i18n::Translator> translator = nullptr);

  /// 析构函数
  ~DiagContext();

  // 禁止拷贝和移动（持有资源）
  DiagContext(const DiagContext &) = delete;
  auto operator=(const DiagContext &) -> DiagContext & = delete;
  DiagContext(DiagContext &&) noexcept;
  auto operator=(DiagContext &&) noexcept -> DiagContext &;

  // ========== 发射方法 ==========

  /// 发射诊断
  void emit(Diagnostic diag);

  /// 发射错误诊断并返回保证
  [[nodiscard]] auto emitError(Diagnostic diag) -> ErrorGuaranteed;

  /// 发射警告
  void emitWarning(Diagnostic diag);

  /// 发射注释
  void emitNote(Diagnostic diag);

  // ========== 便捷方法 ==========

  /// 发射简单错误并返回保证
  [[nodiscard]] auto error(Message message) -> ErrorGuaranteed;

  /// 发射带错误码和位置的错误
  [[nodiscard]] auto error(ErrorCode code, Message message, Span span)
      -> ErrorGuaranteed;

  /// 发射简单警告
  void warning(Message message);

  /// 发射简单注释
  void note(Message message);

  // ========== 统计查询 ==========

  /// 获取错误数量
  [[nodiscard]] auto errorCount() const noexcept -> size_t;

  /// 获取警告数量
  [[nodiscard]] auto warningCount() const noexcept -> size_t;

  /// 检查是否有错误
  [[nodiscard]] auto hasErrors() const noexcept -> bool;

  /// 检查是否应该中止
  [[nodiscard]] auto shouldAbort() const noexcept -> bool;

  /// 获取诊断统计信息
  [[nodiscard]] auto stats() const noexcept -> DiagnosticStats;

  /// 发射诊断总结
  void emitSummary();

  // ========== 配置 ==========

  /// 设置源码定位器
  void setLocator(const SourceLocator *locator);

  /// 获取源码定位器
  [[nodiscard]] auto locator() const noexcept -> const SourceLocator *;

  /// 获取配置
  [[nodiscard]] auto config() const noexcept -> const DiagConfig &;

  /// 获取可变配置
  [[nodiscard]] auto config() noexcept -> DiagConfig &;

  /// 获取翻译器
  [[nodiscard]] auto translator() noexcept -> i18n::Translator &;

  /// 获取翻译器
  [[nodiscard]] auto translator() const noexcept -> const i18n::Translator &;

  /// 刷新输出
  void flush();

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;

  /// 创建 ErrorGuaranteed
  [[nodiscard]] auto createErrorGuaranteed() -> ErrorGuaranteed;
};

} // namespace czc::diag

#endif // CZC_DIAG_DIAG_CONTEXT_HPP
