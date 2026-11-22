#include <basalt/proxy.h>
#pragma once
#include <basalt/proxy/core.hpp>
#include <basalt/proxy/tasks.h>

namespace basalt::proxy::tasks {
using namespace basalt::proxy::core;

inline int after(int delay, Handler handler) { return bs_after(delay, handler); }

inline int every(int delay, int period, Handler handler) { return bs_every(delay, period, handler); }

inline int task_cancel(int task) { return bs_task_cancel(task); }
} // namespace basalt::proxy::tasks
