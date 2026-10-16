#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace basalt::kiln {

struct Diagnostic {
    std::string file;
    int line = 0;
    std::string message;
};

/// Walks a plugin folder: its manifest, every Lua source, each `require` and every name it takes from the API.
[[nodiscard]] std::vector<Diagnostic> verifyPlugin(const std::filesystem::path& root, std::string_view target);

[[nodiscard]] bool isPluginFolder(const std::filesystem::path& path);

} // namespace basalt::kiln
