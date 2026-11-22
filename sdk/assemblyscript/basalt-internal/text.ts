import * as raw from "./raw/core";
import { Args } from "./core";
import { ap, finish, lenOf, ptrOf, u } from "./buffers";

export function translate(key: string, args: Args | null = null): string | null {
  const k = u(key);
  const a = ap(args);
  let buf = new ArrayBuffer(256);
  let n = raw.translate(k.ptr, k.len, ptrOf(a), lenOf(a), changetype<usize>(buf), 256);
  if (n > 256) {
    buf = new ArrayBuffer(n);
    n = raw.translate(k.ptr, k.len, ptrOf(a), lenOf(a), changetype<usize>(buf), n);
  }
  return finish(buf, n);
}
