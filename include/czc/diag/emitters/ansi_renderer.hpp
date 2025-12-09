/**
 * @file ansi_renderer.hpp
 * @brief ANSI 颜色渲染器。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-12-04
 *
 * @details
 *   提供 Markdown 到 ANSI 转义序列的转换，遵循 LLVM 错误消息风格。
 */

#ifndef CZC_DIAG_EMITTERS_ANSI_RENDERER_HPP
#define CZC_DIAG_EMITTERS_ANSI_RENDERER_HPP

#include "czc/common/config.hpp"
#include "czc/diag/diagnostic.hpp"
#include "czc/diag/source_locator.hpp"

#include <string>
#include <string_view>

namespace czc::diag {

/// ANSI 颜色枚举
enum class AnsiColor : uint8_t {
  Default,
  Black,
  Red,
  Green,
  Yellow,
  Blue,
  Magenta,
  Cyan,
  White,
  BrightRed,
  BrightGreen,
  BrightYellow,
  BrightBlue,
  BrightMagenta,
  BrightCyan,
  BrightWhite,
};

/// 获取 ANSI 颜色码
[[nodiscard]] auto getAnsiColorCode(AnsiColor color) -> std::string_view;

/// ANSI 样式配置
struct AnsiStyle {
  bool enabled{true}; ///< 是否启用颜色
  AnsiColor errorColor{AnsiColor::BrightRed};
  AnsiColor warningColor{AnsiColor::BrightYellow};
  AnsiColor noteColor{AnsiColor::BrightCyan};
  AnsiColor helpColor{AnsiColor::BrightGreen};
  AnsiColor codeColor{AnsiColor::Cyan};
  AnsiColor lineNumColor{AnsiColor::Blue};

  /// 获取默认样式
  [[nodiscard]] static auto defaultStyle() noexcept -> AnsiStyle {
    return AnsiStyle{};
  }

  /// 获取无颜色样式
  [[nodiscard]] static auto noColor() noexcept -> AnsiStyle {
    AnsiStyle style;
    style.enabled = false;
    return style;
  }
};

/// ANSI 渲染器
/// 将诊断渲染为带 ANSI 转义的字符串
class AnsiRenderer {
public:
  /// 构造渲染器
  explicit AnsiRenderer(AnsiStyle style = AnsiStyle::defaultStyle());

  /// 析构函数
  ~AnsiRenderer() = default;

  // 可拷贝可移动
  AnsiRenderer(const AnsiRenderer &) = default;
  auto operator=(const AnsiRenderer &) -> AnsiRenderer & = default;
  AnsiRenderer(AnsiRenderer &&) noexcept = default;
  auto operator=(AnsiRenderer &&) noexcept -> AnsiRenderer & = default;

  /// 渲染完整诊断
  [[nodiscard]] auto renderDiagnostic(const Diagnostic &diag,
                                      const SourceLocator *locator) const
      -> std::string;

  /// 渲染消息（简单 Markdown -> ANSI）
  [[nodiscard]] auto renderMessage(std::string_view msg) const -> std::string;

  /// 获取诊断级别的颜色
  [[nodiscard]] auto getLevelColor(Level level) const -> AnsiColor;

  /// 包装颜色
  [[nodiscard]] auto wrapColor(std::string_view text, AnsiColor color) const
      -> std::string;

  /// 包装粗体
  [[nodiscard]] auto wrapBold(std::string_view text) const -> std::string;

  /// 获取样式
  [[nodiscard]] auto style() const noexcept -> const AnsiStyle & {
    return style_;
  }

private:
  AnsiStyle style_;

  /// 渲染源码片段
  [[nodiscard]] auto renderSourceSnippet(const Diagnostic &diag,
                                         const SourceLocator *locator) const
      -> std::string;

  /// 渲染标注指示器
  [[nodiscard]] auto renderAnnotation(const LabeledSpan &span,
                                      uint32_t lineStartCol,
                                      AnsiColor color) const -> std::string;
};

} // namespace czc::diag

#endif // CZC_DIAG_EMITTERS_ANSI_RENDERER_HPP
