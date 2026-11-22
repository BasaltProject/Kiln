//! Delayed and repeating tasks, in ticks.
const core = @import("core.zig");
const raw = @import("raw/core.zig");

pub fn after(delay: i32, handler: core.Handler) i32 {
    const token = core.tokenFor(handler);
    return if (token < 0) core.Status.invalid else raw.task_after(delay, token);
}
pub fn every(delay: i32, period: i32, handler: core.Handler) i32 {
    const token = core.tokenFor(handler);
    return if (token < 0) core.Status.invalid else raw.task_every(delay, period, token);
}
pub fn cancel(task: i32) i32 {
    return raw.task_cancel(task);
}
