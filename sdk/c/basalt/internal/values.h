/* Value builder and reader over the byte format of basalt_values.h. Header only, no libc, no heap: the writer fills a
 * buffer you provide and sets an error flag instead of overflowing. */
#ifndef BASALT_INTERNAL_VALUES_H
#define BASALT_INTERNAL_VALUES_H

#include "basalt_abi.h"
#include "areas/core_values.h"

#define BS_VALUE_NULL BASALT_VALUE_NULL
#define BS_VALUE_BOOL 0x10
#define BS_VALUE_INT BASALT_VALUE_INT
#define BS_VALUE_FLOAT BASALT_VALUE_FLOAT
#define BS_VALUE_STRING BASALT_VALUE_STRING
#define BS_VALUE_BYTES BASALT_VALUE_BYTES
#define BS_VALUE_LIST BASALT_VALUE_LIST
#define BS_VALUE_MAP BASALT_VALUE_MAP

#define BS_VW_OK 0
#define BS_VW_OVERFLOW 1
#define BS_VW_INVALID 2

typedef struct {
    uint8_t *data;
    int32_t cap;
    int32_t len;
    int32_t nodes;
    int32_t depth;
    int32_t error;
    int32_t start[BASALT_VALUE_MAX_DEPTH + 1];
    int32_t count[BASALT_VALUE_MAX_DEPTH + 1];
    uint8_t is_map[BASALT_VALUE_MAX_DEPTH + 1];
    uint8_t pending_key[BASALT_VALUE_MAX_DEPTH + 1];
} bs_vw;

static inline void bs_vw_init(bs_vw *w, void *buffer, int32_t cap) {
    w->data = (uint8_t *)buffer;
    w->cap = cap < BASALT_VALUE_MAX_BYTES ? cap : BASALT_VALUE_MAX_BYTES;
    w->len = 0;
    w->nodes = 0;
    w->depth = 0;
    w->error = BS_VW_OK;
}

static inline int bs_vw_ok(const bs_vw *w) { return w->error == BS_VW_OK; }

static inline void bs_vw_fail(bs_vw *w, int32_t error) {
    if (w->error == BS_VW_OK) {
        w->error = error;
    }
}

static inline void bs_vw_byte(bs_vw *w, uint8_t byte) {
    if (w->len >= w->cap) {
        bs_vw_fail(w, BS_VW_OVERFLOW);
        return;
    }
    w->data[w->len++] = byte;
}

static inline int32_t bs_vw_leb_size(uint64_t value) {
    int32_t n = 1;
    while (value >= 0x80u) {
        value >>= 7;
        ++n;
    }
    return n;
}

static inline void bs_vw_leb_at(uint8_t *out, uint64_t value) {
    while (value >= 0x80u) {
        *out++ = (uint8_t)((value & 0x7Fu) | 0x80u);
        value >>= 7;
    }
    *out = (uint8_t)value;
}

static inline void bs_vw_leb(bs_vw *w, uint64_t value) {
    while (value >= 0x80u) {
        bs_vw_byte(w, (uint8_t)((value & 0x7Fu) | 0x80u));
        value >>= 7;
    }
    bs_vw_byte(w, (uint8_t)value);
}

static inline void bs_vw_raw(bs_vw *w, const void *bytes, int32_t n) {
    if (n > w->cap - w->len) {
        bs_vw_fail(w, BS_VW_OVERFLOW);
        return;
    }
    const uint8_t *src = (const uint8_t *)bytes;
    for (int32_t i = 0; i < n; ++i) {
        w->data[w->len + i] = src[i];
    }
    w->len += n;
}

/* Starts a value: checks position, depth and node count, and counts it in the open container. */
static inline int bs_vw_begin(bs_vw *w) {
    if (w->error != BS_VW_OK) {
        return 0;
    }
    if (w->depth == 0) {
        if (w->nodes > 0) {
            bs_vw_fail(w, BS_VW_INVALID);
            return 0;
        }
    } else {
        const int32_t top = w->depth - 1;
        if (w->is_map[top]) {
            if (!w->pending_key[top]) {
                bs_vw_fail(w, BS_VW_INVALID);
                return 0;
            }
            w->pending_key[top] = 0;
        } else {
            w->count[top]++;
        }
    }
    if (w->depth > BASALT_VALUE_MAX_DEPTH || ++w->nodes > BASALT_VALUE_MAX_NODES) {
        bs_vw_fail(w, BS_VW_INVALID);
        return 0;
    }
    return 1;
}

static inline void bs_vw_null(bs_vw *w) {
    if (bs_vw_begin(w)) {
        bs_vw_byte(w, BASALT_VALUE_NULL);
    }
}

static inline void bs_vw_bool(bs_vw *w, int value) {
    if (bs_vw_begin(w)) {
        bs_vw_byte(w, value ? BASALT_VALUE_TRUE : BASALT_VALUE_FALSE);
    }
}

static inline void bs_vw_i64(bs_vw *w, int64_t value) {
    if (bs_vw_begin(w)) {
        bs_vw_byte(w, BASALT_VALUE_INT);
        bs_vw_leb(w, ((uint64_t)value << 1) ^ (uint64_t)(value >> 63));
    }
}

static inline void bs_vw_f64(bs_vw *w, double value) {
    if (bs_vw_begin(w)) {
        bs_vw_byte(w, BASALT_VALUE_FLOAT);
        union {
            double d;
            uint64_t u;
        } bits;
        bits.d = value;
        for (int shift = 0; shift < 64; shift += 8) {
            bs_vw_byte(w, (uint8_t)(bits.u >> shift));
        }
    }
}

static inline void bs_vw_blob(bs_vw *w, uint8_t tag, const void *bytes, int32_t n) {
    if (n < 0) {
        bs_vw_fail(w, BS_VW_INVALID);
    } else if (bs_vw_begin(w)) {
        bs_vw_byte(w, tag);
        bs_vw_leb(w, (uint64_t)n);
        bs_vw_raw(w, bytes, n);
    }
}

static inline void bs_vw_bytes(bs_vw *w, const void *bytes, int32_t n) { bs_vw_blob(w, BASALT_VALUE_BYTES, bytes, n); }

static inline void bs_vw_str_n(bs_vw *w, const char *text, int32_t n) { bs_vw_blob(w, BASALT_VALUE_STRING, text, n); }

static inline void bs_vw_str(bs_vw *w, const char *text) {
    int32_t n = 0;
    while (text != 0 && text[n]) {
        ++n;
    }
    bs_vw_str_n(w, text, n);
}

static inline void bs_vw_open(bs_vw *w, int is_map) {
    if (!bs_vw_begin(w)) {
        return;
    }
    bs_vw_byte(w, is_map ? BASALT_VALUE_MAP : BASALT_VALUE_LIST);
    w->start[w->depth] = w->len;
    w->count[w->depth] = 0;
    w->is_map[w->depth] = (uint8_t)is_map;
    w->pending_key[w->depth] = 0;
    w->depth++;
}

static inline void bs_vw_list(bs_vw *w) { bs_vw_open(w, 0); }

static inline void bs_vw_map(bs_vw *w) { bs_vw_open(w, 1); }

/* Closes the innermost list or map: inserts its count in front of the items. */
static inline void bs_vw_end(bs_vw *w) {
    if (w->error != BS_VW_OK) {
        return;
    }
    if (w->depth == 0 || (w->is_map[w->depth - 1] && w->pending_key[w->depth - 1])) {
        bs_vw_fail(w, BS_VW_INVALID);
        return;
    }
    const int32_t top = w->depth - 1;
    const int32_t size = bs_vw_leb_size((uint64_t)w->count[top]);
    if (size > w->cap - w->len) {
        bs_vw_fail(w, BS_VW_OVERFLOW);
        return;
    }
    for (int32_t i = w->len - 1; i >= w->start[top]; --i) {
        w->data[i + size] = w->data[i];
    }
    bs_vw_leb_at(w->data + w->start[top], (uint64_t)w->count[top]);
    w->len += size;
    w->depth = top;
}

/* Writes the next key of the open map; the value follows. */
static inline void bs_vw_key_n(bs_vw *w, const char *key, int32_t n) {
    if (w->error != BS_VW_OK) {
        return;
    }
    if (w->depth == 0 || !w->is_map[w->depth - 1] || w->pending_key[w->depth - 1] || n < 0 ||
        n > BASALT_VALUE_MAX_KEY_BYTES) {
        bs_vw_fail(w, BS_VW_INVALID);
        return;
    }
    bs_vw_leb(w, (uint64_t)n);
    bs_vw_raw(w, key, n);
    w->count[w->depth - 1]++;
    w->pending_key[w->depth - 1] = 1;
}

static inline void bs_vw_key(bs_vw *w, const char *key) {
    int32_t n = 0;
    while (key != 0 && key[n]) {
        ++n;
    }
    bs_vw_key_n(w, key, n);
}

/* The finished document length, or BASALT_ERR_INVALID when the writer failed or a container is still open. */
static inline int32_t bs_vw_finish(const bs_vw *w) {
    return w->error == BS_VW_OK && w->depth == 0 && w->nodes > 0 ? w->len : BASALT_ERR_INVALID;
}

/* A position inside a validated document. `base == 0` is the absent value. */
typedef struct {
    const uint8_t *base;
    int32_t off;
    int32_t end;
} bs_val;

typedef struct {
    const uint8_t *base;
    int32_t pos;
    int32_t end;
    int32_t left;
    int32_t is_map;
} bs_iter;

static inline int bs_value_present(bs_val v) { return v.base != 0; }

static inline int bs_value_leb(const uint8_t *b, int32_t end, int32_t *pos, uint64_t *out) {
    uint64_t result = 0;
    for (int shift = 0; shift < 70; shift += 7) {
        if (*pos >= end) {
            return 0;
        }
        const uint8_t byte = b[(*pos)++];
        if (shift == 63 && byte > 1u) {
            return 0;
        }
        result |= (uint64_t)(byte & 0x7Fu) << shift;
        if ((byte & 0x80u) == 0) {
            *out = result;
            return 1;
        }
    }
    return 0;
}

static inline int bs_value_same(const uint8_t *a, const uint8_t *b, int32_t n) {
    for (int32_t i = 0; i < n; ++i) {
        if (a[i] != b[i]) {
            return 0;
        }
    }
    return 1;
}

/* Steps over one value of an already validated document. */
static inline int32_t bs_value_skip(const uint8_t *b, int32_t end, int32_t pos) {
    uint64_t n = 0;
    if (pos < 0 || pos >= end) {
        return -1;
    }
    switch (b[pos++]) {
    case BASALT_VALUE_INT:
        return bs_value_leb(b, end, &pos, &n) ? pos : -1;
    case BASALT_VALUE_FLOAT:
        return end - pos >= 8 ? pos + 8 : -1;
    case BASALT_VALUE_STRING:
    case BASALT_VALUE_BYTES:
        return bs_value_leb(b, end, &pos, &n) && n <= (uint64_t)(end - pos) ? pos + (int32_t)n : -1;
    case BASALT_VALUE_LIST:
    case BASALT_VALUE_MAP: {
        const int is_map = b[pos - 1] == BASALT_VALUE_MAP;
        if (!bs_value_leb(b, end, &pos, &n)) {
            return -1;
        }
        for (uint64_t i = 0; i < n && pos >= 0; ++i) {
            if (is_map) {
                uint64_t klen = 0;
                if (!bs_value_leb(b, end, &pos, &klen) || klen > (uint64_t)(end - pos)) {
                    return -1;
                }
                pos += (int32_t)klen;
            }
            pos = bs_value_skip(b, end, pos);
        }
        return pos;
    }
    default:
        return pos;
    }
}

/* Walks one value from `pos`, enforcing the limits; returns the end position or -1. */
static inline int32_t bs_value_walk(const uint8_t *b, int32_t end, int32_t pos, int32_t depth, int32_t *nodes) {
    if (depth > BASALT_VALUE_MAX_DEPTH || ++*nodes > BASALT_VALUE_MAX_NODES || pos >= end) {
        return -1;
    }
    uint64_t n = 0;
    switch (b[pos++]) {
    case BASALT_VALUE_NULL:
    case BASALT_VALUE_FALSE:
    case BASALT_VALUE_TRUE:
        return pos;
    case BASALT_VALUE_INT:
        return bs_value_leb(b, end, &pos, &n) ? pos : -1;
    case BASALT_VALUE_FLOAT:
        return end - pos >= 8 ? pos + 8 : -1;
    case BASALT_VALUE_STRING:
    case BASALT_VALUE_BYTES:
        return bs_value_leb(b, end, &pos, &n) && n <= (uint64_t)(end - pos) ? pos + (int32_t)n : -1;
    case BASALT_VALUE_LIST:
        if (!bs_value_leb(b, end, &pos, &n) || n > (uint64_t)(end - pos)) {
            return -1;
        }
        for (uint64_t i = 0; i < n; ++i) {
            pos = bs_value_walk(b, end, pos, depth + 1, nodes);
            if (pos < 0) {
                return -1;
            }
        }
        return pos;
    case BASALT_VALUE_MAP: {
        if (!bs_value_leb(b, end, &pos, &n) || n > (uint64_t)(end - pos)) {
            return -1;
        }
        const int32_t first = pos;
        for (uint64_t i = 0; i < n; ++i) {
            uint64_t klen = 0;
            if (!bs_value_leb(b, end, &pos, &klen) || klen > BASALT_VALUE_MAX_KEY_BYTES ||
                klen > (uint64_t)(end - pos)) {
                return -1;
            }
            const int32_t key = pos;
            int32_t scan = first;
            for (uint64_t j = 0; j < i; ++j) {
                uint64_t olen = 0;
                bs_value_leb(b, end, &scan, &olen);
                if (olen == klen && bs_value_same(b + scan, b + key, (int32_t)klen)) {
                    return -1;
                }
                scan = bs_value_skip(b, end, scan + (int32_t)olen);
            }
            pos = bs_value_walk(b, end, key + (int32_t)klen, depth + 1, nodes);
            if (pos < 0) {
                return -1;
            }
        }
        return pos;
    }
    default:
        return -1;
    }
}

/* Validates a whole document and points `root` at it; BASALT_ERR_INVALID (and an absent root) when malformed. */
static inline int bs_value_open(bs_val *root, const void *data, int32_t len) {
    root->base = 0;
    root->off = 0;
    root->end = 0;
    if (data == 0 || len <= 0 || len > BASALT_VALUE_MAX_BYTES) {
        return BASALT_ERR_INVALID;
    }
    int32_t nodes = 0;
    if (bs_value_walk((const uint8_t *)data, len, 0, 0, &nodes) != len) {
        return BASALT_ERR_INVALID;
    }
    root->base = (const uint8_t *)data;
    root->off = 0;
    root->end = len;
    return 0;
}


static inline bs_val bs_value_none(void) {
    bs_val v = {0, 0, 0};
    return v;
}

/* The kind of the value: a BS_VALUE_* tag (BS_VALUE_BOOL for both booleans), or -1 for the absent value. */
static inline int bs_value_kind(bs_val v) {
    if (v.base == 0 || v.off >= v.end) {
        return -1;
    }
    const uint8_t tag = v.base[v.off];
    return tag == BASALT_VALUE_FALSE || tag == BASALT_VALUE_TRUE ? BS_VALUE_BOOL : tag;
}

static inline int bs_value_is_null(bs_val v) { return bs_value_kind(v) == BS_VALUE_NULL; }

/* The getters return 1 and fill `out` when the value has that kind, 0 otherwise. */
static inline int bs_value_as_bool(bs_val v, int *out) {
    if (bs_value_kind(v) != BS_VALUE_BOOL) {
        return 0;
    }
    *out = v.base[v.off] == BASALT_VALUE_TRUE;
    return 1;
}

static inline int bs_value_as_i64(bs_val v, int64_t *out) {
    if (bs_value_kind(v) != BS_VALUE_INT) {
        return 0;
    }
    int32_t pos = v.off + 1;
    uint64_t raw = 0;
    if (!bs_value_leb(v.base, v.end, &pos, &raw)) {
        return 0;
    }
    *out = (int64_t)((raw >> 1) ^ (~(raw & 1u) + 1u));
    return 1;
}

static inline int bs_value_as_f64(bs_val v, double *out) {
    if (bs_value_kind(v) != BS_VALUE_FLOAT || v.end - v.off < 9) {
        return 0;
    }
    union {
        double d;
        uint64_t u;
    } bits;
    bits.u = 0;
    for (int i = 0; i < 8; ++i) {
        bits.u |= (uint64_t)v.base[v.off + 1 + i] << (8 * i);
    }
    *out = bits.d;
    return 1;
}

/* Points `data` at the payload of a string or bytes value (not NUL-terminated) and sets its length. */
static inline int bs_value_as_blob(bs_val v, int tag, const char **data, int32_t *len) {
    if (bs_value_kind(v) != tag) {
        return 0;
    }
    int32_t pos = v.off + 1;
    uint64_t n = 0;
    if (!bs_value_leb(v.base, v.end, &pos, &n) || n > (uint64_t)(v.end - pos)) {
        return 0;
    }
    *data = (const char *)(v.base + pos);
    *len = (int32_t)n;
    return 1;
}

static inline int bs_value_as_str(bs_val v, const char **data, int32_t *len) {
    return bs_value_as_blob(v, BS_VALUE_STRING, data, len);
}

static inline int bs_value_as_bytes(bs_val v, const uint8_t **data, int32_t *len) {
    return bs_value_as_blob(v, BS_VALUE_BYTES, (const char **)data, len);
}

/* Number of items of a list or entries of a map, -1 for any other value. */
static inline int32_t bs_value_count(bs_val v) {
    const int kind = bs_value_kind(v);
    int32_t pos = v.off + 1;
    uint64_t n = 0;
    if ((kind != BS_VALUE_LIST && kind != BS_VALUE_MAP) || !bs_value_leb(v.base, v.end, &pos, &n)) {
        return -1;
    }
    return (int32_t)n;
}

/* Starts iterating a list or map; the iterator is empty for any other value. */
static inline void bs_value_iter(bs_val v, bs_iter *it) {
    const int kind = bs_value_kind(v);
    int32_t pos = v.off + 1;
    uint64_t n = 0;
    it->base = v.base;
    it->end = v.end;
    it->left = 0;
    it->is_map = kind == BS_VALUE_MAP;
    if ((kind == BS_VALUE_LIST || kind == BS_VALUE_MAP) && bs_value_leb(v.base, v.end, &pos, &n)) {
        it->left = (int32_t)n;
    }
    it->pos = pos;
}

/* Next item (or entry of a map: `key`, `key_len` are filled when not null); 0 at the end. */
static inline int bs_iter_next(bs_iter *it, const char **key, int32_t *key_len, bs_val *item) {
    if (it->left <= 0 || it->pos >= it->end) {
        return 0;
    }
    int32_t pos = it->pos;
    if (it->is_map) {
        uint64_t klen = 0;
        if (!bs_value_leb(it->base, it->end, &pos, &klen) || klen > (uint64_t)(it->end - pos)) {
            it->left = 0;
            return 0;
        }
        if (key != 0) {
            *key = (const char *)(it->base + pos);
        }
        if (key_len != 0) {
            *key_len = (int32_t)klen;
        }
        pos += (int32_t)klen;
    }
    item->base = it->base;
    item->off = pos;
    item->end = it->end;
    it->pos = bs_value_skip(it->base, it->end, pos);
    it->left = it->pos < 0 ? 0 : it->left - 1;
    return 1;
}

/* Item `index` of a list (absent when out of range or not a list). */
static inline bs_val bs_value_at(bs_val v, int32_t index) {
    bs_iter it;
    bs_val item = bs_value_none();
    if (bs_value_kind(v) != BS_VALUE_LIST || index < 0) {
        return bs_value_none();
    }
    bs_value_iter(v, &it);
    for (int32_t i = 0; bs_iter_next(&it, 0, 0, &item); ++i) {
        if (i == index) {
            return item;
        }
    }
    return bs_value_none();
}

/* The value under `key` of a map (absent when missing or not a map). */
static inline bs_val bs_value_find_n(bs_val v, const char *key, int32_t key_len) {
    bs_iter it;
    bs_val item;
    const char *name = 0;
    int32_t name_len = 0;
    if (bs_value_kind(v) != BS_VALUE_MAP) {
        return bs_value_none();
    }
    bs_value_iter(v, &it);
    while (bs_iter_next(&it, &name, &name_len, &item)) {
        if (name_len == key_len && bs_value_same((const uint8_t *)name, (const uint8_t *)key, key_len)) {
            return item;
        }
    }
    return bs_value_none();
}

static inline bs_val bs_value_find(bs_val v, const char *key) {
    int32_t n = 0;
    while (key != 0 && key[n]) {
        ++n;
    }
    return bs_value_find_n(v, key, n);
}

/* An import that fills `(out, cap)` and returns the length the full answer needs. */
typedef int32_t (*bs_out_fn)(void *user, char *out, int32_t cap);

/* Calls `fn` with `buf`; when the answer needs more, calls it once more with `spare`. Returns the answer length, with
 * `*result` pointing at the buffer that holds it, a negative error from `fn`, or BASALT_ERR_INVALID when neither
 * buffer is big enough (or the answer changed size between the two calls). */
static inline int32_t bs_out_call(bs_out_fn fn, void *user, char *buf, int32_t cap, char *spare, int32_t spare_cap,
                                  char **result) {
    int32_t n = fn(user, buf, cap);
    if (n < 0) {
        return n;
    }
    if (n <= cap) {
        *result = buf;
        return n;
    }
    if (spare == 0 || n > spare_cap) {
        return BASALT_ERR_INVALID;
    }
    n = fn(user, spare, spare_cap);
    if (n < 0) {
        return n;
    }
    if (n > spare_cap) {
        return BASALT_ERR_INVALID;
    }
    *result = spare;
    return n;
}

/* The same, then validates the answer as a document: 0 and `root` set, or a negative error. */
static inline int bs_out_value(bs_out_fn fn, void *user, char *buf, int32_t cap, char *spare, int32_t spare_cap,
                               bs_val *root) {
    char *at = 0;
    const int32_t n = bs_out_call(fn, user, buf, cap, spare, spare_cap, &at);
    if (n < 0) {
        *root = bs_value_none();
        return n;
    }
    return bs_value_open(root, at, n);
}

#endif /* BASALT_INTERNAL_VALUES_H */
