#include "FileLock.hpp"
#include "Process.hpp"
#include "Update.hpp"

#include <array>
#include <charconv>
#include <fstream>
#include <iostream>

#if defined(_WIN32)
// clang-format off
#include <windows.h>
#include <msi.h>
// clang-format on
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#include <unistd.h>
#elif defined(__FreeBSD__)
#include <sys/sysctl.h>
#include <unistd.h>
#else
#include <unistd.h>
#endif

namespace fs = std::filesystem;

namespace basalt::kiln {
namespace {

std::expected<void, std::string> replacePortable(const fs::path& executable, const fs::path& source,
                                                 std::string_view version) {
    std::error_code error;
    fs::permissions(source, fs::perms::owner_all, fs::perm_options::replace, error);
    if (error) {
        return std::unexpected("cannot make the downloaded binary executable: " + error.message());
    }
    const ProcessResult probe = runProcess({source.string(), "--version"}, source.parent_path());
    if (!probe.started || probe.exitCode != 0 ||
        (probe.output != "Kiln " + std::string{version} + "\n" &&
         probe.output != "Kiln " + std::string{version} + "\r\n")) {
        return std::unexpected("the downloaded executable failed its version check; the installed version was kept");
    }
    const ProcessResult current = runProcess({executable.string(), "--version"}, executable.parent_path());
    std::string installed = current.output;
    while (!installed.empty() && (installed.back() == '\n' || installed.back() == '\r')) {
        installed.pop_back();
    }
    if (!current.started || current.exitCode != 0 || !installed.starts_with("Kiln ")) {
        return std::unexpected("cannot read the installed version; nothing was replaced");
    }
    if (!newerVersion(version, std::string_view{installed}.substr(5))) {
        std::cout << "The installed executable is already at this release or a newer one.\n";
        return {};
    }
    const fs::path next = executable.string() + ".update";
    const fs::path previous = executable.string() + ".previous";
    if (fs::exists(next) || fs::is_symlink(next) || fs::is_symlink(previous)) {
        return std::unexpected("an update staging path already exists or is a symlink; resolve it before retrying");
    }
    fs::copy_file(source, next, fs::copy_options::none, error);
    if (!error) {
        fs::permissions(next, fs::status(executable).permissions(), fs::perm_options::replace, error);
    }
    if (error) {
        fs::remove(next);
        return std::unexpected("cannot stage the update beside the executable: " + error.message());
    }
    fs::copy_file(executable, previous, fs::copy_options::overwrite_existing, error);
    if (error) {
        fs::remove(next);
        return std::unexpected("cannot save the previous executable: " + error.message());
    }
#if defined(_WIN32)
    const bool replaced =
        MoveFileExW(next.c_str(), executable.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
    if (!replaced) {
        error = std::error_code{static_cast<int>(GetLastError()), std::system_category()};
    }
#else
    fs::rename(next, executable, error);
#endif
    if (error) {
        fs::remove(next);
        return std::unexpected("cannot replace the executable; the installed version was kept: " + error.message());
    }
    std::cout << "Kiln " << version << " installed. Previous executable: " << previous.string() << '\n';
    return {};
}

#if defined(_WIN32)
std::wstring quoted(const std::wstring& text) {
    std::wstring result = L"\"";
    std::size_t slashes = 0;
    for (const wchar_t c : text) {
        if (c == L'\\') {
            ++slashes;
            continue;
        }
        result.append(c == L'\"' ? slashes * 2 + 1 : slashes, L'\\');
        slashes = 0;
        result += c;
    }
    result.append(slashes * 2, L'\\');
    return result + L'\"';
}

std::expected<void, std::string> launchHelper(const fs::path& staging, const fs::path& executable) {
    const fs::path helper = staging / "kiln-update-helper.exe";
    fs::copy_file(executable, helper);
    for (const std::string_view runtime :
         {"msvcp140.dll", "msvcp140_1.dll", "msvcp140_2.dll", "msvcp140_atomic_wait.dll", "msvcp140_codecvt_ids.dll",
          "vcruntime140.dll", "vcruntime140_1.dll", "concrt140.dll"}) {
        const fs::path dll = executable.parent_path() / runtime;
        if (fs::is_regular_file(dll)) {
            fs::copy_file(dll, staging / runtime);
        }
    }
    std::wstring command = quoted(helper.wstring()) + L" __apply-update " + quoted(staging.wstring()) + L" " +
                           quoted(executable.wstring()) + L" " + std::to_wstring(GetCurrentProcessId());
    STARTUPINFOW startup{};
    startup.cb = sizeof(startup);
    PROCESS_INFORMATION process{};
    if (!CreateProcessW(helper.c_str(), command.data(), nullptr, nullptr, FALSE, CREATE_NEW_PROCESS_GROUP, nullptr,
                        staging.c_str(), &startup, &process)) {
        return std::unexpected("cannot start the update helper: " + std::to_string(GetLastError()));
    }
    CloseHandle(process.hThread);
    CloseHandle(process.hProcess);
    std::cout << "The update will be installed after Kiln exits. Log: " << (staging / "install.log").string() << '\n';
    return {};
}
#endif

} // namespace

std::expected<fs::path, std::string> runningExecutable() {
#if defined(_WIN32)
    std::array<wchar_t, 32768> buffer{};
    const DWORD size = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    if (size == 0 || size >= buffer.size()) {
        return std::unexpected("cannot locate the running executable");
    }
    return fs::canonical(fs::path{std::wstring{buffer.data(), size}});
#elif defined(__APPLE__)
    std::uint32_t size = 0;
    _NSGetExecutablePath(nullptr, &size);
    std::string buffer(size, '\0');
    if (_NSGetExecutablePath(buffer.data(), &size) != 0) {
        return std::unexpected("cannot locate the running executable");
    }
    return fs::canonical(fs::path{buffer.c_str()});
#elif defined(__FreeBSD__)
    std::array<char, 4096> buffer{};
    std::size_t size = buffer.size();
    std::array<int, 4> query{CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1};
    if (sysctl(query.data(), static_cast<unsigned int>(query.size()), buffer.data(), &size, nullptr, 0) != 0) {
        return std::unexpected("cannot locate the running executable");
    }
    return fs::canonical(fs::path{buffer.data()});
#else
    std::error_code error;
    const fs::path executable = fs::canonical("/proc/self/exe", error);
    if (error) {
        return std::unexpected("cannot locate the running executable: " + error.message());
    }
    return executable;
#endif
}

bool managedInstallation(const fs::path& executable) {
    if (fs::exists(executable.parent_path() / "kiln.installation")) {
        return true;
    }
#if !defined(_WIN32)
    const std::string path = executable.generic_string();
    for (const std::string_view prefix : {"/usr/", "/opt/", "/bin/", "/sbin/", "/nix/", "/snap/", "/app/"}) {
        if (path.starts_with(prefix)) {
            return true;
        }
    }
#endif
    return false;
}

std::expected<void, std::string> installUpdate(const fs::path& staging, const fs::path& executable,
                                               const UpdateRelease& release) {
    const bool managed = managedInstallation(executable);
    const fs::path asset = staging / updateAssetName(release.version, managed);
    if (const auto verified = verifyUpdateAsset(release, asset); !verified) {
        return verified;
    }
#if defined(_WIN32)
    return launchHelper(staging, executable);
#elif defined(__APPLE__)
    if (managed) {
        if (!fs::is_regular_file(executable.parent_path() / "kiln.installation")) {
            return std::unexpected("update this installation with its package manager");
        }
        const ProcessResult launched = runProcess({"/usr/bin/open", asset.string()}, staging);
        if (!launched.started || launched.exitCode != 0) {
            return std::unexpected("cannot open the verified installer: " + asset.string());
        }
        std::cout << "Complete the update in macOS Installer. Verified package: " << asset.string() << '\n';
        return {};
    }
#else
    if (managed) {
        return std::unexpected("this installation is managed by the system; update Kiln using its package manager");
    }
#endif
    return replacePortable(executable, asset, release.version);
}

int applyWindowsUpdate(const std::vector<std::string>& args) {
#if defined(_WIN32)
    if (args.size() != 4) {
        return 2;
    }
    const fs::path staging{args[1]};
    const fs::path executable{args[2]};
    DWORD pid = 0;
    const auto [end, error] = std::from_chars(args[3].data(), args[3].data() + args[3].size(), pid);
    if (error != std::errc{} || end != args[3].data() + args[3].size() || pid == GetCurrentProcessId()) {
        return 2;
    }
    std::ofstream log{staging / "install.log"};
    if (HANDLE parent = OpenProcess(SYNCHRONIZE, FALSE, pid); parent != nullptr) {
        const DWORD waited = WaitForSingleObject(parent, 120000);
        CloseHandle(parent);
        if (waited != WAIT_OBJECT_0) {
            log << "Kiln did not exit; no update was installed.\n";
            return 1;
        }
    }
    std::string lockError;
    const auto lock = FileLock::acquire(staging.parent_path() / "update.lock", lockError);
    if (!lock) {
        log << lockError << '\n';
        return 1;
    }
    const auto release = loadUpdateRelease(staging);
    if (!release) {
        log << release.error() << '\n';
        return 1;
    }
    const bool managed = managedInstallation(executable);
    const fs::path asset = staging / updateAssetName(release->version, managed);
    if (const auto verified = verifyUpdateAsset(*release, asset); !verified) {
        log << verified.error() << '\n';
        return 1;
    }
    if (managed) {
        const fs::path installerLog = staging / "msi.log";
        const DWORD logging = MsiEnableLogW(INSTALLLOGMODE_ERROR | INSTALLLOGMODE_WARNING | INSTALLLOGMODE_INFO |
                                                INSTALLLOGMODE_ACTIONSTART | INSTALLLOGMODE_ACTIONDATA |
                                                INSTALLLOGMODE_VERBOSE | INSTALLLOGMODE_PROPERTYDUMP,
                                            installerLog.c_str(), INSTALLLOGATTRIBUTES_FLUSHEACHLINE);
        if (logging != ERROR_SUCCESS) {
            log << "Cannot start MSI logging: " << logging << '\n';
            return 1;
        }
        MsiSetInternalUI(INSTALLUILEVEL_BASIC, nullptr);
        const std::wstring properties =
            L"INSTALL_ROOT=\"" + executable.parent_path().parent_path().wstring() + L"\" REBOOT=ReallySuppress";
        const UINT result = MsiInstallProductW(asset.c_str(), properties.c_str());
        MsiEnableLogW(0, nullptr, 0);
        log << "MSI exit code: " << result << '\n';
        return result == ERROR_SUCCESS || result == ERROR_SUCCESS_REBOOT_REQUIRED ? 0 : 1;
    }
    const auto installed = replacePortable(executable, asset, release->version);
    if (!installed) {
        log << installed.error() << '\n';
        return 1;
    }
    log << "Kiln " << release->version << " installed.\n";
    return 0;
#else
    static_cast<void>(args);
    return 2;
#endif
}

} // namespace basalt::kiln
