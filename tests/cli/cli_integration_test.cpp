/**
 * @file cli_integration_test.cpp
 * @brief CLI 模块集成测试。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-12-04
 *
 * @details
 *   本文件包含 CLI 模块的集成测试，验证：
 *   - 完整的命令行工作流程
 *   - 子命令的正确执行
 *   - 输入/输出处理
 *   - 错误处理和诊断输出
 */

#include "czc/cli/cli.hpp"
#include "czc/cli/driver.hpp"

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <sstream>

namespace czc::cli {
namespace {

class CliIntegrationTest : public ::testing::Test {
protected:
  std::filesystem::path testDir_;
  std::vector<std::string> argStorage_;
  std::vector<char *> argv_;

  void SetUp() override {
    // 创建临时测试目录
    testDir_ = std::filesystem::temp_directory_path() / "czc_cli_test";
    std::filesystem::create_directories(testDir_);
  }

  void TearDown() override {
    // 清理临时测试目录
    std::filesystem::remove_all(testDir_);
  }

  /**
   * @brief 创建临时测试文件。
   */
  std::filesystem::path createTestFile(std::string_view filename,
                                       std::string_view content) {
    auto path = testDir_ / filename;
    std::ofstream ofs(path);
    ofs << content;
    return path;
  }

  /**
   * @brief 将字符串参数转换为 argc/argv 格式。
   */
  void makeArgs(const std::vector<std::string> &args) {
    argStorage_ = args;
    argv_.clear();
    for (auto &arg : argStorage_) {
      argv_.push_back(arg.data());
    }
  }

  int getArgc() const { return static_cast<int>(argv_.size()); }
  char **getArgv() { return argv_.data(); }
};

// ============================================================================
// Cli 类基本测试
// ============================================================================

TEST_F(CliIntegrationTest, CliConstructsSuccessfully) {
  EXPECT_NO_THROW({ Cli cli; });
}

TEST_F(CliIntegrationTest, CliRequiresSubcommand) {
  Cli cli;
  makeArgs({"czc"});

  int result = cli.run(getArgc(), getArgv());

  // 没有子命令应该返回非零
  EXPECT_NE(result, 0);
}

// ============================================================================
// Version 命令测试
// ============================================================================

TEST_F(CliIntegrationTest, VersionFlag) {
  Cli cli;
  makeArgs({"czc", "--version"});

  // --version 会导致 CLI11 抛出 CallForVersion 异常
  // 在正常流程中这会被捕获并返回 0
  int result = cli.run(getArgc(), getArgv());
  EXPECT_EQ(result, 0);
}

// ============================================================================
// Lex 命令测试
// ============================================================================

TEST_F(CliIntegrationTest, LexCommandWithValidFile) {
  auto inputPath = createTestFile("valid.zero", "let x = 1;");

  Cli cli;
  makeArgs({"czc", "lex", inputPath.string()});

  int result = cli.run(getArgc(), getArgv());

  EXPECT_EQ(result, 0);
}

TEST_F(CliIntegrationTest, LexCommandWithNonExistentFile) {
  std::string nonExistent = (testDir_ / "does_not_exist.zero").string();

  Cli cli;
  makeArgs({"czc", "lex", nonExistent});

  int result = cli.run(getArgc(), getArgv());

  // 文件不存在应该返回非零
  EXPECT_NE(result, 0);
}

TEST_F(CliIntegrationTest, LexCommandWithTriviaFlag) {
  auto inputPath = createTestFile("trivia.zero", "let x = 1; // comment");

  Cli cli;
  makeArgs({"czc", "lex", "--trivia", inputPath.string()});

  int result = cli.run(getArgc(), getArgv());

  EXPECT_EQ(result, 0);
}

TEST_F(CliIntegrationTest, LexCommandWithJsonOutput) {
  auto inputPath = createTestFile("json.zero", "let x = 1;");
  auto outputPath = testDir_ / "output.json";

  Cli cli;
  // 全局选项 (-f, -o) 应放在子命令之前
  makeArgs({"czc", "-f", "json", "-o", outputPath.string(), "lex",
            inputPath.string()});

  int result = cli.run(getArgc(), getArgv());

  EXPECT_EQ(result, 0);
  EXPECT_TRUE(std::filesystem::exists(outputPath));

  // 验证输出是 JSON 格式
  std::ifstream ifs(outputPath);
  std::string content((std::istreambuf_iterator<char>(ifs)),
                      std::istreambuf_iterator<char>());
  EXPECT_EQ(content.front(), '{');
}

// ============================================================================
// 全局选项测试
// ============================================================================

TEST_F(CliIntegrationTest, VerboseFlag) {
  auto inputPath = createTestFile("verbose.zero", "let x = 1;");

  Cli cli;
  makeArgs({"czc", "-v", "lex", inputPath.string()});

  int result = cli.run(getArgc(), getArgv());

  EXPECT_EQ(result, 0);
  EXPECT_EQ(cli.driver().context().global().logLevel, LogLevel::Verbose);
}

TEST_F(CliIntegrationTest, QuietFlag) {
  auto inputPath = createTestFile("quiet.zero", "let x = 1;");

  Cli cli;
  makeArgs({"czc", "-q", "lex", inputPath.string()});

  int result = cli.run(getArgc(), getArgv());

  EXPECT_EQ(result, 0);
  EXPECT_EQ(cli.driver().context().global().logLevel, LogLevel::Quiet);
}

TEST_F(CliIntegrationTest, NoColorFlag) {
  auto inputPath = createTestFile("nocolor.zero", "let x = 1;");

  Cli cli;
  makeArgs({"czc", "--no-color", "lex", inputPath.string()});

  int result = cli.run(getArgc(), getArgv());

  EXPECT_EQ(result, 0);
  EXPECT_FALSE(cli.driver().context().global().colorDiagnostics);
}

// ============================================================================
// 错误处理测试
// ============================================================================

TEST_F(CliIntegrationTest, LexCommandWithSyntaxError) {
  auto inputPath = createTestFile("error.zero", R"(
let s = "unterminated
let x = 1;
)");

  Cli cli;
  makeArgs({"czc", "lex", inputPath.string()});

  int result = cli.run(getArgc(), getArgv());

  // 有语法错误应该返回非零
  EXPECT_NE(result, 0);
}

// ============================================================================
// 输出文件测试
// ============================================================================

TEST_F(CliIntegrationTest, OutputToFile) {
  auto inputPath = createTestFile("input.zero", "fn main() {}");
  auto outputPath = testDir_ / "tokens.txt";

  Cli cli;
  // 全局选项 (-o) 应放在子命令之前
  makeArgs({"czc", "-o", outputPath.string(), "lex", inputPath.string()});

  int result = cli.run(getArgc(), getArgv());

  EXPECT_EQ(result, 0);
  EXPECT_TRUE(std::filesystem::exists(outputPath));

  // 验证输出文件不为空
  auto fileSize = std::filesystem::file_size(outputPath);
  EXPECT_GT(fileSize, 0u);
}

// ============================================================================
// 复杂源文件测试
// ============================================================================

TEST_F(CliIntegrationTest, LexComplexSourceFile) {
  auto inputPath = createTestFile("complex.zero", R"(
// 复杂的源文件示例
fn fibonacci(n: i32) -> i32 {
    if n <= 1 {
        return n;
    }
    return fibonacci(n - 1) + fibonacci(n - 2);
}

struct Point {
    x: f64,
    y: f64,
}

impl Point {
    fn distance(self, other: Point) -> f64 {
        let dx = self.x - other.x;
        let dy = self.y - other.y;
        return (dx * dx + dy * dy).sqrt();
    }
}

fn main() {
    let n = 10;
    let result = fibonacci(n);
    
    let p1 = Point { x: 0.0, y: 0.0 };
    let p2 = Point { x: 3.0, y: 4.0 };
    let dist = p1.distance(p2);
}
)");

  Cli cli;
  makeArgs({"czc", "lex", inputPath.string()});

  int result = cli.run(getArgc(), getArgv());

  EXPECT_EQ(result, 0);
}

} // namespace
} // namespace czc::cli
