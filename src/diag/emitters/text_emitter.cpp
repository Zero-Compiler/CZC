/**
 * @file text_emitter.cpp
 * @brief 文本发射器实现。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-12-04
 */

#include "czc/diag/emitters/text_emitter.hpp"

#include <format>

namespace czc::diag {

TextEmitter::TextEmitter(std::ostream &out, AnsiStyle style)
    : out_(&out), renderer_(std::move(style)) {}

void TextEmitter::emit(const Diagnostic &diag, const SourceLocator *locator) {
  *out_ << renderer_.renderDiagnostic(diag, locator);
}

void TextEmitter::emitSummary(const DiagnosticStats &stats) {
  if (stats.errorCount == 0 && stats.warningCount == 0) {
    return;
  }

  *out_ << "\n";

  // 输出错误统计
  if (stats.errorCount > 0) {
    std::string errorMsg;
    if (stats.errorCount == 1) {
      errorMsg = renderer_.wrapColor("error", AnsiColor::BrightRed);
      *out_ << errorMsg << ": aborting due to 1 previous error";
    } else {
      errorMsg = renderer_.wrapColor("error", AnsiColor::BrightRed);
      *out_ << errorMsg << ": aborting due to " << stats.errorCount
            << " previous errors";
    }

    if (stats.warningCount > 0) {
      *out_ << "; " << stats.warningCount << " warning"
            << (stats.warningCount > 1 ? "s" : "") << " emitted";
    }
    *out_ << "\n";

    // 提示使用 --explain 查看更多信息
    if (!stats.uniqueErrorCodes.empty()) {
      auto firstCode = *stats.uniqueErrorCodes.begin();
      *out_ << "\nFor more information about this error, try `czc --explain "
            << firstCode.toString() << "`.\n";
    }
  } else if (stats.warningCount > 0) {
    // 只有警告
    std::string warningMsg =
        renderer_.wrapColor("warning", AnsiColor::BrightYellow);
    *out_ << warningMsg << ": " << stats.warningCount << " warning"
          << (stats.warningCount > 1 ? "s" : "") << " emitted\n";
  }
}

void TextEmitter::flush() { out_->flush(); }

} // namespace czc::diag
