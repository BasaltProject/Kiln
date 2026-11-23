#ifndef BASALT_PROXY_INTERACTION_H
#define BASALT_PROXY_INTERACTION_H

#include "basalt/proxy/core.h"
#include "basalt/proxy/values.h"
#include "areas/proxy_interaction.h"

/* Proxy interaction area (abi/areas/proxy_interaction.h): dialogs, resource packs, cookies, transfer and extra player
 * operations. Values are built and read with values.h; every function returns the ABI result. */

static inline int bs_pi_length(const char *text) {
    int n = 0;
    while (text != 0 && text[n]) {
        ++n;
    }
    return n;
}

/* Observes "player.resource_pack" (id, status) or "player.settings" (settings map); read the event with
 * bs_proxy_value_result in the handler. */
static inline int bs_proxy_watch_resource_packs(bs_handler handler) {
    const int token = bs_token_for(handler);
    return token < 0 ? BASALT_ERR_INVALID
                     : basalt_proxy_interaction_watch("player.resource_pack", 20, token);
}

static inline int bs_proxy_watch_settings(bs_handler handler) {
    const int token = bs_token_for(handler);
    return token < 0 ? BASALT_ERR_INVALID : basalt_proxy_interaction_watch("player.settings", 15, token);
}

/* The encoded value of an observed event or a completion; open it with bs_value_open. */
static inline int bs_proxy_value_result(bs_ctx ctx, char *out, int cap) { return basalt_value_result(ctx, out, cap); }

/* Opens a dialog: kind "notice", "confirmation" or "multi_action"; `body` may be null; `count` (label, command) pairs. */
static inline int bs_proxy_dialog_show(bs_player player, const char *kind, const char *title, const char *body,
                                       const char *const *labels, const char *const *commands, int count,
                                       int can_close_with_escape) {
    char buffer[6144];
    bs_vw w;
    bs_vw_init(&w, buffer, (int32_t)sizeof buffer);
    bs_vw_map(&w);
    bs_vw_key(&w, "kind");
    bs_vw_str(&w, kind);
    bs_vw_key(&w, "title");
    bs_vw_str(&w, title);
    if (body != 0) {
        bs_vw_key(&w, "body");
        bs_vw_str(&w, body);
    }
    bs_vw_key(&w, "buttons");
    bs_vw_list(&w);
    for (int i = 0; i < count; ++i) {
        bs_vw_map(&w);
        bs_vw_key(&w, "label");
        bs_vw_str(&w, labels[i]);
        bs_vw_key(&w, "command");
        bs_vw_str(&w, commands[i]);
        bs_vw_end(&w);
    }
    bs_vw_end(&w);
    bs_vw_key(&w, "can_close_with_escape");
    bs_vw_bool(&w, can_close_with_escape);
    bs_vw_end(&w);
    const int32_t n = bs_vw_finish(&w);
    return n < 0 ? BASALT_ERR_INVALID : basalt_proxy_dialog_show(player, buffer, n);
}

static inline int bs_proxy_dialog_clear(bs_player player) { return basalt_proxy_dialog_clear(player); }

/* Pushes a pack; `hash`, `prompt` and `id` may be null. The pack id text goes to `out`; the result is its length. */
static inline int bs_proxy_resource_pack_push(bs_player player, const char *url, const char *hash, int required,
                                              const char *prompt, const char *id, char *out, int cap) {
    char buffer[2048];
    bs_vw w;
    bs_vw_init(&w, buffer, (int32_t)sizeof buffer);
    bs_vw_map(&w);
    bs_vw_key(&w, "url");
    bs_vw_str(&w, url);
    if (hash != 0) {
        bs_vw_key(&w, "hash");
        bs_vw_str(&w, hash);
    }
    bs_vw_key(&w, "required");
    bs_vw_bool(&w, required);
    if (prompt != 0) {
        bs_vw_key(&w, "prompt");
        bs_vw_str(&w, prompt);
    }
    if (id != 0) {
        bs_vw_key(&w, "id");
        bs_vw_str(&w, id);
    }
    bs_vw_end(&w);
    const int32_t n = bs_vw_finish(&w);
    return n < 0 ? BASALT_ERR_INVALID : basalt_proxy_resource_pack_push(player, buffer, n, out, cap);
}

static inline int bs_proxy_resource_pack_remove(bs_player player, const char *id) {
    return basalt_proxy_resource_pack_remove(player, id, bs_pi_length(id));
}

static inline int bs_proxy_resource_pack_clear(bs_player player) { return basalt_proxy_resource_pack_clear(player); }

static inline int bs_proxy_cookie_set(bs_player player, const char *key, const void *data, int length) {
    return basalt_proxy_cookie_set(player, key, bs_pi_length(key), (const char *)data, length);
}

/* Asks the client for a cookie; the handler reads the answer with bs_proxy_cookie_result. Returns the request. */
static inline int bs_proxy_cookie_request(bs_player player, const char *key, bs_handler handler) {
    const int token = bs_once_token(handler);
    return token < 0 ? BASALT_ERR_INVALID
                     : bs_once_bind(token, BS_ONCE_REQUEST,
                                    basalt_proxy_cookie_request(player, key, bs_pi_length(key), token));
}

/* Copies the cookie of a completion into `out` and returns its length (written only when it fits), ABSENT when the
 * client had none. */
static inline int bs_proxy_cookie_result(bs_ctx ctx, void *out, int cap) {
    uint8_t raw[5200];
    const int n = basalt_value_result(ctx, (char *)raw, (int)sizeof raw);
    bs_val root;
    const uint8_t *data = 0;
    int32_t length = 0;
    if (n < 0) {
        return n;
    }
    if (n > (int)sizeof raw || bs_value_open(&root, raw, n) != 0) {
        return BASALT_ERR_INVALID;
    }
    if (bs_value_is_null(root)) {
        return BASALT_ERR_ABSENT;
    }
    if (!bs_value_as_bytes(root, &data, &length)) {
        return BASALT_ERR_INVALID;
    }
    if (length <= cap) {
        for (int32_t i = 0; i < length; ++i) {
            ((uint8_t *)out)[i] = data[i];
        }
    }
    return length;
}

static inline int bs_proxy_request_cancel(int request) { return bs_request_cancel(request); }

static inline int bs_proxy_player_transfer(bs_player player, const char *host, int port) {
    return basalt_proxy_player_transfer(player, host, bs_pi_length(host), port);
}

static inline bs_player bs_proxy_players_by_uuid(const char *uuid) {
    return basalt_proxy_players_by_uuid(uuid, bs_pi_length(uuid));
}

/* Players on the proxy, or on `server` when it is not null or empty. */
static inline int bs_proxy_players_count(const char *server) {
    return basalt_proxy_players_count(server, bs_pi_length(server));
}

static inline int bs_proxy_players_broadcast(const char *text, const bs_args *args) {
    const int has = args != 0 && args->len > 4;
    return basalt_proxy_players_broadcast(text, bs_pi_length(text), has ? args->data : 0, has ? args->len : 0);
}

static inline int bs_proxy_player_address(bs_player player, char *out, int cap) {
    return basalt_proxy_player_address(player, out, cap);
}

static inline int bs_proxy_player_port(bs_player player) { return basalt_proxy_player_port(player); }

/* The client settings as an encoded map for bs_value_open. */
static inline int bs_proxy_player_settings(bs_player player, char *out, int cap) {
    return basalt_proxy_player_settings(player, out, cap);
}

static inline double bs_proxy_player_data_get_f64(bs_player player, const char *key, double fallback) {
    return basalt_proxy_player_data_get_f64(player, key, bs_pi_length(key), fallback);
}

static inline int bs_proxy_player_data_set_f64(bs_player player, const char *key, double value) {
    return basalt_proxy_player_data_set_f64(player, key, bs_pi_length(key), value);
}

#endif
