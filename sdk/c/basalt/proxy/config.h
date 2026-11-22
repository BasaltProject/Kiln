/* Reads the plugin config.toml; bs_proxy_config_value gives arrays and tables as an encoded value. */
#ifndef BASALT_PROXY_CONFIG_H
#define BASALT_PROXY_CONFIG_H

#include "basalt/internal/config.h"
#include "areas/proxy_events.h"

static inline int bs_proxy_config_value(const char *key, char *out, int cap) {
    return basalt_proxy_config_value(key, bs_length(key), out, cap);
}

#endif
