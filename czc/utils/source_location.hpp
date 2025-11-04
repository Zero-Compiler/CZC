/**
 * @file source_location.hpp
 * @brief 源码位置结构定义
 * @author BegoniaHe
 * @date 2025-11-04
 */

#ifndef CZC_SOURCE_LOCATION_HPP
#define CZC_SOURCE_LOCATION_HPP

#include <string>

namespace czc {
namespace utils {

/**
 * @brief 代表源代码中的一个特定区域（或点）。
 * @details
 *   此结构体用于精确定位 Token、AST 节点或诊断信息在源文件中的位置。
 *   它包含了文件名、起始和结束的行号与列号，是实现精确错误报告和
 *   源代码交互（如 IDE 高亮）的基础。
 */
struct SourceLocation {
  // 关联的源文件名。
  std::string filename;
  // 区域开始的行号（从 1 开始计数）。
  size_t line;
  // 区域开始的列号（从 1 开始计数）。
  size_t column;
  // 区域结束的行号（从 1 开始计数）。
  size_t end_line;
  // 区域结束的列号（从 1 开始计数）。
  size_t end_column;

  /**
   * @brief 构造一个新的 SourceLocation 对象。
   * @param[in] file     文件名，默认为 "<stdin>"。
   * @param[in] ln       起始行号，默认为 1。
   * @param[in] col      起始列号，默认为 1。
   * @param[in] end_ln   (可选) 结束行号。若为 0 或未提供，则默认为起始行号。
   * @param[in] end_col  (可选) 结束列号。若为 0 或未提供，则默认为起始列号。
   */
  SourceLocation(const std::string &file = "<stdin>", size_t ln = 1,
                 size_t col = 1, size_t end_ln = 0, size_t end_col = 0)
      : filename(file), line(ln), column(col), end_line(end_ln ? end_ln : ln),
        end_column(end_col ? end_col : col) {}
};

} // namespace utils
} // namespace czc

#endif // CZC_SOURCE_LOCATION_HPP
