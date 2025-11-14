/**
 * @file error_collector.hpp
 * @brief 定义了用于收集 CST 构建阶段错误的 `CSTError` 和 `CSTErrorCollector`。
 * @details 使用统一的模块错误收集器,减少代码重复并提高可维护性
 * @author BegoniaHe
 * @date 2025-11-13
 */

#ifndef CZC_CST_ERROR_COLLECTOR_HPP
#define CZC_CST_ERROR_COLLECTOR_HPP

#include "czc/utils/module_error_collector.hpp"

namespace czc::cst {

/**
 * @brief CST错误类型
 * @details 使用统一的模块错误类型,保持兼容性
 */
using CSTError = utils::ModuleError;

/**
 * @brief CST错误收集器
 * @details
 *   使用统一的模块错误收集器,提供一致的错误处理机制。
 *   该类的核心设计思想是**延迟错误报告**。在 CST 构建过程中,
 *   当遇到错误时,并不会立即中断流程,而是通过 `add` 方法将错误记录下来。
 *   这种机制使得编译器能够一次性分析完整个文件,并向用户报告所有检测到的语法问题,
 *   极大地提升了开发效率。
 *
 * @property {线程安全} 非线程安全。应在单个解析线程中使用。
 */
using CSTErrorCollector = utils::ModuleErrorCollector;

} // namespace czc::cst

#endif // CZC_CST_ERROR_COLLECTOR_HPP
