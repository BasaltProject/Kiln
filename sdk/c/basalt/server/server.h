/* Helpers over the server area: permissions, dynamic commands, broadcast, plugin files and storage, services and the
 * scheduler completions. Header only, no libc, no heap. Every function returns the import status (negative on error)
 * unless it says more; the ones that answer a value fill a bs_val over the buffer you pass. */
#ifndef BASALT_SERVER_SERVER_H
#define BASALT_SERVER_SERVER_H

#include "basalt/server/core.h"
#include "basalt/server/values.h"
#include "areas/server.h"

#define BS_SERVER_BUFFER 1024

#define BS_BROADCAST_PLAYERS 1
#define BS_BROADCAST_CONSOLE 2

#define BS_PROVIDE_CHECK 1
#define BS_PROVIDE_META 2

static inline int32_t bs_srv_len(const char *text) {
    int32_t n = 0;
    while (text[n] != 0) {
        n++;
    }
    return n;
}

typedef struct {
    int32_t which;
    const char *a;
    int32_t alen;
    const char *b;
    int32_t blen;
    int32_t number;
} bs_srv_call;

enum { BS_SRV_RESULT, BS_SRV_INFO, BS_SRV_CALL, BS_SRV_STORE, BS_SRV_DATA, BS_SRV_CONFIG, BS_SRV_LIST, BS_SRV_STAT,
       BS_SRV_WALK, BS_SRV_SERVICES, BS_SRV_QUERY, BS_SRV_COMMAND_SOURCE, BS_SRV_FILES_CAPABILITIES };

static inline int32_t bs_srv_dispatch(void *user, char *out, int32_t cap) {
    const bs_srv_call *c = (const bs_srv_call *)user;
    switch (c->which) {
    case BS_SRV_RESULT: return basalt_value_result(c->number, out, cap);
    case BS_SRV_INFO: return basalt_server_info(out, cap);
    case BS_SRV_CALL: return basalt_server_call(c->a, c->alen, c->b, c->blen, out, cap);
    case BS_SRV_STORE: return basalt_store_get(c->a, c->alen, out, cap);
    case BS_SRV_DATA: return basalt_data_read(c->a, c->alen, out, cap);
    case BS_SRV_CONFIG: return basalt_config_value(c->a, c->alen, out, cap);
    case BS_SRV_LIST: return basalt_files_list(c->a, c->alen, out, cap);
    case BS_SRV_STAT: return basalt_files_stat(c->a, c->alen, out, cap);
    case BS_SRV_WALK: return basalt_files_walk(c->a, c->alen, out, cap);
    case BS_SRV_SERVICES: return basalt_service_list(out, cap);
    case BS_SRV_COMMAND_SOURCE: return basalt_command_source(c->number, out, cap);
    case BS_SRV_FILES_CAPABILITIES: return basalt_files_capabilities(c->a, c->alen, out, cap);
    default: return BASALT_ERR_INVALID;
    }
}

/* Runs the import described by `c` into `buf`, retrying once with `spare`; `root` is the validated answer. */
static inline int32_t bs_srv_value(bs_srv_call *c, char *buf, int32_t cap, char *spare, int32_t spare_cap,
                                   bs_val *root) {
    return bs_out_value(bs_srv_dispatch, c, buf, cap, spare, spare_cap, root);
}

/* The value a completion callback (a wait, a lookup, a service call) carries. */
static inline int32_t bs_result(bs_ctx ctx, char *buf, int32_t cap, bs_val *root) {
    bs_srv_call c = {BS_SRV_RESULT, 0, 0, 0, 0, ctx};
    return bs_srv_value(&c, buf, cap, 0, 0, root);
}

/* ---- Permissions ---- */

static inline int32_t bs_permission_check(const char *subject, const char *node) {
    return basalt_permission_check(subject, bs_srv_len(subject), node, bs_srv_len(node));
}

static inline int32_t bs_permission_has(bs_player player, const char *node) {
    return basalt_permission_has(player, node, bs_srv_len(node));
}

/* mode: 1 grant, 0 deny, -1 drop the plugin's own entry. */
static inline int32_t bs_permission_set(bs_player player, const char *node, int32_t mode) {
    return basalt_permission_set(player, node, bs_srv_len(node), mode);
}

static inline int32_t bs_permission_meta(const char *subject, const char *key, char *out, int32_t cap) {
    return basalt_permission_meta(subject, bs_srv_len(subject), key, bs_srv_len(key), out, cap);
}

static inline int32_t bs_permission_provide(int32_t priority, int32_t kinds, bs_handler handler) {
    const int32_t token = bs_token_for(handler);
    return token < 0 ? BASALT_ERR_INVALID : basalt_permission_provide(priority, kinds, token);
}

/* The node (or meta key) a provider callback is asked about; the answer is length or a negative error. */
static inline int32_t bs_permission_query(bs_ctx ctx, const char *field, char *buf, int32_t cap, const char **text) {
    bs_val root;
    int32_t n = bs_result(ctx, buf, cap, &root);
    int32_t len = 0;
    if (n < 0) {
        return n;
    }
    return bs_value_as_str(bs_value_find(root, field), text, &len) ? len : BASALT_ERR_INVALID;
}

/* Stops a provider made by bs_permission_provide. */
static inline int32_t bs_permission_provider_remove(int32_t provider) {
    return basalt_permission_provider_remove(provider);
}

/* Says the answers of this plugin's providers changed, so command trees are sent again. */
static inline int32_t bs_permission_changed(void) { return basalt_permission_changed(); }

/* ---- Commands ---- */

static inline int32_t bs_command_ex(const char *spec, const char *permission, bs_handler handler,
                                    bs_handler complete) {
    const int32_t token = bs_token_for(handler);
    const int32_t complete_token = complete == 0 ? -1 : bs_token_for(complete);
    return token < 0 || (complete != 0 && complete_token < 0)
               ? BASALT_ERR_INVALID
               : basalt_command_register_ex(spec, bs_srv_len(spec), permission, bs_srv_len(permission), token,
                                            complete_token);
}

static inline int32_t bs_command_alias(const char *name, const char *alias) {
    return basalt_command_alias(name, bs_srv_len(name), alias, bs_srv_len(alias));
}

static inline int32_t bs_command_unregister(const char *name) {
    return basalt_command_unregister(name, bs_srv_len(name));
}

/* Runs a command line as the console (player 0) or as `player`. */
static inline int32_t bs_command_run(const char *line, bs_player player) {
    return basalt_command_run(line, bs_srv_len(line), player);
}

static inline int32_t bs_complete_input(bs_ctx ctx, char *out, int32_t cap) {
    return basalt_command_complete_input(ctx, out, cap);
}

static inline int32_t bs_complete_add(bs_ctx ctx, const char *candidate) {
    return basalt_command_complete_add(ctx, candidate, bs_srv_len(candidate));
}

static inline int32_t bs_command_arg_f64(bs_ctx ctx, const char *name, double *out) {
    return basalt_command_arg_f64(ctx, name, bs_srv_len(name), out);
}

static inline int32_t bs_command_arg_bool(bs_ctx ctx, const char *name, int32_t *out) {
    return basalt_command_arg_bool(ctx, name, bs_srv_len(name), out);
}

/* Who ran the command: a map {name, console, kind, world, x, y, z, caller}. */
static inline int32_t bs_command_source(bs_ctx ctx, char *buf, int32_t cap, bs_val *root) {
    bs_srv_call c = {BS_SRV_COMMAND_SOURCE, 0, 0, 0, 0, ctx};
    return bs_srv_value(&c, buf, cap, 0, 0, root);
}

/* The player handle of the sender, 0 for the console or a sender that is not an online player. */
static inline int32_t bs_command_source_player(bs_ctx ctx) { return basalt_command_source_player(ctx); }

/* ---- Broadcast, placeholders and server information ---- */

/* `text` may be a "@key" lang key; `args` may be NULL. */
static inline int32_t bs_broadcast(const char *text, const bs_args *args, int32_t flags) {
    return basalt_server_broadcast(text, bs_srv_len(text), args == 0 ? 0 : args->data, args == 0 ? 0 : args->len,
                                   flags);
}

static inline int32_t bs_placeholder_fill(const char *text, bs_player player, char *out, int32_t cap) {
    return basalt_placeholder_fill(text, bs_srv_len(text), player, out, cap);
}

static inline int32_t bs_server_info(char *buf, int32_t cap, char *spare, int32_t spare_cap, bs_val *root) {
    bs_srv_call c = {BS_SRV_INFO, 0, 0, 0, 0, 0};
    return bs_srv_value(&c, buf, cap, spare, spare_cap, root);
}

/* `args` is a value document (use bs_vw) of `args_len` bytes, or NULL for null. */
static inline int32_t bs_server_call(const char *op, const void *args, int32_t args_len, char *buf, int32_t cap,
                                     char *spare, int32_t spare_cap, bs_val *root) {
    bs_srv_call c = {BS_SRV_CALL, op, bs_srv_len(op), (const char *)args, args_len, 0};
    return bs_srv_value(&c, buf, cap, spare, spare_cap, root);
}

static inline int32_t bs_profile_lookup(const char *name, bs_handler handler) {
    const int32_t token = bs_once_token(handler);
    return token < 0 ? BASALT_ERR_INVALID
                     : bs_once_bind(token, BS_ONCE_REQUEST, basalt_profile_lookup(name, bs_srv_len(name), token));
}

/* ---- Plugin files, storage and config ---- */

static inline int32_t bs_file_read(const char *path, char *out, int32_t cap) {
    return basalt_files_read(path, bs_srv_len(path), out, cap);
}

static inline int32_t bs_file_write(const char *path, const char *data, int32_t len) {
    return basalt_files_write(path, bs_srv_len(path), data, len);
}

static inline int32_t bs_file_exists(const char *path) { return basalt_files_exists(path, bs_srv_len(path)); }

static inline int32_t bs_file_delete(const char *path, int32_t recursive) {
    return basalt_files_delete(path, bs_srv_len(path), recursive);
}

static inline int32_t bs_file_list(const char *prefix, char *buf, int32_t cap, bs_val *root) {
    bs_srv_call c = {BS_SRV_LIST, prefix, bs_srv_len(prefix), 0, 0, 0};
    return bs_srv_value(&c, buf, cap, 0, 0, root);
}

static inline int32_t bs_file_stat(const char *path, char *buf, int32_t cap, bs_val *root) {
    bs_srv_call c = {BS_SRV_STAT, path, bs_srv_len(path), 0, 0, 0};
    return bs_srv_value(&c, buf, cap, 0, 0, root);
}

static inline int32_t bs_file_walk(const char *path, char *buf, int32_t cap, bs_val *root) {
    bs_srv_call c = {BS_SRV_WALK, path, bs_srv_len(path), 0, 0, 0};
    return bs_srv_value(&c, buf, cap, 0, 0, root);
}

/* A path of the plugin folder: {atomic_move, hard_links} of the file system under it. */
static inline int32_t bs_file_capabilities(const char *path, char *buf, int32_t cap, bs_val *root) {
    bs_srv_call c = {BS_SRV_FILES_CAPABILITIES, path, bs_srv_len(path), 0, 0, 0};
    return bs_srv_value(&c, buf, cap, 0, 0, root);
}

static inline int32_t bs_store_set_i64(const char *key, int64_t value) {
    unsigned char doc[16];
    bs_vw w;
    bs_vw_init(&w, doc, 16);
    bs_vw_i64(&w, value);
    const int32_t n = bs_vw_finish(&w);
    return n < 0 ? n : basalt_store_set(key, bs_srv_len(key), (const char *)doc, n);
}

static inline int32_t bs_store_set_f64(const char *key, double value) {
    unsigned char doc[16];
    bs_vw w;
    bs_vw_init(&w, doc, 16);
    bs_vw_f64(&w, value);
    const int32_t n = bs_vw_finish(&w);
    return n < 0 ? n : basalt_store_set(key, bs_srv_len(key), (const char *)doc, n);
}

static inline int32_t bs_store_set_bool(const char *key, int32_t value) {
    unsigned char doc[16];
    bs_vw w;
    bs_vw_init(&w, doc, 16);
    bs_vw_bool(&w, value);
    const int32_t n = bs_vw_finish(&w);
    return n < 0 ? n : basalt_store_set(key, bs_srv_len(key), (const char *)doc, n);
}

/* Deletes the key. */
static inline int32_t bs_store_set_null(const char *key) {
    const char doc[1] = {(char)BASALT_VALUE_NULL};
    return basalt_store_set(key, bs_srv_len(key), doc, 1);
}

static inline int32_t bs_store_set_str(const char *key, const char *value) {
    unsigned char doc[256];
    bs_vw w;
    bs_vw_init(&w, doc, 256);
    bs_vw_str(&w, value);
    const int32_t n = bs_vw_finish(&w);
    return n < 0 ? n : basalt_store_set(key, bs_srv_len(key), (const char *)doc, n);
}

static inline int32_t bs_store_get(const char *key, char *buf, int32_t cap, bs_val *root) {
    bs_srv_call c = {BS_SRV_STORE, key, bs_srv_len(key), 0, 0, 0};
    return bs_srv_value(&c, buf, cap, 0, 0, root);
}

static inline int32_t bs_data_write(const char *name, const void *doc, int32_t len) {
    return basalt_data_write(name, bs_srv_len(name), (const char *)doc, len);
}

static inline int32_t bs_data_read(const char *name, char *buf, int32_t cap, bs_val *root) {
    bs_srv_call c = {BS_SRV_DATA, name, bs_srv_len(name), 0, 0, 0};
    return bs_srv_value(&c, buf, cap, 0, 0, root);
}

/* An empty key answers the whole config table. */
static inline int32_t bs_config_value(const char *key, char *buf, int32_t cap, bs_val *root) {
    bs_srv_call c = {BS_SRV_CONFIG, key, bs_srv_len(key), 0, 0, 0};
    return bs_srv_value(&c, buf, cap, 0, 0, root);
}

/* ---- Services between plugins ---- */

static inline int32_t bs_service_register(const char *name, bs_handler handler) {
    const int32_t token = bs_token_for(handler);
    return token < 0 ? BASALT_ERR_INVALID : basalt_service_register(name, bs_srv_len(name), token);
}

/* Answers the call a service handler runs for; `doc` is a value document, NULL for null. */
static inline int32_t bs_service_reply(bs_ctx ctx, const void *doc, int32_t len) {
    return basalt_service_reply(ctx, (const char *)doc, len);
}

/* Unregisters a service made by bs_service_register. */
static inline int32_t bs_service_remove(int32_t service) { return basalt_service_remove(service); }

/* Every service offered by loaded plugins, a sorted list of names. */
static inline int32_t bs_service_list(char *buf, int32_t cap, bs_val *root) {
    bs_srv_call c = {BS_SRV_SERVICES, 0, 0, 0, 0, 0};
    return bs_srv_value(&c, buf, cap, 0, 0, root);
}

static inline int32_t bs_service_has(const char *name) { return basalt_service_has(name, bs_srv_len(name)); }

static inline int32_t bs_service_call(const char *name, const void *args, int32_t args_len, int32_t timeout,
                                      bs_handler handler) {
    const int32_t token = bs_once_token(handler);
    return token < 0 ? BASALT_ERR_INVALID
                     : bs_once_bind(token, BS_ONCE_REQUEST,
                                    basalt_service_call(name, bs_srv_len(name), (const char *)args, args_len, timeout,
                                                        token));
}

/* ---- Scheduler completions ---- */

static inline int32_t bs_wait(int32_t ticks, bs_handler handler) {
    const int32_t token = bs_once_token(handler);
    return token < 0 ? BASALT_ERR_INVALID : bs_once_bind(token, BS_ONCE_REQUEST, basalt_task_wait(ticks, token));
}

static inline int32_t bs_at(const char *world, double x, double z, bs_handler handler) {
    const int32_t token = bs_once_token(handler);
    return token < 0 ? BASALT_ERR_INVALID
                     : bs_once_bind(token, BS_ONCE_REQUEST, basalt_task_at(world, bs_srv_len(world), x, z, token));
}

#endif /* BASALT_SERVER_SERVER_H */
