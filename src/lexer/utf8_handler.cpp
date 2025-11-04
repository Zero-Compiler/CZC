/**
 * @file utf8_handler.cpp
 * @brief UTF-8 处理工具实现
 * @author BegoniaHe
 */

#include "czc/lexer/utf8_handler.hpp"

/**
 * @brief 检查字节是否为 UTF-8 连续字节
 * @param ch 待检查的字节
 * @return 如果是连续字节返回 true, 否则返回 false
 */
bool Utf8Handler::is_continuation(unsigned char ch)
{
    return (ch & 0xC0) == 0x80; // 10xxxxxx
}

/**
 * @brief 获取 UTF-8 字符的字节长度
 * @param first_byte 字符的第一个字节
 * @return 字符的字节长度, 如果无效则返回 0
 */
size_t Utf8Handler::get_char_length(unsigned char first_byte)
{
    if ((first_byte & 0x80) == 0)
        return 1; // 0xxxxxxx - ASCII
    if ((first_byte & 0xE0) == 0xC0)
        return 2; // 110xxxxx
    if ((first_byte & 0xF0) == 0xE0)
        return 3; // 1110xxxx
    if ((first_byte & 0xF8) == 0xF0)
        return 4; // 11110xxx
    return 0;     // Invalid
}

/**
 * @brief 将 Unicode 码点转换为 UTF-8 编码字符串
 * @param codepoint Unicode 码点
 * @return 转换后的 UTF-8 字符串
 */
std::string Utf8Handler::codepoint_to_utf8(unsigned int codepoint)
{
    std::string result;

    if (codepoint <= 0x7F)
    {
        // 1-byte sequence (ASCII)
        result.push_back(static_cast<char>(codepoint));
    }
    else if (codepoint <= 0x7FF)
    {
        // 2-byte sequence
        result.push_back(static_cast<char>(0xC0 | ((codepoint >> 6) & 0x1F)));
        result.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    }
    else if (codepoint <= 0xFFFF)
    {
        // 3-byte sequence
        result.push_back(static_cast<char>(0xE0 | ((codepoint >> 12) & 0x0F)));
        result.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
        result.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    }
    else if (codepoint <= 0x10FFFF)
    {
        // 4-byte sequence
        result.push_back(static_cast<char>(0xF0 | ((codepoint >> 18) & 0x07)));
        result.push_back(static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F)));
        result.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
        result.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    }

    return result;
}

/**
 * @brief 从输入字符串中读取一个完整的 UTF-8 字符
 * @param input 输入字符串
 * @param pos 当前位置 (将被更新)
 * @param dest 存储读取字符的目标字符串
 * @return 如果成功读取返回 true, 否则返回 false
 */
bool Utf8Handler::read_char(const std::string &input, size_t &pos, std::string &dest)
{
    if (pos >= input.size())
    {
        return false;
    }

    unsigned char first_byte = static_cast<unsigned char>(input[pos]);
    size_t char_len = get_char_length(first_byte);

    if (char_len == 0 || pos + char_len > input.size())
    {
        return false; // Invalid UTF-8 sequence
    }

    // Validate continuation bytes
    for (size_t i = 1; i < char_len; i++)
    {
        if (!is_continuation(static_cast<unsigned char>(input[pos + i])))
        {
            return false;
        }
    }

    // Append the complete UTF-8 character
    dest.append(input, pos, char_len);
    pos += char_len;

    return true;
}
