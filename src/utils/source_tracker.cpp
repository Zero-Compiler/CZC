/**
 * @file source_tracker.cpp
 * @brief 源码跟踪器实现
 * @author BegoniaHe
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
    input.assign(source.begin(), source.end());
}

/**
 * @brief 向前移动一个字符, 更新位置信息
 * @param c 当前字符
 */
void SourceTracker::advance(char c)
{
    position++;
    if (c == '\n')
    {
        line++;
        column = 1;
    }
    else
    {
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

    size_t current_line = 1;
    size_t line_start = 0;

    for (size_t i = 0; i < input.size(); i++)
    {
        if (current_line == line_num)
        {
            // 找到目标行的结束位置
            size_t line_end = i;
            while (line_end < input.size() && input[line_end] != '\n')
            {
                line_end++;
            }
            return std::string(input.begin() + line_start, input.begin() + line_end);
        }

        if (input[i] == '\n')
        {
            current_line++;
            line_start = i + 1;
        }
    }

    if (current_line == line_num)
    {
        return std::string(input.begin() + line_start, input.end());
    }

    return "";
}
