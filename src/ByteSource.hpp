#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <span>
#include <string>

namespace basalt::kiln {

/// A forward-only stream of bytes; `error` is non-empty once the stream is corrupt or unreadable.
class ByteSource {
public:
    virtual ~ByteSource() = default;

    /// Bytes stored in `out`; 0 at the end of the stream or after a failure.
    virtual std::size_t read(std::span<std::uint8_t> out) = 0;

    std::string error;
};

[[nodiscard]] std::unique_ptr<ByteSource> openFileSource(const std::filesystem::path& path);

[[nodiscard]] std::unique_ptr<ByteSource> openGzipSource(const std::filesystem::path& path);

[[nodiscard]] std::unique_ptr<ByteSource> openXzSource(const std::filesystem::path& path);

} // namespace basalt::kiln
