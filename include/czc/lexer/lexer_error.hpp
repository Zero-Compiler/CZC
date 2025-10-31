#ifndef CZC_LEXER_ERROR_HPP
#define CZC_LEXER_ERROR_HPP

#include <stdexcept>
#include <string>

// 基类
class LexerError : public std::runtime_error
{
protected:
    size_t error_line;
    size_t error_column;

public:
    LexerError(const std::string &message, size_t line, size_t column);
    size_t line() const { return error_line; }
    size_t column() const { return error_column; }
    virtual std::string format_error() const;
};

// 未闭合字符串
class UnterminatedStringError : public LexerError
{
public:
    UnterminatedStringError(size_t line, size_t column);
    std::string format_error() const override;
};

// 无效转义序列
class InvalidEscapeSequenceError : public LexerError
{
private:
    char escape_char;

public:
    InvalidEscapeSequenceError(char ch, size_t line, size_t column);
    std::string format_error() const override;
};

// 无效字符
class InvalidCharacterError : public LexerError
{
private:
    char invalid_char;

public:
    InvalidCharacterError(char ch, size_t line, size_t column);
    std::string format_error() const override;
};

// 无效数字格式
class InvalidNumberFormatError : public LexerError
{
private:
    std::string number_string;

public:
    InvalidNumberFormatError(const std::string &num_str, size_t line, size_t column);
    std::string format_error() const override;
};

#endif // CZC_LEXER_ERROR_HPP
