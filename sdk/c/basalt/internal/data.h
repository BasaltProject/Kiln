/* Per-player data kept by the host, shared by both profiles. */
#ifndef BASALT_INTERNAL_DATA_H
#define BASALT_INTERNAL_DATA_H

#include "basalt/internal/core.h"

#ifdef __cplusplus
extern "C" {
#endif

int64_t bs_data_get_i64(bs_player player, const char *key, int64_t def);
int bs_data_set_i64(bs_player player, const char *key, int64_t value);
int bs_data_get_bool(bs_player player, const char *key, int def);
int bs_data_set_bool(bs_player player, const char *key, int value);
int bs_data_get_str(bs_player player, const char *key, char *out, int cap);
int bs_data_set_str(bs_player player, const char *key, const char *value);
int bs_data_remove(bs_player player, const char *key);

#ifdef __cplusplus
}
#endif

#endif
