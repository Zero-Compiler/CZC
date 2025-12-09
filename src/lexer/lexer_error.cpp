/**
 * @file lexer_error.cpp
 * @brief 词法分析错误处理的实现。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-29
 */

#include "czc/lexer/lexer_error.hpp"
#include "czc/lexer/source_manager.hpp"
#include <format>

namespace czc::lexer {

std::vector<SourceLocation>
getExpansionChain([[maybe_unused]] const LexerError &error,
                  [[maybe_unused]] const SourceManager &sm) {
  std::vector<SourceLocation> chain;

  // 如果错误位置有 expansionId，追溯宏展开链
  // 目前简单实现：返回空（待宏系统完善后实现）

  return chain;
}

std::string formatError(const LexerError &error, const SourceManager &sm) {
  std::string result;

  // 格式：文件名:行:列: 错误码: 消息
  // 例如：main.czc:10:5: L1001: invalid character '@'

  // 获取文件名
  std::string_view filename = sm.getFilename(error.location.buffer);
  if (filename.empty()) {
    filename = "<unknown>";
  }

  result = std::format("{}:{}:{}: {}: {}", filename, error.location.line,
                       error.location.column, error.codeString(),
                       error.formattedMessage);

  // 如果有宏展开链，添加展开上下文
  auto chain = getExpansionChain(error, sm);
  for (const auto &loc : chain) {
    std::string_view chainFilename = sm.getFilename(loc.buffer);
    if (chainFilename.empty()) {
      chainFilename = "<unknown>";
    }
    result += std::format("\n  expanded from {}:{}:{}", chainFilename, loc.line,
                          loc.column);
  }

  return result;
}

} // namespace czc::lexer
