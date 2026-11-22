import * as rawProxy from "./raw/core";
import { Args, Player, Ctx, Handler } from "../basalt-internal/core";
import { tokenFor } from "../basalt-internal/handlers";
import { ap, finish, lenOf, ptrOf, u } from "../basalt-internal/buffers";

export * from "../basalt-internal/players";

export function playerUuid(player: Player): string | null {
  let buf = new ArrayBuffer(64);
  let n = rawProxy.player_uuid(player, changetype<usize>(buf), 64);
  if (n > 64) {
    buf = new ArrayBuffer(n);
    n = rawProxy.player_uuid(player, changetype<usize>(buf), n);
  }
  return finish(buf, n);
}

export function playerServer(player: Player): string | null {
  let buf = new ArrayBuffer(256);
  let n = rawProxy.player_server(player, changetype<usize>(buf), 256);
  if (n > 256) {
    buf = new ArrayBuffer(n);
    n = rawProxy.player_server(player, changetype<usize>(buf), n);
  }
  return finish(buf, n);
}

export function playerPing(player: Player): i32 { return rawProxy.player_ping(player); }

export function playerSettingStr(player: Player, field: string): string | null {
  const f = u(field);
  let buf = new ArrayBuffer(256);
  let n = rawProxy.player_setting_str(player, f.ptr, f.len, changetype<usize>(buf), 256);
  if (n > 256) {
    buf = new ArrayBuffer(n);
    n = rawProxy.player_setting_str(player, f.ptr, f.len, changetype<usize>(buf), n);
  }
  return finish(buf, n);
}

export function playerSettingI64(player: Player, field: string, def: i64): i64 {
  const f = u(field);
  const out = new ArrayBuffer(8);
  return rawProxy.player_setting_i64(player, f.ptr, f.len, changetype<usize>(out)) >= 0 ? load<i64>(changetype<usize>(out)) : def;
}

export function playerHasPermission(player: Player, node: string): bool {
  const n = u(node);
  return rawProxy.player_has_permission(player, n.ptr, n.len) == 1;
}

export function playerTitle(player: Player, title: string, subtitle: string, fadeIn: i32 = -1, stay: i32 = -1, fadeOut: i32 = -1): i32 {
  const t = u(title);
  const s = u(subtitle);
  return rawProxy.player_title(player, t.ptr, t.len, s.ptr, s.len, fadeIn, stay, fadeOut);
}

export function playerConnect(player: Player, server: string): i32 {
  const s = u(server);
  return rawProxy.player_connect(player, s.ptr, s.len);
}

export function playerKick(player: Player, reason: string, args: Args | null = null): i32 {
  const r = u(reason);
  const a = ap(args);
  return rawProxy.player_kick(player, r.ptr, r.len, ptrOf(a), lenOf(a));
}
