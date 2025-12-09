/**
 * @file context.hpp
 * @brief 编译上下文定义。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-30
 *
 * @details
 *   CompilerContext 是编译器的核心上下文对象，聚合所有配置和状态。
 *   设计参考 LLVM/Clang Driver 和 Rust Session 模式：
 *   - 通过引用传递，避免全局状态
 *   - 不可拷贝，确保单一实例
 *   - 聚合选项、诊断系统等组件
 */

#ifndef CZC_CLI_CONTEXT_HPP
#define CZC_CLI_CONTEXT_HPP

#include "czc/common/config.hpp"
#include "czc/diag/diag_context.hpp"
#include "czc/diag/emitters/text_emitter.hpp"

#include <filesystem>
#include <memory>
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
 * @brief 全局选项（影响所有编译阶段）。
 */
struct GlobalOptions {
  std::filesystem::path workingDir{std::filesystem::current_path()};
  LogLevel logLevel{LogLevel::Normal};
  bool colorDiagnostics{true};
};

/**
 * @brief 输出选项。
 */
struct OutputOptions {
  std::optional<std::filesystem::path> file; ///< 输出文件路径
  OutputFormat format{OutputFormat::Text};   ///< 输出格式
};

/**
 * @brief 词法分析阶段选项。
 */
struct LexerOptions {
  bool preserveTrivia{false}; ///< 保留空白和注释信息
  bool dumpTokens{false};     ///< 输出所有 Token
};

/**
 * @brief 语法分析阶段选项（预留）。
 */
struct ParserOptions {
  bool dumpAst{false};         ///< 输出 AST
  bool allowIncomplete{false}; ///< 允许不完整输入
};

/**
 * @brief 编译上下文，聚合所有编译配置和状态。
 *
 * @details
 *   CompilerContext 替代全局单例模式，提供：
 *   - 选项的集中管理
 *   - 诊断系统的统一入口
 *   - 通过引用传递确保无全局状态
 *
 *   使用示例：
 *   @code
 *   CompilerContext ctx;
 *   ctx.global().logLevel = LogLevel::Verbose;
 *
 *   LexerPhase lexer(ctx);
 *   lexer.run(sourceFile);
 *
 *   if (ctx.diagContext().hasErrors()) {
 *     // 处理错误
 *   }
 *   @endcode
 */
class CompilerContext {
public:
  /**
   * @brief 默认构造函数。
   */
  CompilerContext();

  /**
   * @brief 带选项的构造函数。
   *
   * @param global 全局选项
   * @param output 输出选项
   */
  CompilerContext(GlobalOptions global, OutputOptions output);

  ~CompilerContext() = default;

  // 不可拷贝（确保单一实例）
  CompilerContext(const CompilerContext &) = delete;
  CompilerContext &operator=(const CompilerContext &) = delete;

  // 可移动
  CompilerContext(CompilerContext &&) noexcept = default;
  CompilerContext &operator=(CompilerContext &&) noexcept = default;

  // ========== 选项访问 ==========

  /// 获取全局选项（可变）
  [[nodiscard]] GlobalOptions &global() noexcept { return global_; }

  /// 获取全局选项（常量）
  [[nodiscard]] const GlobalOptions &global() const noexcept { return global_; }

  /// 获取输出选项（可变）
  [[nodiscard]] OutputOptions &output() noexcept { return output_; }

  /// 获取输出选项（常量）
  [[nodiscard]] const OutputOptions &output() const noexcept { return output_; }

  /// 获取词法分析选项（可变）
  [[nodiscard]] LexerOptions &lexer() noexcept { return lexer_; }

  /// 获取词法分析选项（常量）
  [[nodiscard]] const LexerOptions &lexer() const noexcept { return lexer_; }

  /// 获取语法分析选项（可变）
  [[nodiscard]] ParserOptions &parser() noexcept { return parser_; }

  /// 获取语法分析选项（常量）
  [[nodiscard]] const ParserOptions &parser() const noexcept { return parser_; }

  // ========== 诊断系统 ==========

  /// 获取诊断上下文（可变）
  [[nodiscard]] diag::DiagContext &diagContext() noexcept {
    return *diagContext_;
  }

  /// 获取诊断上下文（常量）
  [[nodiscard]] const diag::DiagContext &diagContext() const noexcept {
    return *diagContext_;
  }

  // ========== 便捷方法 ==========

  /// 检查是否为详细模式
  [[nodiscard]] bool isVerbose() const noexcept {
    return global_.logLevel == LogLevel::Verbose ||
           global_.logLevel == LogLevel::Debug;
  }

  /// 检查是否为静默模式
  [[nodiscard]] bool isQuiet() const noexcept {
    return global_.logLevel == LogLevel::Quiet;
  }

  /// 检查是否有编译错误
  [[nodiscard]] bool hasErrors() const noexcept {
    return diagContext_->hasErrors();
  }

  /// 获取错误数量
  [[nodiscard]] size_t errorCount() const noexcept {
    return diagContext_->errorCount();
  }

  /// 获取警告数量
  [[nodiscard]] size_t warningCount() const noexcept {
    return diagContext_->warningCount();
  }

private:
  GlobalOptions global_;
  OutputOptions output_;
  LexerOptions lexer_;
  ParserOptions parser_;
  std::unique_ptr<diag::DiagContext> diagContext_;

  /// 创建诊断上下文
  void initDiagContext();
};

} // namespace czc::cli

#endif // CZC_CLI_CONTEXT_HPP
