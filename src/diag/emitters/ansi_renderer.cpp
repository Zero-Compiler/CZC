/**
 * @file ansi_renderer.cpp
 * @brief ANSI 颜色渲染器实现。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-12-04
 */

#include "czc/diag/emitters/ansi_renderer.hpp"

#include <cmark.h>
#include <format>
#include <sstream>

namespace czc::diag {

auto getAnsiColorCode(AnsiColor color) -> std::string_view {
  switch (color) {
  case AnsiColor::Default:
    return "\033[0m";
  case AnsiColor::Black:
    return "\033[30m";
  case AnsiColor::Red:
    return "\033[31m";
  case AnsiColor::Green:
    return "\033[32m";
  case AnsiColor::Yellow:
    return "\033[33m";
  case AnsiColor::Blue:
    return "\033[34m";
  case AnsiColor::Magenta:
    return "\033[35m";
  case AnsiColor::Cyan:
    return "\033[36m";
  case AnsiColor::White:
    return "\033[37m";
  case AnsiColor::BrightRed:
    return "\033[91m";
  case AnsiColor::BrightGreen:
    return "\033[92m";
  case AnsiColor::BrightYellow:
    return "\033[93m";
  case AnsiColor::BrightBlue:
    return "\033[94m";
  case AnsiColor::BrightMagenta:
    return "\033[95m";
  case AnsiColor::BrightCyan:
    return "\033[96m";
  case AnsiColor::BrightWhite:
    return "\033[97m";
  default:
    return "\033[0m";
  }
}

AnsiRenderer::AnsiRenderer(AnsiStyle style) : style_(std::move(style)) {}

auto AnsiRenderer::getLevelColor(Level level) const -> AnsiColor {
  switch (level) {
  case Level::Note:
    return style_.noteColor;
  case Level::Help:
    return style_.helpColor;
  case Level::Warning:
    return style_.warningColor;
  case Level::Error:
  case Level::Fatal:
  case Level::Bug:
    return style_.errorColor;
  default:
    return AnsiColor::Default;
  }
}

auto AnsiRenderer::wrapColor(std::string_view text, AnsiColor color) const
    -> std::string {
  if (!style_.enabled) {
    return std::string(text);
  }
  return std::format("{}{}{}", getAnsiColorCode(color), text,
                     getAnsiColorCode(AnsiColor::Default));
}

auto AnsiRenderer::wrapBold(std::string_view text) const -> std::string {
  if (!style_.enabled) {
    return std::string(text);
  }
  return std::format("\033[1m{}\033[0m", text);
}

namespace {

/// 使用 cmark 遍历节点树并生成 ANSI 格式输出
void renderNodeToAnsi(cmark_node *node, std::string &out,
                      const AnsiStyle &style) {
  if (node == nullptr) {
    return;
  }

  cmark_node_type nodeType = cmark_node_get_type(node);

  switch (nodeType) {
  case CMARK_NODE_TEXT: {
    const char *literal = cmark_node_get_literal(node);
    if (literal != nullptr) {
      out += literal;
    }
    break;
  }

  case CMARK_NODE_CODE: {
    // 行内代码 `code` -> 青色
    const char *literal = cmark_node_get_literal(node);
    if (literal != nullptr) {
      if (style.enabled) {
        out += getAnsiColorCode(style.codeColor);
        out += literal;
        out += getAnsiColorCode(AnsiColor::Default);
      } else {
        out += '`';
        out += literal;
        out += '`';
      }
    }
    break;
  }

  case CMARK_NODE_STRONG: {
    // **粗体** -> ANSI bold
    if (style.enabled) {
      out += "\033[1m";
    }
    for (cmark_node *child = cmark_node_first_child(node); child != nullptr;
         child = cmark_node_next(child)) {
      renderNodeToAnsi(child, out, style);
    }
    if (style.enabled) {
      out += "\033[0m";
    }
    return; // 已处理子节点
  }

  case CMARK_NODE_EMPH: {
    // *斜体* -> ANSI italic (ESC[3m)
    if (style.enabled) {
      out += "\033[3m";
    }
    for (cmark_node *child = cmark_node_first_child(node); child != nullptr;
         child = cmark_node_next(child)) {
      renderNodeToAnsi(child, out, style);
    }
    if (style.enabled) {
      out += "\033[0m";
    }
    return; // 已处理子节点
  }

  case CMARK_NODE_LINK: {
    // 链接 [text](url) -> 蓝色下划线
    if (style.enabled) {
      out += "\033[34;4m"; // 蓝色 + 下划线
    }
    for (cmark_node *child = cmark_node_first_child(node); child != nullptr;
         child = cmark_node_next(child)) {
      renderNodeToAnsi(child, out, style);
    }
    if (style.enabled) {
      out += "\033[0m";
    }
    return;
  }

  case CMARK_NODE_SOFTBREAK:
  case CMARK_NODE_LINEBREAK:
    out += '\n';
    break;

  case CMARK_NODE_CODE_BLOCK: {
    // 代码块 - 青色，前面加缩进
    const char *literal = cmark_node_get_literal(node);
    if (literal != nullptr) {
      if (style.enabled) {
        out += getAnsiColorCode(style.codeColor);
      }
      // 添加缩进
      std::string_view code(literal);
      for (size_t i = 0; i < code.size(); ++i) {
        if (i == 0 || (i > 0 && code[i - 1] == '\n')) {
          out += "    "; // 4空格缩进
        }
        out += code[i];
      }
      if (style.enabled) {
        out += getAnsiColorCode(AnsiColor::Default);
      }
    }
    break;
  }

  default:
    break;
  }

  // 递归处理子节点
  for (cmark_node *child = cmark_node_first_child(node); child != nullptr;
       child = cmark_node_next(child)) {
    renderNodeToAnsi(child, out, style);
  }
}

} // namespace

auto AnsiRenderer::renderMessage(std::string_view msg) const -> std::string {
  if (msg.empty()) {
    return "";
  }

  // 使用 cmark 解析 Markdown
  cmark_node *doc =
      cmark_parse_document(msg.data(), msg.size(), CMARK_OPT_DEFAULT);

  if (doc == nullptr) {
    // 解析失败，返回原始内容
    return std::string(msg);
  }

  std::string result;
  result.reserve(msg.size() * 2);

  renderNodeToAnsi(doc, result, style_);
  cmark_node_free(doc);

  // 移除末尾多余换行（诊断消息通常不需要尾部换行）
  while (!result.empty() && result.back() == '\n') {
    result.pop_back();
  }

  return result;
}

auto AnsiRenderer::renderDiagnostic(const Diagnostic &diag,
                                    const SourceLocator *locator) const
    -> std::string {
  std::ostringstream out;

  auto levelColor = getLevelColor(diag.level);
  auto levelStr = levelToString(diag.level);

  // 第一行：error[L1001]: message
  out << wrapBold(wrapColor(levelStr, levelColor));

  if (diag.hasCode()) {
    out << wrapBold(
        wrapColor(std::format("[{}]", diag.code->toString()), levelColor));
  }

  out << wrapBold(": ");
  out << renderMessage(diag.message.renderPlainText());
  out << "\n";

  // 位置信息
  auto primarySpan = diag.spans.primary();
  if (primarySpan && locator != nullptr) {
    auto filename = locator->getFilename(primarySpan->span);
    auto lc = locator->getLineColumn(primarySpan->span.fileId,
                                     primarySpan->span.startOffset);

    out << "  ";
    out << wrapColor("-->", style_.lineNumColor);
    out << " " << filename << ":" << lc.line << ":" << lc.column;
    out << "\n";

    // 源码片段
    out << renderSourceSnippet(diag, locator);
  }

  // 子诊断
  for (const auto &child : diag.children) {
    auto childColor = getLevelColor(child.level);
    auto childLevelStr = levelToString(child.level);

    out << "  = ";
    out << wrapBold(wrapColor(childLevelStr, childColor));
    out << ": ";
    out << renderMessage(child.message);
    out << "\n";
  }

  // 建议
  for (const auto &suggestion : diag.suggestions) {
    out << "  = ";
    out << wrapBold(wrapColor("help", style_.helpColor));
    out << ": ";
    out << renderMessage(suggestion.message);
    if (!suggestion.replacement.empty()) {
      out << ": ";
      out << wrapColor("`" + suggestion.replacement + "`", style_.codeColor);
    }
    out << "\n";
  }

  return out.str();
}

auto AnsiRenderer::renderSourceSnippet(const Diagnostic &diag,
                                       const SourceLocator *locator) const
    -> std::string {
  if (locator == nullptr) {
    return "";
  }

  auto primarySpan = diag.spans.primary();
  if (!primarySpan) {
    return "";
  }

  std::ostringstream out;

  auto lc = locator->getLineColumn(primarySpan->span.fileId,
                                   primarySpan->span.startOffset);
  auto lineContent = locator->getLineContent(primarySpan->span.fileId, lc.line);

  if (lineContent.empty()) {
    return "";
  }

  // 行号宽度 - 计算行号字符串的显示宽度
  std::string lineNumStr = std::to_string(lc.line);
  size_t lineNumWidth = lineNumStr.size();

  // 创建与行号等宽的空白边距
  std::string margin(lineNumWidth, ' ');

  // 打印空白行 "{margin} |"
  // rustc 格式: "   |" 其中空格数等于行号宽度
  out << " " << margin << " " << wrapColor("|", style_.lineNumColor) << "\n";

  // 打印 "{line_num} | {content}"
  // 右对齐行号，宽度为 lineNumWidth
  out << " " << wrapColor(lineNumStr, style_.lineNumColor);
  out << " " << wrapColor("|", style_.lineNumColor);
  out << " " << lineContent << "\n";

  // 打印标注行 "{margin} | {spaces}{carets}"
  out << " " << margin << " " << wrapColor("|", style_.lineNumColor) << " ";

  // 计算列偏移（1-based 转 0-based）
  size_t col = lc.column > 0 ? lc.column - 1 : 0;
  out << std::string(col, ' ');

  // 打印标注符号
  size_t spanLen = primarySpan->span.length();
  if (spanLen == 0) {
    spanLen = 1;
  }

  auto levelColor = getLevelColor(diag.level);
  out << wrapColor(std::string(spanLen, '^'), levelColor);

  // 打印标签
  if (!primarySpan->label.empty()) {
    out << " " << wrapColor(primarySpan->label, levelColor);
  }
  out << "\n";

  return out.str();
}

auto AnsiRenderer::renderAnnotation(const LabeledSpan & /*span*/,
                                    uint32_t /*lineStartCol*/,
                                    AnsiColor /*color*/) const -> std::string {
  // 简化实现
  return "";
}

} // namespace czc::diag
