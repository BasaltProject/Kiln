#include <basalt/server.h>
#pragma once
#include <basalt/server/core.hpp>
#include <basalt/server/log.h>

namespace basalt::server::log {
using namespace basalt::server::core;

inline void log(Level level, const char* message) { bs_log(level, message); }

inline void log_debug(const char* message) { bs_debug(message); }

inline void log_info(const char* message) { bs_info(message); }

inline void log_warn(const char* message) { bs_warn(message); }

inline void log_error(const char* message) { bs_error(message); }
} // namespace basalt::server::log
