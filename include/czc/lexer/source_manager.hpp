/**
 * @file source_manager.hpp
 * @brief 源码生命周期管理器，统一管理所有源码缓冲区。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-11-29
 *
 * @details
 *   SourceManager 是编译器的核心组件，负责管理所有源码的生命周期。
 *   Token 仅存储 BufferID + 偏移量，通过 SourceManager 获取实际文本。
 *   这种设计确保 Token 的生命周期安全——只要 SourceManager 存活，Token
 * 就永远有效。
 *
 *   设计参考了 Clang、Swift、Rust 编译器的 SourceManager 架构。
 */

#ifndef CZC_LEXER_SOURCE_MANAGER_HPP
#define CZC_LEXER_SOURCE_MANAGER_HPP

#include "czc/common/config.hpp"

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace czc::lexer {

/**
 * @brief 源码缓冲区标识符，用于引用 SourceManager 中的源码。
 *
 * @details
 *   BufferID 是一个轻量级的句柄，用于标识 SourceManager 中的源码缓冲区。
 *   值为 0 表示无效的 BufferID。有效的 BufferID 从 1 开始。
 */
struct BufferID {
  std::uint32_t value{0};

  /// 检查 BufferID 是否相等
  [[nodiscard]] constexpr bool
  operator==(const BufferID &) const noexcept = default;

  /// 检查 BufferID 是否有效（非零）
  [[nodiscard]] constexpr bool isValid() const noexcept { return value != 0; }

  /// 创建一个无效的 BufferID
  [[nodiscard]] static constexpr BufferID invalid() noexcept {
    return BufferID{0};
  }
};

/**
 * @brief 宏展开标识符（预留，当前版本不使用）。
 *
 * @details
 *   ExpansionID 用于追踪 Token 是否来自宏展开，以及展开链信息。
 *   当前版本不实现宏系统，但预留此接口以便未来扩展。
 *
 * @note 此结构体当前未被使用，仅作为未来宏系统的设计预留。
 *       实际实现宏系统时，此结构体将用于：
 *       1. 追踪 Token 的原始位置
 *       2. 追踪 Token 的展开位置
 *       3. 支持嵌套宏展开链的追踪
 *
 * @todo 在实现宏系统时完善此结构体的功能。
 */
struct [[maybe_unused]] ExpansionID {
  std::uint32_t value{0};

  /// 检查 ExpansionID 是否相等
  [[nodiscard]] constexpr bool
  operator==(const ExpansionID &) const noexcept = default;

  /// 检查 ExpansionID 是否有效（非零）
  [[nodiscard]] constexpr bool isValid() const noexcept { return value != 0; }

  /// 创建一个无效的 ExpansionID
  [[nodiscard]] static constexpr ExpansionID invalid() noexcept {
    return ExpansionID{0};
  }
};

/**
 * @brief 源码生命周期管理器。
 *
 * @details
 *   所有源码缓冲区的生命周期由 SourceManager 统一管理。
 *   Token 仅存储 BufferID + 偏移量，通过 SourceManager 获取实际文本。
 *   只要 SourceManager 存活，Token 就永远有效。
 *
 * @note 不可拷贝，可移动
 */
class SourceManager {
public:
  SourceManager() = default;

  // 不可拷贝
  SourceManager(const SourceManager &) = delete;
  SourceManager &operator=(const SourceManager &) = delete;

  // 可移动
  SourceManager(SourceManager &&) noexcept = default;
  SourceManager &operator=(SourceManager &&) noexcept = default;

  ~SourceManager() = default;

  /**
   * @brief 添加源码缓冲区（移动语义，零拷贝）。
   *
   * @param source 源码内容（移动）
   * @param filename 文件名
   * @return 新分配的 BufferID
   */
  [[nodiscard]] BufferID addBuffer(std::string source, std::string filename);

  /**
   * @brief 添加源码缓冲区（拷贝 string_view）。
   *
   * @param source 源码内容（拷贝）
   * @param filename 文件名
   * @return 新分配的 BufferID
   */
  [[nodiscard]] BufferID addBuffer(std::string_view source,
                                   std::string filename);

  /**
   * @brief 获取整个源码。
   *
   * @param id 缓冲区 ID
   * @return 源码视图，若 ID 无效则返回空视图
   *
   * @warning 返回的 string_view 的生命周期与 SourceManager 绑定。
   *          只要 SourceManager 实例存活，返回值就有效。
   */
  [[nodiscard]] std::string_view getSource(BufferID id) const;

  /**
   * @brief 获取源码切片。
   *
   * @param id 缓冲区 ID
   * @param offset 起始字节偏移
   * @param length 字节长度
   * @return 源码切片视图，若参数无效则返回空视图
   *
   * @warning 返回的 string_view 的生命周期与 SourceManager 绑定。
   *          只要 SourceManager 实例存活，返回值就有效。
   */
  [[nodiscard]] std::string_view slice(BufferID id, std::uint32_t offset,
                                       std::uint16_t length) const;

  /**
   * @brief 获取文件名。
   *
   * @param id 缓冲区 ID
   * @return 文件名视图，若 ID 无效则返回空视图
   *
   * @warning 返回的 string_view 的生命周期与 SourceManager 绑定。
   */
  [[nodiscard]] std::string_view getFilename(BufferID id) const;

  /**
   * @brief 获取指定行的内容。
   *
   * @param id 缓冲区 ID
   * @param lineNum 行号（1-based）
   * @return 行内容视图（不含换行符），若参数无效则返回空视图
   *
   * @warning 返回的 string_view 的生命周期与 SourceManager 绑定。
   */
  [[nodiscard]] std::string_view getLineContent(BufferID id,
                                                std::uint32_t lineNum) const;

  /**
   * @brief 获取缓冲区数量。
   *
   * @return 已添加的缓冲区数量
   */
  [[nodiscard]] std::size_t bufferCount() const noexcept {
    return buffers_.size();
  }

  /**
   * @brief 添加虚拟文件缓冲区（宏展开生成的代码）。
   *
   * @param source 生成的源码
   * @param syntheticName 虚拟文件名，如 "<derive(Debug) for Foo>"
   * @param parentBuffer 宏调用所在的文件（直接父级）
   * @return 新分配的 BufferID
   */
  [[nodiscard]] BufferID addSyntheticBuffer(std::string source,
                                            std::string syntheticName,
                                            BufferID parentBuffer);

  /**
   * @brief 查询文件是否为虚拟文件（宏展开生成）。
   *
   * @param id 缓冲区 ID
   * @return 若为虚拟文件返回 true
   */
  [[nodiscard]] bool isSynthetic(BufferID id) const;

  /**
   * @brief 获取虚拟文件的直接父级缓冲区。
   *
   * @param id 缓冲区 ID
   * @return 父级 BufferID，若不存在则返回 std::nullopt
   */
  [[nodiscard]] std::optional<BufferID> getParentBuffer(BufferID id) const;

  /**
   * @brief 获取文件链（从当前文件追溯到最终的真实文件）。
   *
   * @details
   *   用于错误报告，如：src/main.czc -> <macro foo> -> <macro bar>
   *
   * @param id 缓冲区 ID
   * @return 文件名链，从最内层到最外层
   */
  [[nodiscard]] std::vector<std::string> getFileChain(BufferID id) const;

  /**
   * @brief 宏展开信息结构。
   *
   * @details
   *   使用基本类型存储位置信息，避免与 SourceLocation 的循环依赖。
   */
  struct ExpansionInfo {
    BufferID callSiteBuffer;       ///< 宏调用所在的缓冲区
    std::uint32_t callSiteOffset;  ///< 宏调用的字节偏移
    std::uint32_t callSiteLine;    ///< 宏调用的行号
    std::uint32_t callSiteColumn;  ///< 宏调用的列号
    BufferID macroDefBuffer;       ///< 宏定义所在的缓冲区
    std::uint32_t macroNameOffset; ///< 宏名在缓冲区中的偏移
    std::uint16_t macroNameLength; ///< 宏名长度
    ExpansionID parent;            ///< 父级展开（嵌套宏），invalid() 表示最外层
  };

  /**
   * @brief 添加宏展开信息。
   *
   * @param info 宏展开信息结构体
   * @return 新分配的 ExpansionID
   */
  [[nodiscard]] ExpansionID addExpansionInfo(ExpansionInfo info);

  /**
   * @brief 获取宏展开信息（当前版本不实现）。
   *
   * @param id 展开 ID
   * @return 展开信息的引用包装，若 ID 无效则返回 std::nullopt
   *
   * @note 生命周期由 SourceManager 管理。返回的引用只在以下条件下有效：
   *       - SourceManager 实例存活
   *       - 未向 SourceManager 添加新的展开信息（vector 可能重新分配）
   *       建议：获取后立即使用，不要长期持有引用。
   */
  [[nodiscard]] std::optional<std::reference_wrapper<const ExpansionInfo>>
  getExpansionInfo(ExpansionID id) const;

private:
  /**
   * @brief 内部缓冲区结构。
   */
  struct Buffer {
    std::string source;                           ///< 源码内容
    std::string filename;                         ///< 文件名
    mutable std::vector<std::size_t> lineOffsets; ///< 行偏移缓存（惰性构建）
    mutable bool lineOffsetsBuilt{false};         ///< 行偏移是否已构建

    // 虚拟文件支持
    bool isSynthetic{false};              ///< true 表示宏展开生成的虚拟文件
    std::optional<BufferID> parentBuffer; ///< 直接父级（用于追溯展开链）

    /**
     * @brief 惰性构建行偏移表。
     */
    void buildLineOffsets() const;
  };

  std::vector<Buffer> buffers_; ///< 稳定存储，BufferID.value 为索引+1
  std::vector<ExpansionInfo>
      expansions_; ///< 宏展开信息，ExpansionID.value 为索引+1
};

} // namespace czc::lexer

#endif // CZC_LEXER_SOURCE_MANAGER_HPP
