/**
 * @file config.hpp
 * @brief 项目统一配置和编译器特性检测。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-30
 *
 * @details
 *   本文件提供项目统一的配置定义：
 *   - C++ 版本检查
 *   - 编译器特性检测
 *   - 平台相关宏定义
 *
 *   所有模块应包含此头文件以确保一致的编译环境。
 */

#ifndef CZC_COMMON_CONFIG_HPP
#define CZC_COMMON_CONFIG_HPP

#include <cstddef> // for std::size_t

// =============================================================================
// C++ 版本检查
// =============================================================================

#if __cplusplus < 202302L
#error "CZC requires C++23 or later. Please use a C++23 compliant compiler."
#endif

// =============================================================================
// C++23 特性检测
// =============================================================================

// std::expected (C++23)
#ifdef __cpp_lib_expected
#define CZC_HAS_EXPECTED 1
#else
#define CZC_HAS_EXPECTED 0
#endif

// std::unreachable (C++23)
#ifdef __cpp_lib_unreachable
#define CZC_HAS_UNREACHABLE 1
#else
#define CZC_HAS_UNREACHABLE 0
#endif

// std::ranges (C++20)
#ifdef __cpp_lib_ranges
#define CZC_HAS_RANGES 1
#else
#define CZC_HAS_RANGES 0
#endif

// std::source_location (C++20)
#ifdef __cpp_lib_source_location
#define CZC_HAS_SOURCE_LOCATION 1
#else
#define CZC_HAS_SOURCE_LOCATION 0
#endif

// =============================================================================
// 编译器检测
// =============================================================================

#if defined(__clang__)
#define CZC_COMPILER_CLANG 1
#define CZC_COMPILER_VERSION                                                   \
  (__clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__)
#elif defined(__GNUC__)
#define CZC_COMPILER_GCC 1
#define CZC_COMPILER_VERSION                                                   \
  (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#elif defined(_MSC_VER)
#define CZC_COMPILER_MSVC 1
#define CZC_COMPILER_VERSION _MSC_VER
#else
#define CZC_COMPILER_UNKNOWN 1
#define CZC_COMPILER_VERSION 0
#endif

#if defined(_WIN32) || defined(_WIN64)
#define CZC_PLATFORM_WINDOWS 1
#elif defined(__APPLE__) && defined(__MACH__)
#define CZC_PLATFORM_MACOS 1
#elif defined(__linux__)
#define CZC_PLATFORM_LINUX 1
#else
#define CZC_PLATFORM_UNKNOWN 1
#endif

/// 标记未使用的参数，避免编译器警告
#define CZC_UNUSED(x) (void)(x)

/// 强制内联（性能关键路径）
#if defined(CZC_COMPILER_CLANG) || defined(CZC_COMPILER_GCC)
#define CZC_FORCE_INLINE __attribute__((always_inline)) inline
#elif defined(CZC_COMPILER_MSVC)
#define CZC_FORCE_INLINE __forceinline
#else
#define CZC_FORCE_INLINE inline
#endif

/// 禁止内联
#if defined(CZC_COMPILER_CLANG) || defined(CZC_COMPILER_GCC)
#define CZC_NOINLINE __attribute__((noinline))
#elif defined(CZC_COMPILER_MSVC)
#define CZC_NOINLINE __declspec(noinline)
#else
#define CZC_NOINLINE
#endif

/// 不可达代码标记（C++23 std::unreachable）
#if CZC_HAS_UNREACHABLE
#include <utility>
#define CZC_UNREACHABLE() std::unreachable()
#elif defined(CZC_COMPILER_CLANG) || defined(CZC_COMPILER_GCC)
#define CZC_UNREACHABLE() __builtin_unreachable()
#elif defined(CZC_COMPILER_MSVC)
#define CZC_UNREACHABLE() __assume(false)
#else
#define CZC_UNREACHABLE() ((void)0)
#endif

// =============================================================================
// 项目常量
// =============================================================================

namespace czc {

/// 项目版本信息
inline constexpr struct {
  int major = 0;
  int minor = 0;
  int patch = 1;
  const char *string = "0.0.1";
} kVersion;

/// 资源限制常量
inline constexpr struct {
  /// 最大源文件大小 (16 MB)
  std::size_t maxFileSize = 16 * 1024 * 1024;

  /// 最大 Token 长度 (64 KB)
  std::size_t maxTokenLength = 64 * 1024;

  /// 最大行长度 (4 KB)
  std::size_t maxLineLength = 4 * 1024;

  /// 最大嵌套深度
  std::size_t maxNestingDepth = 256;
} kLimits;

} // namespace czc

#endif // CZC_COMMON_CONFIG_HPP
