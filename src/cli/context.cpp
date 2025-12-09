/**
 * @file context.cpp
 * @brief 编译上下文实现。
 * @author BegoniaHe
 * @version 0.0.1
 * @date 2025-12-04
 */

#include "czc/cli/context.hpp"
#include "czc/diag/emitters/ansi_renderer.hpp"
#include "czc/diag/emitters/text_emitter.hpp"
#include "czc/diag/i18n.hpp"

#include <filesystem>
#include <iostream>

namespace czc::cli {

namespace {

/// 尝试加载 i18n 翻译文件到指定的 Translator
void loadI18nFiles(diag::i18n::Translator &translator) {
  // 尝试多个可能的路径
  std::vector<std::filesystem::path> searchPaths = {
      "resources/i18n/en.toml",
      "../resources/i18n/en.toml",
      "../../resources/i18n/en.toml",
      std::filesystem::current_path() / "resources/i18n/en.toml",
  };

  for (const auto &path : searchPaths) {
    if (std::filesystem::exists(path)) {
      (void)translator.loadFromFile(path);
      return;
    }
  }
}

} // namespace

CompilerContext::CompilerContext() { initDiagContext(); }

CompilerContext::CompilerContext(GlobalOptions global, OutputOptions output)
    : global_(std::move(global)), output_(std::move(output)) {
  initDiagContext();
}

void CompilerContext::initDiagContext() {
  // 创建 Translator 并加载翻译文件
  auto translator = std::make_unique<diag::i18n::Translator>();
  loadI18nFiles(*translator);

  // 创建 ANSI 样式
  auto style = global_.colorDiagnostics ? diag::AnsiStyle::defaultStyle()
                                        : diag::AnsiStyle(); // 空样式 = 无颜色

  // 创建默认的 TextEmitter
  auto emitter = std::make_unique<diag::TextEmitter>(std::cerr, style);

  // 创建 DiagContext
  diag::DiagConfig config;
  config.colorOutput = global_.colorDiagnostics;
  diagContext_ = std::make_unique<diag::DiagContext>(
      std::move(emitter), nullptr, config, std::move(translator));
}

} // namespace czc::cli
