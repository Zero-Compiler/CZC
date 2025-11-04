/**
 * @file error_collector.hpp
 * @brief 错误收集器类定义
 * @author BegoniaHe
 */

#ifndef CZC_ERROR_COLLECTOR_HPP
#define CZC_ERROR_COLLECTOR_HPP

#include "czc/diagnostics/diagnostic_code.hpp"
#include "source_location.hpp"
#include <vector>
#include <string>

/**
 * @brief 词法错误记录结构
 */
struct LexerError
{
    DiagnosticCode code;           ///< 诊断代码
    SourceLocation location;       ///< 源码位置
    std::vector<std::string> args; ///< 消息参数

    /**
     * @brief 构造函数
     * @param c 诊断代码
     * @param loc 源码位置
     * @param arguments 消息参数列表
     */
    LexerError(DiagnosticCode c, const SourceLocation &loc,
               const std::vector<std::string> &arguments = {})
        : code(c), location(loc), args(arguments) {}
};

/**
 * @brief 错误收集器类 - 负责收集词法分析过程中的错误
 */
class ErrorCollector
{
private:
    std::vector<LexerError> errors; ///< 错误列表

public:
    /**
     * @brief 添加错误
     * @param code 诊断代码
     * @param loc 源码位置
     * @param args 消息参数列表
     */
    void add(DiagnosticCode code, const SourceLocation &loc,
             const std::vector<std::string> &args = {});

    /**
     * @brief 获取所有错误
     * @return 错误列表的常量引用
     */
    const std::vector<LexerError> &get_errors() const { return errors; }

    /**
     * @brief 检查是否有错误
     * @return 如果有错误返回 true，否则返回 false
     */
    bool has_errors() const { return !errors.empty(); }

    /**
     * @brief 清空错误
     */
    void clear() { errors.clear(); }

    /**
     * @brief 获取错误数量
     * @return 错误数量
     */
    size_t count() const { return errors.size(); }
};

#endif // CZC_ERROR_COLLECTOR_HPP
