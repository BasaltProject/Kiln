#pragma once

#include "Languages.hpp"

#include <expected>
#include <filesystem>
#include <optional>
#include <string>

namespace basalt::kiln {

struct BuildOptions {
    std::optional<Language> language;
    std::optional<std::string> target;
    bool verbose = false;
};

/// Compiles the plugin's sources to the module its `main` names, under `build/`; returns that path.
[[nodiscard]] std::expected<std::filesystem::path, std::string> buildPlugin(const std::filesystem::path& root,
                                                                            const BuildOptions& options);

} // namespace basalt::kiln
