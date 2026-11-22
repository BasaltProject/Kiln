#include "FileLock.hpp"

#if defined(_WIN32)
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>
#endif

namespace fs = std::filesystem;

namespace basalt::kiln {

struct FileLock::Handle {
#if defined(_WIN32)
    HANDLE file = INVALID_HANDLE_VALUE;

    ~Handle() {
        if (file != INVALID_HANDLE_VALUE) {
            OVERLAPPED overlapped{};
            UnlockFileEx(file, 0, MAXDWORD, MAXDWORD, &overlapped);
            CloseHandle(file);
        }
    }
#else
    int descriptor = -1;

    ~Handle() {
        if (descriptor >= 0) {
            flock(descriptor, LOCK_UN);
            close(descriptor);
        }
    }
#endif
};

FileLock::FileLock(std::unique_ptr<Handle> handle) : handle_(std::move(handle)) {}

FileLock::FileLock(FileLock&&) noexcept = default;

FileLock& FileLock::operator=(FileLock&&) noexcept = default;

FileLock::~FileLock() = default;

std::optional<FileLock> FileLock::acquire(const fs::path& path, std::string& error) {
    std::error_code ignored;
    fs::create_directories(path.parent_path(), ignored);
    auto handle = std::make_unique<Handle>();
#if defined(_WIN32)
    handle->file = CreateFileW(path.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                               OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (handle->file == INVALID_HANDLE_VALUE) {
        error = "cannot open the lock file " + path.string();
        return std::nullopt;
    }
    OVERLAPPED overlapped{};
    if (!LockFileEx(handle->file, LOCKFILE_EXCLUSIVE_LOCK, 0, MAXDWORD, MAXDWORD, &overlapped)) {
        error = "cannot lock " + path.string();
        return std::nullopt;
    }
#else
    handle->descriptor = open(path.c_str(), O_RDWR | O_CREAT | O_CLOEXEC, 0644);
    if (handle->descriptor < 0) {
        error = "cannot open the lock file " + path.string();
        return std::nullopt;
    }
    if (flock(handle->descriptor, LOCK_EX) != 0) {
        error = "cannot lock " + path.string();
        return std::nullopt;
    }
#endif
    return FileLock{std::move(handle)};
}

} // namespace basalt::kiln
