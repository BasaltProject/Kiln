#include "Languages.hpp"

#include <algorithm>
#include <fstream>
#include <iterator>

namespace fs = std::filesystem;

namespace basalt::kiln {
namespace {

bool hasFileWith(const fs::path& root, std::initializer_list<std::string_view> extensions) {
    std::error_code error;
    if (!fs::is_directory(root, error)) {
        return false;
    }
    for (auto it = fs::recursive_directory_iterator(root, fs::directory_options::skip_permission_denied, error);
         it != fs::recursive_directory_iterator(); it.increment(error)) {
        if (error) {
            return false;
        }
        const std::string name = it->path().filename().string();
        if (name == "build" || name == "node_modules" || name == "target" || name.starts_with(".")) {
            it.disable_recursion_pending();
            continue;
        }
        if (it->is_regular_file()) {
            const std::string extension = it->path().extension().string();
            if (std::ranges::find(extensions, extension) != extensions.end()) {
                return true;
            }
        }
    }
    return false;
}

bool fileMentions(const fs::path& path, std::string_view needle) {
    std::ifstream in{path};
    const std::string text{std::istreambuf_iterator<char>{in}, std::istreambuf_iterator<char>{}};
    return text.find(needle) != std::string::npos;
}

} // namespace

const LanguageInfo& infoOf(Language language) {
    return *std::ranges::find(kLanguages, language, &LanguageInfo::language);
}

std::optional<Language> languageNamed(std::string_view name) {
    std::string lowered{name};
    std::ranges::transform(lowered, lowered.begin(),
                           [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    for (const LanguageInfo& info : kLanguages) {
        if (info.name == lowered) {
            return info.language;
        }
    }
    static constexpr std::array<std::pair<std::string_view, Language>, 10> kAliases{{
        {"c++", Language::Cpp},
        {"cxx", Language::Cpp},
        {"as", Language::AssemblyScript},
        {"ts", Language::AssemblyScript},
        {"typescript", Language::AssemblyScript},
        {"golang", Language::Go},
        {"tinygo", Language::Go},
        {"kt", Language::Kotlin},
        {"c#", Language::CSharp},
        {"cs", Language::CSharp},
    }};
    for (const auto& [alias, language] : kAliases) {
        if (alias == lowered) {
            return language;
        }
    }
    return std::nullopt;
}

std::string languageList() {
    std::string list;
    for (const LanguageInfo& info : kLanguages) {
        list += list.empty() ? "" : ", ";
        list += info.name;
    }
    return list;
}

std::optional<Language> detectLanguage(const fs::path& root, std::optional<std::string_view> declared) {
    if (declared.has_value()) {
        return languageNamed(*declared);
    }
    if (fs::exists(root / "Cargo.toml")) {
        return Language::Rust;
    }
    if (fs::exists(root / "build.zig") || hasFileWith(root / "src", {".zig"})) {
        return Language::Zig;
    }
    if (fs::exists(root / "asconfig.json") || hasFileWith(root / "assembly", {".ts"}) ||
        (fs::exists(root / "package.json") && fileMentions(root / "package.json", "assemblyscript"))) {
        return Language::AssemblyScript;
    }
    if (fs::exists(root / "go.mod")) {
        return Language::Go;
    }
    if (fs::exists(root / "build.gradle.kts") || hasFileWith(root / "src", {".kt"})) {
        return Language::Kotlin;
    }
    if (hasFileWith(root, {".csproj"}) || hasFileWith(root / "src", {".cs"})) {
        return Language::CSharp;
    }
    if (fs::exists(root / "pom.xml") || hasFileWith(root / "src", {".java"})) {
        return Language::Java;
    }
    if (hasFileWith(root / "src", {".cpp", ".cc", ".cxx"}) || fs::exists(root / "CMakeLists.txt")) {
        return Language::Cpp;
    }
    if (hasFileWith(root / "src", {".c"})) {
        return Language::C;
    }
    return std::nullopt;
}

} // namespace basalt::kiln
