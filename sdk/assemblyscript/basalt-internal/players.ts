import * as raw from "./raw/core";
import { Args, Player } from "./core";
import { ap, finish, lenOf, ptrOf, u } from "./buffers";

export function playerByName(name: string): Player { const n = u(name); return raw.player_by_name(n.ptr, n.len); }
export function playersOnline(): Player[] {
  let cap = 64;
  while (true) {
    const out = new Int32Array(cap);
    const count = raw.players_online(changetype<usize>(out.buffer), cap);
    if (count < 0) return [];
    if (count <= cap) {
      const players: Player[] = [];
      for (let i = 0; i < count; i++) players.push(out[i]);
      return players;
    }
    cap = count;
  }
}
export function playerName(player: Player): string | null {
  let buf = new ArrayBuffer(256);
  let n = raw.player_name(player, changetype<usize>(buf), 256);
  if (n > 256) {
    buf = new ArrayBuffer(n);
    n = raw.player_name(player, changetype<usize>(buf), n);
  }
  return finish(buf, n);
}
export function send(player: Player, text: string, args: Args | null = null): i32 {
  const t = u(text);
  const a = ap(args);
  return raw.player_send(player, t.ptr, t.len, ptrOf(a), lenOf(a));
}
export function actionBar(player: Player, text: string, args: Args | null = null): i32 {
  const t = u(text);
  const a = ap(args);
  return raw.player_action_bar(player, t.ptr, t.len, ptrOf(a), lenOf(a));
}
