#include "lexer_error.hpp"
#include <sstream>

// LexerError 基类
LexerError::LexerError(const std::string &message, size_t line, size_t column)
    : std::runtime_error(message), error_line(line), error_column(column)
{
}

std::string LexerError::format_error() const
{
    std::ostringstream oss;
    oss << "Lexer error at line " << error_line << ", column " << error_column
        << ": " << what();
    return oss.str();
}

// UnterminatedStringError - 未闭合字符串
UnterminatedStringError::UnterminatedStringError(size_t line, size_t column)
    : LexerError("Unterminated string literal", line, column)
{
}

std::string UnterminatedStringError::format_error() const
{
    std::ostringstream oss;
    oss << "Lexer error at line " << error_line << ", column " << error_column
        << ": Unterminated string literal (missing closing quote)";
    return oss.str();
}

// InvalidEscapeSequenceError - 无效转义序列
InvalidEscapeSequenceError::InvalidEscapeSequenceError(char ch, size_t line, size_t column)
    : LexerError("Invalid escape sequence", line, column), escape_char(ch)
{
}

std::string InvalidEscapeSequenceError::format_error() const
{
    std::ostringstream oss;
    oss << "Lexer error at line " << error_line << ", column " << error_column
        << ": Invalid escape sequence '\\" << escape_char << "'";
    return oss.str();
}

// InvalidCharacterError - 无效字符
InvalidCharacterError::InvalidCharacterError(char ch, size_t line, size_t column)
    : LexerError("Invalid character", line, column), invalid_char(ch)
{
}

std::string InvalidCharacterError::format_error() const
{
    std::ostringstream oss;
    oss << "Lexer error at line " << error_line << ", column " << error_column
        << ": Invalid character '" << invalid_char << "' (ASCII: " << static_cast<int>(invalid_char) << ")";
    return oss.str();
}

// InvalidNumberFormatError - 数字格式错误
InvalidNumberFormatError::InvalidNumberFormatError(const std::string &num_str, size_t line, size_t column)
    : LexerError("Invalid number format", line, column), number_string(num_str)
{
}

std::string InvalidNumberFormatError::format_error() const
{
    std::ostringstream oss;
    oss << "Lexer error at line " << error_line << ", column " << error_column
        << ": Invalid number format '" << number_string << "'";
    return oss.str();
}
