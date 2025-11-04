/**
 * @file error_collector.cpp
 * @brief 错误收集器实现
 * @author BegoniaHe
 */

#include "czc/lexer/error_collector.hpp"

void ErrorCollector::add(DiagnosticCode code, const SourceLocation &loc,
                         const std::vector<std::string> &args)
{
    errors.emplace_back(code, loc, args);
}
