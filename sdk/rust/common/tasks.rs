//! Delayed and repeating tasks, in ticks.
use crate::core::{token_for, Ctx};
use crate::raw;

pub fn after(delay: i32, handler: impl FnMut(Ctx) + 'static) -> i32 {
    unsafe { raw::task_after(delay, token_for(Box::new(handler))) }
}

pub fn every(delay: i32, period: i32, handler: impl FnMut(Ctx) + 'static) -> i32 {
    unsafe { raw::task_every(delay, period, token_for(Box::new(handler))) }
}

pub fn cancel(task: i32) -> i32 {
    unsafe { raw::task_cancel(task) }
}
