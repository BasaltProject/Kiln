import * as raw from "./raw/core";
import { Ctx, Handler, Player } from "./core";
import { tokenFor } from "./handlers";
import { finish, u } from "./buffers";

export function on(event: string, priority: i32, handler: Handler): i32 {
  const e = u(event);
  return raw.event_on(e.ptr, e.len, priority, tokenFor(handler));
}
export function eventPlayer(ctx: Ctx): Player { return raw.event_player(ctx); }
export function eventFieldStr(ctx: Ctx, field: string): string | null {
  const f = u(field);
  let buf = new ArrayBuffer(256);
  let n = raw.event_field_str(ctx, f.ptr, f.len, changetype<usize>(buf), 256);
  if (n > 256) {
    buf = new ArrayBuffer(n);
    n = raw.event_field_str(ctx, f.ptr, f.len, changetype<usize>(buf), n);
  }
  return finish(buf, n);
}
export function eventFieldI64(ctx: Ctx, field: string, def: i64): i64 {
  const f = u(field);
  const out = new ArrayBuffer(8);
  return raw.event_field_i64(ctx, f.ptr, f.len, changetype<usize>(out)) == 0 ? load<i64>(changetype<usize>(out)) : def;
}
export function eventCancel(ctx: Ctx, cancelled: bool): i32 { return raw.event_cancel(ctx, cancelled ? 1 : 0); }
