import * as raw from "./raw/core";
import { DEBUG, ERROR, INFO, WARN } from "./core";
import { u } from "./buffers";

export function log(level: i32, msg: string): void { const m = u(msg); raw.log(level, m.ptr, m.len); }
export function logDebug(msg: string): void { log(DEBUG, msg); }
export function logInfo(msg: string): void { log(INFO, msg); }
export function logWarn(msg: string): void { log(WARN, msg); }
export function logError(msg: string): void { log(ERROR, msg); }
