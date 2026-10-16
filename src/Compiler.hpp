#pragma once

#include <expected>
#include <string>
#include <string_view>

namespace basalt::kiln {

struct Options {
  bool strip = false;
};

/// Compiles Lua source to the bytecode the Basalt runtime loads; the chunk name
/// is what errors will show.
[[nodiscard]] std::expected<std::string, std::string>
compile(std::string_view source, std::string_view chunkName,
        const Options &options);

} // namespace basalt::kiln
