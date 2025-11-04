/**
 * @file file_collector.cpp
 * @brief 文件收集器实现
 * @author BegoniaHe
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
        // Check for wildcard characters
        if (arg.find('*') != std::string::npos || arg.find('?') != std::string::npos)
        {
            std::filesystem::path pattern_path(arg);
            std::filesystem::path parent_path = pattern_path.parent_path();
            std::string pattern = pattern_path.filename().string();

            if (parent_path.empty())
            {
                parent_path = ".";
            }

            if (std::filesystem::exists(parent_path) && std::filesystem::is_directory(parent_path))
            {
                for (const auto &entry : std::filesystem::directory_iterator(parent_path))
                {
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
            // 直接添加文件路径（不检查是否存在，让调用方处理）
            files_to_process.push_back(arg);
        }
    }

    // 排序文件列表
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
            // Handle * wildcard
            if (pattern_idx == pattern.length() - 1)
            {
                // * at end matches everything
                return true;
            }
            pattern_idx++;
            // Find next matching character
            while (filename_idx < filename.length() && filename[filename_idx] != pattern[pattern_idx])
            {
                filename_idx++;
            }
        }
        else if (pattern[pattern_idx] == '?')
        {
            // ? matches any single character
            pattern_idx++;
            filename_idx++;
        }
        else if (pattern[pattern_idx] == filename[filename_idx])
        {
            // Exact match
            pattern_idx++;
            filename_idx++;
        }
        else
        {
            // No match
            return false;
        }
    }

    // Both must be fully consumed for a match
    return pattern_idx == pattern.length() && filename_idx == filename.length();
}
