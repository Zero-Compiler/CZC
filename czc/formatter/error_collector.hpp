/**
 * @file error_collector.hpp
 * @brief 定义了 `FormatterErrorCollector` 类，用于收集格式化过程中的错误。
 * @author BegoniaHe
 * @date 2025-11-11
 */

#ifndef CZC_FORMATTER_ERROR_COLLECTOR_HPP
#define CZC_FORMATTER_ERROR_COLLECTOR_HPP

#include "czc/diagnostics/diagnostic.hpp"
#include "czc/diagnostics/diagnostic_code.hpp"
#include "czc/utils/source_location.hpp"

#include <string>
#include <vector>

namespace czc {
namespace formatter {

/**
 * @struct FormatterError
 * @brief 表示格式化过程中的单个错误。
 */
struct FormatterError {
  diagnostics::DiagnosticCode code; ///< 错误代码。
  utils::SourceLocation location;   ///< 错误位置。
  std::vector<std::string> args;    ///< 错误参数。

  /**
   * @brief 构造函数。
   * @param[in] c 错误代码。
   * @param[in] loc 错误位置。
   * @param[in] a 错误参数。
   */
  FormatterError(diagnostics::DiagnosticCode c,
                 const utils::SourceLocation& loc,
                 const std::vector<std::string>& a = {})
      : code(c), location(loc), args(a) {}
};

/**
 * @class FormatterErrorCollector
 * @brief 收集格式化过程中的错误。
 */
class FormatterErrorCollector {
public:
  /**
   * @brief 添加一个错误。
   * @param[in] code 错误代码。
   * @param[in] loc 错误位置。
   * @param[in] args 错误参数（可选）。
   */
  void add(diagnostics::DiagnosticCode code, const utils::SourceLocation& loc,
           const std::vector<std::string>& args = {});

  /**
   * @brief 获取所有错误。
   * @return 错误列表的常量引用。
   */
  const std::vector<FormatterError>& get_errors() const {
    return errors;
  }

  /**
   * @brief 检查是否有错误。
   * @return 如果有错误返回 true，否则返回 false。
   */
  bool has_errors() const {
    return !errors.empty();
  }

  /**
   * @brief 清空所有错误。
   */
  void clear() {
    errors.clear();
  }

private:
  std::vector<FormatterError> errors; ///< 错误列表。
};

} // namespace formatter
} // namespace czc

#endif // CZC_FORMATTER_ERROR_COLLECTOR_HPP
