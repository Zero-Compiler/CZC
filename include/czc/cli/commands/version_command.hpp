/**
 * @file version_command.hpp
 * @brief 版本信息命令定义。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-30
 *
 * @details
 *   显示 CZC 编译器的版本信息。
 */

#ifndef CZC_CLI_COMMANDS_VERSION_COMMAND_HPP
#define CZC_CLI_COMMANDS_VERSION_COMMAND_HPP

#include "czc/common/config.hpp"

#include "czc/cli/commands/command.hpp"

namespace czc::cli {

/**
 * @brief 版本信息命令。
 *
 * @details
 *   显示编译器版本、构建信息等。
 */
class VersionCommand : public Command {
public:
  VersionCommand() = default;
  ~VersionCommand() override = default;

  /**
   * @brief 设置命令行选项。
   *
   * @param app CLI11 子命令 App 指针
   */
  void setup(CLI::App *app) override;

  /**
   * @brief 执行命令，输出版本信息。
   *
   * @return 退出码（始终为 0）
   */
  [[nodiscard]] Result<int> execute() override;

  /**
   * @brief 获取命令名称。
   *
   * @return "version"
   */
  [[nodiscard]] std::string_view name() const noexcept override {
    return "version";
  }

  /**
   * @brief 获取命令描述。
   *
   * @return 命令描述
   */
  [[nodiscard]] std::string_view description() const noexcept override {
    return "Display version information";
  }
};

} // namespace czc::cli

#endif // CZC_CLI_COMMANDS_VERSION_COMMAND_HPP
