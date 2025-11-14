/**
 * @file error_collector.hpp
 * @brief 定义了用于收集词法分析错误的 `LexerError` 和 `LexErrorCollector`。
 * @details 使用统一的模块错误收集器,减少代码重复并提高可维护性
 * @author BegoniaHe
 * @date 2025-11-13
 */

#ifndef CZC_LEX_ERROR_COLLECTOR_HPP
#define CZC_LEX_ERROR_COLLECTOR_HPP

#include "czc/utils/module_error_collector.hpp"

namespace czc::lexer {

/**
 * @brief Lexer错误类型
 * @details 使用统一的模块错误类型,保持兼容性
 */
using LexerError = utils::ModuleError;

/**
 * @brief Lexer错误收集器
 * @details
 *   使用统一的模块错误收集器,提供一致的错误处理机制。
 *   延迟错误报告机制使得编译器能够一次性分析完整个文件,
 *   并向用户报告所有检测到的词法问题。
 *
 * @property {线程安全} 非线程安全。应在单个词法分析线程中使用。
 */
using LexErrorCollector = utils::ModuleErrorCollector;

} // namespace czc::lexer

#endif // CZC_LEX_ERROR_COLLECTOR_HPP
