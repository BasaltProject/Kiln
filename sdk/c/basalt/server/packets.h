/* Plugin channels and raw packets (abi/areas/packets.h). Strings are NUL-terminated, bodies are bytes with a length.
 * Calls return the import result: a count or handle, or a negative BASALT_ERR_*. The out-calls fill a buffer you give
 * and return the length the full answer needs; open the buffer with bs_value_open for the value ones. */
#ifndef BASALT_SERVER_PACKETS_H
#define BASALT_SERVER_PACKETS_H

#include "basalt/server/core.h"
#include "basalt/server/values.h"
#include "areas/packets.h"

static inline int bs_pk_len(const char *text) {
    int n = 0;
    while (text != 0 && text[n]) {
        ++n;
    }
    return n;
}

static inline int bs_channel_listen(const char *name, bs_handler handler) {
    const int token = bs_token_for(handler);
    return token < 0 ? BASALT_ERR_INVALID : basalt_channel_listen(name, bs_pk_len(name), token);
}

static inline int bs_channel_send(bs_player player, const char *name, const void *body, int length) {
    return basalt_channel_send(player, name, bs_pk_len(name), (const char *)body, length);
}

/* `priority` is "first", "early", "normal", "late", "last" or NULL for the default. */
static inline int bs_packet_listen(const char *pattern, const char *priority, int observe, bs_handler handler) {
    char document[96];
    int length = 0;
    if ((priority != 0 && priority[0] != 0) || observe) {
        bs_vw writer;
        bs_vw_init(&writer, document, (int32_t)sizeof document);
        bs_vw_map(&writer);
        if (priority != 0 && priority[0] != 0) {
            bs_vw_key(&writer, "priority");
            bs_vw_str(&writer, priority);
        }
        if (observe) {
            bs_vw_key(&writer, "observe");
            bs_vw_bool(&writer, 1);
        }
        bs_vw_end(&writer);
        length = bs_vw_finish(&writer);
        if (length < 0) {
            return BASALT_ERR_INVALID;
        }
    }
    const int token = bs_token_for(handler);
    return token < 0 ? BASALT_ERR_INVALID
                     : basalt_packet_listen(pattern, bs_pk_len(pattern), document, length, token);
}

static inline int bs_packet_unlisten(int listener) { return basalt_packet_unlisten(listener); }

static inline int bs_packet_send(bs_player player, const char *name, const void *body, int length, int intercept) {
    return basalt_packet_send(player, name, bs_pk_len(name), (const char *)body, length, intercept);
}

static inline int bs_packet_receive(bs_player player, const char *name, const void *body, int length, int intercept) {
    return basalt_packet_receive(player, name, bs_pk_len(name), (const char *)body, length, intercept);
}

static inline int bs_packet_lookup(const char *name, char *out, int cap) {
    return basalt_packet_lookup(name, bs_pk_len(name), out, cap);
}

static inline int bs_packet_names(const char *prefix, char *out, int cap) {
    return basalt_packet_names(prefix, bs_pk_len(prefix), out, cap);
}

static inline int bs_packet_player_actions(char *out, int cap) { return basalt_packet_player_actions(out, cap); }

/* `args` is an encoded list value (see values.h). */
static inline int bs_packet_player_receive(bs_player player, const char *action, const void *args, int length) {
    return basalt_packet_player_receive(player, action, bs_pk_len(action), (const char *)args, length);
}

static inline int bs_packet_ctx_info(bs_ctx ctx, char *out, int cap) { return basalt_packet_ctx_info(ctx, out, cap); }

static inline int bs_packet_ctx_body(bs_ctx ctx, char *out, int cap) { return basalt_packet_ctx_body(ctx, out, cap); }

static inline bs_player bs_packet_ctx_player(bs_ctx ctx) { return basalt_packet_ctx_player(ctx); }

static inline int bs_packet_ctx_cancel(bs_ctx ctx) { return basalt_packet_ctx_cancel(ctx); }

static inline int bs_packet_ctx_set_body(bs_ctx ctx, const void *body, int length) {
    return basalt_packet_ctx_set_body(ctx, (const char *)body, length);
}

static inline int bs_packet_ctx_send(bs_ctx ctx, const char *name, const void *body, int length, int intercept) {
    return basalt_packet_ctx_send(ctx, name, bs_pk_len(name), (const char *)body, length, intercept);
}

static inline int bs_packet_ctx_receive(bs_ctx ctx, const char *name, const void *body, int length, int intercept) {
    return basalt_packet_ctx_receive(ctx, name, bs_pk_len(name), (const char *)body, length, intercept);
}

#endif /* BASALT_SERVER_PACKETS_H */
