import * as raw from "./raw/core";
import { Args, Player, Ctx, Handler } from "../basalt-internal/core";
import { tokenFor } from "../basalt-internal/handlers";
import { ap, finish, lenOf, ptrOf, u } from "../basalt-internal/buffers";

export * from "../basalt-internal/players";

export function particle(player: Player, id: string, count: i32, spread: f64): i32 {
  const i = u(id);
  return raw.player_particle(player, i.ptr, i.len, count, spread);
}

export function sound(player: Player, id: string, volume: f32, pitch: f32): i32 {
  const i = u(id);
  return raw.player_sound(player, i.ptr, i.len, volume, pitch);
}
