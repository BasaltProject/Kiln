/* Events: listeners, fields, cancellation and emits; documents come back as bs_val over the buffer you pass. */
#ifndef BASALT_SERVER_EVENTS_H
#define BASALT_SERVER_EVENTS_H

#include "basalt/internal/events.h"
#include "basalt/server/values.h"
#include "areas/events.h"

static inline int32_t bs_ev_len(const char *text) {
    int32_t n = 0;
    while (text != 0 && text[n]) {
        ++n;
    }
    return n;
}

static inline const char *bs_ev_args(const bs_args *args) { return args != 0 && args->len > 4 ? args->data : 0; }
static inline int32_t bs_ev_args_len(const bs_args *args) { return args != 0 && args->len > 4 ? args->len : 0; }

/* Finishes a text answer: the length it needs, or a negative error; NUL-terminates when it fits. */
static inline int32_t bs_ev_text(int32_t n, char *out, int32_t cap) {
    if (n >= 0 && n < cap) {
        out[n] = 0;
    }
    return n;
}

typedef struct {
    int32_t a;
    int32_t b;
    const char *name;
    int32_t name_len;
    int32_t kind;
} bs_ev_call;

static inline int32_t bs_ev_event_get_fn(void *user, char *out, int32_t cap) {
    const bs_ev_call *c = (const bs_ev_call *)user;
    return basalt_event_get(c->a, c->name, c->name_len, out, cap);
}
static inline int32_t bs_ev_event_fields_fn(void *user, char *out, int32_t cap) {
    return basalt_event_fields(((const bs_ev_call *)user)->a, out, cap);
}
static inline int32_t bs_ev_event_payload_fn(void *user, char *out, int32_t cap) {
    return basalt_event_payload(((const bs_ev_call *)user)->a, out, cap);
}
static inline int32_t bs_ev_event_describe_fn(void *user, char *out, int32_t cap) {
    const bs_ev_call *c = (const bs_ev_call *)user;
    return basalt_event_describe(c->name, c->name_len, out, cap);
}
static inline int32_t bs_ev_event_names_fn(void *user, char *out, int32_t cap) {
    (void)user;
    return basalt_event_names(out, cap);
}

/* Listens to `name`; `filters` is a finished document (a map) or 0. */
static inline int bs_event_on_ex(const char *name, int priority, int flags, const void *filters, int filters_len,
                                 bs_handler handler) {
    const int token = bs_token_for(handler);
    return token < 0 ? BASALT_ERR_INVALID
                     : basalt_event_on_ex(name, bs_ev_len(name), priority, flags, (const char *)filters, filters_len,
                                          token);
}

static inline int bs_event_name(bs_ctx ctx, char *out, int cap) {
    return bs_ev_text(basalt_event_name(ctx, out, cap), out, cap);
}

static inline int bs_event_get(bs_ctx ctx, const char *field, char *buf, int cap, bs_val *root) {
    bs_ev_call c = {ctx, 0, field, bs_ev_len(field), 0};
    return bs_out_value(bs_ev_event_get_fn, &c, buf, cap, 0, 0, root);
}

static inline int bs_event_set(bs_ctx ctx, const char *field, const void *value, int len) {
    return basalt_event_set(ctx, field, bs_ev_len(field), (const char *)value, len);
}

static inline int bs_event_fields(bs_ctx ctx, char *buf, int cap, bs_val *root) {
    bs_ev_call c = {ctx, 0, 0, 0, 0};
    return bs_out_value(bs_ev_event_fields_fn, &c, buf, cap, 0, 0, root);
}

static inline int bs_event_payload(bs_ctx ctx, char *buf, int cap, bs_val *root) {
    bs_ev_call c = {ctx, 0, 0, 0, 0};
    return bs_out_value(bs_ev_event_payload_fn, &c, buf, cap, 0, 0, root);
}

static inline int bs_event_describe(const char *name, char *buf, int cap, bs_val *root) {
    bs_ev_call c = {0, 0, name, bs_ev_len(name), 0};
    return bs_out_value(bs_ev_event_describe_fn, &c, buf, cap, 0, 0, root);
}

static inline int bs_event_names(char *buf, int cap, bs_val *root) {
    return bs_out_value(bs_ev_event_names_fn, 0, buf, cap, 0, 0, root);
}

static inline int bs_event_cancelled(bs_ctx ctx) { return basalt_event_cancelled(ctx); }

static inline int bs_event_cancel_with(bs_ctx ctx, const char *text, const bs_args *args) {
    return basalt_event_cancel_with(ctx, text, bs_ev_len(text), bs_ev_args(args), bs_ev_args_len(args));
}

static inline int bs_event_cancel_message(bs_ctx ctx, char *out, int cap) {
    return bs_ev_text(basalt_event_cancel_message(ctx, out, cap), out, cap);
}

static inline int bs_event_is_post(bs_ctx ctx) { return basalt_event_is_post(ctx); }

static inline int bs_event_emit(const char *name, const void *value, int len) {
    return basalt_event_emit(name, bs_ev_len(name), (const char *)value, len);
}

#endif
