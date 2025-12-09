/**
 * @file json_formatter.cpp
 * @brief JSON 格式化器实现。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-30
 */

#include "czc/cli/output/json_formatter.hpp"
#include "czc/cli/output/text_formatter.hpp"
#include "czc/lexer/token.hpp"

#include <glaze/glaze.hpp>

#include <vector>

namespace czc::cli {

// JSON 数据结构
namespace json_types {

/// Token 的 JSON 表示结构
struct TokenJson {
  std::string type;
  std::string value;
  std::uint32_t line;
  std::uint32_t column;
  std::uint32_t offset;
  std::uint16_t length;
};

/// 错误的 JSON 表示结构
struct ErrorJson {
  int code;
  std::string message;
  std::string file;
  std::uint32_t line;
  std::uint32_t column;
};

/// Token 列表的 JSON 响应
struct TokensResponse {
  bool success{true};
  std::size_t count{0};
  std::vector<TokenJson> tokens;
};

/// 错误列表的 JSON 响应
struct ErrorsResponse {
  bool success{false};
  std::size_t count{0};
  std::vector<ErrorJson> errors;
};

} // namespace json_types

using namespace json_types;

std::string JsonFormatter::formatTokens(std::span<const lexer::Token> tokens,
                                        const lexer::SourceManager &sm) const {
  TokensResponse response;
  response.count = tokens.size();
  response.tokens.reserve(tokens.size());

  for (const auto &token : tokens) {
    const auto &loc = token.location();

    TokenJson json_token;
    json_token.type = std::string(lexer::tokenTypeName(token.type()));
    json_token.value = std::string(token.value(sm));
    json_token.line = loc.line;
    json_token.column = loc.column;
    json_token.offset = loc.offset;
    json_token.length = token.length();

    response.tokens.push_back(std::move(json_token));
  }

  // 使用 glaze 序列化为 JSON
  std::string json;
  auto result = glz::write_json(response, json);
  if (result) {
    // 序列化失败，返回错误 JSON
    return R"({"success": false, "error": "JSON serialization failed"})";
  }

  return json;
}

std::string
JsonFormatter::formatErrors(std::span<const lexer::LexerError> errors,
                            const lexer::SourceManager &sm) const {
  ErrorsResponse response;
  response.count = errors.size();
  response.errors.reserve(errors.size());

  for (const auto &error : errors) {
    const auto &loc = error.location;

    ErrorJson json_error;
    json_error.code = static_cast<int>(error.code);
    json_error.message = error.formattedMessage;
    json_error.file = std::string(sm.getFilename(loc.buffer));
    json_error.line = loc.line;
    json_error.column = loc.column;

    response.errors.push_back(std::move(json_error));
  }

  // 使用 glaze 序列化为 JSON
  std::string json;
  auto result = glz::write_json(response, json);
  if (result) {
    // 序列化失败，返回错误 JSON
    return R"({"success": false, "error": "JSON serialization failed"})";
  }

  return json;
}

// 工厂函数实现
std::unique_ptr<OutputFormatter> createFormatter(OutputFormat format) {
  switch (format) {
  case OutputFormat::Json:
    return std::make_unique<JsonFormatter>();
  case OutputFormat::Text:
  default:
    return std::make_unique<TextFormatter>();
  }
}

} // namespace czc::cli
