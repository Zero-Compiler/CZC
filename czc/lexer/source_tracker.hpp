/**
 * @file source_tracker.hpp
 * @brief 源码跟踪器类定义
 * @author BegoniaHe
 */

#ifndef CZC_SOURCE_TRACKER_HPP
#define CZC_SOURCE_TRACKER_HPP

#include "source_location.hpp"
#include <string>
#include <vector>

/**
 * @brief 源码跟踪器类
 */
class SourceTracker
{
private:
    std::string filename;    ///< 文件名
    std::vector<char> input; ///< 输入源码
    size_t position;         ///< 当前位置
    size_t line;             ///< 当前行号
    size_t column;           ///< 当前列号

public:
    /**
     * @brief 构造函数
     * @param source 源码字符串
     * @param fname 文件名，默认为 "<stdin>"
     */
    SourceTracker(const std::string &source, const std::string &fname = "<stdin>");

    /**
     * @brief 前进一个字符
     * @param c 当前字符
     */
    void advance(char c);

    /**
     * @brief 获取当前位置
     * @return 当前位置
     */
    size_t get_position() const { return position; }

    /**
     * @brief 获取当前行号
     * @return 当前行号
     */
    size_t get_line() const { return line; }

    /**
     * @brief 获取当前列号
     * @return 当前列号
     */
    size_t get_column() const { return column; }

    /**
     * @brief 获取文件名
     * @return 文件名引用
     */
    const std::string &get_filename() const { return filename; }

    /**
     * @brief 创建源码位置对象
     * @param start_line 起始行号
     * @param start_col 起始列号
     * @return 源码位置对象
     */
    SourceLocation make_location(size_t start_line, size_t start_col) const;

    /**
     * @brief 获取指定行的源码内容
     * @param line_num 行号
     * @return 源码行内容
     */
    std::string get_source_line(size_t line_num) const;

    /**
     * @brief 获取输入源码
     * @return 输入源码的常量引用
     */
    const std::vector<char> &get_input() const { return input; }
};

#endif // CZC_SOURCE_TRACKER_HPP
