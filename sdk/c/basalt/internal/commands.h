/* Commands and their arguments, shared by both profiles. */
#ifndef BASALT_INTERNAL_COMMANDS_H
#define BASALT_INTERNAL_COMMANDS_H

#include "basalt/internal/core.h"

#ifdef __cplusplus
extern "C" {
#endif

int bs_command(const char *spec, const char *permission, bs_handler handler);
int bs_command_reply(bs_ctx ctx, const char *text, const bs_args *args);
int bs_command_arg_str(bs_ctx ctx, const char *name, char *out, int cap);
int bs_command_arg_i64(bs_ctx ctx, const char *name, int64_t *out);
int bs_command_source_name(bs_ctx ctx, char *out, int cap);
int bs_command_is_console(bs_ctx ctx);

#ifdef __cplusplus
}
#endif

#endif
