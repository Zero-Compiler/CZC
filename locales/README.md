# CZC Compiler Localization

欢迎为 CZC 编译器贡献翻译！

Welcome to contribute translations for CZC compiler!

## 支持的语言 Supported Locales

- `en_US` - English (United States) - **默认 Default**
- `zh_CN` - 简体中文 (中国) - **已支持 Supported**
- `zh_TW` - 繁体中文 (台湾) - **欢迎贡献 Contributions welcome!**
- `ja_JP` - 日本語 - **欢迎贡献 Contributions welcome!**
- `ko_KR` - 한국어 - **欢迎贡献 Contributions welcome!**
- `fr_FR` - Français - **欢迎贡献 Contributions welcome!**

## 添加新语言 Adding a New Language

1. 创建新目录：`locales/<locale>/`
2. 复制 `en_US/diagnostics.toml` 到新目录
3. 翻译 `message` 和 `help` 字段
4. **不要修改** `source` 字段
5. 提交 Pull Request

Steps:

1. Create a new directory: `locales/<locale>/`
2. Copy `en_US/diagnostics.toml` to the new directory
3. Translate the `message` and `help` fields
4. **DO NOT** modify the `source` field
5. Submit a Pull Request!

## 翻译指南 Translation Guidelines

- 保持技术术语的一致性（例如："literal"、"exponent"）
- 保持占位符格式：`{0}`, `{1}` 等
- 保持转义序列不变：`\\n`, `\\t` 等
- 保留格式，如引号和括号

Guidelines:

- Keep technical terms consistent (e.g., "literal", "exponent")
- Maintain the placeholder format: `{0}`, `{1}`, etc.
- Keep escape sequences intact: `\\n`, `\\t`, etc.
- Preserve formatting like quotes and brackets

## 示例 Example

```toml
[L0001]
message = "hexadecimal literal has no digits"  # 翻译这个 Translate this
help = "expected at least one hexadecimal digit after '0x'"  # 翻译这个 Translate this
source = "lexer"  # 不要改动 DO NOT change this
```

## 测试翻译 Testing Your Translation

```bash
# 使用 --locale 选项 Use --locale option
./czc-cli --locale zh_CN tokenize examples/test.zero

# 或者设置 ZERO_LOCALE_PATH 环境变量指定 locale 文件路径
# Or set ZERO_LOCALE_PATH environment variable to specify locale file path
export ZERO_LOCALE_PATH=/path/to/your/locales
./czc-cli --locale zh_CN tokenize examples/test.zero
```

## 环境变量 Environment Variables

- `ZERO_LOCALE_PATH`: 指定 locale 文件的搜索路径（可选）
  - Specify the search path for locale files (optional)
  - 例如 Example: `export ZERO_LOCALE_PATH=/usr/local/share/czc/locales`
  - 如果未设置，编译器将按以下顺序搜索：
    - If not set, the compiler searches in the following order:
    1. `locales/` (相对于当前目录 relative to current directory)
    2. `../locales/` (父目录 parent directory)
    3. `../../locales/` (祖父目录 grandparent directory)
    4. `/usr/local/share/czc/locales/` (系统安装路径 system installation path)

## 语言格式 Locale Format

语言代码遵循 IETF BCP 47 格式：
Locale codes follow IETF BCP 47 format:

- `en_US` - English (United States)
- `zh_CN` - Chinese (Simplified, China)
- `zh_TW` - Chinese (Traditional, Taiwan)
- `ja_JP` - Japanese (Japan)
- `ko_KR` - Korean (Korea)
- `fr_FR` - French (France)
