/**
 * @file utf8_handler.cpp
 * @brief UTF-8 处理工具实现
 * @author BegoniaHe
 * @date 2025-11-04
 */

#include "czc/lexer/utf8_handler.hpp"

/**
 * @brief 检查字节是否为 UTF-8 连续字节
 * @param ch 待检查的字节
 * @return 如果是连续字节返回 true, 否则返回 false
 */
bool Utf8Handler::is_continuation(unsigned char ch)
{
    // UTF-8 的续字节（Continuation Byte）的二进制格式总是以 "10" 开头。
    // 因此，我们可以通过检查高两位是否为 `10` 来判断。
    // `(ch & 0xC0)` 会屏蔽掉除了高两位之外的所有位，`0xC0` 的二进制是 `11000000`。
    // `0x80` 的二进制是 `10000000`。
    return (ch & 0xC0) == 0x80;
}

/**
 * @brief 获取 UTF-8 字符的字节长度
 * @param first_byte 字符的第一个字节
 * @return 字符的字节长度, 如果无效则返回 0
 */
size_t Utf8Handler::get_char_length(unsigned char first_byte)
{
    // 根据 UTF-8 编码规则，字符的第一个字节决定了整个字符序列的长度。
    // 0xxxxxxx: 1字节 (ASCII)
    if ((first_byte & 0x80) == 0)
        return 1;
    // 110xxxxx: 2字节
    if ((first_byte & 0xE0) == 0xC0)
        return 2;
    // 1110xxxx: 3字节
    if ((first_byte & 0xF0) == 0xE0)
        return 3;
    // 11110xxx: 4字节
    if ((first_byte & 0xF8) == 0xF0)
        return 4;
    // 如果不匹配以上任何模式，则它不是一个有效的 UTF-8 起始字节。
    return 0;
}

/**
 * @brief 将 Unicode 码点转换为 UTF-8 编码字符串
 * @param codepoint Unicode 码点
 * @return 转换后的 UTF-8 字符串
 */
std::string Utf8Handler::codepoint_to_utf8(unsigned int codepoint)
{
    std::string result;

    // 此函数实现了将 Unicode 码点转换为 UTF-8 字节序列的标准算法。
    // 根据码点的大小，决定需要1、2、3还是4个字节来表示。

    // ASCII 范围，直接转换。
    if (codepoint <= 0x7F)
    {
        result.push_back(static_cast<char>(codepoint));
    }
    // 2字节序列，格式为 110xxxxx 10xxxxxx
    else if (codepoint <= 0x7FF)
    {
        result.push_back(static_cast<char>(0xC0 | ((codepoint >> 6) & 0x1F)));
        result.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    }
    // 3字节序列，格式为 1110xxxx 10xxxxxx 10xxxxxx
    else if (codepoint <= 0xFFFF)
    {
        result.push_back(static_cast<char>(0xE0 | ((codepoint >> 12) & 0x0F)));
        result.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
        result.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    }
    // 4字节序列，格式为 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
    else if (codepoint <= 0x10FFFF)
    {
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

    // 检查计算出的字符长度是否有效，以及输入字符串中是否还有足够的字节来构成一个完整的字符。
    if (char_len == 0 || pos + char_len > input.size())
    {
        // 如果起始字节无效或剩余字节不足，则这是一个无效的 UTF-8 序列。
        return false;
    }

    // 对于多字节字符，验证其后的所有字节是否都是合法的续字节。
    // 这是确保 UTF-8 序列有效性的关键步骤。
    for (size_t i = 1; i < char_len; i++)
    {
        if (!is_continuation(static_cast<unsigned char>(input[pos + i])))
        {
            return false; // 如果任何一个后续字节不是续字节，则序列无效。
        }
    }

    // 如果所有检查都通过，则将这组字节作为一个完整的 UTF-8 字符追加到目标字符串中。
    dest.append(input, pos, char_len);
    // 更新位置指针，跳过刚刚读取的整个字符。
    pos += char_len;

    return true;
}
