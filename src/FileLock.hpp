#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <string>

namespace basalt::kiln {

/// An exclusive advisory lock, released when the object goes away.
class FileLock {
public:
    [[nodiscard]] static std::optional<FileLock> acquire(const std::filesystem::path& path, std::string& error,
                                                         bool wait = true);

    FileLock(FileLock&&) noexcept;
    FileLock& operator=(FileLock&&) noexcept;
    FileLock(const FileLock&) = delete;
    FileLock& operator=(const FileLock&) = delete;
    ~FileLock();

private:
    struct Handle;

    explicit FileLock(std::unique_ptr<Handle> handle);

    std::unique_ptr<Handle> handle_;
};

} // namespace basalt::kiln
