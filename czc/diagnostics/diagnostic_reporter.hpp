/**
 * @file diagnostic_reporter.hpp
 * @brief 诊断报告器接口定义
 * @author BegoniaHe
 * @date 2025-11-04
 */

#ifndef CZC_DIAGNOSTIC_REPORTER_HPP
#define CZC_DIAGNOSTIC_REPORTER_HPP

#include <memory>

class Diagnostic;

/**
 * @brief 定义了诊断报告器的抽象接口。
 * @details
 *   任何希望接收和处理诊断信息的类（例如 DiagnosticEngine）都应实现此接口。
 *   它提供了一种解耦机制，使得编译器组件（如词法分析器、解析器）
 *   可以报告诊断，而无需了解其最终如何被存储或显示。
 */
class IDiagnosticReporter {
public:
  /**
   * @brief 虚析构函数。
   */
  virtual ~IDiagnosticReporter() = default;

  /**
   * @brief 提交一个诊断事件以供处理。
   * @param[in] diag 一个包含诊断详细信息的共享指针。
   */
  virtual void report(std::shared_ptr<Diagnostic> diag) = 0;

  /**
   * @brief 查询报告器是否已接收到任何错误级别的诊断。
   * @return 如果至少报告了一个错误，则返回 true；否则返回 false。
   */
  virtual bool has_errors() const = 0;
};

#endif // CZC_DIAGNOSTIC_REPORTER_HPP
