#include "Toolchain.hpp"

#include "Archive.hpp"
#include "FileLock.hpp"
#include "Http.hpp"
#include "Sha256.hpp"

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <random>
#include <set>

namespace fs = std::filesystem;

namespace basalt::kiln {
namespace {

constexpr std::string_view kMarker = ".kiln-installed";

std::string environment(const char* name) {
    const char* value = std::getenv(name);
    return value == nullptr ? std::string{} : std::string{value};
}

std::string randomSuffix() {
    std::random_device device;
    return std::to_string(device()) + std::to_string(device());
}

std::string mebibytes(std::uint64_t bytes) { return std::to_string((bytes + 1048575) / 1048576) + " MiB"; }

ArchiveFormat formatOf(PinKind kind) {
    switch (kind) {
    case PinKind::TarGz:
        return ArchiveFormat::TarGz;
    case PinKind::TarXz:
        return ArchiveFormat::TarXz;
    case PinKind::Zip:
    case PinKind::Raw:
        return ArchiveFormat::Zip;
    }
    return ArchiveFormat::TarGz;
}

std::string fileNameOf(std::string_view url) { return std::string{url.substr(url.rfind('/') + 1)}; }

/// Downloads a pinned artifact into the cache, verified; an already verified copy is reused.
std::expected<fs::path, std::string> fetch(const Pin& pin) {
    const fs::path downloads = cacheSettings().root / "downloads";
    const fs::path verified = downloads / std::string{pin.sha256};
    if (fs::is_regular_file(verified)) {
        return verified;
    }
    const fs::path part = downloads / (std::string{pin.sha256} + ".part");
    std::cerr << "kiln: downloading " << fileNameOf(pin.url);
    if (pin.size > 0) {
        std::cerr << " (" << mebibytes(pin.size) << ")";
    }
    std::cerr << '\n';
    std::uint64_t reported = 0;
    const ProgressFn progress = [&](std::uint64_t received, std::uint64_t total) {
        if (total > 0 && received * 4 / total > reported && received < total) {
            reported = received * 4 / total;
            std::cerr << "kiln:   " << reported * 25 << "%\n";
        }
    };
    for (int attempt = 0; attempt < 2; ++attempt) {
        if (const auto downloaded = httpsDownload(pin.url, part, progress); !downloaded.has_value()) {
            return std::unexpected("download of " + std::string{pin.url} + " failed: " + downloaded.error() +
                                   " (the partial file is kept, run again to resume)");
        }
        const auto actual = sha256OfFile(part);
        if (actual.has_value() && *actual == pin.sha256) {
            std::error_code error;
            fs::rename(part, verified, error);
            if (error) {
                return std::unexpected("cannot move the download into place: " + error.message());
            }
            return verified;
        }
        std::error_code ignored;
        fs::remove(part, ignored);
        if (attempt == 1) {
            return std::unexpected("sha256 mismatch for " + std::string{pin.url} + ": expected " +
                                   std::string{pin.sha256} + ", got " + actual.value_or("unreadable file"));
        }
    }
    return std::unexpected("unreachable");
}

} // namespace

CacheSettings& cacheSettings() {
    static CacheSettings settings;
    return settings;
}

fs::path defaultCacheRoot() {
    if (const std::string home = environment("KILN_HOME"); !home.empty()) {
        return fs::path{home};
    }
#if defined(_WIN32)
    if (const std::string local = environment("LOCALAPPDATA"); !local.empty()) {
        return fs::path{local} / "basalt" / "kiln" / "cache";
    }
    return fs::path{environment("USERPROFILE")} / "AppData" / "Local" / "basalt" / "kiln" / "cache";
#elif defined(__APPLE__)
    return fs::path{environment("HOME")} / "Library" / "Caches" / "basalt" / "kiln";
#else
    if (const std::string xdg = environment("XDG_CACHE_HOME"); !xdg.empty()) {
        return fs::path{xdg} / "basalt" / "kiln";
    }
    return fs::path{environment("HOME")} / ".cache" / "basalt" / "kiln";
#endif
}

std::string_view currentPlatform() {
#if defined(_WIN32)
    return "windows-x64";
#elif defined(__APPLE__)
#if defined(__aarch64__) || defined(__arm64__)
    return "macos-arm64";
#else
    return "macos-x64";
#endif
#else
#if defined(__aarch64__)
    return "linux-arm64";
#else
    return "linux-x64";
#endif
#endif
}

std::string executableName(std::string_view name) {
#if defined(_WIN32)
    return std::string{name} + ".exe";
#else
    return std::string{name};
#endif
}

std::vector<Pin> pinsFor(std::string_view id, std::string_view platform) {
    std::vector<Pin> found;
    for (const Pin& pin : kPins) {
        if (pin.id == id && (pin.platform == platform || pin.platform == "any")) {
            found.push_back(pin);
        }
    }
    return found;
}

std::vector<std::string_view> toolchainIds() {
    std::vector<std::string_view> ids;
    for (const Pin& pin : kPins) {
        if (std::ranges::find(ids, pin.id) == ids.end()) {
            ids.push_back(pin.id);
        }
    }
    return ids;
}

std::string toolchainVersion(std::string_view id) {
    for (const Pin& pin : kPins) {
        if (pin.id == id) {
            return std::string{pin.version};
        }
    }
    return {};
}

fs::path toolchainDir(std::string_view id) {
    return cacheSettings().root / "toolchains" / (std::string{id} + "-" + toolchainVersion(id));
}

bool isInstalled(std::string_view id) { return fs::exists(toolchainDir(id) / std::string{kMarker}); }

std::uint64_t downloadSize(std::string_view id) {
    std::uint64_t total = 0;
    for (const Pin& pin : pinsFor(id, currentPlatform())) {
        total += pin.size;
    }
    return total;
}

std::expected<fs::path, std::string> ensureToolchain(std::string_view id) {
    const fs::path final = toolchainDir(id);
    if (fs::exists(final / std::string{kMarker})) {
        return final;
    }
    const std::vector<Pin> pins = pinsFor(id, currentPlatform());
    if (pins.empty()) {
        return std::unexpected("there is no " + std::string{id} + " toolchain for " + std::string{currentPlatform()});
    }
    const std::string wanted = std::string{id} + " " + toolchainVersion(id);
    if (cacheSettings().offline) {
        return std::unexpected(
            "the " + wanted +
            " toolchain is not installed and --offline forbids downloading it; run `kiln toolchain install " +
            std::string{id} + "` while online");
    }
    if (cacheSettings().noDownload) {
        return std::unexpected("the " + wanted + " toolchain is not installed and --no-download is set");
    }
    std::string failure;
    const std::optional<FileLock> lock =
        FileLock::acquire(cacheSettings().root / "locks" / (std::string{id} + ".lock"), failure);
    if (!lock.has_value()) {
        return std::unexpected(failure);
    }
    if (fs::exists(final / std::string{kMarker})) {
        return final;
    }
    const fs::path staging = final.parent_path() / ("." + final.filename().string() + ".tmp-" + randomSuffix());
    std::error_code error;
    fs::remove_all(staging, error);
    fs::create_directories(staging, error);
    if (error) {
        return std::unexpected("cannot create " + staging.string() + ": " + error.message());
    }
    std::vector<fs::path> archives;
    for (const Pin& pin : pins) {
        const auto archive = fetch(pin);
        if (!archive.has_value()) {
            fs::remove_all(staging, error);
            return std::unexpected(archive.error());
        }
        archives.push_back(*archive);
        std::cerr << "kiln: unpacking " << fileNameOf(pin.url) << '\n';
        if (pin.kind == PinKind::Raw) {
            std::error_code copied;
            fs::create_directories(staging / std::string{pin.into}, copied);
            fs::copy_file(*archive, staging / std::string{pin.into} / fileNameOf(pin.url), copied);
            if (copied) {
                fs::remove_all(staging, error);
                return std::unexpected("cannot store " + fileNameOf(pin.url) + ": " + copied.message());
            }
            continue;
        }
        const auto unpacked =
            extractArchive(*archive, formatOf(pin.kind), staging / std::string{pin.into},
                           ExtractRules{.skip = static_cast<std::size_t>(pin.skip), .only = std::string{pin.only}});
        if (!unpacked.has_value()) {
            fs::remove_all(staging, error);
            return std::unexpected("cannot unpack " + fileNameOf(pin.url) + ": " + unpacked.error());
        }
    }
    std::string record = wanted + "\n";
    for (const Pin& pin : pins) {
        record += std::string{pin.sha256} + "  " + fileNameOf(pin.url) + "\n";
    }
    std::ofstream marker{staging / std::string{kMarker}};
    marker << record;
    marker.close();
    fs::remove_all(final, error);
    fs::rename(staging, final, error);
    if (error) {
        fs::remove_all(staging, error);
        return std::unexpected("cannot move the toolchain into place: " + error.message());
    }
    for (const fs::path& archive : archives) {
        fs::remove(archive, error);
    }
    std::cerr << "kiln: installed " << wanted << " in " << final.string() << '\n';
    return final;
}

std::expected<void, std::string> cleanCache() {
    std::error_code error;
    for (const char* folder : {"toolchains", "downloads", "work"}) {
        fs::remove_all(cacheSettings().root / folder, error);
        if (error) {
            return std::unexpected("cannot remove " + (cacheSettings().root / folder).string() + ": " +
                                   error.message());
        }
    }
    return {};
}

} // namespace basalt::kiln
