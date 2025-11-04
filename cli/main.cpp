/**
 * @file main.cpp
 * @brief CZC 编译器命令行工具入口
 * @author BegoniaHe
 */

#include "czc/lexer/lexer.hpp"
#include "czc/lexer/source_tracker.hpp"
#include "czc/token_preprocessor/token_preprocessor.hpp"
#include "czc/diagnostics/diagnostic.hpp"
#include "czc/utils/file_collector.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>
#include <iomanip>

/**
 * @brief 打印使用说明
 * @param program_name 程序名称
 */
void print_usage(const char *program_name)
{
    std::cout << "Usage: " << program_name << " [options] <command> <file>..." << std::endl;
    std::cout << "\nOptions:" << std::endl;
    std::cout << "  --locale <locale>         Set the locale for diagnostic messages (default: en_US)" << std::endl;
    std::cout << "                            Available: en_US, zh_CN, ne_KO" << std::endl;
    std::cout << "\nCommands:" << std::endl;
    std::cout << "  tokenize <input_file>...  Tokenize one or more input files" << std::endl;
    std::cout << "                            Output will be saved as <input_file>.tokens" << std::endl;
    std::cout << "                            Supports multiple files and wildcards" << std::endl;
    std::cout << "\nExamples:" << std::endl;
    std::cout << "  " << program_name << " tokenize example.zero" << std::endl;
    std::cout << "  " << program_name << " --locale zh_CN tokenize example.zero" << std::endl;
    std::cout << "  " << program_name << " tokenize file1.zero file2.zero" << std::endl;
    std::cout << "  " << program_name << " tokenize test_*.zero" << std::endl;
}

/**
 * @brief 转义字符串用于输出
 * @param s 待转义的字符串
 * @return 转义后的字符串
 */
static std::string escape_for_output(const std::string &s)
{
    std::ostringstream oss;
    for (unsigned char c : s)
    {
        switch (c)
        {
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
            if (c < 0x20)
            {
                oss << "\\x" << std::hex << std::setw(2) << std::setfill('0') << (int)c << std::dec;
            }
            else
            {
                oss << c;
            }
            break;
        }
    }
    return oss.str();
}

/**
 * @brief 对单个文件进行词法分析
 * @param input_path 输入文件路径
 * @param locale 语言环境代码
 * @return 成功返回 true，失败返回 false
 */
bool tokenize_file(const std::string &input_path, const std::string &locale)
{
    if (input_path.empty())
    {
        std::cerr << "Error: Input file path is empty" << std::endl;
        return false;
    }

    if (!std::filesystem::exists(input_path))
    {
        std::cerr << "Error: File '" << input_path << "' does not exist" << std::endl;
        return false;
    }

    if (!std::filesystem::is_regular_file(input_path))
    {
        std::cerr << "Error: '" << input_path << "' is not a regular file" << std::endl;
        return false;
    }

    std::ifstream input_file(input_path);
    if (!input_file.is_open())
    {
        std::cerr << "Error: Cannot open file '" << input_path << "'" << std::endl;
        return false;
    }

    std::stringstream buffer;
    buffer << input_file.rdbuf();
    std::string content = buffer.str();
    input_file.close();

    std::cout << "Tokenizing file: " << input_path << std::endl;

    DiagnosticEngine diagnostics(locale);

    Lexer lexer(content, input_path);
    auto tokens = lexer.tokenize();

    if (lexer.get_errors().has_errors())
    {
        for (const auto &error : lexer.get_errors().get_errors())
        {
            auto diag = std::make_shared<Diagnostic>(
                DiagnosticLevel::Error,
                error.code,
                error.location,
                error.args);

            // 使用 SourceTracker 提取源码行
            SourceTracker temp_tracker(content, input_path);
            diag->set_source_line(temp_tracker.get_source_line(error.location.line));
            diagnostics.report(diag);
        }
    }

    if (diagnostics.has_errors())
    {
        std::cerr << "\nErrors found during lexical analysis:\n"
                  << std::endl;
        diagnostics.print_all(true);
        return false;
    }

    auto processed_tokens = TokenPreprocessor::process(tokens, input_path, content, &diagnostics);

    if (diagnostics.has_errors())
    {
        std::cerr << "\nErrors found during token preprocessing:\n"
                  << std::endl;
        diagnostics.print_all(true);
        return false;
    }

    std::filesystem::path input_fs_path(input_path);
    std::string output_path = input_path + ".tokens";

    std::ofstream output_file(output_path, std::ios::binary);
    if (!output_file.is_open())
    {
        std::cerr << "Error: Cannot create output file '" << output_path << "'" << std::endl;
        return false;
    }

    output_file << "# Tokenization Result" << std::endl;
    output_file << "# Source: " << input_path << std::endl;
    output_file << "# Total tokens: " << processed_tokens.size() << std::endl;
    output_file << "# Format: Index\tLine:Column\tType\tValue" << std::endl;
    output_file << std::endl;

    for (size_t i = 0; i < processed_tokens.size(); i++)
    {
        output_file << i << "\t"
                    << processed_tokens[i].line << ":" << processed_tokens[i].column << "\t"
                    << token_type_to_string(processed_tokens[i].token_type) << "\t"
                    << "\"" << escape_for_output(processed_tokens[i].value) << "\"" << std::endl;
    }

    output_file.close();

    std::cout << "Successfully tokenized " << processed_tokens.size() << " tokens" << std::endl;
    std::cout << "Output saved to: " << output_path << std::endl;

    return true;
}

/**
 * @brief 程序主入口
 * @param argc 命令行参数数量
 * @param argv 命令行参数数组
 * @return 程序退出码
 */
int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        print_usage(argv[0]);
        return 1;
    }

    // Parse global options
    std::string locale = "en_US"; // Default locale
    int arg_offset = 1;

    // Check for --locale option
    while (arg_offset < argc && argv[arg_offset][0] == '-')
    {
        std::string option = argv[arg_offset];

        if (option == "--locale")
        {
            if (arg_offset + 1 >= argc)
            {
                std::cerr << "Error: --locale requires an argument" << std::endl;
                print_usage(argv[0]);
                return 1;
            }
            locale = argv[arg_offset + 1];
            arg_offset += 2;
        }
        else
        {
            std::cerr << "Error: Unknown option '" << option << "'" << std::endl;
            print_usage(argv[0]);
            return 1;
        }
    }

    if (arg_offset >= argc)
    {
        std::cerr << "Error: Missing command" << std::endl;
        print_usage(argv[0]);
        return 1;
    }

    std::string command = argv[arg_offset];

    if (command == "tokenize")
    {
        if (arg_offset + 1 >= argc)
        {
            std::cerr << "Error: Missing input file argument" << std::endl;
            print_usage(argv[0]);
            return 1;
        }

        // 收集所有要处理的文件模式
        std::vector<std::string> patterns;
        for (int i = arg_offset + 1; i < argc; i++)
        {
            patterns.push_back(argv[i]);
        }

        // 使用 FileCollector 收集文件
        auto files_to_process = FileCollector::collect_files(patterns);

        if (files_to_process.empty())
        {
            std::cerr << "Error: No files found to process" << std::endl;
            return 1;
        }

        size_t total_files = files_to_process.size();
        size_t success_count = 0;
        size_t failed_count = 0;

        for (size_t i = 0; i < files_to_process.size(); i++)
        {
            if (total_files > 1)
            {
                std::cout << "[" << (i + 1) << "/" << total_files << "] ";
            }

            if (tokenize_file(files_to_process[i], locale))
            {
                success_count++;
            }
            else
            {
                failed_count++;
            }

            if (i < files_to_process.size() - 1)
            {
                std::cout << std::endl;
            }
        }

        if (total_files > 1)
        {
            std::cout << "\n========================================" << std::endl;
            std::cout << "Summary: " << success_count << " succeeded, "
                      << failed_count << " failed" << std::endl;
            std::cout << "========================================" << std::endl;
        }

        return (failed_count == 0) ? 0 : 1;
    }
    else
    {
        std::cerr << "Error: Unknown command '" << command << "'" << std::endl;
        print_usage(argv[0]);
        return 1;
    }

    return 0;
}