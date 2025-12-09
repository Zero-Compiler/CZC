/**
 * @file cli.hpp
 * @brief CLI 主入口类定义。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-30
 *
 * @details
 *   Cli 是命令行接口的门面类，负责：
 *   - 初始化 CLI11 应用
 *   - 注册子命令
 *   - 设置全局选项
 *   - 协调命令执行
 *
 *   架构说明：
 *   - Cli: 门面类，处理 CLI11 解析
 *   - Driver: 编译驱动，管理上下文
 *   - Command: 命令接口，处理子命令逻辑
 *   - Phase: 编译阶段，执行实际编译工作
 */

#ifndef CZC_CLI_CLI_HPP
#define CZC_CLI_CLI_HPP

#include "czc/common/config.hpp"

#include "czc/cli/commands/command.hpp"
#include "czc/cli/driver.hpp"
#include "czc/common/result.hpp"

#include <CLI/CLI.hpp>

#include <memory>
#include <vector>

namespace czc::cli {

/// 程序名称
inline constexpr std::string_view kProgramName = "czc";

/// 程序描述
inline constexpr std::string_view kProgramDescription =
    "CZC Compiler - A modern zerolang compiler written in C++";

/**
 * @brief CLI 门面类，协调命令行解析与执行。
 *
 * @details
 *   采用门面模式设计，对外提供简洁的接口：
 *   - 解析命令行参数
 *   - 分发到对应子命令执行
 *   - 统一错误处理和输出
 *
 *   使用 Driver 管理编译上下文，避免全局状态。
 */
class Cli {
public:
  /**
   * @brief 构造函数，初始化 CLI11 应用和 Driver。
   */
  Cli();

  /**
   * @brief 析构函数。
   */
  ~Cli() = default;

  // 不可拷贝，不可移动
  Cli(const Cli &) = delete;
  Cli &operator=(const Cli &) = delete;
  Cli(Cli &&) = delete;
  Cli &operator=(Cli &&) = delete;

  /**
   * @brief 解析命令行参数并执行。
   *
   * @param argc 参数个数
   * @param argv 参数数组
   * @return 退出码（0 成功，非 0 失败）
   */
  [[nodiscard]] int run(int argc, char **argv);

  /**
   * @brief 获取 CLI11 App 引用（用于测试）。
   *
   * @return CLI11 App 引用
   */
  [[nodiscard]] CLI::App &app() noexcept { return app_; }

  /**
   * @brief 获取 Driver 引用。
   *
   * @return Driver 引用
   */
  [[nodiscard]] Driver &driver() noexcept { return driver_; }

private:
  CLI::App app_;                                   ///< CLI11 应用实例
  Driver driver_;                                  ///< 编译驱动器
  std::vector<std::unique_ptr<Command>> commands_; ///< 已注册的命令列表
  Command *activeCommand_{nullptr};                ///< 当前激活的命令

  /**
   * @brief 注册所有子命令。
   */
  void registerCommands();

  /**
   * @brief 设置全局选项。
   */
  void setupGlobalOptions();

  /**
   * @brief 加载配置文件（预留）。
   *
   * @return 成功或错误
   */
  [[nodiscard]] VoidResult loadConfig();

  /**
   * @brief 注册需要 Driver 的命令。
   *
   * @tparam T 命令类型（构造函数接受 Driver& 参数）
   */
  template <typename T> void registerCommandWithDriver() {
    auto cmd = std::make_unique<T>(driver_);
    auto *sub = app_.add_subcommand(std::string(cmd->name()),
                                    std::string(cmd->description()));
    cmd->setup(sub);

    // 设置回调，记录激活的命令
    Command *raw_ptr = cmd.get();
    sub->callback([this, raw_ptr]() { activeCommand_ = raw_ptr; });

    commands_.push_back(std::move(cmd));
  }

  /**
   * @brief 注册不需要 Driver 的简单命令。
   *
   * @tparam T 命令类型（默认构造）
   */
  template <typename T> void registerSimpleCommand() {
    auto cmd = std::make_unique<T>();
    auto *sub = app_.add_subcommand(std::string(cmd->name()),
                                    std::string(cmd->description()));
    cmd->setup(sub);

    // 设置回调，记录激活的命令
    Command *raw_ptr = cmd.get();
    sub->callback([this, raw_ptr]() { activeCommand_ = raw_ptr; });

    commands_.push_back(std::move(cmd));
  }
};

} // namespace czc::cli

#endif // CZC_CLI_CLI_HPP
