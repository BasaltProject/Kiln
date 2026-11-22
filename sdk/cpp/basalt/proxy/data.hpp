#include <basalt/proxy.h>
#pragma once
#include <basalt/proxy/core.hpp>
#include <basalt/proxy/data.h>

namespace basalt::proxy::data {
using namespace basalt::proxy::core;

inline long long data_get_i64(Player player, const char* key, long long def) {
    return bs_data_get_i64(player, key, def);
}

inline int data_set_i64(Player player, const char* key, long long value) { return bs_data_set_i64(player, key, value); }

inline bool data_get_bool(Player player, const char* key, bool def) { return bs_data_get_bool(player, key, def) != 0; }

inline int data_set_bool(Player player, const char* key, bool value) { return bs_data_set_bool(player, key, value); }

inline Text<256> data_get_str(Player player, const char* key) {
    return core::detail::fill<256>([&](char* out, int cap) { return bs_data_get_str(player, key, out, cap); });
}

inline int data_set_str(Player player, const char* key, const char* value) {
    return bs_data_set_str(player, key, value);
}

inline int data_remove(Player player, const char* key) { return bs_data_remove(player, key); }
} // namespace basalt::proxy::data
