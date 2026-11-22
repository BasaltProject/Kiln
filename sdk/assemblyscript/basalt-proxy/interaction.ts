// Proxy interaction area (abi/areas/proxy_interaction.h): dialogs, resource packs, cookies, transfer and extra player
// operations. Observed events and completions are read with proxyValueResult and the Value reader.
import { Args, Ctx, Handler, Player, tokenFor } from "../basalt-internal/index";
import { raw_core_values as core } from "../basalt-internal/raw/core_values";
import { raw_proxy_interaction as raw } from "./raw/interaction";
import { OutCall, Value, ValueWriter, callOut } from "./values";

/** A dialog button: the label and the command the client runs as the player when it is clicked. */
export class DialogButton {
  constructor(public label: string, public command: string) {}
}

class ResultCall extends OutCall {
  constructor(private ctx: Ctx) { super(); }
  call(out: usize, cap: i32): i32 { return core.value_result(this.ctx, out, cap); }
}

class SettingsCall extends OutCall {
  constructor(private player: Player) { super(); }
  call(out: usize, cap: i32): i32 { return raw.player_settings(this.player, out, cap); }
}

function fetchText(call: OutCall): string | null {
  const result = callOut(call);
  if (result.error != 0) return null;
  return String.UTF8.decodeUnsafe(changetype<usize>(result.bytes.dataStart), result.bytes.length);
}

class PackCall extends OutCall {
  constructor(private player: Player, private doc: Uint8Array) { super(); }
  call(out: usize, cap: i32): i32 {
    return raw.resource_pack_push(this.player, changetype<usize>(this.doc.dataStart), this.doc.length, out, cap);
  }
}

class AddressCall extends OutCall {
  constructor(private player: Player) { super(); }
  call(out: usize, cap: i32): i32 { return raw.player_address(this.player, out, cap); }
}

/** Observes player.resource_pack: the value holds player, player_name, id and status. */
export function proxyWatchResourcePacks(handler: Handler): i32 {
  const n = String.UTF8.encode("player.resource_pack");
  return raw.interaction_watch(changetype<usize>(n), n.byteLength, tokenFor(handler));
}

/** Observes player.settings: the value holds player, player_name and settings. */
export function proxyWatchSettings(handler: Handler): i32 {
  const n = String.UTF8.encode("player.settings");
  return raw.interaction_watch(changetype<usize>(n), n.byteLength, tokenFor(handler));
}

/** The value of an observed event or a completion, or null. */
export function proxyValueResult(ctx: Ctx): Value | null {
  return callOut(new ResultCall(ctx)).value();
}

/** kind is "notice", "confirmation" or "multi_action"; body may be null. */
export function proxyDialogShow(player: Player, kind: string, title: string, body: string | null, buttons: Array<DialogButton>, closable: bool): i32 {
  const w = new ValueWriter();
  w.beginMap().key("kind").string(kind).key("title").string(title);
  if (body !== null) w.key("body").string(body);
  w.key("buttons").beginList();
  for (let i = 0; i < buttons.length; i++) {
    w.beginMap().key("label").string(buttons[i].label).key("command").string(buttons[i].command).end();
  }
  w.end().key("can_close_with_escape").boolean(closable).end();
  const doc = w.finish();
  if (doc === null) return -3;
  return raw.dialog_show(player, changetype<usize>(doc.dataStart), doc.length);
}

export function proxyDialogClear(player: Player): i32 {
  return raw.dialog_clear(player);
}

/** Pushes a pack and returns its id text, or null when it failed; hash, prompt and id may be null. */
export function proxyResourcePackPush(player: Player, url: string, hash: string | null, required: bool, prompt: string | null, id: string | null): string | null {
  const w = new ValueWriter();
  w.beginMap().key("url").string(url);
  if (hash !== null) w.key("hash").string(hash);
  w.key("required").boolean(required);
  if (prompt !== null) w.key("prompt").string(prompt);
  if (id !== null) w.key("id").string(id);
  w.end();
  const doc = w.finish();
  if (doc === null) return null;
  return fetchText(new PackCall(player, doc));
}

export function proxyResourcePackRemove(player: Player, id: string): i32 {
  const i = String.UTF8.encode(id);
  return raw.resource_pack_remove(player, changetype<usize>(i), i.byteLength);
}

export function proxyResourcePackClear(player: Player): i32 {
  return raw.resource_pack_clear(player);
}

export function proxyCookieSet(player: Player, key: string, data: Uint8Array): i32 {
  const k = String.UTF8.encode(key);
  return raw.cookie_set(player, changetype<usize>(k), k.byteLength, changetype<usize>(data.dataStart), data.length);
}

/** Returns the request handle; the handler reads the cookie with proxyCookieResult. */
export function proxyCookieRequest(player: Player, key: string, handler: Handler): i32 {
  const k = String.UTF8.encode(key);
  return raw.cookie_request(player, changetype<usize>(k), k.byteLength, tokenFor(handler));
}

/** The cookie bytes of a completion, or null when the client had none. */
export function proxyCookieResult(ctx: Ctx): Uint8Array | null {
  const value = proxyValueResult(ctx);
  return value === null ? null : value.toBytes();
}

export function proxyRequestCancel(request: i32): i32 {
  return core.request_cancel(request);
}

export function proxyPlayerTransfer(player: Player, host: string, port: i32): i32 {
  const h = String.UTF8.encode(host);
  return raw.player_transfer(player, changetype<usize>(h), h.byteLength, port);
}

export function proxyPlayersByUuid(uuid: string): Player {
  const u = String.UTF8.encode(uuid);
  return raw.players_by_uuid(changetype<usize>(u), u.byteLength);
}

/** Players on the proxy, or on `server` when it is not empty. */
export function proxyPlayersCount(server: string): i32 {
  const s = String.UTF8.encode(server);
  return raw.players_count(changetype<usize>(s), s.byteLength);
}

export function proxyPlayersBroadcast(text: string, args: Args | null): i32 {
  const t = String.UTF8.encode(text);
  const a: ArrayBuffer | null = args === null ? null : args.packed();
  if (a === null) return raw.players_broadcast(changetype<usize>(t), t.byteLength, 0, 0);
  return raw.players_broadcast(changetype<usize>(t), t.byteLength, changetype<usize>(a), a.byteLength);
}

export function proxyPlayerAddress(player: Player): string | null {
  return fetchText(new AddressCall(player));
}

/** The client port, a negative code when unknown. */
export function proxyPlayerPort(player: Player): i32 {
  return raw.player_port(player);
}

/** The client settings map, or null until the client sent it. */
export function proxyPlayerSettings(player: Player): Value | null {
  return callOut(new SettingsCall(player)).value();
}

export function proxyPlayerDataGetF64(player: Player, key: string, fallback: f64): f64 {
  const k = String.UTF8.encode(key);
  return raw.player_data_get_f64(player, changetype<usize>(k), k.byteLength, fallback);
}

export function proxyPlayerDataSetF64(player: Player, key: string, value: f64): i32 {
  const k = String.UTF8.encode(key);
  return raw.player_data_set_f64(player, changetype<usize>(k), k.byteLength, value);
}
