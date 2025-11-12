/**
 * @file diagnostic.cpp
 * @brief 诊断系统核心组件的实现。
 * @author BegoniaHe
 * @date 2025-11-11
 */

#include "czc/diagnostics/diagnostic.hpp"

#include "czc/utils/color.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace czc::diagnostics;
using namespace czc::utils;

I18nMessages::I18nMessages(const std::string& locale) : current_locale(locale) {
  // NOTE: 尝试加载用户指定的语言环境。如果失败（例如，文件不存在或格式错误），
  //       则立即回退到默认的 "en_US" 语言环境。这种“失败安全” (fail-safe)
  //       的设计确保了诊断系统在任何情况下都能正常工作，至少能提供英文的
  //       错误信息，从而增强了编译器的健壮性。
  if (!load_from_file(locale)) {
    load_from_file("en_US");
  }
}

bool I18nMessages::load_from_file(const std::string& locale) {
  std::vector<std::string> search_paths;

  // --- 建立本地化文件的搜索路径 ---
  // NOTE: 采用多路径搜索策略是为了提高程序的灵活性和可移植性，使其能够
  //       适应不同的部署和开发环境。搜索顺序经过精心设计：
  //       1. 环境变量 (`ZERO_LOCALE_PATH`): 优先级最高，允许用户或构建系统
  //          在运行时动态指定本地化文件的位置，非常适合容器化或自定义安装。
  //       2. 相对路径: 其次尝试相对于当前工作目录的常见路径，这覆盖了
  //          大多数本地开发和标准构建输出的场景。

  // 1. 检查环境变量 `ZERO_LOCALE_PATH`
  const char* env_path = std::getenv("ZERO_LOCALE_PATH");
  if (env_path != nullptr && env_path[0] != '\0') {
    std::string base_path(env_path);
    // 规范化路径，移除末尾可能存在的斜杠，以确保路径拼接的正确性。
    if (!base_path.empty() &&
        (base_path.back() == '/' || base_path.back() == '\\')) {
      base_path.pop_back();
    }
    search_paths.push_back(base_path + "/" + locale + "/diagnostics.toml");
  }

  // 2. 添加相对于当前工作目录的常见相对路径
  search_paths.push_back("locales/" + locale + "/diagnostics.toml");
  search_paths.push_back("../locales/" + locale + "/diagnostics.toml");
  search_paths.push_back("../../locales/" + locale + "/diagnostics.toml");

  // --- 查找并打开文件 ---
  std::string filepath;
  for (const auto& path : search_paths) {
    if (std::filesystem::exists(path)) {
      filepath = path;
      break; // NOTE: 找到第一个有效文件后立即停止搜索，确保了搜索路径的优先级。
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
  // NOTE: 这里采用了一个简化的、手写的 TOML 解析器，而不是引入一个完整的
  //       第三方 TOML 库。这样做的权衡是为了：
  //       - 优点: 减少外部依赖，使编译器更轻量、更易于构建。
  //       - 缺点: 解析器功能有限，仅支持 `[section]` 和 `key = "value"`
  //         格式，对 TOML 文件的格式要求更严格。
  // 在加载新文件之前，清空旧的消息映射表，这是支持动态语言切换的关键步骤。
  messages.clear();

  std::string line;
  std::string current_code;
  MessageTemplate current_template;

  while (std::getline(file, line)) {
    // 检查读取错误
    if (file.bad()) {
      return false; // I/O 错误，读取失败
    }
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

    // --- 清理键和值 ---
    // NOTE:
    // 对键和值进行修剪（trimming）操作，去除两端的空白字符和值两端的引号。
    //       这使得 .toml 文件的格式可以更加宽松（例如，允许 `key = " value "
    //       `）， 提高了用户编写本地化文件时的容错性。 `find_last_not_of`
    //       的结果加 1 是为了正确处理 `erase` 的范围。
    //       如果字符串全为空白，`find_last_not_of` 返回 `npos`，`npos + 1`
    //       溢出为 0， `erase(0)` 恰好能清空整个字符串，这是一个巧妙的处理。
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

void I18nMessages::set_locale(const std::string& locale) {
  current_locale = locale;
  if (!load_from_file(locale)) {
    load_from_file("en_US"); // Fallback to English
  }
}

const MessageTemplate& I18nMessages::get_message(DiagnosticCode code) const {
  std::string code_str = diagnostic_code_to_string(code);
  auto it = messages.find(code_str);

  // NOTE: 如果在消息映射中找不到对应的模板（例如，.toml 文件不完整或
  //       代码中新增了诊断码但忘记更新 .toml），我们不能让程序崩溃。
  //       返回一个静态的、通用的未知错误模板是一种健壮的错误处理方式，
  //       确保了即使在配置不完整的情况下，程序也能继续运行并提供有意义的
  //       （尽管是通用的）反馈。
  if (it == messages.end()) {
    static MessageTemplate unknown{"unknown error", "", "system"};
    return unknown;
  }

  return it->second;
}

std::string
I18nMessages::format_message(DiagnosticCode code,
                             const std::vector<std::string>& args) const {
  const auto& tmpl = get_message(code);
  std::string result = tmpl.message;

  // --- 替换占位符 {0}, {1}, ... ---
  // 这是一个简单的模板替换逻辑。它会查找 `{0}`, `{1}` 等占位符，
  // 并将它们替换为 `args` 向量中对应索引的字符串。
  for (size_t i = 0; i < args.size(); ++i) {
    std::string placeholder = "{" + std::to_string(i) + "}";
    size_t pos = 0;
    // NOTE: 这里使用循环来查找并替换所有出现的同一个占位符（例如，消息
    //       `"{0} is not compatible with {0}"`）。在 `replace` 之后，
    //       必须将搜索起始位置 `pos` 更新到被替换内容之后，以防止
    //       当替换内容本身也包含占位符时（虽然不太可能，但仍是好的实践）
    //       导致的无限循环。
    while ((pos = result.find(placeholder, pos)) != std::string::npos) {
      result.replace(pos, placeholder.length(), args[i]);
      pos += args[i].length();
    }
  }

  return result;
}

std::string Diagnostic::format(const I18nMessages& i18n, bool use_color) const {
  std::ostringstream oss;
  const auto& tmpl = i18n.get_message(code);

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
    // NOTE: 设置一个最小行号宽度（例如 3），可以确保即使在文件的前几行
    //       出错，诊断信息的整体布局也能保持对齐和美观，不会因为行号
    //       从 1 位数变成 2 位数而导致格式错乱。
    if (line_width < 3) {
      line_width = 3;
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

DiagnosticEngine::DiagnosticEngine(const std::string& locale)
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
  for (const auto& diag : diagnostics) {
    std::cerr << diag->format(*i18n, use_color);
  }

  // 在打印完所有详细的诊断信息后，如果存在错误，
  // 打印一个总结性的中止信息。
  if (error_count > 0) {
    std::cerr << "\nerror: aborting due to " << error_count << " previous error"
              << (error_count > 1 ? "s" : "") << "\n";
  }
}
