/**
 * @file error_collector.hpp
 * @brief 定义了用于收集 Token 预处理阶段错误的 `TPError` 和
 * `TPErrorCollector`。
 * @details 使用统一的模块错误收集器,减少代码重复并提高可维护性
 * @author BegoniaHe
 * @date 2025-11-13
 */

#ifndef CZC_TP_ERROR_COLLECTOR_HPP
#define CZC_TP_ERROR_COLLECTOR_HPP

#include "czc/utils/module_error_collector.hpp"

namespace czc::token_preprocessor {

/**
 * @brief Token预处理错误类型
 * @details 使用统一的模块错误类型,保持兼容性
 */
using TPError = utils::ModuleError;

/**
 * @brief Token预处理错误收集器
 * @details
 *   使用统一的模块错误收集器,提供一致的错误处理机制。
 *   该类为 `TokenPreprocessor` 提供了一个统一的错误记录机制。
 *   当检测到如数值溢出等问题时,预处理器会通过 `add` 方法记录错误,
 *   然后继续处理下一个 Token。这种**延迟错误报告**的设计允许一次性向用户
 *   展示所有在预处理阶段发现的问题。
 *
 * @property {线程安全} 非线程安全。
 */
using TPErrorCollector = utils::ModuleErrorCollector;

} // namespace czc::token_preprocessor

#endif // CZC_TP_ERROR_COLLECTOR_HPP
