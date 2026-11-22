/* Proxy events: fields as encoded values, decisions (deny, set) and watchers; read values with bs_value_* of values.h. */
#ifndef BASALT_PROXY_EVENTS_H
#define BASALT_PROXY_EVENTS_H

#include "basalt/internal/events.h"
#include "basalt/proxy/values.h"
#include "basalt_proxy_abi.h"
#include "areas/proxy_events.h"

#ifdef __cplusplus
extern "C" {
#endif

int bs_proxy_event_set_str(bs_ctx ctx, const char *field, const char *value);
int bs_proxy_event_set_i64(bs_ctx ctx, const char *field, int64_t value);

#ifdef __cplusplus
}
#endif

enum { BS_PE_TAG_FALSE = 0x01, BS_PE_TAG_TRUE = 0x02, BS_PE_TAG_INT = 0x03, BS_PE_TAG_STRING = 0x05 };

static inline int bs_pe_length(const char *text) {
    int n = 0;
    while (text != 0 && text[n]) {
        ++n;
    }
    return n;
}

static inline int bs_pe_leb(uint8_t *out, uint64_t value) {
    int n = 0;
    do {
        uint8_t byte = (uint8_t)(value & 0x7f);
        value >>= 7;
        out[n++] = (uint8_t)(byte | (value ? 0x80 : 0));
    } while (value);
    return n;
}

static inline int bs_pe_read_leb(const uint8_t *in, int len, int *pos, uint64_t *out) {
    uint64_t result = 0;
    for (int shift = 0; shift < 70 && *pos < len; shift += 7) {
        const uint8_t byte = in[(*pos)++];
        result |= (uint64_t)(byte & 0x7f) << shift;
        if (!(byte & 0x80)) {
            *out = result;
            return 1;
        }
    }
    return 0;
}

static inline int bs_proxy_event_get_value(bs_ctx ctx, const char *field, char *out, int cap) {
    return basalt_proxy_event_get(ctx, field, bs_pe_length(field), out, cap);
}

static inline int bs_proxy_event_get_str(bs_ctx ctx, const char *field, char *out, int cap) {
    uint8_t raw[4160];
    const int n = bs_proxy_event_get_value(ctx, field, (char *)raw, (int)sizeof raw);
    if (n < 0) {
        return n;
    }
    int pos = 1;
    uint64_t len = 0;
    if (n > (int)sizeof raw || raw[0] != BS_PE_TAG_STRING || !bs_pe_read_leb(raw, n, &pos, &len) ||
        pos + (int64_t)len > n) {
        return BASALT_ERR_INVALID;
    }
    if (len <= (uint64_t)cap) {
        for (int i = 0; i < (int)len; ++i) {
            out[i] = (char)raw[pos + i];
        }
    }
    return (int)len;
}

static inline int bs_proxy_event_get_i64(bs_ctx ctx, const char *field, int64_t *out) {
    uint8_t raw[16];
    const int n = bs_proxy_event_get_value(ctx, field, (char *)raw, (int)sizeof raw);
    int pos = 1;
    uint64_t zigzag = 0;
    if (n < 0) {
        return n;
    }
    if (n > (int)sizeof raw || raw[0] != BS_PE_TAG_INT || !bs_pe_read_leb(raw, n, &pos, &zigzag)) {
        return BASALT_ERR_INVALID;
    }
    *out = (int64_t)(zigzag >> 1) ^ -(int64_t)(zigzag & 1);
    return BASALT_OK;
}

static inline int bs_proxy_event_get_bool(bs_ctx ctx, const char *field, int *out) {
    uint8_t raw[4];
    const int n = bs_proxy_event_get_value(ctx, field, (char *)raw, (int)sizeof raw);
    if (n < 0) {
        return n;
    }
    if (n != 1 || (raw[0] != BS_PE_TAG_FALSE && raw[0] != BS_PE_TAG_TRUE)) {
        return BASALT_ERR_INVALID;
    }
    *out = raw[0] == BS_PE_TAG_TRUE;
    return BASALT_OK;
}

static inline int bs_proxy_event_set_value(bs_ctx ctx, const char *field, const char *value, int len) {
    return basalt_proxy_event_set(ctx, field, bs_pe_length(field), value, len);
}

static inline int bs_proxy_event_set_text(bs_ctx ctx, const char *field, const char *text) {
    uint8_t raw[4120];
    const int len = bs_pe_length(text);
    if (len > 4096) {
        return BASALT_ERR_INVALID;
    }
    raw[0] = BS_PE_TAG_STRING;
    const int head = 1 + bs_pe_leb(raw + 1, (uint64_t)len);
    for (int i = 0; i < len; ++i) {
        raw[head + i] = (uint8_t)text[i];
    }
    return bs_proxy_event_set_value(ctx, field, (const char *)raw, head + len);
}

static inline int bs_proxy_event_set_int(bs_ctx ctx, const char *field, int64_t value) {
    uint8_t raw[12];
    raw[0] = BS_PE_TAG_INT;
    const int n = 1 + bs_pe_leb(raw + 1, ((uint64_t)value << 1) ^ (uint64_t)(value >> 63));
    return bs_proxy_event_set_value(ctx, field, (const char *)raw, n);
}

static inline int bs_proxy_event_set_flag(bs_ctx ctx, const char *field, int flag) {
    const char raw = flag ? (char)BS_PE_TAG_TRUE : (char)BS_PE_TAG_FALSE;
    return bs_proxy_event_set_value(ctx, field, &raw, 1);
}

/* Cancels a decided event (chat, command, login, server_pre_connect) and shows `reason` to the player. */
static inline int bs_proxy_event_deny(bs_ctx ctx, const char *reason) {
    const int shown = reason != 0 ? bs_proxy_event_set_text(ctx, "reason", reason) : BASALT_OK;
    return shown < 0 ? shown : bs_proxy_event_set_flag(ctx, "cancelled", 1);
}

static inline int bs_proxy_event_fields(bs_ctx ctx, char *out, int cap) {
    return basalt_proxy_event_fields(ctx, out, cap);
}

static inline int bs_proxy_event_watch(const char *event, bs_handler handler) {
    const int token = bs_token_for(handler);
    return token < 0 ? BASALT_ERR_INVALID : basalt_proxy_event_watch(event, bs_pe_length(event), token);
}

#endif
