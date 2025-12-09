/**
 * @file lexer_phase.cpp
 * @brief 词法分析阶段实现。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-30
 */

#include "czc/cli/phases/lexer_phase.hpp"
#include "czc/lexer/lexer_source_locator.hpp"

#include <fstream>
#include <sstream>

namespace czc::cli {

Result<LexResult> LexerPhase::runOnFile(const std::filesystem::path &filepath) {
  // 检查文件是否存在
  if (!std::filesystem::exists(filepath)) {
    return err<LexResult>("File not found: " + filepath.string(), "E001");
  }

  // 检查文件大小
  auto fileSize = std::filesystem::file_size(filepath);
  if (fileSize > kLimits.maxFileSize) {
    return err<LexResult>("File too large: " + filepath.string() + " (" +
                              std::to_string(fileSize) + " bytes, max " +
                              std::to_string(kLimits.maxFileSize) + " bytes)",
                          "E002");
  }

  // 读取文件内容
  std::ifstream ifs(filepath);
  if (!ifs) {
    return err<LexResult>("Failed to open file: " + filepath.string(), "E003");
  }

  std::ostringstream oss;
  oss << ifs.rdbuf();
  std::string content = oss.str();

  // 添加到 SourceManager
  auto bufferId =
      sourceManager_.addBuffer(std::move(content), filepath.string());

  // 执行词法分析
  return ok(runLexer(bufferId));
}

Result<LexResult> LexerPhase::runOnSource(std::string_view source,
                                          std::string_view filename) {
  // 检查源码大小
  if (source.size() > kLimits.maxFileSize) {
    return err<LexResult>("Source too large: " + std::to_string(source.size()) +
                              " bytes, max " +
                              std::to_string(kLimits.maxFileSize) + " bytes",
                          "E002");
  }

  // 添加到 SourceManager
  auto bufferId = sourceManager_.addBuffer(source, std::string(filename));

  // 执行词法分析
  return ok(runLexer(bufferId));
}

LexResult LexerPhase::runLexer(lexer::BufferID bufferId) {
  LexResult result;

  // 创建 Lexer
  lexer::Lexer lex(sourceManager_, bufferId);

  // 根据选项执行词法分析
  const auto &opts = ctx_.lexer();
  if (opts.preserveTrivia) {
    result.tokens = lex.tokenizeWithTrivia();
  } else {
    result.tokens = lex.tokenize();
  }

  // 收集错误到诊断系统
  if (lex.hasErrors()) {
    result.hasErrors = true;
    // 使用新的诊断系统桥接层发射 lexer 错误
    lexer::emitLexerErrors(ctx_.diagContext(), lex.errors(), sourceManager_,
                           bufferId);
  }

  return result;
}

} // namespace czc::cli
