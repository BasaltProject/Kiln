#pragma once

#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace basalt::kiln {

struct ProcessResult {
    bool started = false;
    int exitCode = -1;
    /// Standard output and error, interleaved.
    std::string output;
};

/// Runs a program without a shell; `environment` entries replace the inherited ones and `PATH` entries are prepended.
[[nodiscard]] ProcessResult runProcess(const std::vector<std::string>& command,
                                       const std::filesystem::path& workingDirectory,
                                       const std::map<std::string, std::string>& environment = {});

/// The first executable called `name` on `PATH`, with the platform's executable suffixes tried.
[[nodiscard]] std::optional<std::filesystem::path> findOnPath(const std::string& name);

} // namespace basalt::kiln
