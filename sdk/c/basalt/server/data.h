/* Per-player data kept by the host across sessions, and the rankings of the plugin store. */
#ifndef BASALT_SERVER_DATA_H
#define BASALT_SERVER_DATA_H

#include "basalt/internal/data.h"

#ifdef __cplusplus
extern "C" {
#endif

int bs_store_top(const char *key, int limit, char *out, int cap);

#ifdef __cplusplus
}
#endif

#endif
