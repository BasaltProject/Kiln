#include "runtime.h"

#include "basalt/internal/commands.h"
#include "basalt/internal/config.h"
#include "basalt/internal/data.h"
#include "basalt/internal/events.h"
#include "basalt/internal/log.h"
#include "basalt/internal/players.h"
#include "basalt/internal/tasks.h"
#include "basalt/internal/text.h"
#include "areas/core_values.h"

#define kMaxHandlers 1024

typedef unsigned long size_t_;

__attribute__((no_builtin("memcpy"))) void *memcpy(void *dst, const void *src, size_t_ n) {
    unsigned char *d = dst;
    const unsigned char *s = src;
    while (n--) {
        *d++ = *s++;
    }
    return dst;
}

__attribute__((no_builtin("memmove"))) void *memmove(void *dst, const void *src, size_t_ n) {
    unsigned char *d = dst;
    const unsigned char *s = src;
    if (d < s) {
        while (n--) {
            *d++ = *s++;
        }
    } else {
        d += n;
        s += n;
        while (n--) {
            *--d = *--s;
        }
    }
    return dst;
}

__attribute__((no_builtin("memset"))) void *memset(void *dst, int value, size_t_ n) {
    unsigned char *d = dst;
    while (n--) {
        *d++ = (unsigned char)value;
    }
    return dst;
}

__attribute__((no_builtin("memcmp"))) int memcmp(const void *a, const void *b, size_t_ n) {
    const unsigned char *x = a;
    const unsigned char *y = b;
    for (; n; --n, ++x, ++y) {
        if (*x != *y) {
            return *x - *y;
        }
    }
    return 0;
}

#define kSlotShift 10
#define kOncePending 3

static bs_handler handlers[kMaxHandlers];
static unsigned char once_kind[kMaxHandlers];
static int32_t once_handle[kMaxHandlers];
static unsigned short generation[kMaxHandlers];
static int free_slots[kMaxHandlers];
static int free_count = 0;
static int handler_count = 0;

static int take_slot(bs_handler handler, int kind) {
    int slot;
    if (free_count > 0) {
        slot = free_slots[--free_count];
    } else if (handler_count < kMaxHandlers) {
        slot = handler_count++;
    } else {
        return -1;
    }
    handlers[slot] = handler;
    once_kind[slot] = (unsigned char)kind;
    once_handle[slot] = 0;
    return (((int)generation[slot] << kSlotShift) | slot) + 1;
}

static void free_slot(int slot) {
    handlers[slot] = 0;
    once_kind[slot] = 0;
    generation[slot]++;
    free_slots[free_count++] = slot;
}

static int find_slot(int token) {
    if (token <= 0) {
        return -1;
    }
    const int slot = (token - 1) & (kMaxHandlers - 1);
    return handlers[slot] != 0 && ((token - 1) >> kSlotShift) == generation[slot] ? slot : -1;
}

static int token_for(bs_handler handler) { return handler == 0 ? 0 : take_slot(handler, 0); }

static void release_once(int kind, int handle) {
    for (int slot = 0; slot < handler_count; ++slot) {
        if (handlers[slot] != 0 && once_kind[slot] == kind && once_handle[slot] == handle) {
            free_slot(slot);
            return;
        }
    }
}

BASALT_EXPORT(basalt_abi_version) int32_t basalt_abi_version(void) { return BASALT_ABI_VERSION; }

BASALT_EXPORT(basalt_init) void basalt_init(void) { plugin_init(); }

BASALT_EXPORT(basalt_callback) void basalt_callback(int32_t token, int32_t ctx) {
    const int slot = find_slot(token);
    if (slot < 0) {
        return;
    }
    const bs_handler handler = handlers[slot];
    if (once_kind[slot] != 0) {
        free_slot(slot);
    }
    handler(ctx);
}

static void put_u32(char *at, int32_t value) {
    at[0] = (char)value;
    at[1] = (char)(value >> 8);
    at[2] = (char)(value >> 16);
    at[3] = (char)(value >> 24);
}

void bs_args_init(bs_args *args) {
    args->len = 4;
    put_u32(args->data, 0);
}

int bs_args_add(bs_args *args, const char *key, const char *value) {
    const int klen = bs_length(key);
    const int vlen = bs_length(value);
    if (args->len + 8 + klen + vlen > (int)sizeof(args->data)) {
        return BASALT_ERR_INVALID;
    }
    char *at = args->data + args->len;
    put_u32(at, klen);
    memcpy(at + 4, key, (size_t_)klen);
    put_u32(at + 4 + klen, vlen);
    memcpy(at + 8 + klen, value, (size_t_)vlen);
    args->len += 8 + klen + vlen;
    put_u32(args->data, (unsigned char)args->data[0] + ((unsigned char)args->data[1] << 8) + 1);
    return BASALT_OK;
}

int bs_format_i64(int64_t value, char *out) {
    char digits[24];
    int n = 0;
    uint64_t magnitude = value < 0 ? 0 - (uint64_t)value : (uint64_t)value;
    do {
        digits[n++] = (char)('0' + magnitude % 10);
        magnitude /= 10;
    } while (magnitude != 0);
    int len = 0;
    if (value < 0) {
        out[len++] = '-';
    }
    while (n > 0) {
        out[len++] = digits[--n];
    }
    out[len] = 0;
    return len;
}

void bs_log(int level, const char *msg) { basalt_log(level, msg, bs_length(msg)); }
void bs_debug(const char *msg) { bs_log(BASALT_LOG_DEBUG, msg); }
void bs_info(const char *msg) { bs_log(BASALT_LOG_INFO, msg); }
void bs_warn(const char *msg) { bs_log(BASALT_LOG_WARN, msg); }
void bs_error(const char *msg) { bs_log(BASALT_LOG_ERROR, msg); }

int64_t bs_config_i64(const char *key, int64_t def) { return basalt_config_i64(key, bs_length(key), def); }
double bs_config_f64(const char *key, double def) { return basalt_config_f64(key, bs_length(key), def); }
int bs_config_bool(const char *key, int def) { return basalt_config_bool(key, bs_length(key), def); }
int bs_config_str(const char *key, char *out, int cap) { return basalt_config_str(key, bs_length(key), out, cap); }

int bs_translate(const char *key, const bs_args *args, char *out, int cap) {
    return basalt_translate(key, bs_length(key), ARGS_PTR(args), ARGS_LEN(args), out, cap);
}

bs_player bs_player_by_name(const char *name) { return basalt_player_by_name(name, bs_length(name)); }
int bs_players_online(bs_player *out, int cap) { return basalt_players_online(out, cap); }
int bs_player_name(bs_player player, char *out, int cap) { return basalt_player_name(player, out, cap); }

int bs_send(bs_player player, const char *text, const bs_args *args) {
    return basalt_player_send(player, text, bs_length(text), ARGS_PTR(args), ARGS_LEN(args));
}

int bs_action_bar(bs_player player, const char *text, const bs_args *args) {
    return basalt_player_action_bar(player, text, bs_length(text), ARGS_PTR(args), ARGS_LEN(args));
}

int64_t bs_data_get_i64(bs_player player, const char *key, int64_t def) {
    return basalt_data_get_i64(player, key, bs_length(key), def);
}

int bs_data_set_i64(bs_player player, const char *key, int64_t value) {
    return basalt_data_set_i64(player, key, bs_length(key), value);
}

int bs_data_get_bool(bs_player player, const char *key, int def) {
    return basalt_data_get_bool(player, key, bs_length(key), def);
}

int bs_data_set_bool(bs_player player, const char *key, int value) {
    return basalt_data_set_bool(player, key, bs_length(key), value);
}

int bs_data_get_str(bs_player player, const char *key, char *out, int cap) {
    return basalt_data_get_str(player, key, bs_length(key), out, cap);
}

int bs_data_set_str(bs_player player, const char *key, const char *value) {
    return basalt_data_set_str(player, key, bs_length(key), value, bs_length(value));
}

int bs_data_remove(bs_player player, const char *key) { return basalt_data_remove(player, key, bs_length(key)); }

int bs_token_for(bs_handler handler) { return token_for(handler); }

int bs_once_token(bs_handler handler) { return handler == 0 ? 0 : take_slot(handler, kOncePending); }

int bs_once_bind(int token, int kind, int handle) {
    const int slot = find_slot(token);
    if (slot < 0 || once_kind[slot] == 0) {
        return handle;
    }
    if (handle < 0) {
        free_slot(slot);
    } else {
        once_kind[slot] = (unsigned char)kind;
        once_handle[slot] = handle;
    }
    return handle;
}

int bs_request_cancel(int request) {
    const int result = basalt_request_cancel(request);
    if (result == BASALT_OK) {
        release_once(BS_ONCE_REQUEST, request);
    }
    return result;
}

int bs_on(const char *event, int priority, bs_handler handler) {
    const int token = token_for(handler);
    return token < 0 ? BASALT_ERR_INVALID : basalt_event_on(event, bs_length(event), priority, token);
}

bs_player bs_event_player(bs_ctx ctx) { return basalt_event_player(ctx); }

int bs_event_field_str(bs_ctx ctx, const char *field, char *out, int cap) {
    return basalt_event_field_str(ctx, field, bs_length(field), out, cap);
}

int bs_event_field_i64(bs_ctx ctx, const char *field, int64_t *out) {
    return basalt_event_field_i64(ctx, field, bs_length(field), out);
}

int bs_event_cancel(bs_ctx ctx, int cancelled) { return basalt_event_cancel(ctx, cancelled); }

int bs_command(const char *spec, const char *permission, bs_handler handler) {
    const int token = token_for(handler);
    return token < 0 ? BASALT_ERR_INVALID
                     : basalt_command_register(spec, bs_length(spec), permission, bs_length(permission), token);
}

int bs_command_reply(bs_ctx ctx, const char *text, const bs_args *args) {
    return basalt_command_reply(ctx, text, bs_length(text), ARGS_PTR(args), ARGS_LEN(args));
}

int bs_command_arg_str(bs_ctx ctx, const char *name, char *out, int cap) {
    return basalt_command_arg_str(ctx, name, bs_length(name), out, cap);
}

int bs_command_arg_i64(bs_ctx ctx, const char *name, int64_t *out) {
    return basalt_command_arg_i64(ctx, name, bs_length(name), out);
}

int bs_command_source_name(bs_ctx ctx, char *out, int cap) { return basalt_command_source_name(ctx, out, cap); }
int bs_command_is_console(bs_ctx ctx) { return basalt_command_is_console(ctx); }

int bs_after(int delay, bs_handler handler) {
    const int token = bs_once_token(handler);
    return token < 0 ? BASALT_ERR_INVALID : bs_once_bind(token, BS_ONCE_TASK, basalt_task_after(delay, token));
}

int bs_every(int delay, int period, bs_handler handler) {
    const int token = token_for(handler);
    return token < 0 ? BASALT_ERR_INVALID : basalt_task_every(delay, period, token);
}

int bs_task_cancel(int task) {
    const int result = basalt_task_cancel(task);
    if (result == BASALT_OK) {
        release_once(BS_ONCE_TASK, task);
    }
    return result;
}
