/**
 * @file command.hpp
 * @brief 命令接口定义。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-30
 *
 * @details
 *   定义子命令的通用接口，所有子命令都需实现此接口。
 */

#ifndef CZC_CLI_COMMANDS_COMMAND_HPP
#define CZC_CLI_COMMANDS_COMMAND_HPP

#include "czc/common/config.hpp"

#include "czc/common/result.hpp"

#include <CLI/CLI.hpp>

#include <string_view>

namespace czc::cli {

// 前向声明
class CompilerPhase;

/**
 * @brief 命令接口，定义子命令的通用行为。
 *
 * @details
 *   所有子命令（如 lex、parse、compile 等）都需实现此接口。
 *   接口设计遵循以下原则：
 *   - 单一职责：每个命令只做一件事
 *   - 低耦合：命令之间互不依赖
 *   - 可扩展：支持 Pipeline 扩展
 */
class Command {
public:
  virtual ~Command() = default;

  // 不可拷贝
  Command(const Command &) = delete;
  Command &operator=(const Command &) = delete;

  // 可移动
  Command(Command &&) noexcept = default;
  Command &operator=(Command &&) noexcept = default;

  /**
   * @brief 设置命令行选项和参数。
   *
   * @param app CLI11 子命令 App 指针
   */
  virtual void setup(CLI::App *app) = 0;

  /**
   * @brief 执行命令逻辑。
   *
   * @return 执行结果（成功返回退出码，失败返回错误）
   */
  [[nodiscard]] virtual Result<int> execute() = 0;

  /**
   * @brief 获取命令名称。
   *
   * @return 命令名称（如 "lex", "parse"）
   */
  [[nodiscard]] virtual std::string_view name() const noexcept = 0;

  /**
   * @brief 获取命令描述。
   *
   * @return 命令描述
   */
  [[nodiscard]] virtual std::string_view description() const noexcept = 0;

  /**
   * @brief 获取关联的编译阶段（可选，用于 Pipeline）。
   *
   * @return 编译阶段指针，若不支持则返回 nullptr
   */
  [[nodiscard]] virtual CompilerPhase *asPhase() noexcept { return nullptr; }

  /**
   * @brief 获取关联的编译阶段（常量版本）。
   *
   * @return 编译阶段常量指针
   */
  [[nodiscard]] virtual const CompilerPhase *asPhase() const noexcept {
    return nullptr;
  }

protected:
  Command() = default;
};

} // namespace czc::cli

#endif // CZC_CLI_COMMANDS_COMMAND_HPP
