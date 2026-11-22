#pragma once

#include "Plugin.hpp"

#include <cstdint>
#include <filesystem>
#include <span>
#include <string>
#include <vector>

namespace basalt::kiln {

/// The module a plugin's `main` names: next to the manifest, or in `build/` when it was built there.
[[nodiscard]] std::filesystem::path builtModulePath(const std::filesystem::path& root, const std::string& main);

[[nodiscard]] bool isWasmMain(const std::string& main);

/// Checks a module against the plugin ABI of a target (`server` or `proxy`) without running it: imports, exports,
/// memory and size.
[[nodiscard]] std::vector<Diagnostic> verifyModule(std::span<const std::uint8_t> bytes, const std::string& file,
                                                   std::string_view target = "server");

} // namespace basalt::kiln
