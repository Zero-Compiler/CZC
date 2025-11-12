/**
 * @file main.cpp
 * @brief CZC 编译器命令行工具入口。
 * @author BegoniaHe
 * @date 2025-11-11
 */

#include "czc/diagnostics/diagnostic.hpp"
#include "czc/formatter/formatter.hpp"
#include "czc/lexer/lexer.hpp"
#include "czc/parser/parser.hpp"
#include "czc/token_preprocessor/token_preprocessor.hpp"
#include "czc/utils/color.hpp"
#include "czc/utils/file_collector.hpp"
#include "czc/utils/source_tracker.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace czc::diagnostics;
using namespace czc::formatter;
using namespace czc::lexer;
using namespace czc::parser;
using namespace czc::token_preprocessor;
using namespace czc::utils;

// 版本信息
const std::string VERSION = "0.1.0";
/**
 * @brief 打印错误消息（红色）。
 * @param[in] message 错误消息内容。
 */
inline void print_error(const std::string& message) {
  std::cerr << Color::Red << "Error:" << Color::Reset << " " << message
            << std::endl;
}

/**
 * @brief 打印成功消息（绿色）。
 * @param[in] message 成功消息内容。
 */
inline void print_success(const std::string& message) {
  std::cout << Color::Green << message << Color::Reset << std::endl;
}

/**
 * @brief 打印警告消息（黄色）。
 * @param[in] message 警告消息内容。
 */
inline void print_warning(const std::string& message) {
  std::cout << Color::Yellow << "Warning:" << Color::Reset << " " << message
            << std::endl;
}

/**
 * @brief 打印信息消息（青色）。
 * @param[in] message 信息消息内容。
 */
inline void print_info(const std::string& message) {
  std::cout << Color::Cyan << message << Color::Reset << std::endl;
}

/**
 * @brief 打印粗体文本。
 * @param[in] text 要加粗的文本。
 */
inline void print_bold(const std::string& text) {
  std::cout << Color::Bold << text << Color::Reset;
}

/**
 * @brief 打印带颜色的文本到标准输出。
 * @param[in] text 要打印的文本。
 * @param[in] color 颜色代码。
 */
inline void print_colored(const std::string& text, const std::string& color) {
  std::cout << color << text << Color::Reset;
}

/**
 * @brief 打印阶段标题（用于区分不同的错误阶段）。
 * @param[in] title 标题文本。
 */
inline void print_error_stage(const std::string& title) {
  std::cerr << "\n" << Color::Red << title << Color::Reset << "\n" << std::endl;
}

/**
 * @brief 打印命令行工具的使用说明。
 * @param[in] program_name 程序的名称。
 */
void print_usage(const std::string& program_name) {
  print_bold("Usage:");
  std::cout << " " << program_name << " [options] <command> <file>..."
            << std::endl;

  std::cout << "\n";
  print_bold("Options:");
  std::cout << std::endl;
  std::cout << "  ";
  print_colored("--locale", Color::Green);
  std::cout << " <locale>         Set the locale for diagnostic messages "
               "(default: en_US)"
            << std::endl;
  std::cout << "                            Available: en_US, zh_CN, ne_KO"
            << std::endl;
  std::cout << "  ";
  print_colored("--help", Color::Green);
  std::cout << ", ";
  print_colored("-h", Color::Green);
  std::cout << "              Show this help message" << std::endl;
  std::cout << "  ";
  print_colored("--version", Color::Green);
  std::cout << ", ";
  print_colored("-v", Color::Green);
  std::cout << "           Show version information" << std::endl;

  std::cout << "\n";
  print_bold("Commands:");
  std::cout << std::endl;
  std::cout << "  ";
  print_colored("tokenize", Color::Yellow);
  std::cout << " <input_file>...  Tokenize one or more input files"
            << std::endl;
  std::cout << "                            Output will be saved as "
               "<input_file>.tokens"
            << std::endl;
  std::cout << "                            Supports multiple files and "
               "wildcards"
            << std::endl;
  std::cout << "  ";
  print_colored("parse", Color::Yellow);
  std::cout << " <input_file>...     Parse one or more input files and report "
               "errors"
            << std::endl;
  std::cout << "                            No output files are generated"
            << std::endl;
  std::cout << "  ";
  print_colored("fmt", Color::Yellow);
  std::cout << " <input_file>...       Format one or more input files"
            << std::endl;
  std::cout << "                            Formatted code will be written to "
               "<input_file>.formatted"
            << std::endl;
  std::cout << "                            Use --in-place to modify files "
               "directly"
            << std::endl;

  std::cout << "\n";
  print_bold("Format Options:");
  std::cout << std::endl;
  std::cout << "  ";
  print_colored("--in-place", Color::Green);
  std::cout << ", ";
  print_colored("-i", Color::Green);
  std::cout << "          Format files in-place (modifies original files)"
            << std::endl;
  std::cout << "  ";
  print_colored("--indent-width", Color::Green);
  std::cout << " <n>        Set indentation width (default: 4)" << std::endl;
  std::cout << "  ";
  print_colored("--use-tabs", Color::Green);
  std::cout << "               Use tabs for indentation instead of spaces"
            << std::endl;

  std::cout << "\n";
  print_bold("Examples:");
  std::cout << std::endl;
  std::cout << "  " << program_name << " tokenize example.zero" << std::endl;
  std::cout << "  " << program_name << " parse example.zero" << std::endl;
  std::cout << "  " << program_name << " fmt example.zero" << std::endl;
  std::cout << "  " << program_name << " fmt --in-place example.zero"
            << std::endl;
  std::cout << "  " << program_name << " --locale zh_CN tokenize example.zero"
            << std::endl;
  std::cout << "  " << program_name << " tokenize file1.zero file2.zero"
            << std::endl;
  std::cout << "  " << program_name << " fmt test_*.zero" << std::endl;
}

/**
 * @brief 对单个文件执行格式化并输出结果。
 * @details
 *   此函数执行完整的格式化流程：
 *   1. **文件读取与验证**: 确保文件存在且可读。
 *   2. **词法分析**: 将源码转换为 Token 序列。
 *   3. **Token 预处理**: 处理科学计数法等特殊 Token。
 *   4. **语法分析**: 构建 CST。
 *   5. **代码格式化**: 使用 Formatter 格式化 CST。
 *   6. **结果输出**: 将格式化后的代码写入文件或输出到控制台。
 *
 * @param[in] input_path 输入文件的路径。
 * @param[in] locale     用于诊断消息的语言环境代码。
 * @param[in] options    格式化选项。
 * @param[in] in_place   是否就地修改文件（默认为 false）。
 * @return 如果格式化成功返回 `true`，否则返回 `false`。
 */
bool format_file(const std::string& input_path, const std::string& locale,
                 const FormatOptions& options, bool in_place = false) {
  // --- 1. 文件校验和读取 ---
  if (input_path.empty()) {
    print_error("Input file path is empty");
    return false;
  }
  if (!std::filesystem::exists(input_path)) {
    print_error("File '" + input_path + "' does not exist");
    return false;
  }
  if (!std::filesystem::is_regular_file(input_path)) {
    print_error("'" + input_path + "' is not a regular file");
    return false;
  }

  std::ifstream input_file(input_path);
  if (!input_file.is_open()) {
    print_error("Cannot open file '" + input_path + "'");
    return false;
  }

  std::stringstream buffer;
  buffer << input_file.rdbuf();
  std::string content = buffer.str();
  input_file.close();

  std::cout << "Formatting file: " << input_path << std::endl;

  DiagnosticEngine diagnostics(locale);
  SourceTracker source_tracker(content, input_path);

  // --- 2. 词法分析 ---
  Lexer lexer(content, input_path);
  auto tokens = lexer.tokenize();

  // --- 3. 报告词法分析错误 ---
  if (lexer.get_errors().has_errors()) {
    for (const auto& error : lexer.get_errors().get_errors()) {
      auto diag = std::make_shared<Diagnostic>(
          DiagnosticLevel::Error, error.code, error.location, error.args);
      diag->set_source_line(
          source_tracker.get_source_line(error.location.line));
      diagnostics.report(diag);
    }
  }

  if (diagnostics.has_errors()) {
    print_error_stage("Errors found during lexical analysis:");
    diagnostics.print_all(true);
    return false;
  }

  // --- 4. Token 预处理 ---
  TokenPreprocessor preprocessor;
  auto processed_tokens = preprocessor.process(tokens, input_path, content);

  // --- 5. 报告 Token 预处理错误 ---
  if (preprocessor.get_errors().has_errors()) {
    for (const auto& error : preprocessor.get_errors().get_errors()) {
      auto diag = std::make_shared<Diagnostic>(
          DiagnosticLevel::Error, error.code, error.location, error.args);
      diag->set_source_line(
          source_tracker.get_source_line(error.location.line));
      diagnostics.report(diag);
    }
  }

  if (diagnostics.has_errors()) {
    print_error_stage("Errors found during token preprocessing:");
    diagnostics.print_all(true);
    return false;
  }

  // --- 6. 语法分析 ---
  Parser parser(processed_tokens, input_path);
  auto cst = parser.parse();

  // --- 7. 报告语法分析错误 ---
  if (parser.has_errors()) {
    for (const auto& error : parser.get_errors()) {
      auto diag = std::make_shared<Diagnostic>(
          DiagnosticLevel::Error, error.code, error.location, error.args);
      diag->set_source_line(
          source_tracker.get_source_line(error.location.line));
      diagnostics.report(diag);
    }
  }

  if (diagnostics.has_errors()) {
    print_error_stage("Errors found during parsing:");
    diagnostics.print_all(true);
    return false;
  }

  // --- 8. 格式化 ---
  Formatter formatter(options);
  std::string formatted_code = formatter.format(cst.get());

  // --- 9. 报告格式化错误 ---
  if (formatter.get_error_collector().has_errors()) {
    for (const auto& error : formatter.get_error_collector().get_errors()) {
      auto diag = std::make_shared<Diagnostic>(
          DiagnosticLevel::Error, error.code, error.location, error.args);
      diag->set_source_line(
          source_tracker.get_source_line(error.location.line));
      diagnostics.report(diag);
    }
  }

  if (diagnostics.has_errors()) {
    print_error_stage("Errors found during formatting:");
    diagnostics.print_all(true);
    return false;
  }

  // --- 10. 输出结果 ---
  std::string output_path;
  if (in_place) {
    output_path = input_path;
  } else {
    output_path = input_path + ".formatted";
  }

  std::ofstream output_file(output_path, std::ios::binary);
  if (!output_file.is_open()) {
    print_error("Cannot create output file '" + output_path + "'");
    return false;
  }

  output_file << formatted_code;
  output_file.close();

  if (in_place) {
    print_success("Successfully formatted in-place");
  } else {
    print_success("Successfully formatted");
    std::cout << "Output saved to: " << output_path << std::endl;
  }

  return true;
}

/**
 * @brief 转义字符串中的特殊字符，以便安全地输出为可读的文本格式。
 * @details
 *   此函数处理常见的不可见字符（如换行、制表符）和可能与输出格式
 *   冲突的字符（如双引号、反斜杠），将它们转换为 C 风格的转义序列。
 *   对于其他不可打印的控制字符，则使用十六进制表示法（`\xHH`），
 *   以确保输出文件的格式正确且内容无歧义。
 * @param[in] s 待转义的原始字符串。
 * @return 包含转义序列的新字符串。
 */
static std::string escape_for_output(const std::string& s) {
  std::ostringstream oss;
  for (unsigned char c : s) {
    switch (c) {
    case '\n':
      oss << "\\n";
      break;
    case '\t':
      oss << "\\t";
      break;
    case '\r':
      oss << "\\r";
      break;
    case '\0':
      oss << "\\0";
      break;
    case '\\':
      oss << "\\\\";
      break;
    case '"':
      oss << "\\\"";
      break;
    default:
      // 对于不可打印的控制字符，使用十六进制表示。
      if (c < 0x20) {
        oss << "\\x" << std::hex << std::setw(2) << std::setfill('0') << (int)c
            << std::dec;
      } else {
        oss << c;
      }
      break;
    }
  }
  return oss.str();
}

/**
 * @brief 对单个文件执行完整的词法分析前端流水线。
 * @details
 *   此函数封装了从读取文件到生成最终 Token 列表的完整流程，包括：
 *   1.  **文件读取与验证**: 确保文件存在且可读。
 *   2.  **词法分析**: 调用 `Lexer` 将源码转换为原始 Token 序列。
 *   3.  **错误处理**: 收集并报告词法分析阶段的错误。
 *   4.  **Token 预处理**: 调用 `TokenPreprocessor`
 * 对科学计数法等进行类型推断和转换。
 *   5.  **错误处理**: 收集并报告预处理阶段的错误。
 *   6.  **结果输出**: 将处理完成的 Token 序列写入 `.tokens` 文件。
 *   任何阶段的失败都会导致整个流程中止并返回 `false`。
 *
 * @param[in] input_path 输入文件的路径（第一个参数是文件路径）。
 * @param[in] locale     用于诊断消息的语言环境代码（第二个参数是语言环境）。
 *
 * @warning 参数顺序很重要：先文件路径，后语言环境。不要混淆这两个参数。
 *
 * @return 如果所有阶段都成功，返回 `true`，否则返回 `false`。
 */
bool tokenize_file(const std::string& input_path, const std::string& locale) {
  // --- 1. 文件校验和读取 ---
  if (input_path.empty()) {
    print_error("Input file path is empty");
    return false;
  }
  if (!std::filesystem::exists(input_path)) {
    print_error("File '" + input_path + "' does not exist");
    return false;
  }
  if (!std::filesystem::is_regular_file(input_path)) {
    print_error("'" + input_path + "' is not a regular file");
    return false;
  }

  std::ifstream input_file(input_path);
  if (!input_file.is_open()) {
    print_error("Cannot open file '" + input_path + "'");
    return false;
  }

  std::stringstream buffer;
  buffer << input_file.rdbuf();
  std::string content = buffer.str();
  input_file.close();

  std::cout << "Tokenizing file: " << input_path << std::endl;

  DiagnosticEngine diagnostics(locale);
  SourceTracker source_tracker(content, input_path);

  // --- 2. 词法分析 ---
  Lexer lexer(content, input_path);
  auto tokens = lexer.tokenize();

  // --- 3. 报告词法分析错误 ---
  if (lexer.get_errors().has_errors()) {
    // NOTE:
    // 词法分析器本身只收集错误信息（`LexerError`），但不知道如何显示它们。
    //       这里的代码负责将每个 `LexerError` 转换为一个通用的 `Diagnostic`
    //       对象， 并使用 `SourceTracker` 提取错误所在行的源码上下文，
    //       最后将这个完整的诊断对象交给 `DiagnosticEngine`
    //       进行统一处理和报告。
    //       这种分层设计使得错误收集和错误报告的逻辑相互分离。
    for (const auto& error : lexer.get_errors().get_errors()) {
      auto diag = std::make_shared<Diagnostic>(
          DiagnosticLevel::Error, error.code, error.location, error.args);
      diag->set_source_line(
          source_tracker.get_source_line(error.location.line));
      diagnostics.report(diag);
    }
  }

  if (diagnostics.has_errors()) {
    print_error_stage("Errors found during lexical analysis:");
    diagnostics.print_all(true);
    return false;
  }

  // --- 4. Token 预处理 ---
  TokenPreprocessor preprocessor;
  auto processed_tokens = preprocessor.process(tokens, input_path, content);

  // --- 5. 报告 Token 预处理错误 ---
  if (preprocessor.get_errors().has_errors()) {
    for (const auto& error : preprocessor.get_errors().get_errors()) {
      auto diag = std::make_shared<Diagnostic>(
          DiagnosticLevel::Error, error.code, error.location, error.args);
      diag->set_source_line(
          source_tracker.get_source_line(error.location.line));
      diagnostics.report(diag);
    }
  }

  if (diagnostics.has_errors()) {
    print_error_stage("Errors found during token preprocessing:");
    diagnostics.print_all(true);
    return false;
  }

  // --- 6. 将结果写入输出文件 ---
  std::string output_path = input_path + ".tokens";
  std::ofstream output_file(output_path, std::ios::binary);
  if (!output_file.is_open()) {
    print_error("Cannot create output file '" + output_path + "'");
    return false;
  }

  output_file << "# Tokenization Result\n";
  output_file << "# Source: " << input_path << "\n";
  output_file << "# Total tokens: " << processed_tokens.size() << "\n";
  output_file << "# Format: Index\tLine:Column\tType\tValue\n\n";

  for (size_t i = 0; i < processed_tokens.size(); i++) {
    output_file << i << "\t" << processed_tokens[i].line << ":"
                << processed_tokens[i].column << "\t"
                << token_type_to_string(processed_tokens[i].token_type) << "\t"
                << "\"" << escape_for_output(processed_tokens[i].value)
                << "\"\n";
  }

  output_file.close();

  print_success("Successfully tokenized " +
                std::to_string(processed_tokens.size()) + " tokens");
  std::cout << "Output saved to: " << output_path << std::endl;

  return true;
}

/**
 * @brief 对单个文件执行完整的解析流程并报告错误。
 * @details
 *   此函数执行完整的编译前端流程：
 *   1. **文件读取与验证**: 确保文件存在且可读。
 *   2. **词法分析**: 将源码转换为 Token 序列。
 *   3. **Token 预处理**: 处理科学计数法等特殊 Token。
 *   4. **语法分析**: 构建 CST 并检查语法错误。
 *   5. **错误报告**: 统一报告所有阶段的错误。
 *
 *   与 tokenize_file 不同，此函数不生成任何输出文件，
 *   只进行解析并报告发现的所有错误。
 *
 * @param[in] input_path 输入文件的路径。
 * @param[in] locale     用于诊断消息的语言环境代码。
 * @return 如果没有错误返回 `true`，否则返回 `false`。
 */
bool parse_file(const std::string& input_path, const std::string& locale) {
  // --- 1. 文件校验和读取 ---
  if (input_path.empty()) {
    print_error("Input file path is empty");
    return false;
  }
  if (!std::filesystem::exists(input_path)) {
    print_error("File '" + input_path + "' does not exist");
    return false;
  }
  if (!std::filesystem::is_regular_file(input_path)) {
    print_error("'" + input_path + "' is not a regular file");
    return false;
  }

  std::ifstream input_file(input_path);
  if (!input_file.is_open()) {
    print_error("Cannot open file '" + input_path + "'");
    return false;
  }

  std::stringstream buffer;
  buffer << input_file.rdbuf();
  std::string content = buffer.str();
  input_file.close();

  std::cout << "Parsing file: " << input_path << std::endl;

  DiagnosticEngine diagnostics(locale);
  SourceTracker source_tracker(content, input_path);

  // --- 2. 词法分析 ---
  Lexer lexer(content, input_path);
  auto tokens = lexer.tokenize();

  // --- 3. 报告词法分析错误 ---
  if (lexer.get_errors().has_errors()) {
    for (const auto& error : lexer.get_errors().get_errors()) {
      auto diag = std::make_shared<Diagnostic>(
          DiagnosticLevel::Error, error.code, error.location, error.args);
      diag->set_source_line(
          source_tracker.get_source_line(error.location.line));
      diagnostics.report(diag);
    }
  }

  if (diagnostics.has_errors()) {
    print_error_stage("Errors found during lexical analysis:");
    diagnostics.print_all(true);
    return false;
  }

  // --- 4. Token 预处理 ---
  TokenPreprocessor preprocessor;
  auto processed_tokens = preprocessor.process(tokens, input_path, content);

  // --- 5. 报告 Token 预处理错误 ---
  if (preprocessor.get_errors().has_errors()) {
    for (const auto& error : preprocessor.get_errors().get_errors()) {
      auto diag = std::make_shared<Diagnostic>(
          DiagnosticLevel::Error, error.code, error.location, error.args);
      diag->set_source_line(
          source_tracker.get_source_line(error.location.line));
      diagnostics.report(diag);
    }
  }

  if (diagnostics.has_errors()) {
    print_error_stage("Errors found during token preprocessing:");
    diagnostics.print_all(true);
    return false;
  }

  // --- 6. 语法分析 ---
  Parser parser(processed_tokens);
  auto cst = parser.parse();

  // --- 7. 报告语法分析错误 ---
  if (parser.has_errors()) {
    for (const auto& error : parser.get_errors()) {
      auto diag = std::make_shared<Diagnostic>(
          DiagnosticLevel::Error, error.code, error.location, error.args);
      diag->set_source_line(
          source_tracker.get_source_line(error.location.line));
      diagnostics.report(diag);
    }
  }

  if (diagnostics.has_errors()) {
    print_error_stage("Errors found during parsing:");
    diagnostics.print_all(true);
    return false;
  }

  // --- 8. 成功 ---
  print_success("Successfully parsed with no errors");
  return true;
}

/**
 * @brief 程序主入口。
 * @param[in] argc 命令行参数数量。
 * @param[in] argv 命令行参数数组。
 * @return 程序退出码 (0 表示成功, 1 表示失败)。
 */
int main(int argc, char* argv[]) {
  // 将 argv 转换为 std::vector<std::string> 以避免指针算术
  std::vector<std::string> args(argv, argv + argc);

  if (args.size() < 2) {
    print_usage(args[0]);
    return 1;
  }

  // --- 解析全局选项 (如 --locale, --help, --version) ---
  std::string locale = "en_US"; // Default locale
  size_t arg_offset = 1;

  // NOTE: 这是一个简单的手动命令行参数解析循环。它首先处理所有以 `-`
  //       开头的选项参数（如 `--locale`），然后再处理命令和文件参数。
  //       这种方式适用于简单的命令行工具，对于更复杂的场景，可以考虑
  //       使用专门的参数解析库（如 `cxxopts` 或 `Boost.Program_options`）。
  while (arg_offset < args.size() && !args[arg_offset].empty() &&
         args[arg_offset][0] == '-') {
    const std::string& option = args[arg_offset];

    if (option == "--help" || option == "-h") {
      print_usage(args[0]);
      return 0;
    } else if (option == "--version" || option == "-v") {
      std::cout << "CZC Compiler version " << VERSION << std::endl;
      return 0;
    } else if (option == "--locale") {
      if (arg_offset + 1 >= args.size()) {
        print_error("--locale requires an argument");
        print_usage(args[0]);
        return 1;
      }
      locale = args[arg_offset + 1];
      arg_offset += 2;
    } else if (option == "--in-place" || option == "-i") {
      // --in-place is a fmt-specific option, will be parsed in fmt command
      print_error(
          "Option '--in-place' must be used with the 'fmt' command: czc-cli "
          "fmt --in-place <file>");
      return 1;
    } else if (option == "--indent-width") {
      // --indent-width is a fmt-specific option
      print_error(
          "Option '--indent-width' must be used with the 'fmt' command: "
          "czc-cli fmt --indent-width <n> <file>");
      return 1;
    } else if (option == "--use-tabs") {
      // --use-tabs is a fmt-specific option
      print_error(
          "Option '--use-tabs' must be used with the 'fmt' command: czc-cli "
          "fmt --use-tabs <file>");
      return 1;
    } else {
      print_error("Unknown option '" + option + "'");
      print_usage(args[0]);
      return 1;
    }
  }

  // --- 解析命令和文件参数 ---
  if (arg_offset >= args.size()) {
    print_error("Missing command");
    print_usage(args[0]);
    return 1;
  }

  const std::string& command = args[arg_offset];
  if (command == "tokenize") {
    if (arg_offset + 1 >= args.size()) {
      print_error("Missing input file argument");
      print_usage(args[0]);
      return 1;
    }

    // 收集所有文件模式参数。
    std::vector<std::string> patterns;
    for (size_t i = arg_offset + 1; i < args.size(); i++) {
      patterns.push_back(args[i]);
    }

    // 使用 FileCollector 将通配符模式扩展为具体的文件列表。
    auto files_to_process = FileCollector::collect_files(patterns);
    if (files_to_process.empty()) {
      print_error("No files found to process");
      return 1;
    }

    // --- 批量处理文件 ---
    size_t total_files = files_to_process.size();
    size_t success_count = 0;
    size_t failed_count = 0;

    for (size_t i = 0; i < files_to_process.size(); i++) {
      if (total_files > 1) {
        std::cout << "[" << (i + 1) << "/" << total_files << "] ";
      }
      if (tokenize_file(files_to_process[i], locale)) {
        success_count++;
      } else {
        failed_count++;
      }
      if (i < files_to_process.size() - 1) {
        std::cout << std::endl;
      }
    }

    // --- 打印总结信息 ---
    if (total_files > 1) {
      std::cout << "\n========================================" << std::endl;
      std::cout << "Summary: " << success_count << " succeeded, "
                << failed_count << " failed" << std::endl;
      std::cout << "========================================" << std::endl;
    }

    return (failed_count == 0) ? 0 : 1;
  } else if (command == "parse") {
    if (arg_offset + 1 >= args.size()) {
      print_error("Missing input file argument");
      print_usage(args[0]);
      return 1;
    }

    // 收集所有文件模式参数。
    std::vector<std::string> patterns;
    for (size_t i = arg_offset + 1; i < args.size(); i++) {
      patterns.push_back(args[i]);
    }

    // 使用 FileCollector 将通配符模式扩展为具体的文件列表。
    auto files_to_process = FileCollector::collect_files(patterns);
    if (files_to_process.empty()) {
      print_error("No files found to process");
      return 1;
    }

    // --- 批量处理文件 ---
    size_t total_files = files_to_process.size();
    size_t success_count = 0;
    size_t failed_count = 0;

    for (size_t i = 0; i < files_to_process.size(); i++) {
      if (total_files > 1) {
        std::cout << "[" << (i + 1) << "/" << total_files << "] ";
      }
      if (parse_file(files_to_process[i], locale)) {
        success_count++;
      } else {
        failed_count++;
      }
      if (i < files_to_process.size() - 1) {
        std::cout << std::endl;
      }
    }

    // --- 打印总结信息 ---
    if (total_files > 1) {
      std::cout << "\n========================================" << std::endl;
      std::cout << "Summary: " << success_count << " succeeded, "
                << failed_count << " failed" << std::endl;
      std::cout << "========================================" << std::endl;
    }

    return (failed_count == 0) ? 0 : 1;
  } else if (command == "fmt") {
    if (arg_offset + 1 >= args.size()) {
      print_error("Missing input file argument");
      print_usage(args[0]);
      return 1;
    }

    // 解析 fmt 命令的选项
    size_t fmt_arg_offset = arg_offset + 1;
    bool fmt_in_place = false;
    size_t fmt_indent_width = 4;
    bool fmt_use_tabs = false;

    // 收集所有文件模式参数（跳过格式选项）。
    std::vector<std::string> patterns;
    for (size_t i = fmt_arg_offset; i < args.size(); i++) {
      // 处理选项
      if (args[i] == "--in-place" || args[i] == "-i") {
        fmt_in_place = true;
        continue;
      } else if (args[i] == "--indent-width") {
        if (i + 1 >= args.size()) {
          print_error("--indent-width requires an argument");
          return 1;
        }
        try {
          fmt_indent_width = std::stoul(args[i + 1]);
          if (fmt_indent_width == 0 || fmt_indent_width > 16) {
            print_error("Indent width must be between 1 and 16");
            return 1;
          }
        } catch (...) {
          print_error("Invalid indent width: " + args[i + 1]);
          return 1;
        }
        i++; // 跳过值
        continue;
      } else if (args[i] == "--use-tabs") {
        fmt_use_tabs = true;
        continue;
      }
      patterns.push_back(args[i]);
    }

    if (patterns.empty()) {
      print_error("Missing input file argument");
      print_usage(args[0]);
      return 1;
    }

    // 使用 FileCollector 将通配符模式扩展为具体的文件列表。
    auto files_to_process = FileCollector::collect_files(patterns);
    if (files_to_process.empty()) {
      print_error("No files found to process");
      return 1;
    }

    // 创建格式化选项
    FormatOptions format_options;
    format_options.indent_width = fmt_indent_width;
    format_options.indent_style =
        fmt_use_tabs ? IndentStyle::TABS : IndentStyle::SPACES;

    // --- 批量处理文件 ---
    size_t total_files = files_to_process.size();
    size_t success_count = 0;
    size_t failed_count = 0;

    for (size_t i = 0; i < files_to_process.size(); i++) {
      if (total_files > 1) {
        std::cout << "[" << (i + 1) << "/" << total_files << "] ";
      }
      if (format_file(files_to_process[i], locale, format_options,
                      fmt_in_place)) {
        success_count++;
      } else {
        failed_count++;
      }
      if (i < files_to_process.size() - 1) {
        std::cout << std::endl;
      }
    }

    // --- 打印总结信息 ---
    if (total_files > 1) {
      std::cout << "\n========================================" << std::endl;
      std::cout << "Summary: " << success_count << " succeeded, "
                << failed_count << " failed" << std::endl;
      std::cout << "========================================" << std::endl;
    }

    return (failed_count == 0) ? 0 : 1;
  }

  print_error("Unknown command '" + command + "'");
  print_usage(args[0]);
  return 1;
}