/**
 * @file file_collector.hpp
 * @brief 文件收集器类定义
 * @author BegoniaHe
 */

#ifndef CZC_FILE_COLLECTOR_HPP
#define CZC_FILE_COLLECTOR_HPP

#include <string>
#include <vector>

/**
 * @brief 文件收集器类 - 负责收集和匹配文件
 */
class FileCollector
{
public:
    /**
     * @brief 从模式列表中收集文件
     * @param patterns 文件路径或通配符模式列表（支持 * 和 ?）
     * @return 匹配的文件路径列表（已排序）
     */
    static std::vector<std::string> collect_files(const std::vector<std::string> &patterns);

private:
    /**
     * @brief 检查文件名是否匹配通配符模式
     * @param filename 文件名
     * @param pattern 通配符模式
     * @return 如果匹配返回 true，否则返回 false
     */
    static bool matches_pattern(const std::string &filename, const std::string &pattern);
};

#endif // CZC_FILE_COLLECTOR_HPP
