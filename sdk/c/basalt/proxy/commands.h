/* Registers commands and reads their arguments, typed or as one encoded value. */
#ifndef BASALT_PROXY_COMMANDS_H
#define BASALT_PROXY_COMMANDS_H

#include "basalt/internal/commands.h"
#include "areas/proxy_events.h"

static inline int bs_proxy_command_arg_f64(bs_ctx ctx, const char *name, double *out) {
    return basalt_proxy_command_arg_f64(ctx, name, bs_length(name), out);
}

static inline int bs_proxy_command_arg_bool(bs_ctx ctx, const char *name, int *out) {
    return basalt_proxy_command_arg_bool(ctx, name, bs_length(name), out);
}

static inline int bs_proxy_command_args(bs_ctx ctx, char *out, int cap) { return basalt_proxy_command_args(ctx, out, cap); }

#endif
