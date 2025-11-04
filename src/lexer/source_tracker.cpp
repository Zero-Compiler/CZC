/**
 * @file source_tracker.cpp
 * @brief 源码跟踪器实现
 * @author BegoniaHe
 */

#include "czc/lexer/source_tracker.hpp"

SourceTracker::SourceTracker(const std::string &source, const std::string &fname)
    : filename(fname), position(0), line(1), column(1)
{
    input.assign(source.begin(), source.end());
}

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

SourceLocation SourceTracker::make_location(size_t start_line, size_t start_col) const
{
    return SourceLocation(filename, start_line, start_col, line, column);
}

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
