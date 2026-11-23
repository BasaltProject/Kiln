/* Core of the C SDK, shared by the server and proxy profiles: handles, named arguments and the plugin entry point. */
#ifndef BASALT_INTERNAL_CORE_H
#define BASALT_INTERNAL_CORE_H

#include "basalt_abi.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int32_t bs_player;
typedef int32_t bs_ctx;
typedef void (*bs_handler)(bs_ctx ctx);

/* Named arguments for text: add pairs, then pass the list to send, translate or reply. */
typedef struct {
    char data[1024];
    int32_t len;
} bs_args;

void bs_args_init(bs_args *args);
int bs_args_add(bs_args *args, const char *key, const char *value);

/* Define it in the plugin and register everything in it. */
void plugin_init(void);

/* Writes the decimal text of `value` and a NUL into `out` (24 bytes are enough); returns the length. */
int bs_format_i64(int64_t value, char *out);

/* The token a callback registered by a wrapper is called with; negative when the handler table is full. */
int bs_token_for(bs_handler handler);

/* Tokens of callbacks that run once: the slot is freed after the callback ran (or the request was cancelled) and
 * reused. Take the token, call the import with it, then pass the import's result through bs_once_bind with the kind
 * of handle it returned (a negative result frees the slot at once). A NULL handler gives token 0. */
#define BS_ONCE_TASK 1
#define BS_ONCE_REQUEST 2
int bs_once_token(bs_handler handler);
int bs_once_bind(int token, int kind, int handle);

/* Drops a pending request (a wait, a lookup, a service call, a world request) so its callback never runs. */
int bs_request_cancel(int request);

#ifdef __cplusplus
}
#endif

/* The byte length of a NUL-terminated text; 0 for NULL. */
static inline int32_t bs_length(const char *text) {
    int32_t n = 0;
    while (text != 0 && text[n]) {
        ++n;
    }
    return n;
}

#endif
