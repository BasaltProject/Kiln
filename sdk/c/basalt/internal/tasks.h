/* Delayed and repeating tasks, shared by both profiles. */
#ifndef BASALT_INTERNAL_TASKS_H
#define BASALT_INTERNAL_TASKS_H

#include "basalt/internal/core.h"

#ifdef __cplusplus
extern "C" {
#endif

int bs_after(int delay, bs_handler handler);
int bs_every(int delay, int period, bs_handler handler);
int bs_task_cancel(int task);

#ifdef __cplusplus
}
#endif

#endif
