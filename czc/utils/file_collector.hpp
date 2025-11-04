/**
 * @file file_collector.hpp
 * @brief 文件收集器类定义
 * @author BegoniaHe
 * @date 2025-11-04
 */

#ifndef CZC_FILE_COLLECTOR_HPP
#define CZC_FILE_COLLECTOR_HPP

#include <string>
#include <vector>

/**
 * @brief 提供用于根据通配符模式查找和收集文件的静态方法。
 * @details
 *   此类主要用于命令行接口，以解析用户输入的文件路径，这些路径
 *   可能包含通配符（如 `*` 和 `?`）。它将这些模式扩展为匹配的
 *   具体文件列表。
 */
class FileCollector
{
public:
  /**
   * @brief 根据一组文件路径或通配符模式收集所有匹配的文件。
   * @param[in] patterns
   *   一个包含文件路径或模式的字符串向量。
   *   模式可以包含 `*`（匹配任意数量的字符）和 `?`（匹配单个字符）。
   * @return 返回一个包含所有匹配的、唯一的、按字母顺序排序的文件路径的向量。
   */
  static std::vector<std::string> collect_files(const std::vector<std::string> &patterns);

private:
  /**
   * @brief 将给定的文件名与单个通配符模式进行匹配。
   * @param[in] filename 要检查的文件名。
   * @param[in] pattern 包含 `*` 和/或 `?` 的通配符模式。
   * @return 如果文件名与模式匹配，则返回 true。
   */
  static bool matches_pattern(const std::string &filename, const std::string &pattern);
};

#endif // CZC_FILE_COLLECTOR_HPP
