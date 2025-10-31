#include "czc/lexer/lexer.hpp"
#include <cctype>
#include <sstream>
#include <iomanip>

bool Lexer::is_utf8_continuation(unsigned char ch) const
{
    return (ch & 0xC0) == 0x80;
}

size_t Lexer::get_utf8_char_length(unsigned char first_byte) const
{
    if ((first_byte & 0x80) == 0)
        return 1; // ASCII
    if ((first_byte & 0xE0) == 0xC0)
        return 2; // 2-byte UTF-8
    if ((first_byte & 0xF0) == 0xE0)
        return 3; // 3-byte UTF-8
    if ((first_byte & 0xF8) == 0xF0)
        return 4; // 4-byte UTF-8
    return 1;     // Invalid, treat as single byte
}

void Lexer::advance()
{
    if (current_char == '\n')
    {
        line++;
        column = 1;
        position++;
    }
    else if (current_char.has_value())
    {
        // 获取当前字符的 UTF-8 字节长度
        unsigned char byte = static_cast<unsigned char>(current_char.value());
        size_t char_length = get_utf8_char_length(byte);

        // 跳过整个 UTF-8 字符的所有字节
        for (size_t i = 0; i < char_length && position < input.size(); ++i)
        {
            position++;
        }

        // 一个完整的 UTF-8 字符算一列
        column++;
    }
    else
    {
        position++;
    }

    if (position < input.size())
    {
        current_char = input[position];
    }
    else
    {
        current_char = std::nullopt;
    }
}

std::optional<char> Lexer::peek(size_t offset) const
{
    size_t peek_pos = position + offset;
    if (peek_pos < input.size())
    {
        return input[peek_pos];
    }
    return std::nullopt;
}

void Lexer::skip_whitespace()
{
    while (current_char.has_value() && std::isspace(current_char.value()))
    {
        advance();
    }
}

void Lexer::skip_comment()
{
    if (current_char == '/' && peek(1) == '/')
    {
        while (current_char.has_value() && current_char != '\n')
        {
            advance();
        }
        if (current_char == '\n')
        {
            advance();
        }
    }
}

Token Lexer::read_number()
{
    size_t start = position;
    size_t token_line = line;
    size_t token_column = column;
    bool has_dot = false;

    // 0x, 0b, 0o prefixes
    if (current_char == '0' && peek(1).has_value())
    {
        char next_ch = peek(1).value();

        // Hexadecimal: 0x
        if (next_ch == 'x' || next_ch == 'X')
        {
            advance();
            advance();

            size_t hex_start = position;

            while (current_char.has_value())
            {
                char ch = current_char.value();
                if (std::isdigit(ch) || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F'))
                {
                    advance();
                }
                else
                {
                    break;
                }
            }

            // 检查是否至少有一个十六进制数字
            if (position == hex_start)
            {
                throw InvalidNumberFormatError("0x", token_line, token_column);
            }

            std::string value(input.begin() + start, input.begin() + position);
            return Token(TokenType::Integer, value, token_line, token_column);
        }

        // Binary: 0b
        else if (next_ch == 'b' || next_ch == 'B')
        {
            advance();
            advance();

            size_t bin_start = position;

            while (current_char.has_value())
            {
                char ch = current_char.value();
                if (ch == '0' || ch == '1')
                {
                    advance();
                }
                else
                {
                    break;
                }
            }

            // 检查是否至少有一个二进制数字
            if (position == bin_start)
            {
                throw InvalidNumberFormatError("0b", token_line, token_column);
            }

            std::string value(input.begin() + start, input.begin() + position);
            return Token(TokenType::Integer, value, token_line, token_column);
        }

        // Octal: 0o
        else if (next_ch == 'o' || next_ch == 'O')
        {
            advance();
            advance();

            size_t oct_start = position;

            while (current_char.has_value())
            {
                char ch = current_char.value();
                if (ch >= '0' && ch <= '7')
                {
                    advance();
                }
                else
                {
                    break;
                }
            }

            // 检查是否至少有一个八进制数字
            if (position == oct_start)
            {
                throw InvalidNumberFormatError("0o", token_line, token_column);
            }

            std::string value(input.begin() + start, input.begin() + position);
            return Token(TokenType::Integer, value, token_line, token_column);
        }
    }

    while (current_char.has_value())
    {
        char ch = current_char.value();
        if (std::isdigit(ch))
        {
            advance();
        }
        else if (ch == '.' && !has_dot)
        {
            auto next = peek(1);
            if (next.has_value() && std::isdigit(next.value()))
            {
                has_dot = true;
                advance();
            }
            else
            {
                break;
            }
        }
        else
        {
            break;
        }
    }

    // 检查科学计数法 (e 或 E)
    if (current_char.has_value() && (current_char.value() == 'e' || current_char.value() == 'E'))
    {
        has_dot = true; // 科学计数法属于浮点数
        advance();

        // 可选的正负号
        if (current_char.has_value() && (current_char.value() == '+' || current_char.value() == '-'))
        {
            advance();
        }

        // 指数部分必须至少有一个数字
        size_t exp_start = position;
        while (current_char.has_value() && std::isdigit(current_char.value()))
        {
            advance();
        }

        // 如果指数部分没有数字，则报错
        if (position == exp_start)
        {
            std::string value(input.begin() + start, input.begin() + position);
            throw InvalidNumberFormatError(value, token_line, token_column);
        }
    }

    // 检查数字后是否紧跟字母或下划线（无效）
    if (current_char.has_value() && (std::isalpha(current_char.value()) || current_char.value() == '_'))
    {
        std::string value(input.begin() + start, input.begin() + position);
        throw InvalidNumberFormatError(value + current_char.value(), token_line, token_column);
    }

    std::string value(input.begin() + start, input.begin() + position);

    if (has_dot)
    {
        return Token(TokenType::Float, value, token_line, token_column);
    }
    else
    {
        return Token(TokenType::Integer, value, token_line, token_column);
    }
}

Token Lexer::read_identifier()
{
    size_t start = position;
    size_t token_line = line;
    size_t token_column = column;
    while (current_char.has_value())
    {
        char ch = current_char.value();
        if (std::isalnum(ch) || ch == '_')
        {
            advance();
        }
        else
        {
            break;
        }
    }

    std::string value(input.begin() + start, input.begin() + position);
    auto keyword_type = get_keyword(value);
    TokenType token_type = keyword_type.value_or(TokenType::Identifier);

    return Token(token_type, value, token_line, token_column);
}

// 解析 Unicode 转义序列
std::string Lexer::parse_unicode_escape(size_t digit_count)
{
    std::string hex_digits;

    for (size_t i = 0; i < digit_count; ++i)
    {
        if (!current_char.has_value())
        {
            throw InvalidEscapeSequenceError('u', line, column);
        }

        char ch = current_char.value();
        if (!std::isxdigit(ch))
        {
            throw InvalidEscapeSequenceError('u', line, column);
        }

        hex_digits += ch;
        position++;
        if (position < input.size())
        {
            current_char = input[position];
        }
        else
        {
            current_char = std::nullopt;
        }
    }

    unsigned int codepoint;
    std::stringstream ss;
    ss << std::hex << hex_digits;
    ss >> codepoint;

    std::string utf8_result;
    if (codepoint <= 0x7F)
    {
        utf8_result += static_cast<char>(codepoint);
    }
    else if (codepoint <= 0x7FF)
    {
        utf8_result += static_cast<char>(0xC0 | (codepoint >> 6));
        utf8_result += static_cast<char>(0x80 | (codepoint & 0x3F));
    }
    else if (codepoint <= 0xFFFF)
    {
        utf8_result += static_cast<char>(0xE0 | (codepoint >> 12));
        utf8_result += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
        utf8_result += static_cast<char>(0x80 | (codepoint & 0x3F));
    }
    else if (codepoint <= 0x10FFFF)
    {
        utf8_result += static_cast<char>(0xF0 | (codepoint >> 18));
        utf8_result += static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F));
        utf8_result += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
        utf8_result += static_cast<char>(0x80 | (codepoint & 0x3F));
    }
    else
    {
        throw InvalidEscapeSequenceError('u', line, column);
    }

    return utf8_result;
}

// 解析十六进制转义序列 \xHH
std::string Lexer::parse_hex_escape()
{
    std::string hex_digits;

    // 读取最多 2 个十六进制数字
    for (size_t i = 0; i < 2; ++i)
    {
        if (!current_char.has_value())
        {
            break;
        }

        char ch = current_char.value();
        if (!std::isxdigit(ch))
        {
            break;
        }

        hex_digits += ch;
        position++;
        if (position < input.size())
        {
            current_char = input[position];
        }
        else
        {
            current_char = std::nullopt;
        }
    }

    if (hex_digits.empty())
    {
        throw InvalidEscapeSequenceError('x', line, column);
    }

    // 将十六进制转换为字节
    unsigned int byte_value;
    std::stringstream ss;
    ss << std::hex << hex_digits;
    ss >> byte_value;

    std::string result;
    result += static_cast<char>(byte_value);
    return result;
}

Token Lexer::read_string()
{
    size_t token_line = line;
    size_t token_column = column;
    advance();

    std::string value;
    bool terminated = false;

    while (current_char.has_value())
    {
        char ch = current_char.value();

        if (ch == '"')
        {
            terminated = true;
            break;
        }

        if (ch == '\n')
        {
            value += ch;
            advance();
            continue;
        }

        if (ch == '\\')
        {
            position++;
            if (position >= input.size())
            {
                current_char = std::nullopt;
                throw UnterminatedStringError(token_line, token_column);
            }
            current_char = input[position];

            if (!current_char.has_value())
            {
                throw UnterminatedStringError(token_line, token_column);
            }

            char escaped = current_char.value();
            switch (escaped)
            {
            case 'n':
                value += '\n';
                advance();
                continue;
            case 't':
                value += '\t';
                advance();
                continue;
            case 'r':
                value += '\r';
                advance();
                continue;
            case '\\':
                value += '\\';
                advance();
                continue;
            case '"':
                value += '"';
                advance();
                continue;
            case '\'':
                value += '\'';
                advance();
                continue;
            case '0':
                value += '\0';
                advance();
                continue;
            case 'x':
                // 十六进制转义 \xHH
                advance();
                value += parse_hex_escape();
                continue;
            case 'u':
                // Unicode 转义
                advance();

                if (current_char.has_value() && current_char.value() == '{')
                {
                    // \u{XXXXXX} 格式
                    position++;
                    if (position < input.size())
                    {
                        current_char = input[position];
                    }
                    else
                    {
                        current_char = std::nullopt;
                    }

                    std::string hex_digits;
                    while (current_char.has_value() && current_char.value() != '}')
                    {
                        if (!std::isxdigit(current_char.value()))
                        {
                            throw InvalidEscapeSequenceError('u', line, column);
                        }
                        hex_digits += current_char.value();
                        position++;
                        if (position < input.size())
                        {
                            current_char = input[position];
                        }
                        else
                        {
                            current_char = std::nullopt;
                        }
                    }

                    if (!current_char.has_value() || current_char.value() != '}')
                    {
                        throw InvalidEscapeSequenceError('u', line, column);
                    }

                    position++; // 跳过 }

                    if (hex_digits.empty() || hex_digits.length() > 6)
                    {
                        throw InvalidEscapeSequenceError('u', line, column);
                    }

                    unsigned int codepoint;
                    std::stringstream ss;
                    ss << std::hex << hex_digits;
                    ss >> codepoint;

                    // 将 Unicode 码点转换为 UTF-8
                    if (codepoint <= 0x7F)
                    {
                        value += static_cast<char>(codepoint);
                    }
                    else if (codepoint <= 0x7FF)
                    {
                        value += static_cast<char>(0xC0 | (codepoint >> 6));
                        value += static_cast<char>(0x80 | (codepoint & 0x3F));
                    }
                    else if (codepoint <= 0xFFFF)
                    {
                        value += static_cast<char>(0xE0 | (codepoint >> 12));
                        value += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
                        value += static_cast<char>(0x80 | (codepoint & 0x3F));
                    }
                    else if (codepoint <= 0x10FFFF)
                    {
                        value += static_cast<char>(0xF0 | (codepoint >> 18));
                        value += static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F));
                        value += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
                        value += static_cast<char>(0x80 | (codepoint & 0x3F));
                    }
                    else
                    {
                        throw InvalidEscapeSequenceError('u', line, column);
                    }

                    if (position < input.size())
                    {
                        current_char = input[position];
                    }
                    else
                    {
                        current_char = std::nullopt;
                    }
                    continue;
                }
                else
                {
                    // \uXXXX
                    value += parse_unicode_escape(4);
                    continue;
                }
            default:
                throw InvalidEscapeSequenceError(escaped, line, column);
            }
        }
        else
        {
            value += ch;
            position++;
            if (position < input.size())
            {
                current_char = input[position];
            }
            else
            {
                current_char = std::nullopt;
            }
        }
    }

    if (!terminated)
    {
        // 到达文件末尾但字符串未闭合
        throw UnterminatedStringError(token_line, token_column);
    }

    if (current_char == '"')
    {
        advance();
    }

    return Token(TokenType::String, value, token_line, token_column);
}

Token Lexer::read_raw_string()
{
    size_t token_line = line;
    size_t token_column = column;

    // 跳过 'r'
    advance();

    if (!current_char.has_value() || current_char.value() != '"')
    {
        throw InvalidCharacterError('r', token_line, token_column);
    }

    advance();

    std::string value;
    bool terminated = false;

    while (current_char.has_value())
    {
        char ch = current_char.value();

        if (ch == '"')
        {
            terminated = true;
            break;
        }

        // Raw string 中的所有字符都按字面值处理
        // 包括 \n, \t, \\, \" 等都不转义
        value += ch;
        advance();
    }

    if (!terminated)
    {
        throw UnterminatedStringError(token_line, token_column);
    }

    advance();

    return Token(TokenType::String, value, token_line, token_column);
}

Lexer::Lexer(const std::string &input_str)
{
    input = std::vector<char>(input_str.begin(), input_str.end());
    position = 0;
    line = 1;
    column = 1;
    if (!input.empty())
    {
        current_char = input[0];
    }
    else
    {
        current_char = std::nullopt;
    }
}

Token Lexer::next_token()
{
    while (true)
    {
        skip_whitespace();

        if (current_char == '/' && peek(1) == '/')
        {
            skip_comment();
            continue;
        }

        break;
    }

    if (!current_char.has_value())
    {
        return Token(TokenType::EndOfFile, "", line, column);
    }

    char ch = current_char.value();
    size_t token_line = line;
    size_t token_column = column;

    if (std::isdigit(ch))
    {
        return read_number();
    }

    if (std::isalpha(ch) || ch == '_')
    {
        // 检查是否是 raw string (r"...")
        if (ch == 'r' && peek(1) == '"')
        {
            return read_raw_string();
        }
        return read_identifier();
    }

    if (ch == '"')
    {
        return read_string();
    }

    if (ch == '\'')
    {
        Token token(TokenType::Unknown, std::string(1, ch), token_line, token_column);
        advance();
        return token;
    }

    Token token(TokenType::Unknown, "", token_line, token_column);

    switch (ch)
    {
    case '+':
        token = Token(TokenType::Plus, std::string(1, ch), token_line, token_column);
        break;
    case '-':
        token = Token(TokenType::Minus, std::string(1, ch), token_line, token_column);
        break;
    case '*':
        token = Token(TokenType::Star, std::string(1, ch), token_line, token_column);
        break;
    case '/':
        token = Token(TokenType::Slash, std::string(1, ch), token_line, token_column);
        break;
    case '%':
        token = Token(TokenType::Percent, std::string(1, ch), token_line, token_column);
        break;
    case '=':
        if (peek(1) == '=')
        {
            advance();
            token = Token(TokenType::EqualEqual, "==", token_line, token_column);
        }
        else
        {
            token = Token(TokenType::Equal, std::string(1, ch), token_line, token_column);
        }
        break;
    case '!':
        if (peek(1) == '=')
        {
            advance();
            token = Token(TokenType::BangEqual, "!=", token_line, token_column);
        }
        else
        {
            token = Token(TokenType::Bang, std::string(1, ch), token_line, token_column);
        }
        break;
    case '<':
        if (peek(1) == '=')
        {
            advance();
            token = Token(TokenType::LessEqual, "<=", token_line, token_column);
        }
        else
        {
            token = Token(TokenType::Less, std::string(1, ch), token_line, token_column);
        }
        break;
    case '>':
        if (peek(1) == '=')
        {
            advance();
            token = Token(TokenType::GreaterEqual, ">=", token_line, token_column);
        }
        else
        {
            token = Token(TokenType::Greater, std::string(1, ch), token_line, token_column);
        }
        break;
    case '&':
        if (peek(1) == '&')
        {
            advance();
            token = Token(TokenType::And, "&&", token_line, token_column);
        }
        else
        {
            token = Token(TokenType::Unknown, std::string(1, ch), token_line, token_column);
        }
        break;
    case '|':
        if (peek(1) == '|')
        {
            advance();
            token = Token(TokenType::Or, "||", token_line, token_column);
        }
        else
        {
            token = Token(TokenType::Unknown, std::string(1, ch), token_line, token_column);
        }
        break;
    case '(':
        token = Token(TokenType::LeftParen, std::string(1, ch), token_line, token_column);
        break;
    case ')':
        token = Token(TokenType::RightParen, std::string(1, ch), token_line, token_column);
        break;
    case '{':
        token = Token(TokenType::LeftBrace, std::string(1, ch), token_line, token_column);
        break;
    case '}':
        token = Token(TokenType::RightBrace, std::string(1, ch), token_line, token_column);
        break;
    case '[':
        token = Token(TokenType::LeftBracket, std::string(1, ch), token_line, token_column);
        break;
    case ']':
        token = Token(TokenType::RightBracket, std::string(1, ch), token_line, token_column);
        break;
    case ',':
        token = Token(TokenType::Comma, std::string(1, ch), token_line, token_column);
        break;
    case ';':
        token = Token(TokenType::Semicolon, std::string(1, ch), token_line, token_column);
        break;
    case ':':
        token = Token(TokenType::Colon, std::string(1, ch), token_line, token_column);
        break;
    case '.':
        if (peek(1) == '.')
        {
            advance();
            token = Token(TokenType::DotDot, "..", token_line, token_column);
        }
        else
        {
            token = Token(TokenType::Dot, std::string(1, ch), token_line, token_column);
        }
        break;
    default:
        token = Token(TokenType::Unknown, std::string(1, ch), token_line, token_column);
        break;
    }

    advance();
    return token;
}

std::vector<Token> Lexer::tokenize()
{
    std::vector<Token> tokens;

    while (true)
    {
        Token token = next_token();
        bool is_eof = (token.token_type == TokenType::EndOfFile);
        tokens.push_back(token);

        if (is_eof)
        {
            break;
        }
    }

    return tokens;
}
