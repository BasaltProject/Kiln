import * as raw from "./raw/core";
import { Args, Ctx, Handler } from "./core";
import { tokenFor } from "./handlers";
import { ap, finish, lenOf, ptrOf, u } from "./buffers";

export function command(spec: string, permission: string, handler: Handler): i32 {
  const s = u(spec);
  const p = u(permission);
  return raw.command_register(s.ptr, s.len, p.ptr, p.len, tokenFor(handler));
}
export function commandReply(ctx: Ctx, text: string, args: Args | null = null): i32 {
  const t = u(text);
  const a = ap(args);
  return raw.command_reply(ctx, t.ptr, t.len, ptrOf(a), lenOf(a));
}
export function commandArgStr(ctx: Ctx, name: string): string | null {
  const key = u(name);
  let buf = new ArrayBuffer(256);
  let n = raw.command_arg_str(ctx, key.ptr, key.len, changetype<usize>(buf), 256);
  if (n > 256) {
    buf = new ArrayBuffer(n);
    n = raw.command_arg_str(ctx, key.ptr, key.len, changetype<usize>(buf), n);
  }
  return finish(buf, n);
}
export function commandArgI64(ctx: Ctx, name: string, def: i64): i64 {
  const n = u(name);
  const out = new ArrayBuffer(8);
  return raw.command_arg_i64(ctx, n.ptr, n.len, changetype<usize>(out)) == 0 ? load<i64>(changetype<usize>(out)) : def;
}
export function commandSourceName(ctx: Ctx): string | null {
  let buf = new ArrayBuffer(256);
  let n = raw.command_source_name(ctx, changetype<usize>(buf), 256);
  if (n > 256) {
    buf = new ArrayBuffer(n);
    n = raw.command_source_name(ctx, changetype<usize>(buf), n);
  }
  return finish(buf, n);
}
export function commandIsConsole(ctx: Ctx): bool { return raw.command_is_console(ctx) != 0; }
