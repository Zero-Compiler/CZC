/**
 * @file source_location.hpp
 * @brief 源码位置结构定义
 * @author BegoniaHe
 */

#ifndef CZC_SOURCE_LOCATION_HPP
#define CZC_SOURCE_LOCATION_HPP

#include <string>

/**
 * @brief 源码位置结构
 */
struct SourceLocation
{
  std::string filename; ///< 文件名
  size_t line;          ///< 起始行号
  size_t column;        ///< 起始列号
  size_t end_line;      ///< 结束行号
  size_t end_column;    ///< 结束列号

  /**
   * @brief 构造函数
   * @param file 文件名，默认为 "<stdin>"
   * @param ln 起始行号，默认为 1
   * @param col 起始列号，默认为 1
   * @param end_ln 结束行号，默认为 0（会被设置为起始行号）
   * @param end_col 结束列号，默认为 0（会被设置为起始列号）
   */
  SourceLocation(const std::string &file = "<stdin>",
                 size_t ln = 1,
                 size_t col = 1,
                 size_t end_ln = 0,
                 size_t end_col = 0)
      : filename(file), line(ln), column(col),
        end_line(end_ln ? end_ln : ln),
        end_column(end_col ? end_col : col) {}
};

#endif // CZC_SOURCE_LOCATION_HPP
