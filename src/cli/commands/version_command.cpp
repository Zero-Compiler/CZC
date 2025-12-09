/**
 * @file version_command.cpp
 * @brief 版本信息命令实现。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-30
 */

#include "czc/cli/commands/version_command.hpp"
#include "czc/cli/cli.hpp"

#include <iostream>

namespace czc::cli {

void VersionCommand::setup([[maybe_unused]] CLI::App *app) {
  // version 命令不需要额外选项
}

Result<int> VersionCommand::execute() {
  std::cout << kProgramName << " version " << kVersion.string << "\n";
  std::cout << "Built with C++23\n";

  // 编译器信息
#if defined(__clang__)
  std::cout << "Compiler: Clang " << __clang_major__ << "." << __clang_minor__
            << "." << __clang_patchlevel__ << "\n";
#elif defined(__GNUC__)
  std::cout << "Compiler: GCC " << __GNUC__ << "." << __GNUC_MINOR__ << "."
            << __GNUC_PATCHLEVEL__ << "\n";
#elif defined(_MSC_VER)
  std::cout << "Compiler: MSVC " << _MSC_VER << "\n";
#else
  std::cout << "Compiler: Unknown\n";
#endif

  return ok(0);
}

} // namespace czc::cli
