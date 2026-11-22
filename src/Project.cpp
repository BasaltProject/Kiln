#include "Project.hpp"

#include "WasmCheck.hpp"

#include <basalt/wire/io/FileIO.hpp>

#include <algorithm>
#include <map>

namespace fs = std::filesystem;

namespace basalt::kiln {

std::expected<Project, std::string> Project::open(const fs::path& root) {
    using wire::plugins::PluginManifest;
    const std::optional<std::string> text =
        wire::io::fileIO::readText((root / std::string{PluginManifest::kFileName}).string());
    if (!text.has_value()) {
        return std::unexpected("there is no plugin.toml in " + root.string());
    }
    std::expected<PluginManifest, wire::plugins::ManifestError> manifest = PluginManifest::parse(*text);
    if (!manifest.has_value()) {
        return std::unexpected("plugin.toml: " + manifest.error().message);
    }
    if (!isWasmMain(manifest->main)) {
        return std::unexpected("`main` must name a .wasm module");
    }
    const fs::path module = builtModulePath(root, manifest->main);
    if (!fs::is_regular_file(module)) {
        return std::unexpected(manifest->main + " is not built yet; run `kiln build` first");
    }

    Project project{.root = root, .manifest = *std::move(manifest), .files = {}};
    project.files.push_back({std::string{PluginManifest::kFileName}, root / std::string{PluginManifest::kFileName}});
    project.files.push_back({fs::path{project.manifest.main}.generic_string(), module});

    std::vector<std::string> resources;
    if (fs::is_directory(root / "resources")) {
        for (const fs::directory_entry& entry : fs::recursive_directory_iterator(root / "resources")) {
            if (entry.is_regular_file()) {
                resources.push_back(fs::relative(entry.path(), root).generic_string());
            }
        }
        std::ranges::sort(resources);
    }
    // A resource is addressed by its name, not by where it is, so two of them cannot share one.
    std::map<std::string, std::string> byName;
    for (const std::string& file : resources) {
        if (const auto [at, added] = byName.emplace(fs::path{file}.filename().string(), file); !added) {
            return std::unexpected("resources " + at->second + " and " + file + " have the same name");
        }
        project.files.push_back({file, root / file});
    }
    return project;
}

} // namespace basalt::kiln
