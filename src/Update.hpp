#pragma once

#include <expected>
#include <filesystem>
#include <map>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace basalt::kiln {

struct UpdateRelease {
    std::string version;
    std::map<std::string, std::string> hashes;
};

[[nodiscard]] std::expected<UpdateRelease, std::string> parseUpdateManifest(std::string_view text);
[[nodiscard]] bool newerVersion(std::string_view available, std::string_view installed);
[[nodiscard]] bool verifyUpdateSignature(std::string_view manifest, std::span<const unsigned char> signature,
                                         std::string_view publicKey);
[[nodiscard]] std::string updateAssetName(std::string_view version, bool installer);
[[nodiscard]] std::expected<UpdateRelease, std::string> loadUpdateRelease(const std::filesystem::path& staging);
[[nodiscard]] std::expected<void, std::string> verifyUpdateAsset(const UpdateRelease& release,
                                                                 const std::filesystem::path& asset);
[[nodiscard]] std::expected<std::filesystem::path, std::string> runningExecutable();
[[nodiscard]] bool managedInstallation(const std::filesystem::path& executable);
[[nodiscard]] std::expected<void, std::string> installUpdate(const std::filesystem::path& staging,
                                                             const std::filesystem::path& executable,
                                                             const UpdateRelease& release);
[[nodiscard]] int applyWindowsUpdate(const std::vector<std::string>& args);
[[nodiscard]] int runUpdate(const std::vector<std::string>& args, std::string_view version);
void autoUpdate(std::string_view version);

} // namespace basalt::kiln
