#ifndef BASALT_PROXY_NETWORK_H
#define BASALT_PROXY_NETWORK_H

#include "basalt/proxy/core.h"
#include "basalt/proxy/values.h"
#include "areas/proxy_network.h"

/* Proxy network area (abi/areas/proxy_network.h): plugin messaging channels, raw packet listeners and senders, and
 * antibot control. Calls that answer a value fill your buffer with the encoded value for bs_value_* of values.h. */

static inline int bs_net_length(const char *text) {
    int n = 0;
    while (text != 0 && text[n]) {
        ++n;
    }
    return n;
}

static inline int bs_net_channel_register(const char *name) {
    return basalt_proxy_network_channel_register(name, bs_net_length(name));
}

static inline int bs_net_channel_unregister(const char *name) {
    return basalt_proxy_network_channel_unregister(name, bs_net_length(name));
}

/* Observes the messages of the plugin's channels; an empty or null filter takes all of them. */
static inline int bs_net_message_on(const char *filter, bs_handler handler) {
    const int token = bs_token_for(handler);
    return token < 0 ? BASALT_ERR_INVALID : basalt_proxy_network_message_on(filter, bs_net_length(filter), token);
}

static inline int bs_net_message_get(bs_ctx ctx, char *out, int cap) {
    return basalt_proxy_network_message_get(ctx, out, cap);
}

/* toward_client 0 sends to the player's backend, 1 to the client. */
static inline int bs_net_message_send(int player, const char *channel, const char *data, int len, int toward_client) {
    return basalt_proxy_network_message_send(player, channel, bs_net_length(channel), data, len, toward_client);
}

static inline int bs_net_packet_resolve(const char *name, char *out, int cap) {
    return basalt_proxy_packet_resolve(name, bs_net_length(name), out, cap);
}

static inline int bs_net_packet_names(char *out, int cap) { return basalt_proxy_packet_names(out, cap); }

/* Decider listener; `opts` is an encoded value map (priority, fail) or null with length 0. Returns the listener. */
static inline int bs_net_packet_on(const char *name, const char *opts, int opts_len, bs_handler handler) {
    const int token = bs_token_for(handler);
    return token < 0 ? BASALT_ERR_INVALID
                     : basalt_proxy_packet_on(name, bs_net_length(name), opts, opts_len, token);
}

/* Observer listener: it sees the final packet and cannot cancel or change it. */
static inline int bs_net_packet_watch(const char *name, const char *opts, int opts_len, bs_handler handler) {
    const int token = bs_token_for(handler);
    return token < 0 ? BASALT_ERR_INVALID
                     : basalt_proxy_packet_watch(name, bs_net_length(name), opts, opts_len, token);
}

static inline int bs_net_packet_remove(int listener) { return basalt_proxy_packet_remove(listener); }

static inline int bs_net_packet_get(bs_ctx ctx, char *out, int cap) { return basalt_proxy_packet_get(ctx, out, cap); }

static inline int bs_net_packet_body(bs_ctx ctx, char *out, int cap) { return basalt_proxy_packet_body(ctx, out, cap); }

static inline int bs_net_packet_set_body(bs_ctx ctx, const char *body, int len) {
    return basalt_proxy_packet_set_body(ctx, body, len);
}

static inline int bs_net_packet_cancel(bs_ctx ctx) { return basalt_proxy_packet_cancel(ctx); }

static inline int bs_net_packet_inject(bs_ctx ctx, const char *name, const char *body, int len, int intercept) {
    return basalt_proxy_packet_inject(ctx, name, bs_net_length(name), body, len, intercept);
}

static inline int bs_net_packet_send(int player, const char *name, const char *body, int len) {
    return basalt_proxy_packet_send(player, name, bs_net_length(name), body, len);
}

static inline int bs_net_packet_receive(int player, const char *name, const char *body, int len) {
    return basalt_proxy_packet_receive(player, name, bs_net_length(name), body, len);
}

static inline int bs_net_antibot_verify(const char *name) {
    return basalt_proxy_antibot_verify(name, bs_net_length(name));
}

static inline int bs_net_antibot_unverify(const char *name) {
    return basalt_proxy_antibot_unverify(name, bs_net_length(name));
}

static inline int bs_net_antibot_is_verified(const char *name) {
    return basalt_proxy_antibot_is_verified(name, bs_net_length(name));
}

static inline int bs_net_antibot_ban(const char *address) {
    return basalt_proxy_antibot_ban(address, bs_net_length(address));
}

static inline int bs_net_antibot_unban(const char *address) {
    return basalt_proxy_antibot_unban(address, bs_net_length(address));
}

static inline int bs_net_antibot_under_attack(void) { return basalt_proxy_antibot_under_attack(); }

static inline int bs_net_antibot_on(bs_handler handler) {
    const int token = bs_token_for(handler);
    return token < 0 ? BASALT_ERR_INVALID : basalt_proxy_antibot_on(token);
}

static inline int bs_net_antibot_event_get(bs_ctx ctx, char *out, int cap) {
    return basalt_proxy_antibot_event_get(ctx, out, cap);
}

#endif
