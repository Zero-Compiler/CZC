/**
 * @file error_collector.cpp
 * @brief 词元预处理器错误收集器实现
 * @author BegoniaHe
 */

#include "czc/token_preprocessor/error_collector.hpp"

/**
 * @brief 添加一个诊断错误
 * @param code 诊断代码
 * @param loc 源码位置
 * @param args 格式化参数
 */
void TPErrorCollector::add(DiagnosticCode code, const SourceLocation &loc,
                            const std::vector<std::string> &args)
{
    errors.emplace_back(code, loc, args);
}