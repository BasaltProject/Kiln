#include "Scaffold.hpp"

#include "Plugin.hpp"

#include <basalt/wire/io/FileIO.hpp>

namespace fs = std::filesystem;

namespace basalt::kiln {
namespace {

constexpr std::string_view kMain =
    "on(\"player.join\", function(e)\n    e.player:send(\"<green>Welcome, <name>\", { name = e.player.name })\nend)\n";
constexpr std::string_view kConfig = "# Settings the operator can change; this file is copied next to the plugin.\n";

} // namespace

std::string scaffold(const fs::path& root, std::string_view id, std::string_view name) {
    if (!validPluginId(id) || id.front() < 'a' || id.front() > 'z') {
        return "the id must be lower case letters, digits, '_' or '-', and start with a letter";
    }
    const fs::path folder = root / std::string{id};
    if (fs::exists(folder)) {
        return folder.string() + " already exists";
    }
    const std::string manifest = "id = \"" + std::string{id} + "\"\nname = \"" + std::string{name} +
                                 "\"\nversion = \"0.1.0\"\nauthors = []\napi = 1\nmain = \"src/main.lua\"\n";
    namespace io = wire::io::fileIO;
    fs::create_directories(folder / "src");
    fs::create_directories(folder / "resources");
    if (!io::writeText((folder / "plugin.toml").string(), manifest) ||
        !io::writeText((folder / "src" / "main.lua").string(), kMain) ||
        !io::writeText((folder / "resources" / "config.toml").string(), kConfig)) {
        return "could not write the files under " + folder.string();
    }
    return {};
}

} // namespace basalt::kiln
