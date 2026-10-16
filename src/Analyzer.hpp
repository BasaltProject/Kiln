#pragma once

#include <expected>
#include <string>
#include <vector>

namespace basalt::kiln {

struct Access {
    std::string path;
    int line = 0;
};

/// What a chunk reads and writes of the shared environment, found by walking its bytecode.
struct Usage {
    std::vector<Access> globalReads;
    std::vector<Access> fieldReads;
    std::vector<Access> requires_;
    std::vector<std::string> globalWrites;
};

[[nodiscard]] std::expected<Usage, std::string> analyze(std::string_view source, std::string_view chunkName);

} // namespace basalt::kiln
