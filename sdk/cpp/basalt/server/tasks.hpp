#include <basalt/server.h>
#pragma once
#include <basalt/server/core.hpp>
#include <basalt/server/tasks.h>

namespace basalt::server::tasks {
using namespace basalt::server::core;

inline int after(int delay, Handler handler) { return bs_after(delay, handler); }

inline int every(int delay, int period, Handler handler) { return bs_every(delay, period, handler); }

inline int task_cancel(int task) { return bs_task_cancel(task); }
} // namespace basalt::server::tasks
