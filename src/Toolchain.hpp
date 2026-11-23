#pragma once

#include "ToolchainPins.hpp"

#include <cstdint>
#include <expected>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace basalt::kiln {

struct CacheSettings {
    std::filesystem::path root;
    bool offline = false;
    bool noDownload = false;
};

[[nodiscard]] CacheSettings& cacheSettings();

/// `--cache-dir`, then `$KILN_HOME`, then the per-user cache folder of the platform.
[[nodiscard]] std::filesystem::path defaultCacheRoot();

/// The current OS and CPU, for example linux-x64 or freebsd-arm64.
[[nodiscard]] std::string_view currentPlatform();

[[nodiscard]] std::vector<Pin> pinsFor(std::string_view id, std::string_view platform);

[[nodiscard]] std::vector<std::string_view> toolchainIds();

[[nodiscard]] std::string toolchainVersion(std::string_view id);

[[nodiscard]] std::filesystem::path toolchainDir(std::string_view id);

[[nodiscard]] bool isInstalled(std::string_view id);

/// Total bytes to download for a toolchain on this platform, 0 when the pins do not say.
[[nodiscard]] std::uint64_t downloadSize(std::string_view id);

/// The folder of an installed toolchain, downloading and unpacking it first when needed and allowed.
[[nodiscard]] std::expected<std::filesystem::path, std::string> ensureToolchain(std::string_view id);

[[nodiscard]] std::expected<void, std::string> cleanCache();

[[nodiscard]] std::string executableName(std::string_view name);

} // namespace basalt::kiln
