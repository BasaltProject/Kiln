#pragma once

#include <filesystem>
#include <string>
#include <string_view>

namespace basalt::kiln {

/// Writes a new plugin under `root / id`; the empty string is success.
[[nodiscard]] std::string scaffold(const std::filesystem::path& root, std::string_view id, std::string_view name);

} // namespace basalt::kiln
