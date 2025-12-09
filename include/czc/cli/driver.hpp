/**
 * @file driver.hpp
 * @brief 编译驱动器定义。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-30
 *
 * @details
 *   Driver 是编译器的核心协调者，负责：
 *   - 管理 CompilerContext
 *   - 协调各编译阶段的执行
 *   - 处理输入/输出
 *
 */

#ifndef CZC_CLI_DRIVER_HPP
#define CZC_CLI_DRIVER_HPP

#include "czc/cli/context.hpp"
#include "czc/common/config.hpp"
#include "czc/common/result.hpp"
#include "czc/diag/diagnostic.hpp"

#include <filesystem>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

namespace czc::cli {

/**
 * @brief 编译驱动器，协调整个编译过程。
 *
 * @details
 *   Driver 是编译器的入口点，负责：
 *   - 初始化编译上下文
 *   - 设置诊断系统
 *   - 协调各编译阶段
 *   - 返回退出码
 *
 *   使用示例：
 *   @code
 *   Driver driver;
 *   driver.setVerbose(true);
 *
 *   int exitCode = driver.runLexer("source.zl");
 *   @endcode
 */
class Driver {
public:
  /**
   * @brief 默认构造函数。
   */
  Driver();

  /**
   * @brief 带上下文的构造函数。
   *
   * @param ctx 编译上下文
   */
  explicit Driver(CompilerContext ctx);

  ~Driver() = default;

  // 不可拷贝
  Driver(const Driver &) = delete;
  Driver &operator=(const Driver &) = delete;

  // 可移动
  Driver(Driver &&) noexcept = default;
  Driver &operator=(Driver &&) noexcept = default;

  // ========== 上下文访问 ==========

  /// 获取编译上下文（可变）
  [[nodiscard]] CompilerContext &context() noexcept { return ctx_; }

  /// 获取编译上下文（常量）
  [[nodiscard]] const CompilerContext &context() const noexcept { return ctx_; }

  /// 获取诊断上下文
  [[nodiscard]] diag::DiagContext &diagContext() noexcept {
    return ctx_.diagContext();
  }

  // ========== 配置方法 ==========

  /// 设置详细模式
  void setVerbose(bool verbose) noexcept {
    ctx_.global().logLevel = verbose ? LogLevel::Verbose : LogLevel::Normal;
  }

  /// 设置静默模式
  void setQuiet(bool quiet) noexcept {
    if (quiet) {
      ctx_.global().logLevel = LogLevel::Quiet;
    }
  }

  /// 设置输出格式
  void setOutputFormat(OutputFormat format) noexcept {
    ctx_.output().format = format;
  }

  /// 设置输出文件
  void setOutputFile(std::filesystem::path path) {
    ctx_.output().file = std::move(path);
  }

  /// 设置颜色输出
  void setColorDiagnostics(bool enabled) noexcept {
    ctx_.global().colorDiagnostics = enabled;
  }

  // ========== 执行方法 ==========

  /**
   * @brief 执行词法分析。
   *
   * @param inputFile 输入文件路径
   * @return 退出码（0 成功，非 0 失败）
   */
  [[nodiscard]] int runLexer(const std::filesystem::path &inputFile);

  /**
   * @brief 打印诊断摘要。
   */
  void printDiagnosticSummary();

  /**
   * @brief 设置错误输出流。
   *
   * @param stream 输出流引用
   */
  void setErrorStream(std::ostream &stream) noexcept { errStream_ = &stream; }

private:
  CompilerContext ctx_;
  std::ostream *errStream_{&std::cerr}; ///< 错误输出流（默认 stderr）
};

} // namespace czc::cli

#endif // CZC_CLI_DRIVER_HPP
