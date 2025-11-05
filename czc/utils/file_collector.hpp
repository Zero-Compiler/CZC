/**
 * @file file_collector.hpp
 * @brief 提供基于通配符模式的文件收集功能。
 * @author BegoniaHe
 * @date 2025-11-04
 */

#ifndef CZC_FILE_COLLECTOR_HPP
#define CZC_FILE_COLLECTOR_HPP

#include <string>
#include <vector>

namespace czc {
namespace utils {

/**
 * @brief 提供根据通配符模式查找和收集文件的静态工具方法。
 * @details
 *   此类主要用于命令行接口（CLI），以解析用户输入的文件路径，
 *   这些路径可能包含通配符（如 `*` 和 `?`）。它负责将这些模式
 *   扩展为匹配的具体文件列表。
 * @property {线程安全} 此类仅包含静态方法，不维护状态，因此是线程安全的。
 * @example
 *   std::vector<std::string> patterns = {"src/ *.cpp", "include/ *.h"};
 *   std::vector<std::string> files =
 * czc::utils::FileCollector::collect_files(patterns);
 */
class FileCollector {
public:
  /**
   * @brief 根据一组文件路径或通配符模式收集所有匹配的文件。
   * @param[in] patterns
   *   一个包含文件路径或模式的字符串向量。模式可以包含 `*`
   *   （匹配任意数量的字符）和 `?`（匹配单个字符）。
   * @return
   *   返回一个包含所有匹配的、按字母顺序排序的文件路径的向量。
   *   如果模式匹配了重复的文件，结果中可能包含重复项。
   *   如果没有找到匹配的文件，则返回空向量。
   */
  static std::vector<std::string>
  collect_files(const std::vector<std::string> &patterns);

private:
  /**
   * @brief 将给定的文件名与单个通配符模式进行匹配。
   * @param[in] filename 要检查的文件名。
   * @param[in] pattern 包含 `*` 和/或 `?` 的通配符模式。
   * @return 如果文件名与模式匹配，则返回 `true`，否则返回 `false`。
   */
  static bool matches_pattern(const std::string &filename,
                              const std::string &pattern);
};

} // namespace utils
} // namespace czc

#endif // CZC_FILE_COLLECTOR_HPP
