/**
 * @file color.hpp
 * @brief 定义了用于终端输出的 ANSI 颜色代码常量。
 * @author BegoniaHe
 * @date 2025-11-11
 */

#ifndef CZC_COLOR_HPP
#define CZC_COLOR_HPP

#include <string>

namespace czc {
namespace diagnostics {

/**
 * @brief 定义用于终端输出的 ANSI 颜色代码常量。
 * @details 这些常量用于在诊断消息中高亮不同严重级别的文本，以增强可读性。
 */
namespace Color {
// 重置所有文本属性为默认值。
const std::string Reset = "\033[0m";
// 设置文本为粗体。
const std::string Bold = "\033[1m";
// 设置文本颜色为红色。
const std::string Red = "\033[31m";
// 设置文本颜色为黄色。
const std::string Yellow = "\033[33m";
// 设置文本颜色为蓝色。
const std::string Blue = "\033[34m";
// 设置文本颜色为青色。
const std::string Cyan = "\033[36m";
} // namespace Color
} // namespace diagnostics
} // namespace czc

#endif // CZC_COLOR_HPP
