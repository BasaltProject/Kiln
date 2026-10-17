#pragma once

#include <array>
#include <optional>
#include <string_view>

namespace basalt::kiln {

/// A product whose plugins Kiln compiles for; a new one is a new row here and, if it needs a step, a hook in Main.
struct Target {
    std::string_view name;
    std::string_view extension;
};

inline constexpr std::array<Target, 2> kTargets{{{"server", ".luac"}, {"proxy", ".luac"}}};

[[nodiscard]] inline std::optional<Target> targetNamed(std::string_view name) {
    for (const Target& target : kTargets) {
        if (target.name == name) {
            return target;
        }
    }
    return std::nullopt;
}

} // namespace basalt::kiln
