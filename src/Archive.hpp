#pragma once

#include <expected>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

namespace basalt::kiln {

enum class ArchiveFormat { TarGz, TarXz, Zip };

struct ExtractRules {
    /// Leading path components dropped from every entry.
    std::size_t skip = 0;
    /// After skipping, only entries below this folder are extracted, and the folder is dropped too.
    std::string only;
};

/// The path an entry lands on below the destination, or nothing when it is dropped; fails on a path that escapes.
[[nodiscard]] std::expected<std::optional<std::filesystem::path>, std::string>
entryDestination(std::string_view entry, const ExtractRules& rules);

/// Extracts below `destination`, refusing absolute paths, `..`, and symlinks or entries that would land outside.
[[nodiscard]] std::expected<void, std::string> extractArchive(const std::filesystem::path& archive,
                                                              ArchiveFormat format,
                                                              const std::filesystem::path& destination,
                                                              const ExtractRules& rules);

} // namespace basalt::kiln
