#include "BuildInfo.hpp"

#ifndef BASALT_VERSION
#define BASALT_VERSION "0.0.0-dev"
#endif

namespace basalt::kiln::buildInfo {

const std::string_view kVersion = BASALT_VERSION;

} // namespace basalt::kiln::buildInfo
