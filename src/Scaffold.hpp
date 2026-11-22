#pragma once

#include "Languages.hpp"

#include <filesystem>
#include <string>
#include <string_view>

namespace basalt::kiln {

/// Writes a new plugin under `root / id` from the built-in template of `language`; the empty string is success.
[[nodiscard]] std::string scaffold(const std::filesystem::path& root, std::string_view id, std::string_view name,
                                   Language language, std::string_view target);

} // namespace basalt::kiln
