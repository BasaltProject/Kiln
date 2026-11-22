/* Lang file text, shared by both profiles. */
#ifndef BASALT_INTERNAL_TEXT_H
#define BASALT_INTERNAL_TEXT_H

#include "basalt/internal/core.h"

#ifdef __cplusplus
extern "C" {
#endif

int bs_translate(const char *key, const bs_args *args, char *out, int cap);

#ifdef __cplusplus
}
#endif

#endif
