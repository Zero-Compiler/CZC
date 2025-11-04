/**
 * @file utf8_handler.hpp
 * @brief UTF-8 处理工具类定义
 * @author BegoniaHe
 */

#ifndef CZC_UTF8_HANDLER_HPP
#define CZC_UTF8_HANDLER_HPP

#include <string>

/**
 * @brief UTF-8 处理工具类 - 处理所有 UTF-8 相关操作
 */
class Utf8Handler
{
public:
    /**
     * @brief 检查是否是 UTF-8 的续字节
     * @param ch 待检查的字节
     * @return 如果是续字节返回 true，否则返回 false
     */
    static bool is_continuation(unsigned char ch);

    /**
     * @brief 获取 UTF-8 字符的字节长度
     * @param first_byte 第一个字节
     * @return 字符的字节长度
     */
    static size_t get_char_length(unsigned char first_byte);

    /**
     * @brief 将 Unicode code point 转换为 UTF-8 字符串
     * @param codepoint Unicode code point
     * @return UTF-8 编码的字符串
     */
    static std::string codepoint_to_utf8(unsigned int codepoint);

    /**
     * @brief 从输入中读取一个完整的 UTF-8 字符
     * @param input 输入字符串
     * @param pos 当前位置（会被更新到下一个字符的位置）
     * @param dest 目标字符串（会追加读取的字符）
     * @return 是否成功读取
     */
    static bool read_char(const std::string &input, size_t &pos, std::string &dest);
};

#endif // CZC_UTF8_HANDLER_HPP
