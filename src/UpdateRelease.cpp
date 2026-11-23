#include "Sha256.hpp"
#include "Update.hpp"
#include "UpdateKey.hpp"

#include <basalt_mbedtls_threads.h>

extern "C" {
#include <mbedtls/pk.h>
#include <mbedtls/sha256.h>
}

#include <algorithm>
#include <array>
#include <charconv>
#include <fstream>
#include <optional>
#include <sstream>

namespace fs = std::filesystem;

namespace basalt::kiln {
namespace {

std::optional<std::array<unsigned int, 3>> versionParts(std::string_view value) {
    std::array<unsigned int, 3> parts{};
    for (std::size_t i = 0; i < parts.size(); ++i) {
        const std::size_t separator = value.find('.');
        const std::string_view part = value.substr(0, separator);
        const auto [end, error] = std::from_chars(part.data(), part.data() + part.size(), parts[i]);
        if (error != std::errc{} || end != part.data() + part.size() ||
            (i < 2 && separator == std::string_view::npos) || (i == 2 && separator != std::string_view::npos)) {
            return std::nullopt;
        }
        value.remove_prefix(i < 2 ? separator + 1 : value.size());
    }
    return parts;
}

bool safeAsset(std::string_view name) {
    return name.starts_with("Kiln-") && name.size() < 200 &&
           std::ranges::all_of(
               name,
               [](unsigned char c) {
                   return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '-' ||
                          c == '.';
               });
}

std::string readFile(const fs::path& file, std::uintmax_t limit) {
    std::error_code error;
    const auto size = fs::file_size(file, error);
    if (error || size > limit) {
        return {};
    }
    std::ifstream input{file, std::ios::binary};
    return std::string{std::istreambuf_iterator<char>{input}, {}};
}

} // namespace

std::expected<UpdateRelease, std::string> parseUpdateManifest(std::string_view text) {
    std::istringstream input{std::string{text}};
    std::string line;
    UpdateRelease release;
    if (!std::getline(input, line) || line != "kiln-update-v1" || !std::getline(input, line) ||
        !line.starts_with("version ") || !versionParts(std::string_view{line}.substr(8))) {
        return std::unexpected("invalid update manifest header or version");
    }
    release.version = line.substr(8);
    while (std::getline(input, line)) {
        if (line.size() < 67 || line.substr(64, 2) != "  " ||
            !std::ranges::all_of(std::string_view{line}.substr(0, 64),
                                 [](unsigned char c) { return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'); }) ||
            !safeAsset(std::string_view{line}.substr(66))) {
            return std::unexpected("invalid update manifest asset");
        }
        const std::string name = line.substr(66);
        const std::string prefix = name.starts_with("Kiln-Installer-") ? "Kiln-Installer-" : "Kiln-";
        if (!name.starts_with(prefix + release.version + "-") ||
            !release.hashes.emplace(name, line.substr(0, 64)).second) {
            return std::unexpected("duplicate asset or asset from a different version");
        }
    }
    if (release.hashes.empty()) {
        return std::unexpected("the update manifest has no assets");
    }
    return release;
}

bool newerVersion(std::string_view available, std::string_view installed) {
    const auto next = versionParts(available);
    const std::size_t suffix = installed.find('-');
    const auto current = versionParts(installed.substr(0, suffix));
    return next && current && (*next > *current || (*next == *current && suffix != std::string_view::npos));
}

bool verifyUpdateSignature(std::string_view manifest, std::span<const unsigned char> signature,
                           std::string_view publicKey) {
    basalt_mbedtls_threading_init();
    mbedtls_pk_context key{};
    mbedtls_pk_init(&key);
    const std::string pem{publicKey};
    std::array<unsigned char, 32> digest{};
    const bool valid =
        mbedtls_pk_parse_public_key(&key, reinterpret_cast<const unsigned char*>(pem.c_str()), pem.size() + 1) == 0 &&
        mbedtls_pk_can_do(&key, MBEDTLS_PK_RSA) && mbedtls_pk_get_bitlen(&key) >= 3072 &&
        mbedtls_sha256(reinterpret_cast<const unsigned char*>(manifest.data()), manifest.size(), digest.data(), 0) ==
            0 &&
        mbedtls_pk_verify(&key, MBEDTLS_MD_SHA256, digest.data(), digest.size(), signature.data(), signature.size()) ==
            0;
    mbedtls_pk_free(&key);
    return valid;
}

std::string updateAssetName(std::string_view version, bool installer) {
    std::string platform;
#if defined(_WIN32)
    platform = "Windows-x64";
#elif defined(__APPLE__)
#if defined(__aarch64__) || defined(__arm64__)
    platform = "macOS-arm64";
#else
    platform = "macOS-x64";
#endif
#elif defined(__FreeBSD__)
#if defined(__aarch64__)
    platform = "FreeBSD-arm64";
#else
    platform = "FreeBSD-x64";
#endif
#else
#if defined(__aarch64__)
    platform = "Linux-arm64";
#else
    platform = "Linux-x64";
#endif
#endif
    std::string name = std::string{installer ? "Kiln-Installer-" : "Kiln-"} + std::string{version} + "-" + platform;
#if defined(_WIN32)
    name += installer ? ".msi" : ".exe";
#elif defined(__APPLE__)
    if (installer) {
        name += ".pkg";
    }
#else
    static_cast<void>(installer);
#endif
    return name;
}

std::expected<UpdateRelease, std::string> loadUpdateRelease(const fs::path& staging) {
    const std::string manifest = readFile(staging / "update-manifest.txt", 64 * 1024);
    const std::string signature = readFile(staging / "update-manifest.sig", 1024);
    if (!verifyUpdateSignature(manifest, {reinterpret_cast<const unsigned char*>(signature.data()), signature.size()},
                               kUpdatePublicKey)) {
        return std::unexpected("update signature verification failed; nothing was installed");
    }
    return parseUpdateManifest(manifest);
}

std::expected<void, std::string> verifyUpdateAsset(const UpdateRelease& release, const fs::path& asset) {
    const auto expected = release.hashes.find(asset.filename().string());
    if (expected == release.hashes.end()) {
        return std::unexpected("the signed manifest does not list " + asset.filename().string());
    }
    const auto actual = sha256OfFile(asset);
    if (!actual || *actual != expected->second) {
        return std::unexpected("update checksum verification failed; nothing was installed");
    }
    return {};
}

} // namespace basalt::kiln
