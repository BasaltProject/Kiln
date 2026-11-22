import * as raw from "./raw/core";
import { Args, Player, Ctx, Handler } from "../basalt-internal/core";
import { tokenFor } from "../basalt-internal/handlers";
import { ap, finish, lenOf, ptrOf, u } from "../basalt-internal/buffers";

export * from "../basalt-internal/data";

export class TopEntry {
  constructor(public name: string, public value: i64) {}
}

export function storeTop(key: string, limit: i32): TopEntry[] {
  const k = u(key);
  const rows: TopEntry[] = [];
  let cap = 1024;
  while (true) {
    const buffer = new ArrayBuffer(cap);
    const needed = raw.store_top(k.ptr, k.len, limit, changetype<usize>(buffer), cap);
    if (needed < 0) return rows;
    if (needed > cap) {
      cap = needed;
      continue;
    }
    const base = changetype<usize>(buffer);
    let at = 0;
    while (at + 4 <= needed) {
      const n = load<i32>(base + at);
      at += 4;
      if (at + n + 8 > needed) break;
      const name = String.UTF8.decodeUnsafe(base + at, n);
      at += n;
      rows.push(new TopEntry(name, load<i64>(base + at)));
      at += 8;
    }
    return rows;
  }
  return rows;
}
