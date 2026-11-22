#include <basalt/proxy.h>
#pragma once
#include <basalt/proxy/core.hpp>
#include <basalt/proxy/events.h>

namespace basalt::proxy::events {
using namespace basalt::proxy::core;

inline int on(const char* event, Priority priority, Handler handler) { return bs_on(event, priority, handler); }

inline Player event_player(Ctx ctx) { return bs_event_player(ctx); }

inline Text<256> event_field_str(Ctx ctx, const char* field) {
    return core::detail::fill<256>([&](char* out, int cap) { return bs_event_field_str(ctx, field, out, cap); });
}

inline int event_field_i64(Ctx ctx, const char* field, long long* out) { return bs_event_field_i64(ctx, field, out); }

inline int event_cancel(Ctx ctx, bool cancelled) { return bs_event_cancel(ctx, cancelled); }
} // namespace basalt::proxy::events

#include <basalt/proxy/core.hpp>
#include <basalt/proxy/events.h>
#include <basalt/proxy/values.hpp>
// Proxy events area (abi/areas/proxy_events.h) over wrappers/proxy_events.h. The `*_value` calls give the raw encoded
// value for the ValueReader of values.hpp.
#pragma once

namespace basalt::proxy::events {
using namespace basalt::proxy::core;
using namespace basalt::proxy::values;

/// Decides or observes `event`: an observer sees the finished event, its context is read only.
inline int watch(const char* event, Handler handler) { return bs_proxy_event_watch(event, handler); }

inline int event_get_value(Ctx ctx, const char* field, char* out, int capacity) {
    return bs_proxy_event_get_value(ctx, field, out, capacity);
}

inline Text<256> event_get_str(Ctx ctx, const char* field) {
    return core::detail::fill<256>([&](char* out, int cap) { return bs_proxy_event_get_str(ctx, field, out, cap); });
}

inline int event_get_i64(Ctx ctx, const char* field, long long* out) {
    return bs_proxy_event_get_i64(ctx, field, reinterpret_cast<int64_t*>(out));
}

inline int event_get_bool(Ctx ctx, const char* field, bool* out) {
    int flag = 0;
    const int status = bs_proxy_event_get_bool(ctx, field, &flag);
    *out = flag != 0;
    return status;
}

inline int event_fields(Ctx ctx, char* out, int capacity) { return bs_proxy_event_fields(ctx, out, capacity); }

inline int event_set_value(Ctx ctx, const char* field, const char* value, int length) {
    return bs_proxy_event_set_value(ctx, field, value, length);
}

inline int event_set_text(Ctx ctx, const char* field, const char* text) {
    return bs_proxy_event_set_text(ctx, field, text);
}

inline int event_set_int(Ctx ctx, const char* field, long long value) {
    return bs_proxy_event_set_int(ctx, field, value);
}

inline int event_set_flag(Ctx ctx, const char* field, bool flag) { return bs_proxy_event_set_flag(ctx, field, flag); }

/// Cancels a decided event and shows `reason` (MiniMessage, or a '@' lang key) to the player.
inline int event_deny(Ctx ctx, const char* reason) { return bs_proxy_event_deny(ctx, reason); }

inline Text<64> plugin_info(const char* field) {
    return core::detail::fill<64>([&](char* out, int cap) { return bs_proxy_plugin_info(field, out, cap); });
}

inline int plugin_manifest(char* out, int capacity) { return bs_proxy_plugin_manifest(out, capacity); }

inline int config_value(const char* key, char* out, int capacity) { return bs_proxy_config_value(key, out, capacity); }

inline int command_arg_f64(Ctx ctx, const char* name, double* out) { return bs_proxy_command_arg_f64(ctx, name, out); }

inline int command_arg_bool(Ctx ctx, const char* name, bool* out) {
    int flag = 0;
    const int status = bs_proxy_command_arg_bool(ctx, name, &flag);
    *out = flag != 0;
    return status;
}

inline int command_args(Ctx ctx, char* out, int capacity) { return bs_proxy_command_args(ctx, out, capacity); }

} // namespace basalt::proxy::events
