/**
 * @file source_tracker.hpp
 * @brief 源码跟踪器类定义
 * @author BegoniaHe
 * @date 2025-11-04
 */

#ifndef CZC_SOURCE_TRACKER_HPP
#define CZC_SOURCE_TRACKER_HPP

#include "czc/utils/source_location.hpp"
#include <string>
#include <vector>

namespace czc {
namespace utils {

/**
 * @brief 管理源代码文本并跟踪当前解析位置。
 * @details
 *   此类是词法分析器的核心辅助工具。它持有整个源文件的内容，并维护一个
 *   指向当前处理位置的指针，同时精确跟踪当前的行号和列号。
 *   这使得词法分析器可以逐字符地遍历输入，并在需要时轻松地创建
 *   精确的 SourceLocation 对象。
 * @property {线程安全} 非线程安全。此类是有状态的，不可在多线程间共享。
 */
class SourceTracker {
private:
  // 正在处理的源文件的名称。
  std::string filename;
  // 源文件的完整内容，存储为字符向量以便高效索引。
  std::vector<char> input;
  // 当前在 `input` 向量中的字节索引。
  size_t position;
  // 当前位置对应的行号（从 1 开始计数）。
  size_t line;
  // 当前位置在当前行中的列号（从 1 开始计数）。
  size_t column;

public:
  /**
   * @brief 构造一个新的 SourceTracker。
   * @param[in] source 要跟踪的源代码字符串。
   * @param[in] fname  (可选) 源代码的文件名，用于创建 SourceLocation。
   */
  SourceTracker(const std::string &source,
                const std::string &fname = "<stdin>");

  /**
   * @brief 向前移动一个字符，并根据字符内容更新行号和列号。
   * @details 这是推进词法分析器状态的核心方法。
   * @param[in] c 刚刚消耗的字符。此方法会检查它是否为换行符以更新行号。
   */
  void advance(char c);

  /**
   * @brief 获取当前在输入中的字节位置。
   * @return 返回当前位置的字节索引。
   */
  size_t get_position() const { return position; }

  /**
   * @brief 获取当前行号。
   * @return 返回当前位置的行号（从 1 开始）。
   */
  size_t get_line() const { return line; }

  /**
   * @brief 获取当前列号。
   * @return 返回当前位置的列号（从 1 开始）。
   */
  size_t get_column() const { return column; }

  /**
   * @brief 获取源文件名。
   * @return 返回对源文件名的常量引用。
   */
  const std::string &get_filename() const { return filename; }

  /**
   * @brief 创建一个从指定起始点到当前位置的 SourceLocation。
   * @param[in] start_line 区域的起始行号。
   * @param[in] start_col  区域的起始列号。
   * @return 返回一个表示该代码区域的 SourceLocation 对象。
   */
  SourceLocation make_location(size_t start_line, size_t start_col) const;

  /**
   * @brief 提取并返回指定行的源代码文本。
   * @param[in] line_num 要提取的行号（从 1 开始）。
   * @return 返回该行的文本内容。如果行号无效，则返回空字符串。
   */
  std::string get_source_line(size_t line_num) const;

  /**
   * @brief 获取对整个输入源文本的只读访问权限。
   * @return 返回对内部字符向量的常量引用。
   */
  const std::vector<char> &get_input() const { return input; }
};

} // namespace utils
} // namespace czc

#endif // CZC_SOURCE_TRACKER_HPP
