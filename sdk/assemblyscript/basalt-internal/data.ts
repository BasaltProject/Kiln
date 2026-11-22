import * as raw from "./raw/core";
import { Player } from "./core";
import { finish, u } from "./buffers";

export function dataGetI64(player: Player, key: string, def: i64): i64 {
  const k = u(key);
  return raw.data_get_i64(player, k.ptr, k.len, def);
}
export function dataSetI64(player: Player, key: string, value: i64): i32 {
  const k = u(key);
  return raw.data_set_i64(player, k.ptr, k.len, value);
}
export function dataGetBool(player: Player, key: string, def: bool): bool {
  const k = u(key);
  return raw.data_get_bool(player, k.ptr, k.len, def ? 1 : 0) != 0;
}
export function dataSetBool(player: Player, key: string, value: bool): i32 {
  const k = u(key);
  return raw.data_set_bool(player, k.ptr, k.len, value ? 1 : 0);
}
export function dataGetStr(player: Player, key: string): string | null {
  const k = u(key);
  let buf = new ArrayBuffer(256);
  let n = raw.data_get_str(player, k.ptr, k.len, changetype<usize>(buf), 256);
  if (n > 256) {
    buf = new ArrayBuffer(n);
    n = raw.data_get_str(player, k.ptr, k.len, changetype<usize>(buf), n);
  }
  return finish(buf, n);
}
export function dataSetStr(player: Player, key: string, value: string): i32 {
  const k = u(key);
  const v = u(value);
  return raw.data_set_str(player, k.ptr, k.len, v.ptr, v.len);
}
export function dataRemove(player: Player, key: string): i32 {
  const k = u(key);
  return raw.data_remove(player, k.ptr, k.len);
}
