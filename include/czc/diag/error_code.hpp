/**
 * @file error_code.hpp
 * @brief 错误码定义与注册。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-12-04
 *
 * @details
 *   借鉴 rustc 的 ErrCode 和 Registry 设计，实现编译时注册、运行时查询。
 *   错误码格式: [分类字母][4位数字]，如 L1001
 */

#ifndef CZC_DIAG_ERROR_CODE_HPP
#define CZC_DIAG_ERROR_CODE_HPP

#include "czc/common/config.hpp"

#include <cstdint>
#include <functional>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace czc::diag {

/// 错误分类 - 决定错误码前缀
enum class ErrorCategory : uint8_t {
  Lexer = 1,   ///< L1xxx
  Parser = 2,  ///< P2xxx
  Sema = 3,    ///< S3xxx
  Codegen = 4, ///< C4xxx
  Driver = 5,  ///< D5xxx
};

/// 获取错误分类的前缀字符
[[nodiscard]] constexpr auto getCategoryPrefix(ErrorCategory cat) noexcept
    -> char {
  switch (cat) {
  case ErrorCategory::Lexer:
    return 'L';
  case ErrorCategory::Parser:
    return 'P';
  case ErrorCategory::Sema:
    return 'S';
  case ErrorCategory::Codegen:
    return 'C';
  case ErrorCategory::Driver:
    return 'D';
  default:
    return '?';
  }
}

/// 错误码 - 不可变值类型
/// 格式: [分类字母][4位数字]，如 L1001
struct ErrorCode {
  ErrorCategory category{ErrorCategory::Lexer};
  uint16_t code{0};

  /// 默认构造
  constexpr ErrorCode() = default;

  /// 构造错误码
  constexpr ErrorCode(ErrorCategory cat, uint16_t c) noexcept
      : category(cat), code(c) {}

  /// 转换为字符串表示，如 "L1001"
  [[nodiscard]] auto toString() const -> std::string;

  /// 计算哈希值
  [[nodiscard]] auto hash() const noexcept -> size_t {
    return std::hash<uint32_t>{}((static_cast<uint32_t>(category) << 16) |
                                 code);
  }

  /// 检查是否有效
  [[nodiscard]] constexpr auto isValid() const noexcept -> bool {
    return code != 0;
  }

  auto operator<=>(const ErrorCode &) const = default;
};

/// ErrorCode 哈希函数对象
struct ErrorCodeHash {
  auto operator()(const ErrorCode &ec) const noexcept -> size_t {
    return ec.hash();
  }
};

/// 错误条目 - 注册表中的条目
struct ErrorEntry {
  ErrorCode code;                  ///< 错误码
  std::string_view brief;          ///< 简短描述（英文，不翻译）
  std::string_view explanationKey; ///< i18n 键名
};

/// 错误注册表 - 全局单例，线程安全
/// 借鉴 rustc Registry 设计
class ErrorRegistry {
public:
  /// 获取全局单例
  [[nodiscard]] static auto instance() -> ErrorRegistry &;

  /// 注册错误码
  void registerError(ErrorCode code, std::string_view brief,
                     std::string_view explanationKey);

  /// 查找错误码
  [[nodiscard]] auto lookup(ErrorCode code) const -> std::optional<ErrorEntry>;

  /// 获取所有已注册的错误码
  [[nodiscard]] auto allCodes() const -> std::vector<ErrorCode>;

  /// 检查错误码是否已注册
  [[nodiscard]] auto isRegistered(ErrorCode code) const -> bool;

  // 禁止拷贝和移动
  ErrorRegistry(const ErrorRegistry &) = delete;
  auto operator=(const ErrorRegistry &) -> ErrorRegistry & = delete;
  ErrorRegistry(ErrorRegistry &&) = delete;
  auto operator=(ErrorRegistry &&) -> ErrorRegistry & = delete;

private:
  ErrorRegistry() = default;

  mutable std::shared_mutex mutex_;
  std::unordered_map<ErrorCode, ErrorEntry, ErrorCodeHash> entries_;
};

} // namespace czc::diag

// ============================================================================
// 错误码注册宏
// ============================================================================

/// 在头文件中声明错误码常量
/// 用法: CZC_DECLARE_ERROR(kMissingHexDigits, Lexer, 1001)
#define CZC_DECLARE_ERROR(NAME, CAT, CODE)                                     \
  inline constexpr ::czc::diag::ErrorCode NAME {                               \
    ::czc::diag::ErrorCategory::CAT, CODE                                      \
  }

/// 在源文件中注册错误码详情
/// 用法: CZC_REGISTER_ERROR(kMissingHexDigits, "brief", "i18n.key")
#define CZC_REGISTER_ERROR(NAME, BRIEF, EXPLANATION_KEY)                       \
  static const bool kRegistered_##NAME = [] {                                  \
    ::czc::diag::ErrorRegistry::instance().registerError(NAME, BRIEF,          \
                                                         EXPLANATION_KEY);     \
    return true;                                                               \
  }()

/// 模块错误码命名空间开始
#define CZC_BEGIN_ERROR_CODES(MODULE) namespace czc::MODULE::errors {

/// 模块错误码命名空间结束
#define CZC_END_ERROR_CODES() } // namespace

#endif // CZC_DIAG_ERROR_CODE_HPP
