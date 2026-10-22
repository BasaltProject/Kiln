#pragma once

#include "Project.hpp"

namespace basalt::kiln {

/// Runs every `tests/*.lua` of the plugin in its sandbox against a mock server; false if any test failed.
[[nodiscard]] bool runTests(const Project& project);

} // namespace basalt::kiln
