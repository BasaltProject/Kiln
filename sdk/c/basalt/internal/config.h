/* The plugin config.toml, shared by both profiles. */
#ifndef BASALT_INTERNAL_CONFIG_H
#define BASALT_INTERNAL_CONFIG_H

#include "basalt/internal/core.h"

#ifdef __cplusplus
extern "C" {
#endif

int64_t bs_config_i64(const char *key, int64_t def);
double bs_config_f64(const char *key, double def);
int bs_config_bool(const char *key, int def);
int bs_config_str(const char *key, char *out, int cap);

#ifdef __cplusplus
}
#endif

#endif
