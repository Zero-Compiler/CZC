/**
 * @file options.hpp
 * @brief CLI 分层选项定义。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-30
 *
 * @details
 *   定义命令行选项的分层结构：
 *   - Global: 全局选项
 *   - Phase: 阶段选项
 *   - Output: 输出选项
 */

#ifndef CZC_CLI_OPTIONS_HPP
#define CZC_CLI_OPTIONS_HPP

#if __cplusplus < 202302L
#error "C++23 or higher is required"
#endif

#include <filesystem>
#include <optional>
#include <string>

namespace czc::cli {

/**
 * @brief 输出格式枚举。
 */
enum class OutputFormat {
  Text, ///< 人类可读文本格式
  Json  ///< JSON 格式
};

/**
 * @brief 日志级别枚举。
 */
enum class LogLevel {
  Quiet,   ///< 静默模式，仅输出错误
  Normal,  ///< 正常输出
  Verbose, ///< 详细输出
  Debug    ///< 调试输出
};

/**
 * @brief 分层命令行选项。
 *
 * @details
 *   选项按层次组织，便于管理和扩展：
 *   - Level 1: 全局选项
 *   - Level 2: 阶段选项
 *   - Level 3: 输出选项
 */
struct CliOptions {
  /**
   * @brief Level 1: 全局选项。
   */
  struct Global {
    std::filesystem::path workingDir{std::filesystem::current_path()};
    LogLevel logLevel{LogLevel::Normal};
    bool colorDiagnostics{true};
  } global;

  /**
   * @brief Level 2: 阶段选项。
   */
  struct Phase {
    /**
     * @brief 词法分析阶段选项。
     */
    struct Lexer {
      bool preserveTrivia{false}; ///< 保留空白和注释信息
      bool dumpTokens{false};     ///< 输出所有 Token
    } lexer;

    /**
     * @brief 语法分析阶段选项。
     */
    struct Parser {
      bool dumpAst{false};         ///< 输出 AST
      bool allowIncomplete{false}; ///< 允许不完整输入
    } parser;

    // 未来扩展: semantic, codegen...
  } phase;

  /**
   * @brief Level 3: 输出选项。
   */
  struct Output {
    std::optional<std::filesystem::path> file; ///< 输出文件路径
    OutputFormat format{OutputFormat::Text};   ///< 输出格式
  } output;
};

/**
 * @brief 获取全局选项实例。
 *
 * @return 全局选项的可变引用
 */
[[nodiscard]] CliOptions &cliOptions() noexcept;

/**
 * @brief 获取全局选项实例。
 *
 * @return 全局选项的常量引用
 */
[[nodiscard]] const CliOptions &cliOptionsConst() noexcept;

/**
 * @brief 重置选项为默认值。
 */
void resetOptions() noexcept;

} // namespace czc::cli

#endif // CZC_CLI_OPTIONS_HPP
