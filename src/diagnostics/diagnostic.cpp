/**
 * @file diagnostic.cpp
 * @brief 诊断系统实现
 * @author BegoniaHe
 * @date 2025-11-04
 */

#include "czc/diagnostics/diagnostic.hpp"
#include "czc/diagnostics/color.hpp"
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace czc::diagnostics;

I18nMessages::I18nMessages(const std::string &locale) : current_locale(locale) {
  // 尝试加载指定的语言环境文件。
  // 如果失败（例如，文件不存在或格式错误），则回退到默认的 "en_US" 语言环境。
  // 这种设计确保了程序在任何情况下都能提供至少一种语言的诊断信息，增强了健壮性。
  if (!load_from_file(locale)) {
    load_from_file("en_US");
  }
}

bool I18nMessages::load_from_file(const std::string &locale) {
  std::vector<std::string> search_paths;

  // --- 建立本地化文件的搜索路径 ---
  // 这种多路径搜索策略提高了灵活性，适应不同的部署和开发环境。

  // 1. 检查环境变量 `ZERO_LOCALE_PATH`，允许用户覆盖默认的本地化文件位置。
  const char *env_path = std::getenv("ZERO_LOCALE_PATH");
  if (env_path != nullptr && env_path[0] != '\0') {
    std::string base_path(env_path);
    // 规范化路径，移除末尾的斜杠（如果存在）。
    if (!base_path.empty() && base_path.back() == '/') {
      base_path.pop_back();
    }
    search_paths.push_back(base_path + "/" + locale + "/diagnostics.toml");
  }

  // 2. 添加相对于可执行文件的常见相对路径，以适应标准安装布局。
  search_paths.push_back("locales/" + locale + "/diagnostics.toml");
  search_paths.push_back("../locales/" + locale + "/diagnostics.toml");
  search_paths.push_back("../../locales/" + locale + "/diagnostics.toml");

  // --- 查找并打开文件 ---
  std::string filepath;
  for (const auto &path : search_paths) {
    if (std::filesystem::exists(path)) {
      filepath = path;
      break; // 找到第一个有效文件后立即停止搜索。
    }
  }

  if (filepath.empty()) {
    return false; // 在所有搜索路径中都找不到文件。
  }

  std::ifstream file(filepath);
  if (!file.is_open()) {
    return false;
  }

  // --- 手动解析简化的 TOML 格式 ---
  // 在加载新文件之前，清空旧的消息映射表，以支持动态语言切换。
  messages.clear();

  std::string line;
  std::string current_code;
  MessageTemplate current_template;

  while (std::getline(file, line)) {
    // 忽略空行和以 '#' 开头的注释行。
    if (line.empty() || line[0] == '#') {
      continue;
    }

    // --- 解析诊断代码块，例如 `[L0001]` ---
    if (line[0] == '[') {
      // 当遇到一个新的代码块时，先保存上一个已解析完成的模板。
      if (!current_code.empty()) {
        messages[current_code] = current_template;
        current_template = MessageTemplate{}; // 重置模板以供下一个代码块使用。
      }

      size_t end = line.find(']');
      if (end != std::string::npos) {
        current_code = line.substr(1, end - 1);
      }
      continue;
    }

    // --- 解析 `key = "value"` 格式的键值对 ---
    size_t eq_pos = line.find('=');
    if (eq_pos == std::string::npos) {
      continue; // 如果没有 '='，则不是有效的键值对，跳过。
    }

    std::string key = line.substr(0, eq_pos);
    std::string value = line.substr(eq_pos + 1);

    // 清理键和值，去除两端的空白字符和值两端的引号。
    // 这样做可以使 .toml 文件的格式更加宽松，提高用户体验。
    key.erase(0, key.find_first_not_of(" \t"));
    key.erase(key.find_last_not_of(" \t") + 1);

    value.erase(0, value.find_first_not_of(" \t\""));
    value.erase(value.find_last_not_of(" \t\"") + 1);

    if (key == "message") {
      current_template.message = value;
    } else if (key == "help") {
      current_template.help = value;
    } else if (key == "source") {
      current_template.source = value;
    }
  }

  // 文件处理完毕后，保存最后一个解析的模板。
  if (!current_code.empty()) {
    messages[current_code] = current_template;
  }

  return !messages.empty();
}

void I18nMessages::set_locale(const std::string &locale) {
  current_locale = locale;
  if (!load_from_file(locale)) {
    load_from_file("en_US"); // Fallback to English
  }
}

const MessageTemplate &I18nMessages::get_message(DiagnosticCode code) const {
  std::string code_str = diagnostic_code_to_string(code);
  auto it = messages.find(code_str);

  // 如果找不到对应的消息模板（例如，.toml 文件不完整），
  // 返回一个静态的、通用的未知错误模板，以避免程序崩溃。
  if (it == messages.end()) {
    static MessageTemplate unknown{"unknown error", "", "system"};
    return unknown;
  }

  return it->second;
}

std::string
I18nMessages::format_message(DiagnosticCode code,
                             const std::vector<std::string> &args) const {
  const auto &tmpl = get_message(code);
  std::string result = tmpl.message;

  // --- 替换占位符 {0}, {1}, ... ---
  // 这是一个简单的模板替换逻辑。它会查找 `{0}`, `{1}` 等占位符，
  // 并将它们替换为 `args` 向量中对应索引的字符串。
  for (size_t i = 0; i < args.size(); ++i) {
    std::string placeholder = "{" + std::to_string(i) + "}";
    size_t pos = 0;
    // 循环查找并替换所有出现的同一个占位符。
    while ((pos = result.find(placeholder, pos)) != std::string::npos) {
      result.replace(pos, placeholder.length(), args[i]);
      pos += args[i].length(); // 更新搜索起始位置，防止无限循环。
    }
  }

  return result;
}

std::string Diagnostic::format(const I18nMessages &i18n, bool use_color) const {
  std::ostringstream oss;
  const auto &tmpl = i18n.get_message(code);

  // --- 1. 构造诊断信息的标题行 ---
  // 示例: error[L0007]: unterminated string (from: lexer)
  // 根据诊断级别（错误、警告、备注）应用不同的颜色以示区分。
  if (use_color) {
    oss << Color::Bold;
    oss << (level == DiagnosticLevel::Error     ? Color::Red
            : level == DiagnosticLevel::Warning ? Color::Yellow
                                                : Color::Cyan);
  }

  oss << (level == DiagnosticLevel::Error     ? "error"
          : level == DiagnosticLevel::Warning ? "warning"
                                              : "note");

  oss << "[" << diagnostic_code_to_string(code) << "]";
  if (use_color) {
    oss << Color::Reset;
  }

  oss << ": ";
  if (use_color) {
    oss << Color::Bold;
  }
  oss << i18n.format_message(code, args);
  if (use_color) {
    oss << Color::Reset;
  }

  oss << " (from: " << tmpl.source << ")";
  oss << "\n";

  // --- 2. 打印源代码位置信息 ---
  // 示例: --> examples/test_unterminated.zero:1:1
  if (!location.filename.empty()) {
    if (use_color) {
      oss << Color::Blue << Color::Bold;
    }
    oss << "  --> ";
    if (use_color) {
      oss << Color::Reset;
    }
    oss << location.filename << ":" << location.line << ":" << location.column
        << "\n";
  }

  // --- 3. 打印带有上下文的源代码行和高亮标记 ---
  // 示例:
  //      |
  //    1 | "this is an unclosed string
  //      | ^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  if (!source_line.empty()) {
    // 为了对齐，计算行号所需的宽度。
    int line_width = std::to_string(location.line).length();
    if (line_width < 3) {
      line_width = 3; // 最小宽度为3，为了美观。
    }

    // 打印源代码行。
    if (use_color) {
      oss << Color::Blue << Color::Bold;
    }
    oss << std::string(line_width, ' ') << " |\n";
    oss << std::setw(line_width) << location.line << " | ";
    if (use_color) {
      oss << Color::Reset;
    }
    oss << source_line << "\n";

    // 打印波浪线或尖号以高亮错误。
    if (use_color) {
      oss << Color::Blue << Color::Bold;
    }
    oss << std::string(line_width, ' ') << " | ";
    if (use_color) {
      oss << Color::Reset;
    }

    // 计算高亮标记前的空格数。
    size_t spaces = location.column > 0 ? location.column - 1 : 0;
    oss << std::string(spaces, ' ');

    // 计算高亮标记的长度。
    if (use_color) {
      oss << Color::Red << Color::Bold;
    }
    size_t length = location.end_column > location.column
                        ? location.end_column - location.column
                        : 1;
    oss << std::string(length, '^');
    if (use_color) {
      oss << Color::Reset;
    }
    oss << "\n";
  }

  // --- 4. 如果有帮助信息，则打印 ---
  // 示例: = help: add a closing `"` to the string literal
  if (!tmpl.help.empty()) {
    if (use_color) {
      oss << Color::Cyan << Color::Bold;
    }
    oss << "   = help: ";
    if (use_color) {
      oss << Color::Reset;
    }
    oss << tmpl.help << "\n";
  }

  return oss.str();
}

DiagnosticEngine::DiagnosticEngine(const std::string &locale)
    : i18n(std::make_shared<I18nMessages>(locale)) {}

void DiagnosticEngine::report(std::shared_ptr<Diagnostic> diag) {
  if (!diag) {
    return;
  }

  // 根据诊断的严重级别，增加相应的计数器。
  // 这是为了后续可以快速判断编译是否应该因错误而中止。
  if (diag->get_level() == DiagnosticLevel::Error ||
      diag->get_level() == DiagnosticLevel::Fatal) {
    error_count++;
  } else if (diag->get_level() == DiagnosticLevel::Warning) {
    warning_count++;
  }

  diagnostics.push_back(diag);
}

void DiagnosticEngine::print_all(bool use_color) const {
  for (const auto &diag : diagnostics) {
    std::cerr << diag->format(*i18n, use_color);
  }

  // 在打印完所有详细的诊断信息后，如果存在错误，
  // 打印一个总结性的中止信息。
  if (error_count > 0) {
    std::cerr << "\nerror: aborting due to " << error_count << " previous error"
              << (error_count > 1 ? "s" : "") << "\n";
  }
}
