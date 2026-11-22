#include <basalt/proxy.h>
#include <basalt/proxy/admin.h>
#include <basalt/proxy/core.hpp>
#include <basalt/proxy/values.hpp>
// Proxy admin area (abi/areas/proxy_admin.h) over wrappers/proxy_admin.h. Calls that give a value fill your buffer and
// set a Value over it.
#pragma once

namespace basalt::proxy::admin {
using namespace basalt::proxy::core;
using namespace basalt::proxy::values;

/// OK, or INVALID with the reason in `err` (cut at `err_cap`, not NUL terminated).
inline int servers_register(const char* name, const char* host, int port, const char* like, char* err, int err_cap) {
    return bs_proxy_servers_register(name, host, port, like, err, err_cap);
}

inline int servers_unregister(const char* name) { return bs_proxy_servers_unregister(name); }

/// Pings a backend; the handler reads the answer with completion_value. Returns the request handle.
inline int servers_ping(const char* name, Handler handler) { return bs_proxy_servers_ping(name, handler); }

/// Runs the handler after `ticks` ticks of 50 ms. Returns the request handle.
inline int wait(int ticks, Handler handler) { return bs_proxy_wait(ticks, handler); }

inline int completion_value(Ctx ctx, char* buffer, int capacity, Value& root) {
    bs_val raw;
    const int status = bs_proxy_completion_value(ctx, buffer, capacity, &raw);
    root = Value{raw};
    return status;
}

/// 1 when the line named a command, 0 when not, DENIED without the capability and the operator's grant.
inline int console_run(const char* line) { return bs_proxy_console_run(line); }

inline int player_chat(Player player, const char* text) { return bs_proxy_player_chat(player, text); }

inline int player_send_json(Player player, const char* json) { return bs_proxy_player_send_json(player, json); }

inline int player_kick_json(Player player, const char* json) { return bs_proxy_player_kick_json(player, json); }

/// 1 granted, 0 denied, ABSENT undecided. `who` "" is the console; `server` may be null.
inline int permission_check(const char* who, const char* node, const char* server = nullptr) {
    return bs_proxy_permission_check(who, node, server);
}

inline Text<256> permission_meta(const char* who, const char* key) {
    return core::detail::fill<256>([&](char* out, int cap) { return bs_proxy_permission_meta(who, key, out, cap); });
}

/// Registers provider callbacks (either may be null); returns the provider handle.
inline int permission_provide(Handler check, Handler meta, int priority = 0) {
    return bs_proxy_permission_provide(check, meta, priority);
}

inline int permission_unprovide(int provider) { return bs_proxy_permission_unprovide(provider); }

/// The question of a provider callback: a map with kind, name, uuid, console, text and context.
inline int permission_request(Ctx ctx, char* buffer, int capacity, Value& root) {
    bs_val raw;
    const int status = bs_proxy_permission_request(ctx, buffer, capacity, &raw);
    root = Value{raw};
    return status;
}

/// 1 grants, 0 denies, negative leaves it to the next provider.
inline int permission_answer(Ctx ctx, int answer) { return bs_proxy_permission_answer(ctx, answer); }

inline int permission_answer_text(Ctx ctx, const char* text) { return bs_proxy_permission_answer_text(ctx, text); }

inline int permission_changed() { return bs_proxy_permission_changed(); }

/// 1 grants, 0 denies, -1 removes.
inline int permission_override(const char* name, const char* node, int value) {
    return bs_proxy_permission_override(name, node, value);
}

inline int permission_list(const char* name, char* buffer, int capacity, Value& root) {
    bs_val raw;
    const int status = bs_proxy_permission_list(name, buffer, capacity, &raw);
    root = Value{raw};
    return status;
}

inline Text<64> group_get(const char* name) {
    return core::detail::fill<64>([&](char* out, int cap) { return bs_proxy_group_get(name, out, cap); });
}

inline int group_set(const char* name, const char* group) { return bs_proxy_group_set(name, group); }

/// Reads a store key (holder null) or a player data key of `holder` as a value; ABSENT when unset.
inline int data_value(const char* holder, const char* key, char* buffer, int capacity, Value& root) {
    bs_val raw;
    const int status = bs_proxy_data_value(holder, key, buffer, capacity, &raw);
    root = Value{raw};
    return status;
}

inline int data_set_i64(const char* holder, const char* key, long long value) {
    return bs_proxy_data_set_i64(holder, key, value);
}

inline int data_set_f64(const char* holder, const char* key, double value) {
    return bs_proxy_data_set_f64(holder, key, value);
}

inline int data_set_bool(const char* holder, const char* key, bool value) {
    return bs_proxy_data_set_bool(holder, key, value);
}

inline int data_set_str(const char* holder, const char* key, const char* value) {
    return bs_proxy_data_set_str(holder, key, value);
}

inline int data_remove(const char* holder, const char* key) { return bs_proxy_data_remove(holder, key); }

/// The best `limit` numeric values of a store key: a list of maps with uuid, name and value.
inline int store_top(const char* key, int limit, char* buffer, int capacity, Value& root) {
    bs_val raw;
    const int status = bs_proxy_store_top(key, limit, buffer, capacity, &raw);
    root = Value{raw};
    return status;
}

} // namespace basalt::proxy::admin
