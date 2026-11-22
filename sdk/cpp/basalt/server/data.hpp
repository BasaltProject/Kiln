#include <basalt/server.h>
#pragma once
#include <basalt/server/core.hpp>
#include <basalt/server/data.h>

namespace basalt::server::data {
using namespace basalt::server::core;

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

inline int store_top(const char* key, int limit, char* out, int capacity) {
    return bs_store_top(key, limit, out, capacity);
}
} // namespace basalt::server::data
