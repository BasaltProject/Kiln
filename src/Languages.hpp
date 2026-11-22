#pragma once

#include <array>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

namespace basalt::kiln {

enum class Language { Cpp, C, Zig, Rust, AssemblyScript, Go, Kotlin, CSharp, Java };

struct LanguageInfo {
    Language language;
    std::string_view name;
    std::string_view title;
};

inline constexpr std::array<LanguageInfo, 9> kLanguages{{
    {Language::Cpp, "cpp", "C++"},
    {Language::C, "c", "C"},
    {Language::Zig, "zig", "Zig"},
    {Language::Rust, "rust", "Rust"},
    {Language::AssemblyScript, "assemblyscript", "AssemblyScript"},
    {Language::Go, "go", "Go (TinyGo)"},
    {Language::Kotlin, "kotlin", "Kotlin"},
    {Language::CSharp, "csharp", "C#"},
    {Language::Java, "java", "Java"},
}};

[[nodiscard]] const LanguageInfo& infoOf(Language language);

/// Accepts the canonical name and common spellings such as `c++`, `ts`, `golang`, `kt` and `c#`.
[[nodiscard]] std::optional<Language> languageNamed(std::string_view name);

[[nodiscard]] std::string languageList();

/// The language a plugin folder is written in: `declared` (the `lang` key of plugin.toml) first, then its files.
[[nodiscard]] std::optional<Language> detectLanguage(const std::filesystem::path& root,
                                                     std::optional<std::string_view> declared);

} // namespace basalt::kiln
