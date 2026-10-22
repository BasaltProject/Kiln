#pragma once

#include "Package.hpp"
#include "Project.hpp"

#include <expected>
#include <filesystem>
#include <string>

namespace basalt::kiln {

struct PackOptions {
    /// Pack the bytecode that `build` writes instead of the Lua sources.
    bool compiled = false;
    BuildOptions build;
};

/// Writes `build/<id>-<version>.bspkg` under the plugin; the bytes depend only on the files packed.
[[nodiscard]] std::expected<std::filesystem::path, std::string> pack(const Project& project,
                                                                     const PackOptions& options);

} // namespace basalt::kiln
