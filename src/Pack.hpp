#pragma once

#include "Package.hpp"
#include "Project.hpp"

#include <expected>
#include <filesystem>
#include <string>
#include <string_view>

namespace basalt::kiln {

struct PackOptions {
    /// Pack the bytecode that `build` writes instead of the Lua sources.
    bool compiled = false;
    std::string_view packageExtension = ".bspkg";
    BuildOptions build;
};

/// Writes `build/<id>-<version>` plus the target's package extension under the plugin; the bytes depend only on the
/// files packed.
[[nodiscard]] std::expected<std::filesystem::path, std::string> pack(const Project& project,
                                                                     const PackOptions& options);

} // namespace basalt::kiln
