#include "Update.hpp"

#include "FileLock.hpp"
#include "Http.hpp"
#include "Toolchain.hpp"

#include <basalt/wire/common/toml/Toml.hpp>

#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <random>

namespace fs = std::filesystem;

namespace basalt::kiln {
namespace {

constexpr std::string_view kReleaseRoot = "https://github.com/BasaltProject/Kiln/releases/";
constexpr std::int64_t kCheckInterval = 24 * 60 * 60;

std::string environment(const char* name) {
    const char* value = std::getenv(name);
    return value == nullptr ? std::string{} : std::string{value};
}

fs::path configPath() {
    if (const std::string chosen = environment("KILN_CONFIG_HOME"); !chosen.empty()) {
        return fs::path{chosen} / "updates.toml";
    }
#if defined(_WIN32)
    return fs::path{environment("LOCALAPPDATA")} / "basalt" / "kiln" / "config" / "updates.toml";
#elif defined(__APPLE__)
    return fs::path{environment("HOME")} / "Library" / "Application Support" / "basalt" / "kiln" / "config" /
           "updates.toml";
#else
    const std::string xdg = environment("XDG_CONFIG_HOME");
    const fs::path root = xdg.empty() ? fs::path{environment("HOME")} / ".config" : fs::path{xdg};
    return root / "basalt" / "kiln" / "updates.toml";
#endif
}

std::expected<bool, std::string> automaticEnabled() {
    const fs::path path = configPath();
    if (!fs::exists(path)) {
        return false;
    }
    std::ifstream input{path};
    const std::string text{std::istreambuf_iterator<char>{input}, {}};
    const auto table = wire::common::toml::parse(text);
    if (!input || !table || table->size() != 1 || !table->boolean("automatic")) {
        return std::unexpected("invalid update configuration: " + path.string());
    }
    return *table->boolean("automatic");
}

std::int64_t nowSeconds() {
    return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch())
        .count();
}

std::expected<fs::path, std::string> stagingDirectory() {
    const fs::path parent = cacheSettings().root / "updates";
    fs::create_directories(parent);
    const auto cutoff = fs::file_time_type::clock::now() - std::chrono::hours{24 * 30};
    for (const fs::directory_entry& entry : fs::directory_iterator(parent)) {
        std::error_code ignored;
        if (entry.path().filename().string().starts_with("stage-") && !entry.is_symlink() && entry.is_directory() &&
            entry.last_write_time(ignored) < cutoff && !ignored) {
            fs::remove_all(entry.path(), ignored);
        }
    }
    std::random_device random;
    for (int attempt = 0; attempt < 10; ++attempt) {
        const fs::path path = parent / ("stage-" + std::to_string(random()) + std::to_string(random()));
        if (fs::create_directory(path)) {
            fs::permissions(path, fs::perms::owner_all, fs::perm_options::replace);
            return path;
        }
    }
    return std::unexpected("cannot create a private update staging directory");
}

std::expected<void, std::string> download(std::string_view url, const fs::path& destination, std::uint64_t maximum,
                                          bool automatic) {
    DownloadOptions options;
    options.maxBytes = maximum;
    options.readTimeoutMs = automatic ? 5000 : 30000;
    options.attempts = automatic ? 1 : 2;
    return httpsDownload(url, destination, {}, options);
}

std::expected<void, std::string> checkUpdate(std::string_view version, bool checkOnly, bool automatic) {
    if (cacheSettings().offline || cacheSettings().noDownload) {
        if (automatic) {
            return {};
        }
        return std::unexpected("updates require downloads; --offline and --no-download forbid them");
    }
    std::string error;
    auto lock = FileLock::acquire(cacheSettings().root / "updates" / "update.lock", error, !automatic);
    if (!lock) {
        return std::unexpected(error);
    }
    if (automatic) {
        std::int64_t last = 0;
        std::ifstream{cacheSettings().root / "updates" / "last-check"} >> last;
        const auto now = nowSeconds();
        if (last > 0 && now >= last && now - last < kCheckInterval) {
            return {};
        }
        std::ofstream{cacheSettings().root / "updates" / "last-check"} << now << '\n';
    }
    const auto staging = stagingDirectory();
    if (!staging) {
        return std::unexpected(staging.error());
    }

    struct Cleanup {
        fs::path path;
        bool keep = false;

        ~Cleanup() {
            if (!keep) {
                std::error_code ignored;
                fs::remove_all(path, ignored);
            }
        }
    } cleanup{*staging};

    for (const auto& [name, limit] : {std::pair{"update-manifest.txt", std::uint64_t{65536}},
                                      std::pair{"update-manifest.sig", std::uint64_t{1024}}}) {
        const std::string url = std::string{kReleaseRoot} + "latest/download/" + name;
        if (const auto result = download(url, *staging / name, limit, automatic); !result) {
            return result;
        }
    }
    const auto release = loadUpdateRelease(*staging);
    if (!release) {
        return std::unexpected(release.error());
    }
    if (!newerVersion(release->version, version)) {
        if (!automatic) {
            std::cout << "Kiln " << version << " is up to date.\n";
        }
        return {};
    }
    const std::string page = std::string{kReleaseRoot} + "tag/v" + release->version;
    std::cerr << "kiln: version " << release->version << " is available (installed: " << version << ")\n";
    const auto executable = runningExecutable();
    if (!executable) {
        return std::unexpected(executable.error());
    }
    if (checkOnly) {
        std::cout << page << '\n';
        return {};
    }
    const bool managed = managedInstallation(*executable);
#if !defined(_WIN32)
#if defined(__APPLE__)
    const bool packageManaged = managed && !fs::is_regular_file(executable->parent_path() / "kiln.installation");
#else
    const bool packageManaged = managed;
#endif
    if (packageManaged) {
        std::cout << "Update this installation with its package manager. Packages: " << page << '\n';
        return {};
    }
#endif
    const std::string name = updateAssetName(release->version, managed);
    if (!release->hashes.contains(name)) {
        return std::unexpected("no signed update is available for this installation: " + name);
    }
    const std::string url = std::string{kReleaseRoot} + "download/v" + release->version + "/" + name;
    if (const auto result = download(url, *staging / name, 256ULL * 1024ULL * 1024ULL, automatic); !result) {
        return result;
    }
    const auto installed = installUpdate(*staging, *executable, *release);
#if defined(_WIN32) || defined(__APPLE__)
    cleanup.keep = installed.has_value();
#endif
    return installed;
}

} // namespace

int runUpdate(const std::vector<std::string>& args, std::string_view version) {
    try {
        if (args.size() == 3 && args[1] == "--auto" && (args[2] == "on" || args[2] == "off" || args[2] == "status")) {
            if (args[2] == "status") {
                const auto enabled = automaticEnabled();
                if (!enabled) {
                    std::cerr << "kiln update: " << enabled.error() << '\n';
                    return 1;
                }
                std::cout << "Automatic updates: " << (*enabled ? "on" : "off")
                          << "\nConfiguration: " << configPath().string() << '\n';
                return 0;
            }
            const fs::path path = configPath();
            fs::create_directories(path.parent_path());
            std::ofstream output{path};
            output << "automatic = " << (args[2] == "on" ? "true" : "false") << '\n';
            output.close();
            if (!output) {
                std::cerr << "kiln update: cannot write " << path.string() << '\n';
                return 1;
            }
            std::cout << "Automatic updates: " << args[2] << '\n';
            return 0;
        }
        if (args.size() > 2 || (args.size() == 2 && args[1] != "--check")) {
            std::cerr << "usage: kiln update [--check | --auto on|off|status]\n";
            return 2;
        }
        const auto result = checkUpdate(version, args.size() == 2, false);
        if (!result) {
            std::cerr << "kiln update: " << result.error() << '\n';
            return 1;
        }
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "kiln update: " << error.what() << '\n';
        return 1;
    }
}

void autoUpdate(std::string_view version) {
    try {
        if (cacheSettings().offline || cacheSettings().noDownload || !environment("CI").empty() ||
            !environment("KILN_NO_UPDATE").empty()) {
            return;
        }
        const auto enabled = automaticEnabled();
        if (!enabled) {
            std::cerr << "kiln: " << enabled.error() << '\n';
            return;
        }
        if (*enabled) {
            if (const auto result = checkUpdate(version, false, true); !result) {
                std::cerr << "kiln: automatic update skipped: " << result.error() << '\n';
            }
        }
    } catch (const std::exception& error) {
        std::cerr << "kiln: automatic update skipped: " << error.what() << '\n';
    }
}

} // namespace basalt::kiln
