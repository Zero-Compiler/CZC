/**
 * @file emitter.hpp
 * @brief 发射器接口。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-12-04
 *
 * @details
 *   策略模式接口
 *   负责将诊断转换为具体输出格式。
 */

#ifndef CZC_DIAG_EMITTER_HPP
#define CZC_DIAG_EMITTER_HPP

#include "czc/common/config.hpp"
#include "czc/diag/diagnostic.hpp"
#include "czc/diag/source_locator.hpp"

#include <set>

namespace czc::diag {

/// 诊断统计信息
struct DiagnosticStats {
  size_t errorCount{0};                 ///< 错误数量
  size_t warningCount{0};               ///< 警告数量
  size_t noteCount{0};                  ///< 注释数量
  std::set<ErrorCode> uniqueErrorCodes; ///< 唯一错误码集合

  /// 检查是否有错误
  [[nodiscard]] auto hasErrors() const noexcept -> bool {
    return errorCount > 0;
  }

  /// 获取总诊断数量
  [[nodiscard]] auto total() const noexcept -> size_t {
    return errorCount + warningCount + noteCount;
  }
};

/// 发射器接口
/// 负责将诊断转换为具体输出格式
class Emitter {
public:
  virtual ~Emitter() = default;

  /// 发射单个诊断
  virtual void emit(const Diagnostic &diag, const SourceLocator *locator) = 0;

  /// 发射诊断总结信息
  /// @param stats 诊断统计数据
  virtual void emitSummary(const DiagnosticStats &stats) = 0;

  /// 刷新缓冲区
  virtual void flush() = 0;

protected:
  Emitter() = default;
  Emitter(const Emitter &) = default;
  auto operator=(const Emitter &) -> Emitter & = default;
  Emitter(Emitter &&) = default;
  auto operator=(Emitter &&) -> Emitter & = default;
};

} // namespace czc::diag

#endif // CZC_DIAG_EMITTER_HPP
