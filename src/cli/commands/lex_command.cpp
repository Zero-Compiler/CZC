/**
 * @file lex_command.cpp
 * @brief 词法分析命令实现。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-30
 */

#include "czc/cli/commands/lex_command.hpp"

namespace czc::cli {

void LexCommand::setup(CLI::App *app) {
  // 输入文件（位置参数）
  app->add_option("input", inputFile_, "Input source file")
      ->required()
      ->check(CLI::ExistingFile);

  // trivia 模式
  app->add_flag("--trivia,-t", trivia_, "Preserve whitespace and comments")
      ->group("Lexer Options");

  // dump tokens
  app->add_flag("--dump-tokens,-d", dumpTokens_, "Dump all tokens")
      ->group("Lexer Options");
}

Result<int> LexCommand::execute() {
  // 配置编译上下文
  auto &ctx = driver_.context();
  ctx.lexer().preserveTrivia = trivia_;
  ctx.lexer().dumpTokens = dumpTokens_;

  // 执行词法分析
  int exitCode = driver_.runLexer(inputFile_);

  // 打印诊断摘要
  if (ctx.isVerbose()) {
    driver_.printDiagnosticSummary();
  }

  return Result<int>(exitCode);
}

} // namespace czc::cli
