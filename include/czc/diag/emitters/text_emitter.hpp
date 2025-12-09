/**
 * @file text_emitter.hpp
 * @brief 文本发射器。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-12-04
 *
 * @details
 *   人类可读终端输出的发射器。
 *   借鉴 rustc HumanEmitter。
 */

#ifndef CZC_DIAG_EMITTERS_TEXT_EMITTER_HPP
#define CZC_DIAG_EMITTERS_TEXT_EMITTER_HPP

#include "czc/common/config.hpp"
#include "czc/diag/emitter.hpp"
#include "czc/diag/emitters/ansi_renderer.hpp"

#include <ostream>

namespace czc::diag {

/// 文本发射器 - 人类可读终端输出
class TextEmitter final : public Emitter {
public:
  /// 构造文本发射器
  explicit TextEmitter(std::ostream &out,
                       AnsiStyle style = AnsiStyle::defaultStyle());

  /// 析构函数
  ~TextEmitter() override = default;

  // 禁止拷贝，允许移动
  TextEmitter(const TextEmitter &) = delete;
  auto operator=(const TextEmitter &) -> TextEmitter & = delete;
  TextEmitter(TextEmitter &&) noexcept = default;
  auto operator=(TextEmitter &&) noexcept -> TextEmitter & = default;

  /// 发射诊断
  void emit(const Diagnostic &diag, const SourceLocator *locator) override;

  /// 发射诊断总结信息
  void emitSummary(const DiagnosticStats &stats) override;

  /// 刷新缓冲区
  void flush() override;

  /// 获取渲染器
  [[nodiscard]] auto renderer() const noexcept -> const AnsiRenderer & {
    return renderer_;
  }

private:
  std::ostream *out_;
  AnsiRenderer renderer_;
};

} // namespace czc::diag

#endif // CZC_DIAG_EMITTERS_TEXT_EMITTER_HPP
