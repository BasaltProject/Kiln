#pragma once

#include <cstdint>
#include <expected>
#include <filesystem>
#include <functional>
#include <string>
#include <string_view>

namespace basalt::kiln {

/// Called with the bytes received so far and the expected total (0 when the server did not say).
using ProgressFn = std::function<void(std::uint64_t received, std::uint64_t total)>;

struct DownloadOptions {
    std::uint64_t maxBytes = UINT64_MAX;
    std::uint32_t readTimeoutMs = 60000;
    int attempts = 4;
};

/// Downloads over HTTPS with certificate verification, redirects and partial-file resume.
[[nodiscard]] std::expected<void, std::string> httpsDownload(std::string_view url, const std::filesystem::path& part,
                                                             const ProgressFn& progress,
                                                             const DownloadOptions& options = {});

} // namespace basalt::kiln
