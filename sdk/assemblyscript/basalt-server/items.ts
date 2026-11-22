// Items area: an item stack builder, an Inventory over the inventory handles and thin wrappers for the other imports.
import { raw_items as raw } from "./raw/items";
import { Handler, Player, Ctx, tokenFor } from "../basalt-internal/index";
import { ValueWriter, Value, OutCall } from "./values";

const ITEM_BUFFER: i32 = 512;

class Text {
  readonly bytes: Uint8Array;
  constructor(text: string) { this.bytes = Uint8Array.wrap(String.UTF8.encode(text)); }
  get ptr(): usize { return changetype<usize>(this.bytes.dataStart); }
  get len(): i32 { return this.bytes.length; }
}

function dp(data: Uint8Array | null): usize { return data === null ? 0 : changetype<usize>(data.dataStart); }
function dl(data: Uint8Array | null): i32 { return data === null ? 0 : data.length; }

/** The `{id, count[, name]}` document, or null when it does not fit; an empty name leaves the field out. */
export function itemStack(id: string, count: i32, name: string = ""): Uint8Array | null {
  const w = new ValueWriter(ITEM_BUFFER);
  w.beginMap().key("id").string(id).key("count").int(count);
  if (name.length > 0) w.key("name").string(name);
  return w.end().finish();
}

const NULL_DOCUMENT: Uint8Array = new Uint8Array(1);

/** A handle on a player, block, ender chest, window or entity inventory; call release when done. */
export class Inventory {
  private constructor(public handle: i32) {}

  static ofPlayer(player: Player): Inventory { return new Inventory(raw.inventory_of_player(player)); }
  static ofWindow(player: Player): Inventory { return new Inventory(raw.inventory_of_window(player)); }
  static ofEnder(player: Player): Inventory { return new Inventory(raw.inventory_of_ender(player)); }
  static ofEntity(entity: i32): Inventory { return new Inventory(raw.inventory_of_entity(entity)); }
  static ofBlock(world: string, x: i32, y: i32, z: i32): Inventory {
    const w = new Text(world);
    return new Inventory(raw.inventory_of_block(w.ptr, w.len, x, y, z));
  }

  valid(): bool { return this.handle > 0; }
  size(): i32 { return raw.inventory_size(this.handle); }
  heldSlot(): i32 { return raw.inventory_held_slot(this.handle); }
  setHeldSlot(slot: i32): i32 { return raw.inventory_set_held_slot(this.handle, slot); }

  private set(slot: i32, doc: Uint8Array | null): i32 {
    return doc === null ? -3 : raw.inventory_set(this.handle, slot, dp(doc), dl(doc));
  }

  put(slot: i32, id: string, count: i32): i32 { return this.set(slot, itemStack(id, count)); }
  clearSlot(slot: i32): i32 { return this.set(slot, NULL_DOCUMENT); }

  /** How many items did not fit, or a negative error. */
  give(id: string, count: i32, name: string = ""): i32 {
    const doc = itemStack(id, count, name);
    return doc === null ? -3 : raw.inventory_add(this.handle, dp(doc), dl(doc));
  }

  /** How many items were removed. */
  take(id: string, count: i32): i32 {
    const t = new Text(id);
    return raw.inventory_remove(this.handle, t.ptr, t.len, count);
  }

  count(id: string): i32 {
    const t = new Text(id);
    return raw.inventory_count(this.handle, t.ptr, t.len);
  }

  /** The count of the stack in the slot, 0 for an empty slot, negative for an error. */
  slotCount(slot: i32): i32 {
    const buffer = new Uint8Array(ITEM_BUFFER);
    const n = raw.inventory_get(this.handle, slot, dp(buffer), ITEM_BUFFER);
    if (n < 0) return n;
    if (n > ITEM_BUFFER) return -3;
    const root = Value.open(buffer.subarray(0, n));
    if (root === null) return -3;
    if (root.isNull()) return 0;
    const field = root.find("count");
    return field === null ? 0 : <i32>field.toInt();
  }

  clear(): i32 { return raw.inventory_clear(this.handle); }

  /** Writes the slot document to out; the result is the length it needs. */
  get(slot: i32, out: Uint8Array): i32 { return raw.inventory_get(this.handle, slot, dp(out), out.length); }
  contents(out: Uint8Array): i32 { return raw.inventory_contents(this.handle, dp(out), out.length); }

  release(): void {
    if (this.handle > 0) raw.inventory_release(this.handle);
    this.handle = 0;
  }
}

/** Puts an item in a container menu slot; onClick runs when a player clicks it. */
export function menuSlot(menu: i32, slot: i32, id: string, count: i32, name: string, onClick: Handler | null): i32 {
  const doc = itemStack(id, count, name);
  if (doc === null) return -3;
  return raw.container_menu_set(menu, slot, dp(doc), dl(doc), tokenFor(onClick));
}

export function itemCooldown(player: Player, id: string): i32 {
  const t = new Text(id);
  return raw.item_cooldown(player, t.ptr, t.len);
}

export function itemSetCooldown(player: Player, id: string, ticks: i32): i32 {
  const t = new Text(id);
  return raw.item_set_cooldown(player, t.ptr, t.len, ticks);
}

export function recipeKnown(id: string): bool {
  const t = new Text(id);
  return raw.recipe_has(t.ptr, t.len) > 0;
}

export function registryGet(kind: string, key: string, out: Uint8Array): i32 {
  const k = new Text(kind);
  const e = new Text(key);
  return raw.registry_get(k.ptr, k.len, e.ptr, e.len, dp(out), out.length);
}

export function enchantmentLevelValue(spec: Uint8Array, level: i32): f64 {
  return raw.enchantment_level_value(dp(spec), dl(spec), level);
}

export function recipeAdd(id: string, json: string): i32 {
  const i = new Text(id);
  const j = new Text(json);
  return raw.recipe_add(i.ptr, i.len, j.ptr, j.len);
}

export function recipeRemove(id: string): i32 {
  const i = new Text(id);
  return raw.recipe_remove(i.ptr, i.len);
}

export function recipeIds(out: Uint8Array): i32 { return raw.recipe_ids(dp(out), out.length); }

export function recipeGet(id: string, out: Uint8Array): i32 {
  const i = new Text(id);
  return raw.recipe_get(i.ptr, i.len, dp(out), out.length);
}

export function recipeStation(kind: string, input: Uint8Array, out: Uint8Array): i32 {
  const k = new Text(kind);
  return raw.recipe_station(k.ptr, k.len, dp(input), dl(input), dp(out), out.length);
}

/** recipeStation as an OutCall, for callOut when the answer may outgrow a fixed buffer. */
export class RecipeStationCall extends OutCall {
  private kind: Text;

  constructor(kind: string, private input: Uint8Array) {
    super();
    this.kind = new Text(kind);
  }

  call(out: usize, cap: i32): i32 {
    return raw.recipe_station(this.kind.ptr, this.kind.len, dp(this.input), dl(this.input), out, cap);
  }
}

export function recipeBookGet(player: Player, out: Uint8Array): i32 {
  return raw.recipe_book_get(player, dp(out), out.length);
}

export function recipeBookSet(player: Player, ids: Uint8Array, learn: bool): i32 {
  return raw.recipe_book_set(player, dp(ids), dl(ids), learn ? 1 : 0);
}

export function recipeBookHas(player: Player, id: string): i32 {
  const t = new Text(id);
  return raw.recipe_book_has(player, t.ptr, t.len);
}

export function recipeBookResend(player: Player): i32 { return raw.recipe_book_resend(player); }

export function merchantOpen(player: Player, spec: Uint8Array): i32 {
  return raw.merchant_open(player, dp(spec), dl(spec));
}

export function windowOpen(player: Player, spec: Uint8Array): i32 { return raw.window_open(player, dp(spec), dl(spec)); }
export function windowInfo(player: Player, out: Uint8Array): i32 { return raw.window_info(player, dp(out), out.length); }
export function windowClose(player: Player): i32 { return raw.window_close(player); }
export function windowUpdate(player: Player, spec: Uint8Array): i32 {
  return raw.window_update(player, dp(spec), dl(spec));
}

export function containerMenuCreate(spec: Uint8Array): i32 { return raw.container_menu_create(dp(spec), dl(spec)); }
export function containerMenuOpen(menu: i32, player: Player): i32 { return raw.container_menu_open(menu, player); }
export function containerMenuClose(menu: i32, player: Player): i32 { return raw.container_menu_close(menu, player); }

export function containerMenuClickInfo(ctx: Ctx, out: Uint8Array): i32 {
  return raw.container_menu_click_info(ctx, dp(out), out.length);
}

export function containerMenuClickResult(ctx: Ctx, item: Uint8Array): i32 {
  return raw.container_menu_click_result(ctx, dp(item), dl(item));
}

export function containerMenuClickCancel(ctx: Ctx, cancelled: bool): i32 {
  return raw.container_menu_click_cancel(ctx, cancelled ? 1 : 0);
}

export function lootTables(out: Uint8Array): i32 { return raw.loot_tables(dp(out), out.length); }

export function lootRoll(table: string, spec: Uint8Array | null, out: Uint8Array): i32 {
  const t = new Text(table);
  return raw.loot_roll(t.ptr, t.len, dp(spec), dl(spec), dp(out), out.length);
}

export function lootRollJson(json: string, spec: Uint8Array | null, out: Uint8Array): i32 {
  const j = new Text(json);
  return raw.loot_roll_json(j.ptr, j.len, dp(spec), dl(spec), dp(out), out.length);
}

export function lootNumber(json: string, asFloat: bool, seed: i64, out: Uint8Array): i32 {
  const j = new Text(json);
  return raw.loot_number(j.ptr, j.len, asFloat ? 1 : 0, seed, dp(out), out.length);
}

export function lootApply(spec: Uint8Array, out: Uint8Array): i32 {
  return raw.loot_apply(dp(spec), dl(spec), dp(out), out.length);
}

export function lootEnchant(spec: Uint8Array, out: Uint8Array): i32 {
  return raw.loot_enchant(dp(spec), dl(spec), dp(out), out.length);
}

export function tradesRoll(profession: string, level: i32, spec: Uint8Array | null, out: Uint8Array): i32 {
  const p = new Text(profession);
  return raw.trades_roll(p.ptr, p.len, level, dp(spec), dl(spec), dp(out), out.length);
}
