/**
 * @file color.hpp
 * @brief ANSI 颜色代码定义
 * @author BegoniaHe
 */

#ifndef CZC_COLOR_HPP
#define CZC_COLOR_HPP

#include <string>

/**
 * @brief CZC 编译器命名空间
 */
namespace czc
{
    /**
     * @brief 诊断系统命名空间
     */
    namespace diagnostics
    {
        /**
         * @brief ANSI 颜色代码
         */
        namespace Color
        {
            const std::string Reset = "\033[0m";   ///< 重置颜色
            const std::string Bold = "\033[1m";    ///< 粗体
            const std::string Red = "\033[31m";    ///< 红色
            const std::string Yellow = "\033[33m"; ///< 黄色
            const std::string Blue = "\033[34m";   ///< 蓝色
            const std::string Cyan = "\033[36m";   ///< 青色
        }
    }
}

#endif // CZC_COLOR_HPP
