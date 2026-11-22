import * as raw from "./raw/core";
import { Args, Player, Ctx, Handler } from "../basalt-internal/core";
import { tokenFor } from "../basalt-internal/handlers";
import { ap, finish, lenOf, ptrOf, u } from "../basalt-internal/buffers";

export function placeholder(name: string, handler: Handler): i32 {
  const n = u(name);
  return raw.placeholder_register(n.ptr, n.len, tokenFor(handler));
}

export function placeholderPlayer(ctx: Ctx): Player { return raw.placeholder_player(ctx); }

export function placeholderResult(ctx: Ctx, text: string): i32 {
  const t = u(text);
  return raw.placeholder_result(ctx, t.ptr, t.len);
}

export function menuCreate(title: string, rows: i32): i32 {
  const t = u(title);
  return raw.menu_create(t.ptr, t.len, rows);
}

export function menuSet(menu: i32, slot: i32, item: string, name: string, lore: string, onClick: Handler): i32 {
  const i = u(item);
  const n = u(name);
  const l = u(lore);
  return raw.menu_set(menu, slot, i.ptr, i.len, n.ptr, n.len, l.ptr, l.len, tokenFor(onClick));
}

export function menuOpen(menu: i32, player: Player): i32 { return raw.menu_open(menu, player); }

export function menuClickPlayer(ctx: Ctx): Player { return raw.menu_click_player(ctx); }
