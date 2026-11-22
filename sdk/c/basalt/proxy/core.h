/* Handles, named arguments, plugin identity and the entry point: define `void plugin_init(void)`. */
#ifndef BASALT_PROXY_CORE_H
#define BASALT_PROXY_CORE_H

#include "basalt/internal/core.h"
#include "basalt_proxy_abi.h"
#include "areas/proxy_events.h"

static inline int bs_proxy_plugin_info(const char *field, char *out, int cap) {
    return basalt_proxy_plugin_info(field, bs_length(field), out, cap);
}

static inline int bs_proxy_plugin_manifest(char *out, int cap) { return basalt_proxy_plugin_manifest(out, cap); }

#endif
