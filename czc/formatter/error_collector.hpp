/**
 * @file error_collector.hpp
 * @brief 定义了用于收集格式化阶段错误的 `FormatterError` 和
 * `FormatterErrorCollector`。
 * @details 使用统一的模块错误收集器,减少代码重复并提高可维护性
 * @author BegoniaHe
 * @date 2025-11-13
 */

#ifndef CZC_FORMATTER_ERROR_COLLECTOR_HPP
#define CZC_FORMATTER_ERROR_COLLECTOR_HPP

#include "czc/utils/module_error_collector.hpp"

namespace czc::formatter {

/**
 * @brief Formatter错误类型
 * @details 使用统一的模块错误类型,保持兼容性
 */
using FormatterError = utils::ModuleError;

/**
 * @brief Formatter错误收集器
 * @details
 *   使用统一的模块错误收集器,提供一致的错误处理机制。
 *   该类为格式化器提供了一个统一的错误记录机制。
 *
 * @property {线程安全} 非线程安全。应在单个格式化线程中使用。
 */
using FormatterErrorCollector = utils::ModuleErrorCollector;

} // namespace czc::formatter

#endif // CZC_FORMATTER_ERROR_COLLECTOR_HPP
