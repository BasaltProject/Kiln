#include <basalt/proxy.h>
#pragma once
#include <basalt/proxy/core.hpp>
#include <basalt/proxy/log.h>

namespace basalt::proxy::log {
using namespace basalt::proxy::core;

inline void log(Level level, const char* message) { bs_log(level, message); }

inline void log_debug(const char* message) { bs_debug(message); }

inline void log_info(const char* message) { bs_info(message); }

inline void log_warn(const char* message) { bs_warn(message); }

inline void log_error(const char* message) { bs_error(message); }
} // namespace basalt::proxy::log
