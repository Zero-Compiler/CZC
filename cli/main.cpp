/**
 * @file main.cpp
 * @brief CZC 编译器命令行工具入口。
 * @author BegoniaHe
 * @date 2025-11-04
 */

#include "czc/diagnostics/diagnostic.hpp"
#include "czc/lexer/lexer.hpp"
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
using namespace czc::token_preprocessor;
using namespace czc::utils;

/**
 * @brief 打印命令行工具的使用说明。
 * @param[in] program_name 程序的名称 (通常是 argv[0])。
 */
void print_usage(const char *program_name) {
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
  std::cout << "\nExamples:" << std::endl;
  std::cout << "  " << program_name << " tokenize example.zero" << std::endl;
  std::cout << "  " << program_name << " --locale zh_CN tokenize example.zero"
            << std::endl;
  std::cout << "  " << program_name << " tokenize file1.zero file2.zero"
            << std::endl;
  std::cout << "  " << program_name << " tokenize test_*.zero" << std::endl;
}

/**
 * @brief 转义字符串中的特殊字符，以便安全地输出到文本文件。
 * @param[in] s 待转义的字符串。
 * @return 转义后的字符串。
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
 * @brief 对单个文件执行完整的词法分析和预处理流程。
 * @param[in] input_path 输入文件的路径。
 * @param[in] locale     用于诊断消息的语言环境代码。
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
 * @brief 程序主入口。
 * @param[in] argc 命令行参数数量。
 * @param[in] argv 命令行参数数组。
 * @return 程序退出码 (0 表示成功, 1 表示失败)。
 */
int main(int argc, char *argv[]) {
  if (argc < 2) {
    print_usage(argv[0]);
    return 1;
  }

  // --- 解析全局选项 (如 --locale) ---
  std::string locale = "en_US"; // Default locale
  int arg_offset = 1;
  while (arg_offset < argc && argv[arg_offset][0] == '-') {
    std::string option = argv[arg_offset];
    if (option == "--locale") {
      if (arg_offset + 1 >= argc) {
        std::cerr << "Error: --locale requires an argument" << std::endl;
        print_usage(argv[0]);
        return 1;
      }
      locale = argv[arg_offset + 1];
      arg_offset += 2;
    } else {
      std::cerr << "Error: Unknown option '" << option << "'" << std::endl;
      print_usage(argv[0]);
      return 1;
    }
  }

  // --- 解析命令和文件参数 ---
  if (arg_offset >= argc) {
    std::cerr << "Error: Missing command" << std::endl;
    print_usage(argv[0]);
    return 1;
  }

  std::string command = argv[arg_offset];
  if (command == "tokenize") {
    if (arg_offset + 1 >= argc) {
      std::cerr << "Error: Missing input file argument" << std::endl;
      print_usage(argv[0]);
      return 1;
    }

    // 收集所有文件模式参数。
    std::vector<std::string> patterns;
    for (int i = arg_offset + 1; i < argc; i++) {
      patterns.push_back(argv[i]);
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
  }

  std::cerr << "Error: Unknown command '" << command << "'" << std::endl;
  print_usage(argv[0]);
  return 1;
}