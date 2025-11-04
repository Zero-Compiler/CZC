/**
 * @file error_collector.hpp
 * @brief 词元预处理器错误收集器类定义
 * @author BegoniaHe
 * @date 2025-11-04
 */

#ifndef CZC_TP_ERROR_COLLECTOR_HPP
#define CZC_TP_ERROR_COLLECTOR_HPP

#include "czc/diagnostics/diagnostic_code.hpp"
#include "czc/utils/source_location.hpp"
#include <string>
#include <vector>

/**
 * @brief 代表一个在 Token 预处理阶段检测到的错误。
 * @details
 *   此结构体用于封装预处理错误的所有相关信息，例如在分析科学计数法
 *   字面量时发生的溢出错误。
 */
struct TPError {
  /// @brief 标识错误类型的唯一代码。
  DiagnosticCode code;
  /// @brief 错误在源代码中的位置。
  SourceLocation location;
  /// @brief (可选) 用于生成详细错误消息的参数。
  std::vector<std::string> args;

  /**
   * @brief 构造一个新的预处理错误记录。
   * @param[in] c 诊断代码。
   * @param[in] loc 源码位置。
   * @param[in] arguments (可选) 消息参数列表。
   */
  TPError(DiagnosticCode c, const SourceLocation &loc,
          const std::vector<std::string> &arguments = {})
      : code(c), location(loc), args(arguments) {}
};

/**
 * @brief 收集并管理在 Token 预处理过程中产生的所有错误。
 * @details
 *   此类为 TokenPreprocessor 提供了一个统一的错误记录机制。
 *   当检测到如数值溢出等问题时，预处理器会使用此类来记录错误，
 *   而不是中断处理流程。
 * @note 此类不是线程安全的。
 */
class TPErrorCollector {
private:
  /// @brief 存储所有已报告的预处理错误的列表。
  std::vector<TPError> errors;

public:
  /**
   * @brief 向收集中添加一个新的预处理错误。
   * @param[in] code 错误的诊断代码。
   * @param[in] loc 错误在源代码中的位置。
   * @param[in] args (可选) 用于格式化错误消息的参数。
   */
  void add(DiagnosticCode code, const SourceLocation &loc,
           const std::vector<std::string> &args = {});

  /**
   * @brief 获取所有已收集的错误。
   * @return 返回对内部错误列表的常量引用。
   */
  const std::vector<TPError> &get_errors() const { return errors; }

  /**
   * @brief 检查是否收集到了任何错误。
   * @return 如果错误列表不为空，则返回 true。
   */
  bool has_errors() const { return !errors.empty(); }

  /**
   * @brief 清空所有已收集的错误。
   */
  void clear() { errors.clear(); }

  /**
   * @brief 获取当前收集到的错误总数。
   * @return 错误列表的大小。
   */
  size_t count() const { return errors.size(); }
};

#endif // CZC_TP_ERROR_COLLECTOR_HPP
