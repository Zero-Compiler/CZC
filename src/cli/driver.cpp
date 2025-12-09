/**
 * @file driver.cpp
 * @brief 编译驱动器实现。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-30
 */

#include "czc/cli/driver.hpp"
#include "czc/cli/output/formatter.hpp"
#include "czc/cli/phases/lexer_phase.hpp"
#include "czc/diag/diag_builder.hpp"
#include "czc/diag/message.hpp"

#include <fstream>
#include <iostream>

namespace czc::cli {

Driver::Driver() = default;

Driver::Driver(CompilerContext ctx) : ctx_(std::move(ctx)) {}

int Driver::runLexer(const std::filesystem::path &inputFile) {
  // 创建词法分析阶段
  LexerPhase phase(ctx_);

  // 执行词法分析
  auto result = phase.runOnFile(inputFile);

  if (!result.has_value()) {
    // 报告错误
    diagContext().emit(
        diag::error(diag::Message(result.error().message)).build());
    return 1;
  }

  const auto &lexResult = result.value();

  // 格式化输出
  auto formatter = createFormatter(ctx_.output().format);
  std::string output;

  if (lexResult.hasErrors) {
    // 错误已通过诊断系统报告，这里只需返回错误码
    return 1;
  }

  // 格式化 Token 输出
  output = formatter->formatTokens(lexResult.tokens, phase.sourceManager());

  // 输出结果
  if (ctx_.output().file.has_value()) {
    std::ofstream ofs(ctx_.output().file.value());
    if (!ofs) {
      diagContext().emit(
          diag::error(diag::Message("Failed to open output file: " +
                                    ctx_.output().file.value().string()))
              .build());
      return 1;
    }
    ofs << output;
  } else {
    std::cout << output;
  }

  return 0;
}

void Driver::printDiagnosticSummary() {
  // 使用诊断系统的 emitSummary 方法输出统计信息
  ctx_.diagContext().emitSummary();
}

} // namespace czc::cli
