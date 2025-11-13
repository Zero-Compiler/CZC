/**
 * @file module_error_collector.hpp
 * @brief 统一的模块错误收集器接口
 * @details 提供跨模块统一的错误收集机制，减少代码重复
 * @author BegoniaHe
 * @date 2025-11-13
 */

#ifndef CZC_UTILS_MODULE_ERROR_COLLECTOR_HPP
#define CZC_UTILS_MODULE_ERROR_COLLECTOR_HPP

#include "czc/diagnostics/diagnostic_code.hpp"
#include "czc/utils/error_collector.hpp"
#include "czc/utils/source_location.hpp"

#include <string>
#include <vector>

namespace czc::utils {

/**
 * @brief 通用模块错误类型
 * @details 使用SourceLocation作为位置信息的标准错误类型
 */
using ModuleError = ErrorInfo<SourceLocation>;

/**
 * @brief 通用模块错误收集器
 * @details 线程不安全的错误收集器，应在单个线程中使用
 * @example
 *   ModuleErrorCollector collector;
 *   collector.add(DiagnosticCode::P0001_UnexpectedToken, location, {"expected",
 * "actual"});
 */
using ModuleErrorCollector = ErrorCollector<ModuleError>;

/**
 * @brief 模块错误助手类
 * @details 提供便捷的错误报告功能
 */
class ModuleErrorHelper {
public:
  /**
   * @brief 构造函数
   * @param collector 错误收集器引用
   * @param module_name 模块名称，用于错误消息前缀
   */
  explicit ModuleErrorHelper(ModuleErrorCollector& collector,
                             const std::string& module_name = "")
      : collector_(collector), module_name_(module_name) {}

  /**
   * @brief 添加错误
   * @param code 诊断代码
   * @param location 源码位置
   * @param args 格式化参数
   */
  void report_error(diagnostics::DiagnosticCode code,
                    const SourceLocation& location,
                    const std::vector<std::string>& args = {}) {
    ModuleError error(code, location, args);
    collector_.add(error);
  }

  /**
   * @brief 添加错误（便捷方法）
   * @param code 诊断代码
   * @param filename 文件名
   * @param line 行号
   * @param column 列号
   * @param args 格式化参数
   */
  void report_error(diagnostics::DiagnosticCode code,
                    const std::string& filename, size_t line, size_t column,
                    const std::vector<std::string>& args = {}) {
    SourceLocation location(filename, line, column);
    report_error(code, location, args);
  }

  /**
   * @brief 检查是否有错误
   * @return 如果有错误返回true
   */
  bool has_errors() const {
    return collector_.has_errors();
  }

  /**
   * @brief 获取错误数量
   * @return 错误数量
   */
  size_t error_count() const {
    return collector_.count();
  }

  /**
   * @brief 清空所有错误
   */
  void clear_errors() {
    collector_.clear();
  }

  /**
   * @brief 获取错误收集器
   * @return 错误收集器引用
   */
  ModuleErrorCollector& get_collector() {
    return collector_;
  }

  /**
   * @brief 获取错误收集器（常量）
   * @return 错误收集器常量引用
   */
  const ModuleErrorCollector& get_collector() const {
    return collector_;
  }

private:
  ModuleErrorCollector& collector_; ///< 错误收集器引用
  std::string module_name_;         ///< 模块名称
};

/**
 * @brief 创建模块错误助手
 * @param collector 错误收集器
 * @param module_name 模块名称
 * @return 模块错误助手
 */
inline ModuleErrorHelper
make_error_helper(ModuleErrorCollector& collector,
                  const std::string& module_name = "") {
  return ModuleErrorHelper(collector, module_name);
}

} // namespace czc::utils

#endif // CZC_UTILS_MODULE_ERROR_COLLECTOR_HPP