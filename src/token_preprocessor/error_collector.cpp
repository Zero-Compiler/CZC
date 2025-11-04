/**
 * @file error_collector.cpp
 * @brief 词元预处理器错误收集器实现
 * @author BegoniaHe
 * @date 2025-11-04
 */

#include "czc/token_preprocessor/error_collector.hpp"

/**
 * @brief 添加一个诊断错误
 * @param code 诊断代码
 * @param loc 源码位置
 * @param args 格式化参数
 */
void TPErrorCollector::add(DiagnosticCode code, const SourceLocation &loc,
                           const std::vector<std::string> &args) {
  // 使用 emplace_back 直接在 vector 中构造 TPError 对象，
  // 这样比先创建临时对象再 push_back 更高效。
  errors.emplace_back(code, loc, args);
}