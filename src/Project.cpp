#include "Project.hpp"

#include <basalt/wire/io/FileIO.hpp>

#include <algorithm>
#include <map>

namespace fs = std::filesystem;

namespace basalt::kiln {
namespace {

void collect(const fs::path& root, std::string_view folder, std::vector<std::string>& out) {
    const fs::path start = root / std::string{folder};
    if (!fs::is_directory(start)) {
        return;
    }
    for (const fs::directory_entry& entry : fs::recursive_directory_iterator(start)) {
        if (entry.is_regular_file()) {
            out.push_back(fs::relative(entry.path(), root).generic_string());
        }
    }
    std::ranges::sort(out);
}

} // namespace

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
    if (!fs::is_regular_file(root / manifest->main)) {
        return std::unexpected("main file " + manifest->main + " does not exist");
    }

    Project project{.root = root, .manifest = *std::move(manifest), .files = {}};
    project.files.emplace_back(PluginManifest::kFileName);
    std::vector<std::string> code;
    std::vector<std::string> resources;
    collect(root, "src", code);
    collect(root, "resources", resources);

    // A resource is addressed by its name, not by where it is, so two of them cannot share one.
    std::map<std::string, std::string> byName;
    for (const std::string& file : resources) {
        if (const auto [at, added] = byName.emplace(fs::path{file}.filename().string(), file); !added) {
            return std::unexpected("resources " + at->second + " and " + file + " have the same name");
        }
    }
    project.files.insert(project.files.end(), code.begin(), code.end());
    project.files.insert(project.files.end(), resources.begin(), resources.end());
    return project;
}

} // namespace basalt::kiln
