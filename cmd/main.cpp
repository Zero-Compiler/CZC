#include "../lexer/lexer.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <string>

void print_usage(const char *program_name)
{
    std::cout << "Usage: " << program_name << " <command> <filename>" << std::endl;
    std::cout << "\nCommands:" << std::endl;
    std::cout << "  tokenize <input_file>  Tokenize the input file and save results" << std::endl;
    std::cout << "                         Output will be saved as <input_file>.tokens" << std::endl;
    std::cout << "\nExample:" << std::endl;
    std::cout << "  " << program_name << " tokenize example.czc" << std::endl;
}

bool tokenize_file(const std::string &input_path)
{
    // Validate input path is not empty
    if (input_path.empty())
    {
        std::cerr << "Error: Input file path is empty" << std::endl;
        return false;
    }

    // Check if file exists
    if (!std::filesystem::exists(input_path))
    {
        std::cerr << "Error: File '" << input_path << "' does not exist" << std::endl;
        return false;
    }

    // Check if it's a regular file
    if (!std::filesystem::is_regular_file(input_path))
    {
        std::cerr << "Error: '" << input_path << "' is not a regular file" << std::endl;
        return false;
    }

    // Read input file
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

        // Generate output file path (same directory, add .tokens extension)
        std::filesystem::path input_fs_path(input_path);
        std::string output_path = input_path + ".tokens";

        // Write tokens to output file
        std::ofstream output_file(output_path);
        if (!output_file.is_open())
        {
            std::cerr << "Error: Cannot create output file '" << output_path << "'" << std::endl;
            return false;
        }

        output_file << "# Tokenization Result" << std::endl;
        output_file << "# Source: " << input_path << std::endl;
        output_file << "# Total tokens: " << tokens.size() << std::endl;
        output_file << "# Format: Index\tLine:Column\tType\tValue" << std::endl;
        output_file << std::endl;

        for (size_t i = 0; i < tokens.size(); i++)
        {
            output_file << i << "\t"
                        << tokens[i].line << ":" << tokens[i].column << "\t"
                        << token_type_to_string(tokens[i].token_type) << "\t"
                        << "\"" << tokens[i].value << "\"" << std::endl;
        }

        output_file.close();

        std::cout << "Successfully tokenized " << tokens.size() << " tokens" << std::endl;
        std::cout << "Output saved to: " << output_path << std::endl;

        return true;
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

        std::string input_file = argv[2];
        if (!tokenize_file(input_file))
        {
            return 1;
        }
    }
    else
    {
        std::cerr << "Error: Unknown command '" << command << "'" << std::endl;
        print_usage(argv[0]);
        return 1;
    }

    return 0;
}