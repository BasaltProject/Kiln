#include "Plugin.hpp"

#include "Analyzer.hpp"
#include "Manifest.hpp"

#include <algorithm>
#include <fstream>
#include <iterator>
#include <map>
#include <set>

namespace fs = std::filesystem;

namespace basalt::kiln {
namespace {

std::string readAll(const fs::path& path) {
    std::ifstream in{path, std::ios::binary};
    return std::string{std::istreambuf_iterator<char>{in}, std::istreambuf_iterator<char>{}};
}

std::string trimmed(std::string text) {
    const auto blank = [](unsigned char c) { return c == ' ' || c == '\t' || c == '\r'; };
    while (!text.empty() && blank(static_cast<unsigned char>(text.back()))) {
        text.pop_back();
    }
    std::size_t start = 0;
    while (start < text.size() && blank(static_cast<unsigned char>(text[start]))) {
        ++start;
    }
    return text.substr(start);
}

std::map<std::string, std::string> topLevelValues(const std::string& toml) {
    std::map<std::string, std::string> values;
    std::size_t start = 0;
    while (start < toml.size()) {
        std::size_t end = toml.find('\n', start);
        end = end == std::string::npos ? toml.size() : end;
        const std::string line = trimmed(toml.substr(start, end - start));
        start = end + 1;
        if (line.starts_with("[")) {
            break;
        }
        const std::size_t equals = line.find('=');
        if (line.empty() || line[0] == '#' || equals == std::string::npos) {
            continue;
        }
        std::string value = trimmed(line.substr(equals + 1));
        if (value.size() >= 2 && value.front() == '"') {
            value = value.substr(1, value.find('"', 1) - 1);
        }
        values[trimmed(line.substr(0, equals))] = value;
    }
    return values;
}

bool validId(const std::string& id) {
    return !id.empty() && std::ranges::all_of(id, [](char c) {
        return (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_' || c == '-';
    });
}

std::string moduleFile(const std::string& module) {
    std::string path = module;
    std::ranges::replace(path, '.', '/');
    return path;
}

} // namespace

bool isPluginFolder(const fs::path& path) { return fs::is_regular_file(path / "plugin.toml"); }

std::vector<Diagnostic> verifyPlugin(const fs::path& root, std::string_view target) {
    std::vector<Diagnostic> found;
    const fs::path manifestPath = root / "plugin.toml";
    const std::map<std::string, std::string> values = topLevelValues(readAll(manifestPath));
    for (const char* required : {"id", "name", "version", "main"}) {
        if (!values.contains(required) || values.at(required).empty()) {
            found.push_back({"plugin.toml", 0, std::string{"missing `"} + required + "`"});
        }
    }
    if (values.contains("id") && !validId(values.at("id"))) {
        found.push_back({"plugin.toml", 0, "`id` takes lowercase letters, digits, `_` and `-`"});
    }
    if (values.contains("main") && !fs::is_regular_file(root / values.at("main"))) {
        found.push_back({"plugin.toml", 0, "main file " + values.at("main") + " does not exist"});
    }

    const fs::path sources = root / "src";
    std::vector<fs::path> files;
    if (fs::is_directory(sources)) {
        for (const fs::directory_entry& entry : fs::recursive_directory_iterator(sources)) {
            if (entry.is_regular_file() && entry.path().extension() == ".lua") {
                files.push_back(entry.path());
            }
        }
    }
    std::ranges::sort(files);

    std::set<std::string> modules;
    for (const fs::path& file : files) {
        fs::path relative = fs::relative(file, sources);
        relative.replace_extension();
        modules.insert(relative.generic_string());
    }

    const Manifest manifest = manifestFor(target);
    std::map<std::string, Usage> usages;
    std::set<std::string> defined;
    for (const fs::path& file : files) {
        const std::string name = fs::relative(file, root).generic_string();
        std::expected<Usage, std::string> usage = analyze(readAll(file), name);
        if (!usage.has_value()) {
            found.push_back({name, 0, usage.error()});
            continue;
        }
        defined.insert(usage->globalWrites.begin(), usage->globalWrites.end());
        usages.emplace(name, std::move(*usage));
    }

    for (const auto& [name, usage] : usages) {
        for (const Access& read : usage.globalReads) {
            if (!manifest.permissive() && !manifest.hasRoot(read.path) && !defined.contains(read.path)) {
                found.push_back({name, read.line, "unknown global `" + read.path + "`"});
            }
        }
        for (const Access& read : usage.fieldReads) {
            const std::string root = read.path.substr(0, read.path.find('.'));
            if (defined.contains(root)) {
                continue;
            }
            const Resolution resolved = manifest.resolve(read.path);
            if (resolved.result == Lookup::UnknownField && !manifest.permissive()) {
                found.push_back({name, read.line, "`" + resolved.parent + "` is not part of the API"});
            } else if (resolved.result == Lookup::IndexesFunction) {
                found.push_back({name, read.line, "`" + resolved.parent + "` is a function, it has no fields"});
            }
        }
        for (const Access& required : usage.requires_) {
            const std::string wanted = moduleFile(required.path);
            if (!modules.contains(wanted) && !modules.contains(wanted + "/init")) {
                found.push_back({name, required.line, "no module `" + required.path + "` in src/"});
            }
        }
    }
    std::ranges::sort(found, [](const Diagnostic& left, const Diagnostic& right) {
        return std::tie(left.file, left.line, left.message) < std::tie(right.file, right.line, right.message);
    });
    return found;
}

} // namespace basalt::kiln
