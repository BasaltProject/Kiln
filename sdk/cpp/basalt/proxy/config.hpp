#include <basalt/proxy.h>
#pragma once
#include <basalt/proxy/config.h>
#include <basalt/proxy/core.hpp>

namespace basalt::proxy::config {
using namespace basalt::proxy::core;

inline long long config_i64(const char* key, long long def) { return bs_config_i64(key, def); }

inline double config_f64(const char* key, double def) { return bs_config_f64(key, def); }

inline bool config_bool(const char* key, bool def) { return bs_config_bool(key, def) != 0; }

inline Text<256> config_str(const char* key) {
    return core::detail::fill<256>([&](char* out, int cap) { return bs_config_str(key, out, cap); });
}
} // namespace basalt::proxy::config
