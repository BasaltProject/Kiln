#pragma once

#include <string>
#include <vector>

namespace basalt::kiln {

/// The project commands; `args` starts with the command name. Each returns the process exit code.
[[nodiscard]] int runNew(const std::vector<std::string>& args);
[[nodiscard]] int runPack(const std::vector<std::string>& args);
[[nodiscard]] int runTest(const std::vector<std::string>& args);

} // namespace basalt::kiln
