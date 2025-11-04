/**
 * @file file_collector.cpp
 * @brief 文件收集器实现
 * @author BegoniaHe
 * @date 2025-11-04
 */

#include "czc/utils/file_collector.hpp"
#include <filesystem>
#include <algorithm>

/**
 * @brief 根据模式收集文件
 * @param patterns 文件模式向量 (支持 * 和 ? 通配符)
 * @return 收集到的文件路径向量
 */
std::vector<std::string> FileCollector::collect_files(const std::vector<std::string> &patterns)
{
    std::vector<std::string> files_to_process;

    for (const auto &arg : patterns)
    {
        // 检查参数是否包含通配符。
        if (arg.find('*') != std::string::npos || arg.find('?') != std::string::npos)
        {
            // 如果包含通配符，则执行模式匹配。
            std::filesystem::path pattern_path(arg);
            std::filesystem::path parent_path = pattern_path.parent_path();
            std::string pattern = pattern_path.filename().string();

            // 如果路径中没有目录部分（例如 "*.txt"），则默认为当前目录。
            if (parent_path.empty())
            {
                parent_path = ".";
            }

            // 遍历指定目录下的所有条目。
            if (std::filesystem::exists(parent_path) && std::filesystem::is_directory(parent_path))
            {
                for (const auto &entry : std::filesystem::directory_iterator(parent_path))
                {
                    // 只对常规文件进行模式匹配。
                    if (entry.is_regular_file())
                    {
                        std::string filename = entry.path().filename().string();
                        if (matches_pattern(filename, pattern))
                        {
                            files_to_process.push_back(entry.path().string());
                        }
                    }
                }
            }
        }
        else
        {
            // 如果不包含通配符，则假定它是一个具体的文件路径。
            // 注意：这里不检查文件是否存在，将这个责任留给调用者，
            // 这样更灵活，因为调用者可能想在文件不存在时报告更具体的错误。
            files_to_process.push_back(arg);
        }
    }

    // 对结果进行排序，以确保每次运行都有一致和可预测的输出顺序。
    std::sort(files_to_process.begin(), files_to_process.end());

    return files_to_process;
}

/**
 * @brief 检查文件名是否匹配模式
 * @param filename 文件名
 * @param pattern 模式字符串
 * @return 如果匹配返回 true, 否则返回 false
 */
bool FileCollector::matches_pattern(const std::string &filename, const std::string &pattern)
{
    size_t pattern_idx = 0;
    size_t filename_idx = 0;

    while (pattern_idx < pattern.length() && filename_idx < filename.length())
    {
        if (pattern[pattern_idx] == '*')
        {
            // 这是一个简化的通配符 '*' 实现。它不是一个完全的回溯算法，
            // 对于复杂的模式（如 "a*b*c"）可能行为不正确，但对于常见情况（如 "*.txt"）是有效的。
            // TODO(BegoniaHe): 实现一个更健壮的、支持回溯的通配符匹配算法。

            // 如果 '*' 是模式的最后一个字符，它匹配剩余的所有文件名部分。
            if (pattern_idx == pattern.length() - 1)
            {
                return true;
            }
            // 跳过 '*'，查看模式中的下一个字符。
            pattern_idx++;
            // 在文件名中向前搜索，直到找到与模式中 '*' 之后字符匹配的字符。
            while (filename_idx < filename.length() && filename[filename_idx] != pattern[pattern_idx])
            {
                filename_idx++;
            }
        }
        // '?' 匹配文件名中的任何单个字符。
        else if (pattern[pattern_idx] == '?')
        {
            pattern_idx++;
            filename_idx++;
        }
        // 如果模式字符和文件名字符相同，则两者都向前移动。
        else if (pattern[pattern_idx] == filename[filename_idx])
        {
            pattern_idx++;
            filename_idx++;
        }
        else
        {
            // 不匹配
            return false;
        }
    }

    // 只有当模式和文件名都同时到达末尾时，才算完全匹配。
    return pattern_idx == pattern.length() && filename_idx == filename.length();
}
