/**
 * @file error_collector.hpp
 * @brief 通用错误收集器模板，用于统一各模块的错误收集逻辑。
 * @author BegoniaHe
 * @date 2025-11-11
 */

#ifndef CZC_UTILS_ERROR_COLLECTOR_HPP
#define CZC_UTILS_ERROR_COLLECTOR_HPP

#include "czc/diagnostics/diagnostic_code.hpp"
#include "czc/utils/source_tracker.hpp"

#include <string>
#include <vector>

namespace czc::utils {

/**
 * @brief 通用错误信息结构体模板。
 * @tparam LocationType 位置类型（通常是 SourceLocation）
 * @details
 *   该模板定义了一个通用的错误信息结构，可以在不同的编译阶段
 *   （词法分析、语法分析、语义分析等）复用，避免重复代码。
 */
template <typename LocationType = SourceLocation>
struct ErrorInfo {
  using LocationType_t = LocationType; ///< 位置类型别名

  diagnostics::DiagnosticCode code; ///< 错误的诊断代码
  LocationType location;            ///< 错误发生的位置
  std::vector<std::string> args;    ///< 用于格式化错误消息的参数

  /**
   * @brief 构造错误信息。
   * @param[in] code 诊断代码
   * @param[in] loc 源码位置
   * @param[in] args 格式化参数
   */
  ErrorInfo(diagnostics::DiagnosticCode code, const LocationType& loc,
            const std::vector<std::string>& args = {})
      : code(code), location(loc), args(args) {}
};

/**
 * @brief 通用错误收集器模板类。
 * @tparam ErrorType 错误类型（通常是 ErrorInfo<SourceLocation>）
 * @details
 *   该模板类封装了错误收集的通用逻辑，包括添加、查询、清空等操作。
 *   通过模板参数化，可以适应不同模块的错误类型需求，同时保持代码
 *   的一致性和可维护性。
 *
 * @example
 *   using LexerError = ErrorInfo<SourceLocation>;
 *   ErrorCollector<LexerError> collector;
 *   collector.add(DiagnosticCode::L0001_InvalidCharacter, loc, {"@"});
 */
template <typename ErrorType>
class ErrorCollector {
public:
  /**
   * @brief 添加一个错误到收集器中。
   * @param[in] code 错误的诊断代码
   * @param[in] location 错误位置
   * @param[in] args 用于格式化错误消息的参数
   */
  void add(diagnostics::DiagnosticCode code,
           const typename ErrorType::LocationType_t& location,
           const std::vector<std::string>& args = {}) {
    errors_.emplace_back(code, location, args);
  }

  /**
   * @brief 直接添加一个已构造的错误对象。
   * @param[in] error 错误对象
   */
  void add(const ErrorType& error) {
    errors_.push_back(error);
  }

  /**
   * @brief 获取所有收集到的错误。
   * @return 错误列表的常量引用
   */
  const std::vector<ErrorType>& get_errors() const {
    return errors_;
  }

  /**
   * @brief 检查是否有错误。
   * @return 如果有错误返回 true，否则返回 false
   */
  bool has_errors() const {
    return !errors_.empty();
  }

  /**
   * @brief 清空所有错误。
   */
  void clear() {
    errors_.clear();
  }

  /**
   * @brief 获取错误数量。
   * @return 错误数量
   */
  size_t count() const {
    return errors_.size();
  }

private:
  std::vector<ErrorType> errors_; ///< 错误列表
};

} // namespace czc::utils

#endif // CZC_UTILS_ERROR_COLLECTOR_HPP
