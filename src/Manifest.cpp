#include "Manifest.hpp"

#include <sstream>
#include <vector>

namespace basalt::kiln {

Manifest Manifest::parse(std::string_view text) {
    Manifest manifest;
    std::istringstream lines{std::string{text}};
    std::string line;
    while (std::getline(lines, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }
        if (line == "@permissive") {
            manifest.permissive_ = true;
            continue;
        }
        const std::size_t space = line.find(' ');
        if (space == std::string::npos) {
            continue;
        }
        const std::string kind = line.substr(space + 1);
        manifest.entries_[line.substr(0, space)] =
            kind == "table" ? Kind::Table : (kind == "function" ? Kind::Function : Kind::Value);
    }
    return manifest;
}

bool Manifest::hasChildren(const std::string& path) const {
    const std::string prefix = path + ".";
    const auto first = entries_.lower_bound(prefix);
    return first != entries_.end() && first->first.starts_with(prefix);
}

Resolution Manifest::resolve(const std::string& path) const {
    std::vector<std::string> parts;
    std::size_t start = 0;
    while (start <= path.size()) {
        const std::size_t dot = path.find('.', start);
        parts.push_back(path.substr(start, dot == std::string::npos ? std::string::npos : dot - start));
        if (dot == std::string::npos) {
            break;
        }
        start = dot + 1;
    }
    std::string walked = parts[0];
    const auto root = entries_.find(walked);
    if (root == entries_.end()) {
        return {Lookup::UnknownRoot, {}};
    }
    for (std::size_t i = 1; i < parts.size(); ++i) {
        const Kind parent = entries_.at(walked);
        if (parent == Kind::Function) {
            return {Lookup::IndexesFunction, walked};
        }
        if (parent == Kind::Value || !hasChildren(walked)) {
            return {};
        }
        walked += "." + parts[i];
        if (!entries_.contains(walked)) {
            return {Lookup::UnknownField, walked};
        }
    }
    return {};
}

} // namespace basalt::kiln
