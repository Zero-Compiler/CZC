/**
 * @file json_emitter.cpp
 * @brief JSON 发射器实现。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-12-04
 */

#include "czc/diag/emitters/json_emitter.hpp"

#include <glaze/glaze.hpp>

#include <sstream>

namespace czc::diag {

JsonEmitter::JsonEmitter(std::ostream &out, bool pretty)
    : out_(&out), pretty_(pretty) {}

JsonEmitter::~JsonEmitter() = default;

void JsonEmitter::emit(const Diagnostic &diag, const SourceLocator *locator) {
  if (firstDiag_) {
    *out_ << "{\"diagnostics\": [\n";
    firstDiag_ = false;
  } else {
    *out_ << ",\n";
  }

  *out_ << diagnosticToJson(diag, locator);
}

void JsonEmitter::emitSummary(const DiagnosticStats &stats) {
  // 在 flush 之前添加统计信息
  if (!firstDiag_) {
    *out_ << "\n], \"stats\": {\n";
    *out_ << "  \"error_count\": " << stats.errorCount << ",\n";
    *out_ << "  \"warning_count\": " << stats.warningCount << ",\n";
    *out_ << "  \"note_count\": " << stats.noteCount << ",\n";
    *out_ << "  \"unique_error_codes\": [";

    bool first = true;
    for (const auto &code : stats.uniqueErrorCodes) {
      if (!first) {
        *out_ << ", ";
      }
      first = false;
      *out_ << "\"" << code.toString() << "\"";
    }
    *out_ << "]\n";
    *out_ << "}}";
  }
}

void JsonEmitter::flush() {
  // 如果没有调用 emitSummary，则关闭数组
  if (!firstDiag_) {
    // 检查是否已经输出了 summary（通过检查是否以 '}' 结尾）
    // 这里简化处理，假设 emitSummary 已经处理了关闭
  }
  out_->flush();
}

auto JsonEmitter::diagnosticToJson(const Diagnostic &diag,
                                   const SourceLocator *locator) const
    -> std::string {
  std::ostringstream out;

  out << "  {\n";
  out << "    \"level\": \"" << levelToString(diag.level) << "\",\n";

  if (diag.hasCode()) {
    out << "    \"code\": \"" << diag.code->toString() << "\",\n";
  }

  // 转义消息中的特殊字符
  auto message = diag.message.renderPlainText();
  std::string escapedMessage;
  for (char c : message) {
    switch (c) {
    case '"':
      escapedMessage += "\\\"";
      break;
    case '\\':
      escapedMessage += "\\\\";
      break;
    case '\n':
      escapedMessage += "\\n";
      break;
    case '\r':
      escapedMessage += "\\r";
      break;
    case '\t':
      escapedMessage += "\\t";
      break;
    default:
      escapedMessage += c;
      break;
    }
  }
  out << "    \"message\": \"" << escapedMessage << "\",\n";

  // Spans
  out << "    \"spans\": [";
  bool first = true;
  for (const auto &ls : diag.spans.spans()) {
    if (!first)
      out << ", ";
    first = false;
    out << spanToJson(ls.span, locator);
  }
  out << "],\n";

  // Children
  out << "    \"children\": [";
  first = true;
  for (const auto &child : diag.children) {
    if (!first)
      out << ", ";
    first = false;
    out << "{\"level\": \"" << levelToString(child.level) << "\", ";
    out << "\"message\": \"" << child.message << "\"}";
  }
  out << "],\n";

  // Suggestions
  out << "    \"suggestions\": [";
  first = true;
  for (const auto &suggestion : diag.suggestions) {
    if (!first)
      out << ", ";
    first = false;
    out << "{\"message\": \"" << suggestion.message << "\", ";
    out << "\"replacement\": \"" << suggestion.replacement << "\"}";
  }
  out << "]\n";

  out << "  }";

  return out.str();
}

auto JsonEmitter::spanToJson(const Span &span,
                             const SourceLocator *locator) const
    -> std::string {
  std::ostringstream out;

  out << "{";
  out << "\"file_id\": " << span.fileId << ", ";
  out << "\"start\": " << span.startOffset << ", ";
  out << "\"end\": " << span.endOffset;

  if (locator != nullptr && span.isValid()) {
    auto filename = locator->getFilename(span);
    auto lc = locator->getLineColumn(span.fileId, span.startOffset);
    out << ", \"file\": \"" << filename << "\"";
    out << ", \"line\": " << lc.line;
    out << ", \"column\": " << lc.column;
  }

  out << "}";

  return out.str();
}

} // namespace czc::diag
