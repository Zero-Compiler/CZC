/**
 * @file compiler_phase.hpp
 * @brief 编译阶段接口定义（Pipeline 预留）。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-30
 *
 * @details
 *   定义编译器各阶段的通用接口，为 Pipeline 组合预留扩展点。
 *   当 Parser、Semantic 等模块完成后，可以实现完整的 Pipeline。
 */

#ifndef CZC_CLI_COMMANDS_COMPILER_PHASE_HPP
#define CZC_CLI_COMMANDS_COMPILER_PHASE_HPP

#include "czc/common/config.hpp"

#include "czc/common/result.hpp"

#include <any>
#include <string_view>

namespace czc::cli {

/**
 * @brief 阶段执行选项（预留）。
 *
 * @details
 *   用于传递给各编译阶段的选项，可以根据需要扩展。
 */
struct PhaseOptions {
  bool verbose{false};
  // 可根据需要扩展
};

/**
 * @brief 编译阶段接口，为 Pipeline 组合预留。
 *
 * @details
 *   定义编译器各阶段的通用行为，支持：
 *   - 输入/输出类型声明（用于 Pipeline 连接验证）
 *   - 带选项的执行接口
 *   - 独立运行能力标记
 *
 * @note 这是一个预留接口，当前仅 LexPhase 会实现。
 *       完整的 Pipeline 功能将在 Parser 模块完成后实现。
 */
class CompilerPhase {
public:
  virtual ~CompilerPhase() = default;

  // 不可拷贝
  CompilerPhase(const CompilerPhase &) = delete;
  CompilerPhase &operator=(const CompilerPhase &) = delete;

  // 可移动
  CompilerPhase(CompilerPhase &&) noexcept = default;
  CompilerPhase &operator=(CompilerPhase &&) noexcept = default;

  /**
   * @brief 获取输入数据类型。
   *
   * @return 类型标识，如 "source", "tokens", "ast"
   */
  [[nodiscard]] virtual std::string_view inputType() const noexcept = 0;

  /**
   * @brief 获取输出数据类型。
   *
   * @return 类型标识，如 "source", "tokens", "ast"
   */
  [[nodiscard]] virtual std::string_view outputType() const noexcept = 0;

  /**
   * @brief 是否支持独立运行（作为子命令）。
   *
   * @return 若支持独立运行返回 true
   */
  [[nodiscard]] virtual bool canRunStandalone() const noexcept { return true; }

  /**
   * @brief 执行阶段（预留接口）。
   *
   * @param input 输入数据（使用 std::any 以支持多种类型）
   * @param opts 阶段选项
   * @return 输出数据，失败时返回错误
   *
   * @note 这是一个预留接口，具体实现将在 Pipeline 功能完成时添加。
   */
  [[nodiscard]] virtual Result<std::any> execute(std::any input,
                                                 const PhaseOptions &opts) = 0;

protected:
  CompilerPhase() = default;
};

} // namespace czc::cli

#endif // CZC_CLI_COMMANDS_COMPILER_PHASE_HPP
