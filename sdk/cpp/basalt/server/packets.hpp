#include <basalt/server.h>
#include <basalt/server/core.hpp>
#include <basalt/server/packets.h>
#include <basalt/server/values.hpp>
// Plugin channels and raw packets (abi/areas/packets.h) over wrappers/packets.h. Calls return the import result: a
// count or handle, or a negative BASALT_ERR_*. The value-returning calls fill a buffer you give and return the length
// the full answer needs; open it with Value::open.
#pragma once

namespace basalt::server::packets {
using namespace basalt::server::core;
using namespace basalt::server::values;

inline int channel_listen(const char* name, Handler handler) { return bs_channel_listen(name, handler); }

inline int channel_send(Player player, const char* name, const void* body, int length) {
    return bs_channel_send(player, name, body, length);
}

/// `priority` is "first", "early", "normal", "late", "last" or null for the default.
inline int packet_listen(const char* pattern, const char* priority, bool observe, Handler handler) {
    return bs_packet_listen(pattern, priority, observe ? 1 : 0, handler);
}

inline int packet_unlisten(int listener) { return bs_packet_unlisten(listener); }

inline int packet_send(Player player, const char* name, const void* body, int length, bool intercept) {
    return bs_packet_send(player, name, body, length, intercept ? 1 : 0);
}

inline int packet_receive(Player player, const char* name, const void* body, int length, bool intercept) {
    return bs_packet_receive(player, name, body, length, intercept ? 1 : 0);
}

inline int packet_lookup(const char* name, char* out, int cap) { return bs_packet_lookup(name, out, cap); }

inline int packet_names(const char* prefix, char* out, int cap) { return bs_packet_names(prefix, out, cap); }

inline int packet_player_actions(char* out, int cap) { return bs_packet_player_actions(out, cap); }

/// `args` is an encoded list value.
inline int packet_player_receive(Player player, const char* action, const void* args, int length) {
    return bs_packet_player_receive(player, action, args, length);
}

inline int packet_ctx_info(Ctx ctx, char* out, int cap) { return bs_packet_ctx_info(ctx, out, cap); }

inline int packet_ctx_body(Ctx ctx, char* out, int cap) { return bs_packet_ctx_body(ctx, out, cap); }

inline Player packet_ctx_player(Ctx ctx) { return bs_packet_ctx_player(ctx); }

inline int packet_ctx_cancel(Ctx ctx) { return bs_packet_ctx_cancel(ctx); }

inline int packet_ctx_set_body(Ctx ctx, const void* body, int length) {
    return bs_packet_ctx_set_body(ctx, body, length);
}

inline int packet_ctx_send(Ctx ctx, const char* name, const void* body, int length, bool intercept) {
    return bs_packet_ctx_send(ctx, name, body, length, intercept ? 1 : 0);
}

inline int packet_ctx_receive(Ctx ctx, const char* name, const void* body, int length, bool intercept) {
    return bs_packet_ctx_receive(ctx, name, body, length, intercept ? 1 : 0);
}

} // namespace basalt::server::packets
