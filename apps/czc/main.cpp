/**
 * @file main.cpp
 * @brief CZC 编译器命令行入口。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-30
 *
 * @details
 *   CZC 编译器的主入口点。
 *   采用门面模式，将所有 CLI 逻辑委托给 Cli 类处理。
 */

#include "czc/cli/cli.hpp"

/**
 * @brief 程序入口点。
 *
 * @param argc 命令行参数个数
 * @param argv 命令行参数数组
 * @return 程序退出码
 */
int main(int argc, char** argv) {
  czc::cli::Cli cli;
  return cli.run(argc, argv);
}
