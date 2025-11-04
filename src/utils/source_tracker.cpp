/**
 * @file source_tracker.cpp
 * @brief 源码跟踪器实现
 * @author BegoniaHe
 * @date 2025-11-04
 */

#include "czc/utils/source_tracker.hpp"

/**
 * @brief SourceTracker 构造函数
 * @param source 源码字符串
 * @param fname 文件名
 */
SourceTracker::SourceTracker(const std::string &source, const std::string &fname)
    : filename(fname), position(0), line(1), column(1)
{
    // 将输入的 std::string 复制到内部的 std::vector<char>。
    // 使用 vector<char> 可能在某些情况下（例如，与C API交互）比 string 更灵活，
    // 但在这里主要是一个设计选择。
    input.assign(source.begin(), source.end());
}

/**
 * @brief 向前移动一个字符, 更新位置信息
 * @param c 当前字符
 */
void SourceTracker::advance(char c)
{
    // 每次消耗一个字符时，字节位置 `position` 总是增加。
    position++;
    // 检查消耗的字符是否是换行符。
    if (c == '\n')
    {
        // 如果是换行符，行号增加，列号重置为1。
        line++;
        column = 1;
    }
    else
    {
        // 如果不是换行符，只增加列号。
        column++;
    }
}

/**
 * @brief 创建一个源码位置对象
 * @param start_line 起始行号
 * @param start_col 起始列号
 * @return 构造的 SourceLocation 对象
 */
SourceLocation SourceTracker::make_location(size_t start_line, size_t start_col) const
{
    // 使用给定的起始位置和跟踪器当前的结束位置来创建一个 SourceLocation 对象。
    // 这对于从词法分析器中标记出 Token 的范围非常有用。
    return SourceLocation(filename, start_line, start_col, line, column);
}

/**
 * @brief 获取指定行的源码内容
 * @param line_num 行号
 * @return 对应行的源码字符串
 */
std::string SourceTracker::get_source_line(size_t line_num) const
{
    if (line_num == 0)
        return "";

    // 这是一个线性扫描，用于从完整的源码中提取特定行。
    // 对于大型文件，如果频繁调用，性能可能会较低。
    // 优化方案可以是在初始化时预处理源码，构建一个行起始位置的索引表。
    size_t current_line = 1;
    size_t line_start = 0; // 当前行的起始字节索引。

    for (size_t i = 0; i < input.size(); i++)
    {
        // 当我们扫描到目标行时...
        if (current_line == line_num)
        {
            // ...继续向前扫描，直到找到该行的末尾（换行符或文件结尾）。
            size_t line_end = i;
            while (line_end < input.size() && input[line_end] != '\n')
            {
                line_end++;
            }
            // 提取并返回该行的子字符串。
            return std::string(input.begin() + line_start, input.begin() + line_end);
        }

        // 如果遇到换行符，意味着我们即将进入下一行。
        if (input[i] == '\n')
        {
            current_line++;
            line_start = i + 1; // 更新下一行的起始位置。
        }
    }

    // 处理文件最后一行没有换行符的特殊情况。
    if (current_line == line_num)
    {
        return std::string(input.begin() + line_start, input.end());
    }

    return "";
}
