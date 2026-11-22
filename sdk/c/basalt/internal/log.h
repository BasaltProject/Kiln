/* Console log lines of the plugin, shared by both profiles. */
#ifndef BASALT_INTERNAL_LOG_H
#define BASALT_INTERNAL_LOG_H

#include "basalt/internal/core.h"

#ifdef __cplusplus
extern "C" {
#endif

void bs_log(int level, const char *msg);
void bs_debug(const char *msg);
void bs_info(const char *msg);
void bs_warn(const char *msg);
void bs_error(const char *msg);

#ifdef __cplusplus
}
#endif

#endif
