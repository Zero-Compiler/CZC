/**
 * @file source_tracker.hpp
 * @brief 定义了 `SourceTracker` 类，用于管理和跟踪源代码的读取位置。
 * @author BegoniaHe
 * @date 2025-11-11
 */

#ifndef CZC_SOURCE_TRACKER_HPP
#define CZC_SOURCE_TRACKER_HPP

#include "czc/utils/source_location.hpp"

#include <string>
#include <vector>

namespace czc::utils {

/**
 * @brief 管理源代码文本并精确跟踪当前的扫描位置。
 * @details
 * 此类是词法分析器（Lexer）的状态管理核心。它封装了对源文件内容的访问，
 *          并维护一个内部指针来跟踪当前正在处理的字节位置 (`position`)、行号
 * (`line`) 和列号 (`column`)。通过将位置跟踪逻辑从词法分析器中分离出来， 使得
 * Lexer 本身可以更专注于 Token 的识别规则。
 *
 *          `SourceTracker` 的主要职责包括：
 *          1. 提供对源码字符的顺序访问。
 *          2. 在每次 `advance`
 * 调用时，根据遇到的字符（特别是换行符）正确更新行号和列号。
 *          3. 能够根据起始和结束位置，快速创建用于错误报告的 `SourceLocation`
 * 对象。
 *
 * @property {线程安全} 非线程安全。此类是有状态的，不可在多线程间共享。
 */
class SourceTracker {
private:
  // 正在处理的源文件的名称，用于生成 `SourceLocation`
  std::string filename;
  // 源文件的完整内容，存储为字符向量以便高效索引
  std::vector<char> input;
  // 当前在 `input` 向量中的字节索引，范围: [0, input.size()]
  size_t position;
  // 当前位置对应的行号（从 1 开始计数）
  size_t line;
  // 当前位置在当前行中的列号（从 1 开始计数）
  size_t column;

  // --- 性能优化: 行索引缓存 ---
  // NOTE(BegoniaHe): 为了优化 `get_source_line`
  // 的性能，我们使用惰性初始化的行索引表。 line_offsets[i] 存储第 i+1
  // 行的起始字节位置（行号从 1 开始）。 例如: line_offsets[0] = 0 (第 1
  // 行从字节 0 开始)
  //      line_offsets[1] = 15 (第 2 行从字节 15 开始，假设第 1 行有 14 个字符 +
  //      '\n')
  // 使用 mutable 允许在 const 方法中构建索引（惰性初始化）。
  mutable std::vector<size_t> line_offsets;
  mutable bool line_offsets_built = false;

  /**
   * @brief 惰性构建行起始位置索引表。
   * @details 此方法在首次调用 `get_source_line` 时执行。它遍历整个输入一次，
   *          记录每一行的起始字节位置。后续的 `get_source_line` 调用将直接使用
   *          这个索引表，实现 O(1) 的行查找性能。
   *
   *          复杂度分析:
   *          - 构建索引: O(n)，其中 n 是源文件的字节数
   *          - 查找行号: O(1)
   *          - 总体复杂度（对于 m 个错误）: O(n + m) vs 原先的 O(n × m)
   *
   * @note 使用 mutable 和 const 标记允许在 const 方法中缓存数据，
   *       这是一种常见的惰性初始化模式。
   */
  void build_line_offsets() const;

public:
  /**
   * @brief 构造一个新的 SourceTracker。
   * @param[in] source 要跟踪的源代码字符串。
   * @param[in] fname  (可选) 源代码的文件名，用于创建 SourceLocation。
   */
  SourceTracker(const std::string& source,
                const std::string& fname = "<stdin>");

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
  size_t get_position() const {
    return position;
  }

  /**
   * @brief 获取当前行号。
   * @return 返回当前位置的行号（从 1 开始）。
   */
  size_t get_line() const {
    return line;
  }

  /**
   * @brief 获取当前列号。
   * @return 返回当前位置的列号（从 1 开始）。
   */
  size_t get_column() const {
    return column;
  }

  /**
   * @brief 获取源文件名。
   * @return 返回对源文件名的常量引用。
   */
  const std::string& get_filename() const {
    return filename;
  }

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
  const std::vector<char>& get_input() const {
    return input;
  }
};

} // namespace czc::utils

#endif // CZC_SOURCE_TRACKER_HPP
