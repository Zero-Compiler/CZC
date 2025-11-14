/**
 * @file error_collector.hpp
 * @brief 定义了用于收集语法分析错误的 `ParserError` 和 `ParserErrorCollector`。
 * @details 使用统一的模块错误收集器，减少代码重复并提高可维护性
 * @author BegoniaHe
 * @date 2025-11-13
 */

#ifndef CZC_PARSER_ERROR_COLLECTOR_HPP
#define CZC_PARSER_ERROR_COLLECTOR_HPP

#include "czc/utils/module_error_collector.hpp"

namespace czc::parser {

/**
 * @brief Parser错误类型
 * @details 使用统一的模块错误类型，保持兼容性
 */
using ParserError = utils::ModuleError;

/**
 * @brief Parser错误收集器
 * @details 使用统一的模块错误收集器，提供一致的错误处理机制
 */
using ParserErrorCollector = utils::ModuleErrorCollector;

/**
 * @brief Parser错误助手类型
 * @details 提供便捷的错误报告功能
 */
using ParserErrorHelper = utils::ModuleErrorHelper;

} // namespace czc::parser

#endif // CZC_PARSER_ERROR_COLLECTOR_HPP
