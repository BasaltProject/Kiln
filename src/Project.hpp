#pragma once

#include <basalt/wire/plugins/PluginManifest.hpp>

#include <expected>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace basalt::kiln {

/// A plugin folder with a valid manifest and the files that go into its package.
struct Project {
    std::filesystem::path root;
    wire::plugins::PluginManifest manifest;
    /// Paths relative to `root` with forward slashes, in the order they are packed.
    std::vector<std::string> files;

    [[nodiscard]] static std::expected<Project, std::string> open(const std::filesystem::path& root);

    [[nodiscard]] std::string packageName(std::string_view extension) const {
        return manifest.id + "-" + manifest.version + std::string{extension};
    }
};

} // namespace basalt::kiln
