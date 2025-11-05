/**
 * @file main.cpp
 * @brief CZC 编译器命令行工具入口。
 * @author BegoniaHe
 * @date 2025-11-05
 */

#include "czc/diagnostics/diagnostic.hpp"
#include "czc/lexer/lexer.hpp"
#include "czc/parser/parser.hpp"
#include "czc/token_preprocessor/token_preprocessor.hpp"
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
using namespace czc::lexer;
using namespace czc::parser;
using namespace czc::token_preprocessor;
using namespace czc::utils;

/**
 * @brief 打印命令行工具的使用说明。
 * @param[in] program_name 程序的名称。
 */
void print_usage(const std::string &program_name) {
  std::cout << "Usage: " << program_name << " [options] <command> <file>..."
            << std::endl;
  std::cout << "\nOptions:" << std::endl;
  std::cout << "  --locale <locale>         Set the locale for diagnostic "
               "messages (default: en_US)"
            << std::endl;
  std::cout << "                            Available: en_US, zh_CN, ne_KO"
            << std::endl;
  std::cout << "\nCommands:" << std::endl;
  std::cout << "  tokenize <input_file>...  Tokenize one or more input files"
            << std::endl;
  std::cout << "                            Output will be saved as "
               "<input_file>.tokens"
            << std::endl;
  std::cout
      << "                            Supports multiple files and wildcards"
      << std::endl;
  std::cout << "  parse <input_file>...     Parse one or more input files and "
               "report errors"
            << std::endl;
  std::cout << "                            No output files are generated"
            << std::endl;
  std::cout << "\nExamples:" << std::endl;
  std::cout << "  " << program_name << " tokenize example.zero" << std::endl;
  std::cout << "  " << program_name << " parse example.zero" << std::endl;
  std::cout << "  " << program_name << " --locale zh_CN tokenize example.zero"
            << std::endl;
  std::cout << "  " << program_name << " tokenize file1.zero file2.zero"
            << std::endl;
  std::cout << "  " << program_name << " tokenize test_*.zero" << std::endl;
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
static std::string escape_for_output(const std::string &s) {
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
bool tokenize_file(const std::string &input_path, const std::string &locale) {
  // --- 1. 文件校验和读取 ---
  if (input_path.empty()) {
    std::cerr << "Error: Input file path is empty" << std::endl;
    return false;
  }
  if (!std::filesystem::exists(input_path)) {
    std::cerr << "Error: File '" << input_path << "' does not exist"
              << std::endl;
    return false;
  }
  if (!std::filesystem::is_regular_file(input_path)) {
    std::cerr << "Error: '" << input_path << "' is not a regular file"
              << std::endl;
    return false;
  }

  std::ifstream input_file(input_path);
  if (!input_file.is_open()) {
    std::cerr << "Error: Cannot open file '" << input_path << "'" << std::endl;
    return false;
  }

  std::stringstream buffer;
  buffer << input_file.rdbuf();
  std::string content = buffer.str();
  input_file.close();

  std::cout << "Tokenizing file: " << input_path << std::endl;

  DiagnosticEngine diagnostics(locale);

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
    for (const auto &error : lexer.get_errors().get_errors()) {
      auto diag = std::make_shared<Diagnostic>(
          DiagnosticLevel::Error, error.code, error.location, error.args);
      SourceTracker temp_tracker(content, input_path);
      diag->set_source_line(temp_tracker.get_source_line(error.location.line));
      diagnostics.report(diag);
    }
  }

  if (diagnostics.has_errors()) {
    std::cerr << "\nErrors found during lexical analysis:\n" << std::endl;
    diagnostics.print_all(true);
    return false;
  }

  // --- 4. Token 预处理 ---
  TokenPreprocessor preprocessor;
  auto processed_tokens = preprocessor.process(tokens, input_path, content);

  // --- 5. 报告 Token 预处理错误 ---
  if (preprocessor.get_errors().has_errors()) {
    for (const auto &error : preprocessor.get_errors().get_errors()) {
      auto diag = std::make_shared<Diagnostic>(
          DiagnosticLevel::Error, error.code, error.location, error.args);
      SourceTracker temp_tracker(content, input_path);
      diag->set_source_line(temp_tracker.get_source_line(error.location.line));
      diagnostics.report(diag);
    }
  }

  if (diagnostics.has_errors()) {
    std::cerr << "\nErrors found during token preprocessing:\n" << std::endl;
    diagnostics.print_all(true);
    return false;
  }

  // --- 6. 将结果写入输出文件 ---
  std::string output_path = input_path + ".tokens";
  std::ofstream output_file(output_path, std::ios::binary);
  if (!output_file.is_open()) {
    std::cerr << "Error: Cannot create output file '" << output_path << "'"
              << std::endl;
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

  std::cout << "Successfully tokenized " << processed_tokens.size() << " tokens"
            << std::endl;
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
bool parse_file(const std::string &input_path, const std::string &locale) {
  // --- 1. 文件校验和读取 ---
  if (input_path.empty()) {
    std::cerr << "Error: Input file path is empty" << std::endl;
    return false;
  }
  if (!std::filesystem::exists(input_path)) {
    std::cerr << "Error: File '" << input_path << "' does not exist"
              << std::endl;
    return false;
  }
  if (!std::filesystem::is_regular_file(input_path)) {
    std::cerr << "Error: '" << input_path << "' is not a regular file"
              << std::endl;
    return false;
  }

  std::ifstream input_file(input_path);
  if (!input_file.is_open()) {
    std::cerr << "Error: Cannot open file '" << input_path << "'" << std::endl;
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
    for (const auto &error : lexer.get_errors().get_errors()) {
      auto diag = std::make_shared<Diagnostic>(
          DiagnosticLevel::Error, error.code, error.location, error.args);
      diag->set_source_line(
          source_tracker.get_source_line(error.location.line));
      diagnostics.report(diag);
    }
  }

  if (diagnostics.has_errors()) {
    std::cerr << "\nErrors found during lexical analysis:\n" << std::endl;
    diagnostics.print_all(true);
    return false;
  }

  // --- 4. Token 预处理 ---
  TokenPreprocessor preprocessor;
  auto processed_tokens = preprocessor.process(tokens, input_path, content);

  // --- 5. 报告 Token 预处理错误 ---
  if (preprocessor.get_errors().has_errors()) {
    for (const auto &error : preprocessor.get_errors().get_errors()) {
      auto diag = std::make_shared<Diagnostic>(
          DiagnosticLevel::Error, error.code, error.location, error.args);
      diag->set_source_line(
          source_tracker.get_source_line(error.location.line));
      diagnostics.report(diag);
    }
  }

  if (diagnostics.has_errors()) {
    std::cerr << "\nErrors found during token preprocessing:\n" << std::endl;
    diagnostics.print_all(true);
    return false;
  }

  // --- 6. 语法分析 ---
  Parser parser(processed_tokens);
  auto cst = parser.parse();

  // --- 7. 报告语法分析错误 ---
  if (parser.has_errors()) {
    for (const auto &error : parser.get_errors()) {
      auto diag = std::make_shared<Diagnostic>(
          DiagnosticLevel::Error, error.code, error.location, error.args);
      diag->set_source_line(
          source_tracker.get_source_line(error.location.line));
      diagnostics.report(diag);
    }
  }

  if (diagnostics.has_errors()) {
    std::cerr << "\nErrors found during parsing:\n" << std::endl;
    diagnostics.print_all(true);
    return false;
  }

  // --- 8. 成功 ---
  std::cout << "Successfully parsed with no errors" << std::endl;
  return true;
}

/**
 * @brief 程序主入口。
 * @param[in] argc 命令行参数数量。
 * @param[in] argv 命令行参数数组。
 * @return 程序退出码 (0 表示成功, 1 表示失败)。
 */
int main(int argc, char *argv[]) {
  // 将 argv 转换为 std::vector<std::string> 以避免指针算术
  std::vector<std::string> args(argv, argv + argc);

  if (args.size() < 2) {
    print_usage(args[0]);
    return 1;
  }

  // --- 解析全局选项 (如 --locale) ---
  std::string locale = "en_US"; // Default locale
  size_t arg_offset = 1;
  // NOTE: 这是一个简单的手动命令行参数解析循环。它首先处理所有以 `-`
  //       开头的选项参数（如 `--locale`），然后再处理命令和文件参数。
  //       这种方式适用于简单的命令行工具，对于更复杂的场景，可以考虑
  //       使用专门的参数解析库（如 `cxxopts` 或 `Boost.Program_options`）。
  while (arg_offset < args.size() && !args[arg_offset].empty() &&
         args[arg_offset][0] == '-') {
    const std::string &option = args[arg_offset];
    if (option == "--locale") {
      if (arg_offset + 1 >= args.size()) {
        std::cerr << "Error: --locale requires an argument" << std::endl;
        print_usage(args[0]);
        return 1;
      }
      locale = args[arg_offset + 1];
      arg_offset += 2;
    } else {
      std::cerr << "Error: Unknown option '" << option << "'" << std::endl;
      print_usage(args[0]);
      return 1;
    }
  }

  // --- 解析命令和文件参数 ---
  if (arg_offset >= args.size()) {
    std::cerr << "Error: Missing command" << std::endl;
    print_usage(args[0]);
    return 1;
  }

  const std::string &command = args[arg_offset];
  if (command == "tokenize") {
    if (arg_offset + 1 >= args.size()) {
      std::cerr << "Error: Missing input file argument" << std::endl;
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
      std::cerr << "Error: No files found to process" << std::endl;
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
      std::cerr << "Error: Missing input file argument" << std::endl;
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
      std::cerr << "Error: No files found to process" << std::endl;
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
  }

  std::cerr << "Error: Unknown command '" << command << "'" << std::endl;
  print_usage(argv[0]);
  return 1;
}