#ifndef LEXER_HPP
#define LEXER_HPP

#include "token.hpp"
#include <vector>
#include <optional>
#include <string>

class Lexer
{
private:
    std::vector<char> input;
    size_t position;
    size_t line;
    size_t column;
    std::optional<char> current_char;

    void advance();
    std::optional<char> peek(size_t offset) const;
    void skip_whitespace();
    void skip_comment();
    Token read_number();
    Token read_identifier();
    Token read_string();
    Token read_char();

public:
    Lexer(const std::string &input_str);
    Token next_token();
    std::vector<Token> tokenize();
};

#endif // LEXER_HPP
