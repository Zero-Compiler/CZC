/**
 * @file file_collector.cpp
 * @brief 文件收集器功能实现。
 * @author BegoniaHe
 * @date 2025-11-11
 */

#include "czc/utils/file_collector.hpp"

#include <algorithm>
#include <filesystem>

namespace czc::utils {

std::vector<std::string>
FileCollector::collect_files(const std::vector<std::string>& patterns) {
  std::vector<std::string> files_to_process;

  for (const auto& arg : patterns) {
    // 检查参数是否包含通配符
    if (arg.find('*') != std::string::npos ||
        arg.find('?') != std::string::npos) {
      // --- 处理通配符模式 ---
      std::filesystem::path pattern_path(arg);
      std::filesystem::path parent_path = pattern_path.parent_path();
      std::string pattern = pattern_path.filename().string();

      // 如果路径中没有目录部分（例如 "*.txt"），则默认为当前目录
      if (parent_path.empty()) {
        parent_path = ".";
      }

      // 遍历指定目录下的所有条目
      if (std::filesystem::exists(parent_path) &&
          std::filesystem::is_directory(parent_path)) {
        try {
          for (const auto& entry :
               std::filesystem::directory_iterator(parent_path)) {
            // 只对常规文件进行模式匹配，忽略目录等
            if (entry.is_regular_file()) {
              std::string filename = entry.path().filename().string();
              if (matches_pattern(filename, pattern)) {
                files_to_process.push_back(entry.path().string());
              }
            }
          }
        } catch (const std::filesystem::filesystem_error& e) {
          // 目录遍历错误（权限不足、I/O错误等），跳过该目录
          // 可以考虑记录警告日志
          continue;
        }
      }
    } else {
      // --- 处理具体文件路径 ---
      // 如果不包含通配符，则假定它是一个具体的文件路径。
      // NOTE(BegoniaHe): 这里不检查文件是否存在。将这个责任留给调用者，
      // 这样更灵活，因为调用者可能想在文件不存在时报告更具体的错误。
      files_to_process.push_back(arg);
    }
  }

  // 对结果进行排序，以确保每次运行都有一致和可预测的输出顺序
  std::sort(files_to_process.begin(), files_to_process.end());

  return files_to_process;
}

bool FileCollector::matches_pattern(const std::string& filename,
                                    const std::string& pattern) {
  // 边界检查：空模式和空文件名的处理
  if (pattern.empty()) {
    return filename.empty();
  }
  if (filename.empty()) {
    // 空文件名只能匹配全是 '*' 的模式
    return pattern.find_first_not_of('*') == std::string::npos;
  }

  size_t p_idx = 0;
  size_t f_idx = 0;
  // NOTE(BegoniaHe): 这是一个经典的通配符匹配算法，支持 '*' 和 '?'。
  // 核心思想是使用回溯：当遇到 '*' 时，我们记录下它的位置以及
  // 对应的文件名位置。如果后续匹配失败，我们可以回退到这个 '*'
  // 的位置，让它多匹配一个字符，然后从新位置继续尝试。

  // star_p_idx 记录最近遇到的 '*' 在模式中的位置
  size_t star_p_idx = std::string::npos;
  // star_f_idx 记录当遇到 '*' 时，文件名字符串的匹配位置
  size_t star_f_idx = std::string::npos;

  while (f_idx < filename.length()) {
    // --- 字符匹配或遇到 '?' ---
    if (p_idx < pattern.length() &&
        (pattern[p_idx] == '?' || pattern[p_idx] == filename[f_idx])) {
      p_idx++;
      f_idx++;
    }
    // --- 遇到 '*' 通配符 ---
    else if (p_idx < pattern.length() && pattern[p_idx] == '*') {
      // 记录 '*' 的位置，并准备让它匹配 0 个字符
      star_p_idx = p_idx;
      star_f_idx = f_idx;
      p_idx++; // 移动模式指针，跳过 '*'
    }
    // --- 匹配失败，尝试回溯 ---
    else if (star_p_idx != std::string::npos) {
      // 当前字符不匹配，但我们之前遇到过 '*'。
      // 这时回溯到上一个 '*' 的状态。
      p_idx = star_p_idx + 1; // 模式指针回到 '*' 的下一个字符
      star_f_idx++;           // 让 '*' 多匹配一个字符
      f_idx = star_f_idx;     // 文件名指针也相应移动
    }
    // --- 无法匹配且无法回溯 ---
    else {
      return false;
    }
  }

  // --- 文件名已遍历完，检查模式剩余部分 ---
  // 如果模式还有剩余字符，只有当它们都是 '*' 时才算匹配成功
  while (p_idx < pattern.length() && pattern[p_idx] == '*') {
    p_idx++;
  }

  // 最终，只有当模式也完全匹配到末尾时，才算成功
  return p_idx == pattern.length();
}

} // namespace czc::utils
