// Callback tokens: the host calls basalt_callback(token, ctx) and dispatch runs the handler behind the token.
import { Ctx, Handler } from "./core";

const handlers: Handler[] = [];

export function tokenFor(handler: Handler | null): i32 {
  if (handler === null) return 0;
  handlers.push(handler);
  return handlers.length;
}

export function dispatch(token: i32, ctx: Ctx): void {
  if (token > 0 && token <= handlers.length) handlers[token - 1](ctx);
}
