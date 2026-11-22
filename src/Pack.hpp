#pragma once

#include "Project.hpp"

#include <expected>
#include <filesystem>
#include <string>
#include <string_view>

namespace basalt::kiln {

struct PackOptions {
    std::string_view packageExtension = ".bspkg";
};

/// Writes `build/<id>-<version>` plus the target's package extension under the plugin; the bytes depend only on the
/// files packed.
[[nodiscard]] std::expected<std::filesystem::path, std::string> pack(const Project& project,
                                                                     const PackOptions& options);

} // namespace basalt::kiln
