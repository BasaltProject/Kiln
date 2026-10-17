#pragma once

#include <filesystem>
#include <optional>
#include <string>

namespace basalt::kiln {

struct BuildOptions {
    bool strip = false;
    /// The file holding the signing key written by `keygen`; empty leaves the package unsigned.
    std::filesystem::path signingKey;
};

/// Writes a seed and its public key as `<path>.key` and `<path>.pub`; the empty string is success.
[[nodiscard]] std::string generateKey(const std::filesystem::path& path);

/// Copies the manifest and resources of `plugin` to `output`, compiles every source to bytecode and signs the result.
[[nodiscard]] std::string buildPackage(const std::filesystem::path& plugin, const std::filesystem::path& output,
                                       const BuildOptions& options);

} // namespace basalt::kiln
