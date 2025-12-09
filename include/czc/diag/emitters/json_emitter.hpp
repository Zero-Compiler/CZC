/**
 * @file json_emitter.hpp
 * @brief JSON 发射器。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-12-04
 *
 * @details
 *   JSON 格式输出的发射器。
 *   借鉴 rustc JsonEmitter。
 */

#ifndef CZC_DIAG_EMITTERS_JSON_EMITTER_HPP
#define CZC_DIAG_EMITTERS_JSON_EMITTER_HPP

#include "czc/common/config.hpp"
#include "czc/diag/emitter.hpp"

#include <ostream>
#include <vector>

namespace czc::diag {

/// JSON 发射器 - 机器可读输出
/// 借鉴 rustc JsonEmitter
class JsonEmitter final : public Emitter {
public:
  /// 构造 JSON 发射器
  explicit JsonEmitter(std::ostream &out, bool pretty = false);

  /// 析构函数
  ~JsonEmitter() override;

  // 禁止拷贝，允许移动
  JsonEmitter(const JsonEmitter &) = delete;
  auto operator=(const JsonEmitter &) -> JsonEmitter & = delete;
  JsonEmitter(JsonEmitter &&) noexcept = default;
  auto operator=(JsonEmitter &&) noexcept -> JsonEmitter & = default;

  /// 发射诊断
  void emit(const Diagnostic &diag, const SourceLocator *locator) override;

  /// 发射诊断总结信息
  void emitSummary(const DiagnosticStats &stats) override;

  /// 刷新缓冲区（输出所有缓冲的诊断）
  void flush() override;

  /// 设置是否美化输出
  void setPretty(bool pretty) noexcept { pretty_ = pretty; }

private:
  std::ostream *out_;
  bool pretty_;
  bool firstDiag_{true}; ///< 是否是第一个诊断

  /// 将诊断转换为 JSON 字符串
  [[nodiscard]] auto diagnosticToJson(const Diagnostic &diag,
                                      const SourceLocator *locator) const
      -> std::string;

  /// 将 Span 转换为 JSON
  [[nodiscard]] auto spanToJson(const Span &span,
                                const SourceLocator *locator) const
      -> std::string;
};

} // namespace czc::diag

#endif // CZC_DIAG_EMITTERS_JSON_EMITTER_HPP
