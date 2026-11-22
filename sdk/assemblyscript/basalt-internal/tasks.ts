import * as raw from "./raw/core";
import { Handler } from "./core";
import { tokenFor } from "./handlers";

export function after(delay: i32, handler: Handler): i32 { return raw.task_after(delay, tokenFor(handler)); }
export function every(delay: i32, period: i32, handler: Handler): i32 {
  return raw.task_every(delay, period, tokenFor(handler));
}
export function taskCancel(task: i32): i32 { return raw.task_cancel(task); }
