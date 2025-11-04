/**
 * @file diagnostic_reporter.hpp
 * @brief 诊断报告器接口定义
 * @author BegoniaHe
 */

#ifndef CZC_DIAGNOSTIC_REPORTER_HPP
#define CZC_DIAGNOSTIC_REPORTER_HPP

#include <memory>

class Diagnostic;

/**
 * @brief 诊断报告器接口
 */
class IDiagnosticReporter
{
public:
    virtual ~IDiagnosticReporter() = default;

    /**
     * @brief 报告诊断信息
     * @param diag 诊断信息智能指针
     */
    virtual void report(std::shared_ptr<Diagnostic> diag) = 0;

    /**
     * @brief 检查是否有错误
     * @return 如果有错误返回 true，否则返回 false
     */
    virtual bool has_errors() const = 0;
};

#endif // CZC_DIAGNOSTIC_REPORTER_HPP
