#pragma once

#include <map>
#include <string>
#include <string_view>

namespace basalt::kiln {

enum class Kind { Table, Function, Value };

enum class Lookup { Known, UnknownRoot, UnknownField, IndexesFunction };

struct Resolution {
    Lookup result = Lookup::Known;
    std::string parent;
};

/// The names a plugin can use for one target, loaded from `api/<target>.api`.
class Manifest {
public:
    [[nodiscard]] static Manifest parse(std::string_view text);

    [[nodiscard]] bool permissive() const { return permissive_; }

    [[nodiscard]] bool hasRoot(const std::string& name) const { return entries_.contains(name); }

    [[nodiscard]] Resolution resolve(const std::string& path) const;

private:
    [[nodiscard]] bool hasChildren(const std::string& path) const;

    std::map<std::string, Kind> entries_;
    bool permissive_ = false;
};

/// The manifest built into Kiln for a target, or an empty permissive one when the target has none.
[[nodiscard]] Manifest manifestFor(std::string_view target);

} // namespace basalt::kiln
