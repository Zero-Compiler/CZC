/**
 * @file lex_command.hpp
 * @brief 词法分析命令定义。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-30
 *
 * @details
 *   实现 `czc lex` 子命令，对源文件进行词法分析。
 *   职责分离：
 *   - LexCommand: 处理 CLI 交互（参数解析、输出控制）
 *   - LexerPhase: 执行词法分析逻辑（在 Driver 中使用）
 */

#ifndef CZC_CLI_COMMANDS_LEX_COMMAND_HPP
#define CZC_CLI_COMMANDS_LEX_COMMAND_HPP

#include "czc/common/config.hpp"

#include "czc/cli/commands/command.hpp"
#include "czc/cli/driver.hpp"

#include <filesystem>
#include <string>

namespace czc::cli {

/**
 * @brief 词法分析命令。
 *
 * @details
 *   实现 `czc lex` 子命令，支持：
 *   - 基础词法分析
 *   - Trivia 模式（保留空白和注释）
 *   - 多种输出格式（Text/JSON）
 *
 *   命令只负责 CLI 交互，实际词法分析由 Driver + LexerPhase 执行。
 */
class LexCommand : public Command {
public:
  /**
   * @brief 构造函数。
   *
   * @param driver 编译驱动器引用
   */
  explicit LexCommand(Driver &driver) : driver_(driver) {}

  ~LexCommand() override = default;

  // ========== Command 接口 ==========

  /**
   * @brief 设置命令行选项。
   *
   * @param app CLI11 子命令 App 指针
   */
  void setup(CLI::App *app) override;

  /**
   * @brief 执行词法分析命令。
   *
   * @return 退出码（0 成功，非 0 失败）
   */
  [[nodiscard]] Result<int> execute() override;

  /**
   * @brief 获取命令名称。
   *
   * @return "lex"
   */
  [[nodiscard]] std::string_view name() const noexcept override {
    return "lex";
  }

  /**
   * @brief 获取命令描述。
   *
   * @return 命令描述
   */
  [[nodiscard]] std::string_view description() const noexcept override {
    return "Perform lexical analysis on source file";
  }

private:
  Driver &driver_;
  std::filesystem::path inputFile_; ///< 输入文件路径
  bool trivia_{false};              ///< 是否保留 trivia
  bool dumpTokens_{false};          ///< 是否输出所有 token
};

} // namespace czc::cli

#endif // CZC_CLI_COMMANDS_LEX_COMMAND_HPP
