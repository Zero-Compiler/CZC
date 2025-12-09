/**
 * @file source_manager.cpp
 * @brief SourceManager 的实现。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-29
 */

#include "czc/lexer/source_manager.hpp"
#include "czc/lexer/token.hpp"

#include <algorithm>

namespace czc::lexer {

void SourceManager::Buffer::buildLineOffsets() const {
  if (lineOffsetsBuilt) {
    return;
  }

  lineOffsets.clear();
  lineOffsets.push_back(0); // 第一行从偏移 0 开始

  for (std::size_t i = 0; i < source.size(); ++i) {
    if (source[i] == '\n') {
      lineOffsets.push_back(i + 1); // 下一行从换行符后开始
    }
  }

  lineOffsetsBuilt = true;
}

BufferID SourceManager::addBuffer(std::string source, std::string filename) {
  Buffer buffer;
  buffer.source = std::move(source);
  buffer.filename = std::move(filename);
  buffer.isSynthetic = false;
  buffer.parentBuffer = std::nullopt;

  buffers_.push_back(std::move(buffer));

  // BufferID.value 从 1 开始，0 表示无效
  return BufferID{static_cast<std::uint32_t>(buffers_.size())};
}

BufferID SourceManager::addBuffer(std::string_view source,
                                  std::string filename) {
  return addBuffer(std::string(source), std::move(filename));
}

std::string_view SourceManager::getSource(BufferID id) const {
  if (!id.isValid() || id.value > buffers_.size()) {
    return {};
  }
  return buffers_[id.value - 1].source;
}

std::string_view SourceManager::slice(BufferID id, std::uint32_t offset,
                                      std::uint16_t length) const {
  if (!id.isValid() || id.value > buffers_.size()) {
    return {};
  }

  const auto &source = buffers_[id.value - 1].source;

  if (offset >= source.size()) {
    return {};
  }

  // 防止越界
  std::size_t actualLength =
      std::min(static_cast<std::size_t>(length), source.size() - offset);

  return std::string_view(source.data() + offset, actualLength);
}

std::string_view SourceManager::getFilename(BufferID id) const {
  if (!id.isValid() || id.value > buffers_.size()) {
    return {};
  }
  return buffers_[id.value - 1].filename;
}

std::string_view SourceManager::getLineContent(BufferID id,
                                               std::uint32_t lineNum) const {
  if (!id.isValid() || id.value > buffers_.size() || lineNum == 0) {
    return {};
  }

  const auto &buffer = buffers_[id.value - 1];
  buffer.buildLineOffsets();

  // lineNum 是 1-based
  std::size_t lineIndex = lineNum - 1;
  if (lineIndex >= buffer.lineOffsets.size()) {
    return {};
  }

  std::size_t lineStart = buffer.lineOffsets[lineIndex];
  std::size_t lineEnd;

  if (lineIndex + 1 < buffer.lineOffsets.size()) {
    // 下一行开始位置 - 1（不包含换行符）
    lineEnd = buffer.lineOffsets[lineIndex + 1];
    // 去掉换行符
    if (lineEnd > lineStart && buffer.source[lineEnd - 1] == '\n') {
      --lineEnd;
    }
    // 去掉可能的 \r
    if (lineEnd > lineStart && buffer.source[lineEnd - 1] == '\r') {
      --lineEnd;
    }
  } else {
    // 最后一行
    lineEnd = buffer.source.size();
  }

  return std::string_view(buffer.source.data() + lineStart,
                          lineEnd - lineStart);
}

BufferID SourceManager::addSyntheticBuffer(std::string source,
                                           std::string syntheticName,
                                           BufferID parentBuffer) {
  Buffer buffer;
  buffer.source = std::move(source);
  buffer.filename = std::move(syntheticName);
  buffer.isSynthetic = true;
  buffer.parentBuffer = parentBuffer;

  buffers_.push_back(std::move(buffer));
  return BufferID{static_cast<std::uint32_t>(buffers_.size())};
}

bool SourceManager::isSynthetic(BufferID id) const {
  if (!id.isValid() || id.value > buffers_.size()) {
    return false;
  }
  return buffers_[id.value - 1].isSynthetic;
}

std::optional<BufferID> SourceManager::getParentBuffer(BufferID id) const {
  if (!id.isValid() || id.value > buffers_.size()) {
    return std::nullopt;
  }
  return buffers_[id.value - 1].parentBuffer;
}

std::vector<std::string> SourceManager::getFileChain(BufferID id) const {
  std::vector<std::string> chain;

  BufferID current = id;
  while (current.isValid() && current.value <= buffers_.size()) {
    const auto &buffer = buffers_[current.value - 1];
    chain.push_back(buffer.filename);

    if (buffer.parentBuffer.has_value()) {
      current = buffer.parentBuffer.value();
    } else {
      break;
    }
  }

  return chain;
}

ExpansionID SourceManager::addExpansionInfo(ExpansionInfo info) {
  expansions_.push_back(std::move(info));
  return ExpansionID{static_cast<std::uint32_t>(expansions_.size())};
}

std::optional<std::reference_wrapper<const SourceManager::ExpansionInfo>>
SourceManager::getExpansionInfo(ExpansionID id) const {
  if (!id.isValid() || id.value > expansions_.size()) {
    return std::nullopt;
  }
  return std::cref(expansions_[id.value - 1]);
}

} // namespace czc::lexer
