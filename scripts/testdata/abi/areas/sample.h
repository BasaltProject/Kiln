#ifndef BASALT_AREA_SAMPLE_H
#define BASALT_AREA_SAMPLE_H
#include "../basalt_abi.h"
#include "../basalt_proxy_abi.h"
BASALT_IMPORT(sample_get) int32_t basalt_sample_get(int32_t handle, const char *key, int32_t klen, char *out, int32_t cap);
BASALT_IMPORT_SHARED(sample_scale) double basalt_sample_scale(double value, float factor, int64_t *wide, int32_t type);
BASALT_PROXY_IMPORT(sample_ping) void basalt_proxy_sample_ping(void);
#endif
