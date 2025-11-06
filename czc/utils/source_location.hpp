/**
 * @file source_location.hpp
 * @brief 定义了用于表示源代码位置的 `SourceLocation` 结构体。
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
 * @details 此结构体用于精确定位 Token、AST 节点或诊断信息在源文件中的位置。
 *          它包含了文件名、起始和结束的行号与列号，是实现精确错误报告和
 *          源代码交互（如 IDE 高亮）的基础。
 * @property {数据成员} 所有成员均为公开的 `size_t` 或 `std::string`，
 *           这是一个纯数据结构 (Plain Old Data, POD-like)。
 */
struct SourceLocation {
  // 关联的源文件名
  std::string filename;
  // 区域开始的行号（从 1 开始计数）
  size_t line;
  // 区域开始的列号（从 1 开始计数）
  size_t column;
  // 区域结束的行号（从 1 开始计数）
  size_t end_line;
  // 区域结束的列号（从 1 开始计数）
  size_t end_column;

  /**
   * @brief 构造一个新的 SourceLocation 对象。
   * @details
   * 如果结束行号或列号未提供（或为0），它们将自动设置为与起始位置相同，
   *          从而创建一个表示单个点的 SourceLocation。
   *
   * @param[in] file     文件名。
   * @param[in] ln       起始行号（1-based）。
   * @param[in] col      起始列号（1-based）。
   * @param[in] end_ln   结束行号（1-based），若为0则等于 `ln`。
   * @param[in] end_col  结束列号（1-based），若为0则等于 `col`。
   *
   * @example
   *   // 创建指向单个位置的 SourceLocation
   *   SourceLocation loc("file.cpp", 10, 5);
   *
   *   // 创建跨越范围的 SourceLocation
   *   SourceLocation range("file.cpp", 10, 5, 10, 15);
   */
  SourceLocation(const std::string &file = "<stdin>", size_t ln = 1,
                 size_t col = 1, size_t end_ln = 0, size_t end_col = 0)
      : filename(file), line(ln), column(col), end_line(end_ln ? end_ln : ln),
        end_column(end_col ? end_col : col) {}
};

} // namespace utils
} // namespace czc

#endif // CZC_SOURCE_LOCATION_HPP
