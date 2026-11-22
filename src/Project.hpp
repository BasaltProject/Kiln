#pragma once

#include <basalt/wire/plugins/PluginManifest.hpp>

#include <expected>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace basalt::kiln {

struct ProjectFile {
    /// Name inside the package, with forward slashes.
    std::string name;
    std::filesystem::path source;
};

/// A plugin folder with a valid manifest and a built module, with the files that go into its package.
struct Project {
    std::filesystem::path root;
    wire::plugins::PluginManifest manifest;
    std::vector<ProjectFile> files;

    [[nodiscard]] static std::expected<Project, std::string> open(const std::filesystem::path& root);

    [[nodiscard]] std::string packageName(std::string_view extension) const {
        return manifest.id + "-" + manifest.version + std::string{extension};
    }
};

} // namespace basalt::kiln
