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

/// Downloads an https URL into `part`, continuing from its current size, following redirects and retrying on a
/// dropped connection. Certificates are always verified; plain http is refused.
[[nodiscard]] std::expected<void, std::string> httpsDownload(std::string_view url, const std::filesystem::path& part,
                                                             const ProgressFn& progress);

} // namespace basalt::kiln
