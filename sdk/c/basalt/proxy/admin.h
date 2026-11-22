#ifndef BASALT_PROXY_ADMIN_H
#define BASALT_PROXY_ADMIN_H

#include "basalt/proxy/core.h"
#include "basalt/proxy/values.h"
#include "areas/proxy_admin.h"

#ifdef __cplusplus
extern "C" {
#endif

int bs_proxy_servers_list(char *out, int cap);
int bs_proxy_server_address(const char *name, char *out, int cap);

#ifdef __cplusplus
}
#endif

/* Proxy admin area (abi/areas/proxy_admin.h): backends, console and chat, permission providers, groups, component JSON,
 * the plugin store, data by holder and waiting. Calls that give a value fill your buffer and set a bs_val over it. */

static inline int bs_pa_length(const char *text) {
    int n = 0;
    while (text != 0 && text[n]) {
        ++n;
    }
    return n;
}

typedef struct {
    int32_t ctx;
    const char *a;
    const char *b;
    int32_t limit;
} bs_pa_args;

static inline int bs_proxy_servers_register(const char *name, const char *host, int port, const char *like, char *err,
                                            int err_cap) {
    return basalt_proxy_servers_register(name, bs_pa_length(name), host, bs_pa_length(host), port, like,
                                         bs_pa_length(like), err, err_cap);
}

static inline int bs_proxy_servers_unregister(const char *name) {
    return basalt_proxy_servers_unregister(name, bs_pa_length(name));
}

/* Starts a ping of a backend; handler runs with the answer, see bs_proxy_completion_value. Returns the request. */
static inline int bs_proxy_servers_ping(const char *name, bs_handler handler) {
    const int token = bs_token_for(handler);
    return token < 0 ? BASALT_ERR_INVALID : basalt_proxy_servers_ping(name, bs_pa_length(name), token);
}

/* Runs handler after `ticks` ticks of 50 ms. Returns the request. */
static inline int bs_proxy_wait(int ticks, bs_handler handler) {
    const int token = bs_token_for(handler);
    return token < 0 ? BASALT_ERR_INVALID : basalt_proxy_wait(ticks, token);
}

static inline int32_t bs_pa_result_fn(void *user, char *out, int32_t cap) {
    return basalt_value_result(((bs_pa_args *)user)->ctx, out, cap);
}

/* The value a completion callback received (the ping map, or null); `root` points into `buf`. */
static inline int bs_proxy_completion_value(bs_ctx ctx, char *buf, int cap, bs_val *root) {
    bs_pa_args args = {ctx, 0, 0, 0};
    return bs_out_value(bs_pa_result_fn, &args, buf, cap, 0, 0, root);
}

static inline int bs_proxy_console_run(const char *line) { return basalt_proxy_console_run(line, bs_pa_length(line)); }

static inline int bs_proxy_player_chat(bs_player player, const char *text) {
    return basalt_proxy_player_chat(player, text, bs_pa_length(text));
}

static inline int bs_proxy_player_send_json(bs_player player, const char *json) {
    return basalt_proxy_player_send_json(player, json, bs_pa_length(json));
}

static inline int bs_proxy_player_kick_json(bs_player player, const char *json) {
    return basalt_proxy_player_kick_json(player, json, bs_pa_length(json));
}

/* 1 granted, 0 denied, BASALT_ERR_ABSENT undecided. `who` is a name, "" for the console; `server` may be 0. */
static inline int bs_proxy_permission_check(const char *who, const char *node, const char *server) {
    char raw[64];
    bs_vw w;
    bs_vw_init(&w, raw, (int32_t)sizeof raw);
    if (server != 0 && server[0] != 0) {
        bs_vw_map(&w);
        bs_vw_key(&w, "server");
        bs_vw_str(&w, server);
        bs_vw_end(&w);
    }
    const int32_t n = server != 0 && server[0] != 0 ? bs_vw_finish(&w) : 0;
    return basalt_proxy_permission_check(who, bs_pa_length(who), node, bs_pa_length(node), raw, n < 0 ? 0 : n);
}

static inline int bs_proxy_permission_meta(const char *who, const char *key, char *out, int cap) {
    return basalt_proxy_permission_meta(who, bs_pa_length(who), key, bs_pa_length(key), 0, 0, out, cap);
}

/* Registers provider callbacks (either may be 0); returns the provider handle. */
static inline int bs_proxy_permission_provide(bs_handler check, bs_handler meta, int priority) {
    const int check_token = check != 0 ? bs_token_for(check) : 0;
    const int meta_token = meta != 0 ? bs_token_for(meta) : 0;
    return check_token < 0 || meta_token < 0 ? BASALT_ERR_INVALID
                                              : basalt_proxy_permission_provide(check_token, meta_token, priority);
}

static inline int bs_proxy_permission_unprovide(int provider) { return basalt_proxy_permission_unprovide(provider); }

static inline int32_t bs_pa_request_fn(void *user, char *out, int32_t cap) {
    return basalt_proxy_permission_request(((bs_pa_args *)user)->ctx, out, cap);
}

/* The question a provider callback is asked, as a map (kind, name, uuid, console, text, context). */
static inline int bs_proxy_permission_request(bs_ctx ctx, char *buf, int cap, bs_val *root) {
    bs_pa_args args = {ctx, 0, 0, 0};
    return bs_out_value(bs_pa_request_fn, &args, buf, cap, 0, 0, root);
}

/* 1 grants, 0 denies, negative leaves it undefined. */
static inline int bs_proxy_permission_answer(bs_ctx ctx, int answer) {
    return basalt_proxy_permission_answer(ctx, answer);
}

static inline int bs_proxy_permission_answer_text(bs_ctx ctx, const char *text) {
    return basalt_proxy_permission_answer_text(ctx, text, bs_pa_length(text));
}

static inline int bs_proxy_permission_changed(void) { return basalt_proxy_permission_changed(); }

/* 1 grants, 0 denies, -1 removes. */
static inline int bs_proxy_permission_override(const char *name, const char *node, int value) {
    return basalt_proxy_permission_override(name, bs_pa_length(name), node, bs_pa_length(node), value);
}

static inline int32_t bs_pa_list_fn(void *user, char *out, int32_t cap) {
    const bs_pa_args *args = (const bs_pa_args *)user;
    return basalt_proxy_permission_list(args->a, bs_pa_length(args->a), out, cap);
}

/* The permission nodes of a player as a list of strings. */
static inline int bs_proxy_permission_list(const char *name, char *buf, int cap, bs_val *root) {
    bs_pa_args args = {0, name, 0, 0};
    return bs_out_value(bs_pa_list_fn, &args, buf, cap, 0, 0, root);
}

static inline int bs_proxy_group_get(const char *name, char *out, int cap) {
    return basalt_proxy_group_get(name, bs_pa_length(name), out, cap);
}

static inline int bs_proxy_group_set(const char *name, const char *group) {
    return basalt_proxy_group_set(name, bs_pa_length(name), group, bs_pa_length(group));
}

/* Writes the document of a scalar (or null when `text` and the others are unused) into `raw`; returns its length. */
static inline int bs_pa_scalar(char *raw, int cap, int kind, int64_t whole, double real, const char *text) {
    bs_vw w;
    bs_vw_init(&w, raw, cap);
    if (kind == BS_VALUE_NULL) {
        bs_vw_null(&w);
    } else if (kind == BS_VALUE_BOOL) {
        bs_vw_bool(&w, whole != 0);
    } else if (kind == BS_VALUE_INT) {
        bs_vw_i64(&w, whole);
    } else if (kind == BS_VALUE_FLOAT) {
        bs_vw_f64(&w, real);
    } else {
        bs_vw_str(&w, text);
    }
    return bs_vw_finish(&w);
}

static inline int32_t bs_pa_store_fn(void *user, char *out, int32_t cap) {
    const bs_pa_args *args = (const bs_pa_args *)user;
    if (args->b != 0) {
        return basalt_proxy_player_data_get(args->b, bs_pa_length(args->b), args->a, bs_pa_length(args->a), out, cap);
    }
    return basalt_proxy_store_get(args->a, bs_pa_length(args->a), out, cap);
}

/* Reads a store key (holder 0) or a player data key of `holder` as a value over `buf`; ABSENT when unset. */
static inline int bs_proxy_data_value(const char *holder, const char *key, char *buf, int cap, bs_val *root) {
    bs_pa_args args = {0, key, holder, 0};
    return bs_out_value(bs_pa_store_fn, &args, buf, cap, 0, 0, root);
}

static inline int bs_pa_data_set(const char *holder, const char *key, const char *raw, int n) {
    if (n < 0) {
        return BASALT_ERR_INVALID;
    }
    if (holder != 0) {
        return basalt_proxy_player_data_set(holder, bs_pa_length(holder), key, bs_pa_length(key), raw, n);
    }
    return basalt_proxy_store_set(key, bs_pa_length(key), raw, n);
}

/* Store (holder 0) and per-holder data writers; the value is deleted by bs_proxy_data_remove. */
static inline int bs_proxy_data_set_i64(const char *holder, const char *key, int64_t value) {
    char raw[16];
    return bs_pa_data_set(holder, key, raw, bs_pa_scalar(raw, (int)sizeof raw, BS_VALUE_INT, value, 0, 0));
}

static inline int bs_proxy_data_set_f64(const char *holder, const char *key, double value) {
    char raw[16];
    return bs_pa_data_set(holder, key, raw, bs_pa_scalar(raw, (int)sizeof raw, BS_VALUE_FLOAT, 0, value, 0));
}

static inline int bs_proxy_data_set_bool(const char *holder, const char *key, int value) {
    char raw[4];
    return bs_pa_data_set(holder, key, raw, bs_pa_scalar(raw, (int)sizeof raw, BS_VALUE_BOOL, value, 0, 0));
}

static inline int bs_proxy_data_set_str(const char *holder, const char *key, const char *value) {
    char raw[4200];
    return bs_pa_data_set(holder, key, raw, bs_pa_scalar(raw, (int)sizeof raw, BS_VALUE_STRING, 0, 0, value));
}

static inline int bs_proxy_data_remove(const char *holder, const char *key) {
    char raw[2];
    return bs_pa_data_set(holder, key, raw, bs_pa_scalar(raw, (int)sizeof raw, BS_VALUE_NULL, 0, 0, 0));
}

static inline int32_t bs_pa_top_fn(void *user, char *out, int32_t cap) {
    const bs_pa_args *args = (const bs_pa_args *)user;
    return basalt_proxy_store_top(args->a, bs_pa_length(args->a), args->limit, out, cap);
}

/* The best `limit` numeric values of a store key as a list of maps (uuid, name, value). */
static inline int bs_proxy_store_top(const char *key, int limit, char *buf, int cap, bs_val *root) {
    bs_pa_args args = {0, key, 0, limit};
    return bs_out_value(bs_pa_top_fn, &args, buf, cap, 0, 0, root);
}

#endif
