#include "czc/lexer/lexer.hpp"
#include "czc/lexer/lexer_error.hpp"
#include "czc/token_preprocessor/token_preprocessor.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>
#include <iomanip>

void print_usage(const char *program_name)
{
    std::cout << "Usage: " << program_name << " <command> <file>..." << std::endl;
    std::cout << "\nCommands:" << std::endl;
    std::cout << "  tokenize <input_file>...  Tokenize one or more input files" << std::endl;
    std::cout << "                            Output will be saved as <input_file>.tokens" << std::endl;
    std::cout << "                            Supports multiple files and wildcards" << std::endl;
    std::cout << "\nExamples:" << std::endl;
    std::cout << "  " << program_name << " tokenize example.zero" << std::endl;
    std::cout << "  " << program_name << " tokenize file1.zero file2.zero" << std::endl;
    std::cout << "  " << program_name << " tokenize test_*.zero" << std::endl;
}

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

bool tokenize_file(const std::string &input_path)
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

    // Tokenize
    std::cout << "Tokenizing file: " << input_path << std::endl;

    try
    {
        Lexer lexer(content);
        auto tokens = lexer.tokenize();
        auto processed_tokens = TokenPreprocessor::process(tokens);

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
    catch (const LexerError &e)
    {
        std::cerr << e.format_error() << std::endl;
        return false;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error during tokenization: " << e.what() << std::endl;
        return false;
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        print_usage(argv[0]);
        return 1;
    }

    std::string command = argv[1];

    if (command == "tokenize")
    {
        if (argc < 3)
        {
            std::cerr << "Error: Missing input file argument" << std::endl;
            print_usage(argv[0]);
            return 1;
        }

        // collect all files to process
        std::vector<std::string> files_to_process;

        for (int i = 2; i < argc; i++)
        {
            std::string arg = argv[i];

            // Check for wildcard characters
            if (arg.find('*') != std::string::npos || arg.find('?') != std::string::npos)
            {
                std::filesystem::path pattern_path(arg);
                std::filesystem::path parent_path = pattern_path.parent_path();
                std::string pattern = pattern_path.filename().string();

                if (parent_path.empty())
                {
                    parent_path = ".";
                }

                if (std::filesystem::exists(parent_path) && std::filesystem::is_directory(parent_path))
                {
                    for (const auto &entry : std::filesystem::directory_iterator(parent_path))
                    {
                        if (entry.is_regular_file())
                        {
                            std::string filename = entry.path().filename().string();

                            bool match = true;
                            size_t pattern_idx = 0;
                            size_t filename_idx = 0;

                            while (pattern_idx < pattern.length() && filename_idx < filename.length())
                            {
                                if (pattern[pattern_idx] == '*')
                                {
                                    // Handle * wildcard
                                    if (pattern_idx == pattern.length() - 1)
                                    {
                                        break;
                                    }
                                    pattern_idx++;
                                    while (filename_idx < filename.length() && filename[filename_idx] != pattern[pattern_idx])
                                    {
                                        filename_idx++;
                                    }
                                }
                                else if (pattern[pattern_idx] == '?')
                                {
                                    pattern_idx++;
                                    filename_idx++;
                                }
                                else if (pattern[pattern_idx] == filename[filename_idx])
                                {
                                    pattern_idx++;
                                    filename_idx++;
                                }
                                else
                                {
                                    match = false;
                                    break;
                                }
                            }

                            if (match && pattern_idx == pattern.length() && filename_idx == filename.length())
                            {
                                files_to_process.push_back(entry.path().string());
                            }
                        }
                    }
                }
            }
            else
            {
                files_to_process.push_back(arg);
            }
        }

        std::sort(files_to_process.begin(), files_to_process.end());

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

            if (tokenize_file(files_to_process[i]))
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