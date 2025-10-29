#include "lexer.hpp"
#include <cctype>

void Lexer::advance()
{
    if (current_char == '\n')
    {
        line++;
        column = 1;
    }
    else
    {
        column++;
    }

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

            std::string value(input.begin() + start, input.begin() + position);
            return Token(TokenType::Integer, value, token_line, token_column);
        }

        // Binary: 0b
        else if (next_ch == 'b' || next_ch == 'B')
        {
            advance();
            advance();

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

            std::string value(input.begin() + start, input.begin() + position);
            return Token(TokenType::Integer, value, token_line, token_column);
        }

        // Octal: 0o
        else if (next_ch == 'o' || next_ch == 'O')
        {
            advance();
            advance();

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

Token Lexer::read_string()
{
    size_t token_line = line;
    size_t token_column = column;
    advance();

    std::string value;
    while (current_char.has_value())
    {
        char ch = current_char.value();

        if (ch == '"')
        {
            break;
        }

        if (ch == '\\')
        {
            advance();
            if (!current_char.has_value())
            {
                break;
            }

            char escaped = current_char.value();
            switch (escaped)
            {
            case 'n':
                value += '\n';
                break;
            case 't':
                value += '\t';
                break;
            case 'r':
                value += '\r';
                break;
            case '\\':
                value += '\\';
                break;
            case '"':
                value += '"';
                break;
            case '\'':
                value += '\'';
                break;
            case '0':
                value += '\0';
                break;
            default:
                // Unknown escape sequence, keep as-is
                value += '\\';
                value += escaped;
                break;
            }
            advance();
        }
        else if (ch == '\n')
        {
            // Unterminated string
            break;
        }
        else
        {
            value += ch;
            advance();
        }
    }

    if (current_char == '"')
    {
        advance();
    }

    return Token(TokenType::String, value, token_line, token_column);
}

Token Lexer::read_char()
{
    size_t token_line = line;
    size_t token_column = column;
    advance();
    std::string value;

    if (!current_char.has_value())
    {
        return Token(TokenType::Unknown, "'", token_line, token_column);
    }

    char ch = current_char.value();

    if (ch == '\\')
    {
        // Handle escape sequences
        advance();
        if (!current_char.has_value())
        {
            return Token(TokenType::Unknown, "'", token_line, token_column);
        }

        char escaped = current_char.value();
        switch (escaped)
        {
        case 'n':
            value = "\n";
            break;
        case 't':
            value = "\t";
            break;
        case 'r':
            value = "\r";
            break;
        case '\\':
            value = "\\";
            break;
        case '\'':
            value = "'";
            break;
        case '"':
            value = "\"";
            break;
        case '0':
            value = "\0";
            break;
        default:
            value = escaped;
            break;
        }
        advance();
    }
    else if (ch == '\'')
    {
        // Empty char literal
        return Token(TokenType::Unknown, "''", token_line, token_column);
    }
    else if (ch == '\n')
    {
        // Unterminated char literal
        return Token(TokenType::Unknown, "'", token_line, token_column);
    }
    else
    {
        value = ch;
        advance();
    }

    if (current_char != '\'')
    {
        return Token(TokenType::Unknown, value, token_line, token_column);
    }

    advance();
    return Token(TokenType::Char, value, token_line, token_column);
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
        return read_identifier();
    }

    if (ch == '"')
    {
        return read_string();
    }

    if (ch == '\'')
    {
        return read_char();
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
