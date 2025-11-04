/**
 * @file lexer.cpp
 * @brief 词法分析器实现
 * @author BegoniaHe
 */

#include "czc/lexer/lexer.hpp"
#include "czc/lexer/utf8_handler.hpp"
#include <cctype>
#include <sstream>
#include <iomanip>

/**
 * @brief 报告一个词法分析错误
 * @param code 诊断代码
 * @param error_line 错误发生的行号
 * @param error_column 错误发生的列号
 * @param args 格式化参数
 */
void Lexer::report_error(DiagnosticCode code,
                         size_t error_line,
                         size_t error_column,
                         const std::vector<std::string> &args)
{
    auto loc = SourceLocation(tracker.get_filename(), error_line, error_column, error_line, error_column);
    error_collector.add(code, loc, args);
}

/**
 * @brief 向前移动一个字符
 * @details 更新源码跟踪器的位置和当前字符
 */
void Lexer::advance()
{
    if (!current_char.has_value())
    {
        return;
    }

    char ch = current_char.value();
    tracker.advance(ch);

    size_t pos = tracker.get_position();
    const auto &input = tracker.get_input();

    if (pos < input.size())
    {
        current_char = input[pos];
    }
    else
    {
        current_char = std::nullopt;
    }
}

/**
 * @brief 查看未来位置的字符
 * @param offset 偏移量
 * @return 如果位置有效, 返回对应的字符, 否则返回 std::nullopt
 */
std::optional<char> Lexer::peek(size_t offset) const
{
    size_t peek_pos = tracker.get_position() + offset;
    const auto &input = tracker.get_input();
    if (peek_pos < input.size())
    {
        return input[peek_pos];
    }
    return std::nullopt;
}

/**
 * @brief 跳过空白字符
 */
void Lexer::skip_whitespace()
{
    while (current_char.has_value() && std::isspace(current_char.value()))
    {
        advance();
    }
}

/**
 * @brief 跳过单行注释
 */
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

/**
 * @brief 读取带前缀的数字 (例如 0x, 0b, 0o)
 * @param valid_chars 允许的数字字符
 * @param prefix_str 前缀字符串 (用于报错)
 * @param error_code 缺少数字时报告的错误码
 * @return 构造的 Token
 */
Token Lexer::read_prefixed_number(const std::string &valid_chars,
                                  const std::string &prefix_str,
                                  DiagnosticCode error_code)
{
    size_t start = tracker.get_position();
    size_t token_line = tracker.get_line();
    size_t token_column = tracker.get_column();

    // 跳过 0x/0b/0o
    advance(); // '0'
    advance(); // 'x'/'b'/'o'

    size_t digit_start = tracker.get_position();

    // 读取有效字符
    while (current_char.has_value())
    {
        char ch = current_char.value();
        if (valid_chars.find(ch) != std::string::npos)
        {
            advance();
        }
        else
        {
            break;
        }
    }

    // 检查是否至少有一个数字
    size_t current_pos = tracker.get_position();
    if (current_pos == digit_start)
    {
        report_error(error_code, token_line, token_column, {prefix_str});
        // 返回一个错误 token
        const auto &input = tracker.get_input();
        return Token(TokenType::Unknown,
                     std::string(&input[start], current_pos - start),
                     token_line, token_column);
    }

    const auto &input = tracker.get_input();
    return Token(TokenType::Integer,
                 std::string(&input[start], current_pos - start),
                 token_line, token_column);
}

/**
 * @brief 读取数字 (整数, 浮点数, 科学计数法)
 * @return 构造的 Token
 */
Token Lexer::read_number()
{
    size_t start = tracker.get_position();
    size_t token_line = tracker.get_line();
    size_t token_column = tracker.get_column();
    bool is_float = false;
    bool is_scientific = false;

    if (current_char == '0' && peek(1).has_value())
    {
        char next_ch = peek(1).value();

        if (next_ch == 'x' || next_ch == 'X')
        {
            return read_prefixed_number(
                "0123456789abcdefABCDEF",
                "0x",
                DiagnosticCode::L0001_MissingHexDigits);
        }
        else if (next_ch == 'b' || next_ch == 'B')
        {
            return read_prefixed_number(
                "01",
                "0b",
                DiagnosticCode::L0002_MissingBinaryDigits);
        }
        else if (next_ch == 'o' || next_ch == 'O')
        {
            return read_prefixed_number(
                "01234567",
                "0o",
                DiagnosticCode::L0003_MissingOctalDigits);
        }
    }

    while (current_char.has_value())
    {
        char ch = current_char.value();
        if (std::isdigit(ch))
        {
            advance();
        }
        else if (ch == '.' && !is_float)
        {
            auto next = peek(1);
            if (next.has_value() && std::isdigit(next.value()))
            {
                is_float = true;
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

    // 检查科学计数法
    if (current_char.has_value() && (current_char.value() == 'e' || current_char.value() == 'E'))
    {
        is_scientific = true;
        advance();

        // 可选的正负号
        if (current_char.has_value() && (current_char.value() == '+' || current_char.value() == '-'))
        {
            advance();
        }

        // 指数部分必须至少有一个数字
        size_t exp_start = tracker.get_position();
        while (current_char.has_value() && std::isdigit(current_char.value()))
        {
            advance();
        }

        // 如果指数部分没有数字，则报错
        size_t current_pos = tracker.get_position();
        if (current_pos == exp_start)
        {
            const auto &input = tracker.get_input();
            report_error(DiagnosticCode::L0004_MissingExponentDigits,
                         token_line, token_column,
                         {std::string(&input[start], current_pos - start)});
            return Token(TokenType::Unknown,
                         std::string(&input[start], current_pos - start),
                         token_line, token_column);
        }
    }

    // 检查数字后是否紧跟字母或下划线
    if (current_char.has_value() && (std::isalpha(current_char.value()) || current_char.value() == '_'))
    {
        std::string bad_suffix(1, current_char.value());
        report_error(DiagnosticCode::L0005_InvalidTrailingChar,
                     token_line, token_column,
                     {bad_suffix});
        advance();
        size_t current_pos = tracker.get_position();
        const auto &input = tracker.get_input();
        return Token(TokenType::Unknown,
                     std::string(&input[start], current_pos - start),
                     token_line, token_column);
    }

    size_t current_pos = tracker.get_position();
    const auto &input = tracker.get_input();
    std::string value(&input[start], current_pos - start);

    if (is_scientific)
    {
        return Token(TokenType::ScientificExponent, value, token_line, token_column);
    }
    else if (is_float)
    {
        return Token(TokenType::Float, value, token_line, token_column);
    }
    else
    {
        return Token(TokenType::Integer, value, token_line, token_column);
    }
}

/**
 * @brief 读取标识符或关键字
 * @return 构造的 Token
 */
Token Lexer::read_identifier()
{
    size_t start = tracker.get_position();
    size_t token_line = tracker.get_line();
    size_t token_column = tracker.get_column();

    advance();

    while (current_char.has_value())
    {
        char ch = current_char.value();
        unsigned char uch = static_cast<unsigned char>(ch);

        if (std::isalnum(ch) || ch == '_')
        {
            advance();
        }
        else if (uch >= 0x80)
        {
            advance();
        }
        else
        {
            break;
        }
    }

    size_t current_pos = tracker.get_position();
    const auto &input = tracker.get_input();
    std::string value(&input[start], current_pos - start);
    auto keyword_type = get_keyword(value);
    TokenType token_type = keyword_type.value_or(TokenType::Identifier);

    return Token(token_type, value, token_line, token_column);
}

// 解析 Unicode 转义序列
/**
 * @brief 解析 Unicode 转义序列
 * @param digit_count 需要解析的十六进制数字个数
 * @return 转换后的 UTF-8 字符串
 */
std::string Lexer::parse_unicode_escape(size_t digit_count)
{
    std::string hex_digits;

    for (size_t i = 0; i < digit_count; ++i)
    {
        if (!current_char.has_value())
        {
            report_error(DiagnosticCode::L0009_InvalidUnicodeEscape,
                         tracker.get_line(), tracker.get_column(), {"u"});
            return "";
        }

        char ch = current_char.value();
        if (!std::isxdigit(ch))
        {
            report_error(DiagnosticCode::L0009_InvalidUnicodeEscape,
                         tracker.get_line(), tracker.get_column(), {"u"});
            return "";
        }

        hex_digits += ch;
        advance();
    }

    unsigned int codepoint;
    std::stringstream ss;
    ss << std::hex << hex_digits;
    ss >> codepoint;

    // 使用 Utf8Handler 转换
    return Utf8Handler::codepoint_to_utf8(codepoint);
}

// 解析十六进制转义序列 \xHH
/**
 * @brief 解析十六进制转义序列 (\\xHH)
 * @return 转换后的字节字符串
 */
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
        advance();
    }

    if (hex_digits.empty())
    {
        report_error(DiagnosticCode::L0008_InvalidHexEscape,
                     tracker.get_line(), tracker.get_column(), {"x"});
        return ""; // 返回空字符串
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

/**
 * @brief 读取字符串字面量
 * @return 构造的 Token
 */
Token Lexer::read_string()
{
    size_t token_line = tracker.get_line();
    size_t token_column = tracker.get_column();
    advance(); // 跳过开头的 "

    std::string value;
    value.reserve(64); // 预留空间，减少重新分配
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
            advance(); // 跳过反斜杠
            if (!current_char.has_value())
            {
                report_error(DiagnosticCode::L0007_UnterminatedString,
                             token_line, token_column, {});
                // 返回不完整的字符串 token
                return Token(TokenType::String, value, token_line, token_column);
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
                    advance(); // 跳过 {

                    std::string hex_digits;
                    while (current_char.has_value() && current_char.value() != '}')
                    {
                        if (!std::isxdigit(current_char.value()))
                        {
                            report_error(DiagnosticCode::L0009_InvalidUnicodeEscape,
                                         tracker.get_line(), tracker.get_column(), {"u"});
                            // 跳过剩余内容直到 } 或字符串结束
                            while (current_char.has_value() && current_char.value() != '}' && current_char.value() != '"')
                            {
                                advance();
                            }
                            break;
                        }
                        hex_digits += current_char.value();
                        advance();
                    }

                    if (!current_char.has_value() || current_char.value() != '}')
                    {
                        report_error(DiagnosticCode::L0009_InvalidUnicodeEscape,
                                     tracker.get_line(), tracker.get_column(), {"u"});
                        continue;
                    }

                    advance(); // 跳过 }

                    if (hex_digits.empty() || hex_digits.length() > 6)
                    {
                        report_error(DiagnosticCode::L0009_InvalidUnicodeEscape,
                                     tracker.get_line(), tracker.get_column(), {"u"});
                        continue;
                    }

                    unsigned int codepoint;
                    std::stringstream ss;
                    ss << std::hex << hex_digits;
                    ss >> codepoint;

                    value += Utf8Handler::codepoint_to_utf8(codepoint);

                    continue;
                }
                else
                {
                    // \uXXXX
                    value += parse_unicode_escape(4);
                    continue;
                }
            default:
                report_error(DiagnosticCode::L0006_InvalidEscapeSequence,
                             tracker.get_line(), tracker.get_column(), {std::string(1, escaped)});
                // 跳过无效的转义字符,继续处理
                value += escaped;
                advance();
                continue;
            }
        }
        else
        {
            // 读取 UTF-8 字符
            if (!current_char.has_value())
                break;

            unsigned char byte = static_cast<unsigned char>(current_char.value());
            size_t char_length = Utf8Handler::get_char_length(byte);
            const auto &input = tracker.get_input();
            size_t pos = tracker.get_position();

            for (size_t i = 0; i < char_length && pos + i < input.size(); ++i)
            {
                value += input[pos + i];
            }
            advance();
        }
    }

    if (!terminated)
    {
        // 到达文件末尾但字符串未闭合
        report_error(DiagnosticCode::L0007_UnterminatedString,
                     token_line, token_column, {});
        // 返回不完整的字符串 token
        return Token(TokenType::String, value, token_line, token_column);
    }

    advance(); // 跳过结尾的 "

    return Token(TokenType::String, value, token_line, token_column);
}

/**
 * @brief 读取原始字符串字面量 (r"...")
 * @return 构造的 Token
 */
Token Lexer::read_raw_string()
{
    size_t token_line = tracker.get_line();
    size_t token_column = tracker.get_column();

    // 跳过 'r'
    advance();

    if (!current_char.has_value() || current_char.value() != '"')
    {
        report_error(DiagnosticCode::L0010_InvalidCharacter,
                     token_line, token_column, {"r"});
        // 返回错误 token
        return Token(TokenType::Unknown, "r", token_line, token_column);
    }

    advance();

    std::string value;
    value.reserve(64); // 预留空间，减少重新分配
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
        // 读取 UTF-8 字符
        unsigned char byte = static_cast<unsigned char>(ch);
        size_t char_length = Utf8Handler::get_char_length(byte);
        const auto &input = tracker.get_input();
        size_t pos = tracker.get_position();

        for (size_t i = 0; i < char_length && pos + i < input.size(); ++i)
        {
            value += input[pos + i];
        }
        advance();
    }

    if (!terminated)
    {
        report_error(DiagnosticCode::L0007_UnterminatedString,
                     token_line, token_column, {});
        // 返回不完整的原始字符串 token
        return Token(TokenType::String, value, token_line, token_column);
    }

    advance();

    return Token(TokenType::String, value, token_line, token_column);
}

/**
 * @brief Lexer 构造函数
 * @param input_str 输入的源码字符串
 * @param fname 文件名
 */
Lexer::Lexer(const std::string &input_str,
             const std::string &fname)
    : tracker(input_str, fname)
{
    const auto &input = tracker.get_input();
    if (!input.empty())
    {
        current_char = input[0];
    }
    else
    {
        current_char = std::nullopt;
    }
}

/**
 * @brief 获取下一个 Token
 * @return 下一个 Token
 */
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
        return Token(TokenType::EndOfFile, "", tracker.get_line(), tracker.get_column());
    }

    char ch = current_char.value();
    unsigned char uch = static_cast<unsigned char>(ch);
    size_t token_line = tracker.get_line();
    size_t token_column = tracker.get_column();

    if (std::isdigit(ch))
    {
        return read_number();
    }

    if (std::isalpha(ch) || ch == '_' || uch >= 0x80)
    {
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
        if (peek(1) == '=')
        {
            advance();
            token = Token(TokenType::PlusEqual, "+=", token_line, token_column);
        }
        else
        {
            token = Token(TokenType::Plus, std::string(1, ch), token_line, token_column);
        }
        break;
    case '-':
        if (peek(1) == '=')
        {
            advance();
            token = Token(TokenType::MinusEqual, "-=", token_line, token_column);
        }
        else
        {
            token = Token(TokenType::Minus, std::string(1, ch), token_line, token_column);
        }
        break;
    case '*':
        if (peek(1) == '=')
        {
            advance();
            token = Token(TokenType::StarEqual, "*=", token_line, token_column);
        }
        else
        {
            token = Token(TokenType::Star, std::string(1, ch), token_line, token_column);
        }
        break;
    case '/':
        if (peek(1) == '=')
        {
            advance();
            token = Token(TokenType::SlashEqual, "/=", token_line, token_column);
        }
        else
        {
            token = Token(TokenType::Slash, std::string(1, ch), token_line, token_column);
        }
        break;
    case '%':
        if (peek(1) == '=')
        {
            advance();
            token = Token(TokenType::PercentEqual, "%=", token_line, token_column);
        }
        else
        {
            token = Token(TokenType::Percent, std::string(1, ch), token_line, token_column);
        }
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

/**
 * @brief 将整个输入源 tokenize
 * @return Token 向量
 */
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
