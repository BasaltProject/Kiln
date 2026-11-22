/* Event listeners and fields, shared by both profiles. */
#ifndef BASALT_INTERNAL_EVENTS_H
#define BASALT_INTERNAL_EVENTS_H

#include "basalt/internal/core.h"

#ifdef __cplusplus
extern "C" {
#endif

int bs_on(const char *event, int priority, bs_handler handler);
bs_player bs_event_player(bs_ctx ctx);
int bs_event_field_str(bs_ctx ctx, const char *field, char *out, int cap);
int bs_event_field_i64(bs_ctx ctx, const char *field, int64_t *out);
int bs_event_cancel(bs_ctx ctx, int cancelled);

#ifdef __cplusplus
}
#endif

#endif
