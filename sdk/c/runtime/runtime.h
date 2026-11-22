#ifndef BASALT_RUNTIME_H
#define BASALT_RUNTIME_H

#include "basalt/internal/core.h"

#define ARGS_PTR(args) ((args) != 0 && (args)->len > 4 ? (args)->data : 0)
#define ARGS_LEN(args) ((args) != 0 && (args)->len > 4 ? (args)->len : 0)

#endif
