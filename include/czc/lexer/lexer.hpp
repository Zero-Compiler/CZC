#ifndef CZC_LEXER_HPP
#define CZC_LEXER_HPP

#include "token.hpp"
#include "lexer_error.hpp"
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
    Token read_raw_string();

    bool is_utf8_continuation(unsigned char ch) const;
    size_t get_utf8_char_length(unsigned char first_byte) const;
    std::string parse_unicode_escape(size_t digit_count);
    std::string parse_hex_escape();

public:
    Lexer(const std::string &input_str);
    Token next_token();
    std::vector<Token> tokenize();
};

#endif // CZC_LEXER_HPP
