/**
 * @file source_reader.cpp
 * @brief SourceReader 的实现。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-29
 */

#include "czc/lexer/source_reader.hpp"
#include "czc/lexer/utf8.hpp"

namespace czc::lexer {

SourceReader::SourceReader(SourceManager &sm, BufferID buffer)
    : sm_(sm), buffer_(buffer), source_(sm.getSource(buffer)) {}

std::optional<char> SourceReader::current() const noexcept {
  if (position_ >= source_.size()) {
    return std::nullopt;
  }
  return source_[position_];
}

std::optional<char> SourceReader::peek(std::size_t offset) const noexcept {
  std::size_t peekPos = position_ + offset;
  if (peekPos >= source_.size()) {
    return std::nullopt;
  }
  return source_[peekPos];
}

bool SourceReader::isAtEnd() const noexcept {
  return position_ >= source_.size();
}

void SourceReader::advance() {
  if (position_ >= source_.size()) {
    return;
  }

  char ch = source_[position_];

  // 处理换行，更新行号和列号
  if (ch == '\n') {
    ++line_;
    column_ = 1;
  } else if (ch == '\r') {
    // 处理 \r\n 序列
    if (position_ + 1 < source_.size() && source_[position_ + 1] == '\n') {
      // \r\n 视为单个换行，\r 不单独更新行号
      // 行号更新在下一次 advance() 处理 \n 时进行
    } else {
      // 单独的 \r（老式 Mac 换行）
      ++line_;
      column_ = 1;
    }
  } else {
    // 对于 UTF-8 多字节字符，只在首字节时增加列号
    auto uch = static_cast<unsigned char>(ch);
    if (!utf8::isContinuationByte(uch)) {
      ++column_;
    }
  }

  ++position_;
}

void SourceReader::advance(std::size_t count) {
  for (std::size_t i = 0; i < count && position_ < source_.size(); ++i) {
    advance();
  }
}

SourceLocation SourceReader::location() const noexcept {
  return SourceLocation{buffer_, line_, column_,
                        static_cast<std::uint32_t>(position_)};
}

SourceReader::Slice
SourceReader::sliceFrom(std::size_t startOffset) const noexcept {
  Slice slice;
  slice.offset = static_cast<std::uint32_t>(startOffset);

  if (position_ >= startOffset) {
    std::size_t len = position_ - startOffset;
    // 截断为 uint16_t 最大值
    slice.length = static_cast<std::uint16_t>(len > 0xFFFF ? 0xFFFF : len);
  } else {
    slice.length = 0;
  }

  return slice;
}

std::string_view SourceReader::textFrom(std::size_t startOffset) const {
  if (startOffset >= source_.size() || startOffset > position_) {
    return {};
  }
  return source_.substr(startOffset, position_ - startOffset);
}

} // namespace czc::lexer
