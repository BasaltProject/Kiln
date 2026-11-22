#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace basalt::kiln {

struct Diagnostic {
    std::string file;
    int line = 0;
    std::string message;
};

/// Checks `plugin.toml` and, when the plugin is built, the module it names against the plugin ABI.
/// `target` overrides the manifest's `target` field (`server` by default).
[[nodiscard]] std::vector<Diagnostic> verifyPlugin(const std::filesystem::path& root,
                                                   std::optional<std::string_view> target = std::nullopt);

/// The `target` field of the plugin's manifest, `server` when it has none or cannot be read.
[[nodiscard]] std::string manifestTarget(const std::filesystem::path& root);

/// Lower case letters, digits and `_`, starting with a letter.
[[nodiscard]] bool validPluginId(std::string_view id);

[[nodiscard]] bool isPluginFolder(const std::filesystem::path& path);

} // namespace basalt::kiln
