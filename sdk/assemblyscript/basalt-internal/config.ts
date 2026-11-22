import * as raw from "./raw/core";
import { finish, u } from "./buffers";

export function configI64(key: string, def: i64): i64 { const k = u(key); return raw.config_i64(k.ptr, k.len, def); }
export function configF64(key: string, def: f64): f64 { const k = u(key); return raw.config_f64(k.ptr, k.len, def); }
export function configBool(key: string, def: bool): bool { const k = u(key); return raw.config_bool(k.ptr, k.len, def ? 1 : 0) != 0; }
export function configStr(key: string): string | null {
  const k = u(key);
  let buf = new ArrayBuffer(256);
  let n = raw.config_str(k.ptr, k.len, changetype<usize>(buf), 256);
  if (n > 256) {
    buf = new ArrayBuffer(n);
    n = raw.config_str(k.ptr, k.len, changetype<usize>(buf), n);
  }
  return finish(buf, n);
}
