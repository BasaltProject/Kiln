#include "Plugin.hpp"

#include "AbiTable.hpp"
#include "Languages.hpp"
#include "Targets.hpp"
#include "WasmCheck.hpp"

#include <basalt/wire/common/toml/Toml.hpp>
#include <basalt/wire/io/FileIO.hpp>
#include <basalt/wire/plugins/PluginManifest.hpp>

#include <algorithm>

namespace fs = std::filesystem;

namespace basalt::kiln {

bool validPluginId(std::string_view id) {
    return !id.empty() && id.front() >= 'a' && id.front() <= 'z' &&
           std::ranges::all_of(id, [](char c) { return (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_'; });
}

bool isPluginFolder(const fs::path& path) { return fs::is_regular_file(path / "plugin.toml"); }

std::string manifestTarget(const fs::path& root) {
    const std::optional<std::string> text = wire::io::fileIO::readText((root / "plugin.toml").string());
    if (text.has_value()) {
        if (const auto table = wire::common::toml::parse(*text); table.has_value()) {
            if (const auto target = table->string("target"); target.has_value()) {
                return std::string{*target};
            }
        }
    }
    return "server";
}

std::vector<Diagnostic> verifyPlugin(const fs::path& root, std::optional<std::string_view> targetOverride) {
    std::vector<Diagnostic> found;
    const auto problem = [&](std::string message) { found.push_back({"plugin.toml", 0, std::move(message)}); };
    const std::optional<std::string> text = wire::io::fileIO::readText((root / "plugin.toml").string());
    if (!text.has_value()) {
        problem("cannot read plugin.toml");
        return found;
    }
    const auto table = wire::common::toml::parse(*text);
    if (!table.has_value()) {
        const std::size_t line = wire::common::toml::tomlErrorLine(*text);
        found.push_back(
            {"plugin.toml", static_cast<int>(line), std::string{wire::common::toml::tomlErrorMessage(table.error())}});
        return found;
    }
    const std::optional<std::string_view> id = table->string("id");
    if (!id.has_value()) {
        problem("missing `id`");
    } else if (!validPluginId(*id)) {
        problem("`id` takes lowercase letters, digits and `_`, and starts with a letter");
    }
    if (!table->string("version").has_value()) {
        problem("missing `version`");
    }
    const std::optional<std::int64_t> api = table->integer("api");
    if (!api.has_value()) {
        problem("missing `api`");
    } else if (*api != wire::plugins::PluginManifest::kApi) {
        problem("`api` is " + std::to_string(*api) + " but this Kiln builds API " +
                std::to_string(wire::plugins::PluginManifest::kApi));
    }
    const std::optional<std::int64_t> abiVersion = table->integer("abi");
    if (!abiVersion.has_value()) {
        problem("missing `abi` (this Kiln builds ABI " + std::to_string(abi::kVersion) + ")");
    } else if (*abiVersion != abi::kVersion) {
        problem("`abi` is " + std::to_string(*abiVersion) + " but this Kiln builds ABI " +
                std::to_string(abi::kVersion));
    }
    if (const auto lang = table->string("lang"); lang.has_value() && !languageNamed(*lang).has_value()) {
        problem("`lang` is not one of " + languageList());
    }
    const std::string target = targetOverride.has_value() ? std::string{*targetOverride} : manifestTarget(root);
    if (!targetNamed(target).has_value()) {
        problem("`target` must be `server` or `proxy`");
        return found;
    }
    const std::optional<std::string_view> main = table->string("main");
    if (!main.has_value()) {
        problem("missing `main` (a .wasm module)");
        return found;
    }
    if (!isWasmMain(std::string{*main})) {
        problem("`main` must name a .wasm module");
        return found;
    }
    const fs::path module = builtModulePath(root, std::string{*main});
    const auto bytes = wire::io::fileIO::readBytes(module.string());
    if (bytes.has_value()) {
        const std::vector<Diagnostic> moduleProblems =
            verifyModule(*bytes, fs::relative(module, root).generic_string(), target);
        found.insert(found.end(), moduleProblems.begin(), moduleProblems.end());
    }
    return found;
}

} // namespace basalt::kiln
