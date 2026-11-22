#pragma once

#include <string>
#include <vector>

namespace basalt::kiln {

/// The commands; `args` starts with the command name. Each returns the process exit code.
[[nodiscard]] int runNew(const std::vector<std::string>& args);
[[nodiscard]] int runBuild(const std::vector<std::string>& args);
[[nodiscard]] int runCheck(const std::vector<std::string>& args);
[[nodiscard]] int runPack(const std::vector<std::string>& args);
[[nodiscard]] int runToolchain(const std::vector<std::string>& args);

} // namespace basalt::kiln
