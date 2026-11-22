#include <basalt/proxy.h>
#include <basalt/proxy/core.hpp>
#include <basalt/proxy/network.h>
#include <basalt/proxy/values.hpp>
// Proxy network area (abi/areas/proxy_network.h) over wrappers/proxy_network.h: messaging channels, raw packets and
// antibot control. Calls that fill a buffer give the raw encoded value for the ValueReader of values.hpp.
#pragma once

namespace basalt::proxy::network {
using namespace basalt::proxy::core;
using namespace basalt::proxy::values;

inline int channel_register(const char* name) { return bs_net_channel_register(name); }

inline int channel_unregister(const char* name) { return bs_net_channel_unregister(name); }

/// Observes the messages of the plugin's channels; an empty filter takes all of them.
inline int message_on(const char* filter, Handler handler) { return bs_net_message_on(filter, handler); }

inline int message_get(Ctx ctx, char* out, int capacity) { return bs_net_message_get(ctx, out, capacity); }

/// `toward_client` false sends to the player's backend, true to the client.
inline int message_send(int player, const char* channel, const char* data, int length, bool toward_client) {
    return bs_net_message_send(player, channel, data, length, toward_client ? 1 : 0);
}

inline int packet_resolve(const char* name, char* out, int capacity) {
    return bs_net_packet_resolve(name, out, capacity);
}

inline int packet_names(char* out, int capacity) { return bs_net_packet_names(out, capacity); }

/// A decider listener; `opts` is an encoded value map (priority, fail) or null with length 0. Returns the listener.
inline int packet_on(const char* name, const char* opts, int opts_length, Handler handler) {
    return bs_net_packet_on(name, opts, opts_length, handler);
}

/// An observer listener: it sees the final packet and cannot cancel or change it.
inline int packet_watch(const char* name, const char* opts, int opts_length, Handler handler) {
    return bs_net_packet_watch(name, opts, opts_length, handler);
}

inline int packet_remove(int listener) { return bs_net_packet_remove(listener); }

inline int packet_get(Ctx ctx, char* out, int capacity) { return bs_net_packet_get(ctx, out, capacity); }

inline int packet_body(Ctx ctx, char* out, int capacity) { return bs_net_packet_body(ctx, out, capacity); }

inline int packet_set_body(Ctx ctx, const char* body, int length) { return bs_net_packet_set_body(ctx, body, length); }

inline int packet_cancel(Ctx ctx) { return bs_net_packet_cancel(ctx); }

inline int packet_inject(Ctx ctx, const char* name, const char* body, int length, bool intercept) {
    return bs_net_packet_inject(ctx, name, body, length, intercept ? 1 : 0);
}

inline int packet_send(int player, const char* name, const char* body, int length) {
    return bs_net_packet_send(player, name, body, length);
}

inline int packet_receive(int player, const char* name, const char* body, int length) {
    return bs_net_packet_receive(player, name, body, length);
}

inline int antibot_verify(const char* name) { return bs_net_antibot_verify(name); }

inline int antibot_unverify(const char* name) { return bs_net_antibot_unverify(name); }

inline int antibot_is_verified(const char* name) { return bs_net_antibot_is_verified(name); }

inline int antibot_ban(const char* address) { return bs_net_antibot_ban(address); }

inline int antibot_unban(const char* address) { return bs_net_antibot_unban(address); }

inline int antibot_under_attack() { return bs_net_antibot_under_attack(); }

inline int antibot_on(Handler handler) { return bs_net_antibot_on(handler); }

inline int antibot_event_get(Ctx ctx, char* out, int capacity) { return bs_net_antibot_event_get(ctx, out, capacity); }

} // namespace basalt::proxy::network
