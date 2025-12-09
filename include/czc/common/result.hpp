/**
 * @file result.hpp
 * @brief 错误处理类型定义，基于 C++23 std::expected。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-30
 *
 * @details
 *   本文件定义了项目统一的错误处理类型：
 *   - Error: 错误信息结构
 *   - Result<T>: 结果类型别名
 *   - VoidResult: 无返回值的结果类型
 */

#ifndef CZC_COMMON_RESULT_HPP
#define CZC_COMMON_RESULT_HPP

#include "czc/common/config.hpp"

#include <expected>
#include <source_location>
#include <string>
#include <string_view>

namespace czc {

/**
 * @brief 错误信息结构。
 *
 * @details
 *   统一的错误表示，包含错误消息、错误码和源码位置。
 */
struct Error {
  std::string message;           ///< 错误消息
  std::string code;              ///< 错误码，如 "E001"
  std::source_location location; ///< 错误发生的源码位置

  /**
   * @brief 构造错误对象。
   *
   * @param msg 错误消息
   * @param err_code 错误码
   * @param loc 源码位置（默认为调用位置）
   */
  explicit Error(std::string_view msg, std::string_view err_code = "",
                 std::source_location loc = std::source_location::current())
      : message(msg), code(err_code), location(loc) {}

  /**
   * @brief 格式化错误信息。
   *
   * @return 格式化后的错误字符串
   */
  [[nodiscard]] std::string format() const {
    std::string result;
    if (!code.empty()) {
      result += "[" + code + "] ";
    }
    result += message;
    return result;
  }

  /**
   * @brief 格式化错误信息（含位置）。
   *
   * @return 格式化后的错误字符串
   */
  [[nodiscard]] std::string formatWithLocation() const {
    std::string result = format();
    result += "\n  at ";
    result += location.file_name();
    result += ":";
    result += std::to_string(location.line());
    result += ":";
    result += std::to_string(location.column());
    result += " in ";
    result += location.function_name();
    return result;
  }
};

/**
 * @brief 结果类型别名，使用 std::expected。
 *
 * @tparam T 成功时的值类型
 */
template <typename T> using Result = std::expected<T, Error>;

/**
 * @brief 无返回值的结果类型。
 */
using VoidResult = std::expected<void, Error>;

/**
 * @brief 创建成功结果的辅助函数。
 *
 * @tparam T 值类型
 * @param value 成功值
 * @return 包含成功值的 Result
 */
template <typename T> [[nodiscard]] constexpr Result<T> ok(T &&value) {
  return Result<T>(std::forward<T>(value));
}

/**
 * @brief 创建成功结果的辅助函数。
 *
 * @return 成功的 VoidResult
 */
[[nodiscard]] inline constexpr VoidResult ok() { return VoidResult(); }

/**
 * @brief 创建错误结果的辅助函数。
 *
 * @tparam T 期望的值类型
 * @param msg 错误消息
 * @param code 错误码
 * @param loc 源码位置
 * @return 包含错误的 Result
 */
template <typename T>
[[nodiscard]] Result<T>
err(std::string_view msg, std::string_view code = "",
    std::source_location loc = std::source_location::current()) {
  return std::unexpected(Error(msg, code, loc));
}

/**
 * @brief 创建错误结果的辅助函数。
 *
 * @param msg 错误消息
 * @param code 错误码
 * @param loc 源码位置
 * @return 包含错误的 VoidResult
 */
[[nodiscard]] inline VoidResult
errVoid(std::string_view msg, std::string_view code = "",
        std::source_location loc = std::source_location::current()) {
  return std::unexpected(Error(msg, code, loc));
}

} // namespace czc

#endif // CZC_COMMON_RESULT_HPP
