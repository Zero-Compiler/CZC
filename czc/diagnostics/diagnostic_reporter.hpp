/**
 * @file diagnostic_reporter.hpp
 * @brief 定义了 `IDiagnosticReporter` 接口，用于报告诊断事件。
 * @author BegoniaHe
 * @date 2025-11-11
 */

#ifndef CZC_DIAGNOSTIC_REPORTER_HPP
#define CZC_DIAGNOSTIC_REPORTER_HPP

#include <memory>

namespace czc {
namespace diagnostics {

class Diagnostic;

/**
 * @brief 定义诊断报告器的抽象接口。
 * @details
 *   此接口定义了一个契约，允许编译器的不同组件（如词法分析器、解析器）
 *   以统一的方式报告诊断事件，而无需关心这些事件最终如何被处理、存储或显示。
 *   实现此接口的类（如 `DiagnosticEngine`）将负责具体的诊断处理逻辑。
 */
class IDiagnosticReporter {
public:
  /**
   * @brief 虚析构函数，确保派生类能够被正确销毁。
   */
  virtual ~IDiagnosticReporter() = default;

  /**
   * @brief 报告一个诊断事件。
   * @details
   *   实现者应接管 `diag` 对象的所有权，并根据其内容进行处理，
   *   例如存储、计数或立即显示。
   * @param[in] diag
   *   一个指向诊断对象的共享指针，包含了事件的全部信息。
   */
  virtual void report(std::shared_ptr<Diagnostic> diag) = 0;

  /**
   * @brief 检查是否已报告任何错误级别的诊断。
   * @details
   *   此方法用于快速判断编译过程是否已失败，以便提前终止后续步骤。
   * @return 如果自报告器创建以来，至少报告了一个错误，则返回 `true`；
   *         否则返回 `false`。
   */
  virtual bool has_errors() const = 0;
};

} // namespace diagnostics
} // namespace czc

#endif // CZC_DIAGNOSTIC_REPORTER_HPP
