/**
 * @file test_source_tracker_performance.cpp
 * @brief 验证 SourceTracker 行索引优化的性能提升 (Google Test 版本)。
 * @author BegoniaHe
 * @date 2025-11-11
 */

#include "czc/utils/source_tracker.hpp"

#include <chrono>
#include <sstream>

#include <gtest/gtest.h>

using namespace czc::utils;

// --- Test Fixtures ---

class SourceTrackerPerformanceTest : public ::testing::Test {
protected:
  std::string generate_large_source(size_t line_count) {
    std::ostringstream oss;
    for (size_t i = 1; i <= line_count; i++) {
      oss << "let variable_" << i << " = " << i << ";\n";
    }
    return oss.str();
  }
};

// --- Performance Tests ---

TEST_F(SourceTrackerPerformanceTest, RandomAccessPerformance) {
  const size_t line_count = 100000;
  const size_t access_count = 100;
  std::string source = generate_large_source(line_count);

  SourceTracker tracker(source, "test_large_file.zero");

  auto start = std::chrono::high_resolution_clock::now();

  for (size_t i = 0; i < access_count; i++) {
    size_t line_num = (i * 10) % line_count + 1;
    std::string line = tracker.get_source_line(line_num);
    EXPECT_FALSE(line.empty());
  }

  auto end = std::chrono::high_resolution_clock::now();
  auto duration =
      std::chrono::duration_cast<std::chrono::microseconds>(end - start);

  // Performance should be reasonable (less than 10ms per access on average)
  EXPECT_LT(duration.count() / access_count, 10000);
}

TEST_F(SourceTrackerPerformanceTest, CorrectLineRetrieval) {
  const size_t line_count = 1000;
  std::string source = generate_large_source(line_count);

  SourceTracker tracker(source, "test.zero");

  std::string line1 = tracker.get_source_line(1);
  std::string line500 = tracker.get_source_line(500);
  std::string line1000 = tracker.get_source_line(1000);

  EXPECT_FALSE(line1.empty());
  EXPECT_FALSE(line500.empty());
  EXPECT_FALSE(line1000.empty());

  EXPECT_NE(line1.find("variable_1"), std::string::npos);
  EXPECT_NE(line500.find("variable_500"), std::string::npos);
  EXPECT_NE(line1000.find("variable_1000"), std::string::npos);
}

// --- Edge Cases Tests ---

TEST_F(SourceTrackerPerformanceTest, EmptyFile) {
  SourceTracker empty_tracker("", "empty.zero");
  std::string empty_line = empty_tracker.get_source_line(1);
  EXPECT_TRUE(empty_line.empty());
}

TEST_F(SourceTrackerPerformanceTest, SingleLineNoNewline) {
  SourceTracker single_line_tracker("let x = 42;", "single.zero");
  std::string single_line = single_line_tracker.get_source_line(1);
  EXPECT_EQ(single_line, "let x = 42;");
}

TEST_F(SourceTrackerPerformanceTest, MultipleEmptyLines) {
  SourceTracker empty_lines_tracker("line1\n\n\nline4\n", "empty_lines.zero");
  std::string line2 = empty_lines_tracker.get_source_line(2);
  std::string line3 = empty_lines_tracker.get_source_line(3);
  EXPECT_TRUE(line2.empty());
  EXPECT_TRUE(line3.empty());
}

TEST_F(SourceTrackerPerformanceTest, OutOfRange) {
  SourceTracker tracker("line1\nline2\n", "test.zero");
  std::string out_of_range = tracker.get_source_line(999);
  EXPECT_TRUE(out_of_range.empty());
}
