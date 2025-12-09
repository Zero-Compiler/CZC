/**
 * @file lexer_phase.hpp
 * @brief 词法分析阶段定义。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-30
 *
 * @details
 *   LexerPhase 是词法分析的核心执行单元，实现 CompilerPhase 接口。
 */

#ifndef CZC_CLI_PHASES_LEXER_PHASE_HPP
#define CZC_CLI_PHASES_LEXER_PHASE_HPP

#include "czc/cli/context.hpp"
#include "czc/common/config.hpp"
#include "czc/common/result.hpp"
#include "czc/lexer/lexer.hpp"
#include "czc/lexer/source_manager.hpp"
#include "czc/lexer/token.hpp"

#include <filesystem>
#include <string>
#include <vector>

namespace czc::cli {

/**
 * @brief 词法分析结果。
 */
struct LexResult {
  std::vector<lexer::Token> tokens; ///< Token 列表
  bool hasErrors{false};            ///< 是否有错误
};

/**
 * @brief 词法分析阶段。
 *
 * @details
 *   执行词法分析的核心逻辑，不涉及 CLI 交互。
 *   通过 CompilerContext 获取配置和诊断系统。
 *
 *   使用示例：
 *   @code
 *   CompilerContext ctx;
 *   ctx.lexer().preserveTrivia = true;
 *
 *   LexerPhase phase(ctx);
 *   auto result = phase.runOnFile("source.zl");
 *
 *   if (result.has_value()) {
 *     for (const auto& token : result->tokens) {
 *       // 处理 token
 *     }
 *   }
 *   @endcode
 */
class LexerPhase {
public:
  /**
   * @brief 构造函数。
   *
   * @param ctx 编译上下文引用
   */
  explicit LexerPhase(CompilerContext &ctx) : ctx_(ctx) {}

  ~LexerPhase() = default;

  // 不可拷贝
  LexerPhase(const LexerPhase &) = delete;
  LexerPhase &operator=(const LexerPhase &) = delete;

  // 可移动
  LexerPhase(LexerPhase &&) noexcept = default;
  LexerPhase &operator=(LexerPhase &&) noexcept = delete;

  /**
   * @brief 对文件执行词法分析。
   *
   * @param filepath 源文件路径
   * @return 词法分析结果，失败时返回错误
   */
  [[nodiscard]] Result<LexResult>
  runOnFile(const std::filesystem::path &filepath);

  /**
   * @brief 对源码字符串执行词法分析。
   *
   * @param source 源码内容
   * @param filename 虚拟文件名
   * @return 词法分析结果，失败时返回错误
   */
  [[nodiscard]] Result<LexResult>
  runOnSource(std::string_view source, std::string_view filename = "<stdin>");

  /**
   * @brief 获取输入数据类型标识。
   *
   * @return "source"
   */
  [[nodiscard]] static constexpr std::string_view inputType() noexcept {
    return "source";
  }

  /**
   * @brief 获取输出数据类型标识。
   *
   * @return "tokens"
   */
  [[nodiscard]] static constexpr std::string_view outputType() noexcept {
    return "tokens";
  }

  /**
   * @brief 获取 SourceManager 引用。
   *
   * @return SourceManager 引用
   *
   * @note 用于获取 Token 的文本内容
   */
  [[nodiscard]] lexer::SourceManager &sourceManager() noexcept {
    return sourceManager_;
  }

  /**
   * @brief 获取 SourceManager 引用（常量）。
   *
   * @return SourceManager 常量引用
   */
  [[nodiscard]] const lexer::SourceManager &sourceManager() const noexcept {
    return sourceManager_;
  }

private:
  CompilerContext &ctx_;
  lexer::SourceManager sourceManager_;

  /**
   * @brief 执行词法分析的内部实现。
   *
   * @param bufferId 源码缓冲区 ID
   * @return 词法分析结果
   */
  [[nodiscard]] LexResult runLexer(lexer::BufferID bufferId);
};

} // namespace czc::cli

#endif // CZC_CLI_PHASES_LEXER_PHASE_HPP
