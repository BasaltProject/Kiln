#pragma once

#include <array>
#include <optional>
#include <string_view>

namespace basalt::kiln {

/// A product whose plugins Kiln packs for; a new one is a new row here.
struct Target {
    std::string_view name;
    std::string_view package;
};

inline constexpr std::array<Target, 2> kTargets{{{"server", ".bspkg"}, {"proxy", ".bppkg"}}};

[[nodiscard]] inline std::optional<Target> targetNamed(std::string_view name) {
    for (const Target& target : kTargets) {
        if (target.name == name) {
            return target;
        }
    }
    return std::nullopt;
}

} // namespace basalt::kiln
