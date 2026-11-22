// Events area for AssemblyScript: strings and finished documents (Uint8Array) over the raw imports, answers as
// Value (null when a call failed).
import { raw_events } from "./raw/events";
import { raw_core_values } from "../basalt-internal/raw/core_values";
import { Args, Handler, tokenFor } from "../basalt-internal/index";
import { OutCall, OutResult, Value, callOut } from "./values";

export const EVENT_POST: i32 = 1;
export const EVENT_CANCELLED: i32 = 2;

class EventsText {
  constructor(public buffer: ArrayBuffer) {}
  get ptr(): usize { return changetype<usize>(this.buffer); }
  get len(): i32 { return this.buffer.byteLength; }
}

function t(text: string): EventsText {
  return new EventsText(String.UTF8.encode(text));
}

function dataPtr(data: Uint8Array | null): usize {
  return data === null ? 0 : changetype<usize>(data.dataStart);
}

function dataLen(data: Uint8Array | null): i32 {
  return data === null ? 0 : data.length;
}

function argsPtr(args: Args | null): usize {
  if (args === null) return 0;
  const packed = args.packed();
  return packed === null ? 0 : changetype<usize>(packed);
}

function argsLen(args: Args | null): i32 {
  if (args === null) return 0;
  const packed = args.packed();
  return packed === null ? 0 : packed.byteLength;
}

const CALL_EVENT_NAME: i32 = 0;
const CALL_EVENT_GET: i32 = 1;
const CALL_EVENT_FIELDS: i32 = 2;
const CALL_EVENT_PAYLOAD: i32 = 3;
const CALL_EVENT_DESCRIBE: i32 = 4;
const CALL_EVENT_NAMES: i32 = 5;
const CALL_EVENT_CANCEL_MESSAGE: i32 = 6;
const CALL_PLAYER_UUID: i32 = 7;
const CALL_PLAYER_GET: i32 = 8;
const CALL_PLAYER_CALL: i32 = 9;
const CALL_PLAYER_DESCRIBE: i32 = 10;
const CALL_PLAYER_GAMEMODE: i32 = 11;
const CALL_RESULT: i32 = 12;

class EventsCall extends OutCall {
  constructor(
    public kind: i32,
    public id: i32,
    public name: EventsText,
    public args: Uint8Array | null,
  ) {
    super();
  }

  call(out: usize, cap: i32): i32 {
    switch (this.kind) {
      case CALL_EVENT_NAME: return raw_events.event_name(this.id, out, cap);
      case CALL_EVENT_GET: return raw_events.event_get(this.id, this.name.ptr, this.name.len, out, cap);
      case CALL_EVENT_FIELDS: return raw_events.event_fields(this.id, out, cap);
      case CALL_EVENT_PAYLOAD: return raw_events.event_payload(this.id, out, cap);
      case CALL_EVENT_DESCRIBE: return raw_events.event_describe(this.name.ptr, this.name.len, out, cap);
      case CALL_EVENT_NAMES: return raw_events.event_names(out, cap);
      case CALL_EVENT_CANCEL_MESSAGE: return raw_events.event_cancel_message(this.id, out, cap);
      case CALL_PLAYER_UUID: return raw_events.player_uuid(this.id, out, cap);
      case CALL_PLAYER_GET: return raw_events.player_get(this.id, this.name.ptr, this.name.len, out, cap);
      case CALL_PLAYER_CALL:
        return raw_events.player_call(this.id, this.name.ptr, this.name.len, dataPtr(this.args), dataLen(this.args), out, cap);
      case CALL_PLAYER_DESCRIBE: return raw_events.player_describe(out, cap);
      case CALL_PLAYER_GAMEMODE: return raw_events.player_gamemode(this.id, out, cap);
      default: return raw_core_values.value_result(this.id, out, cap);
    }
  }
}

const NO_NAME = new EventsText(new ArrayBuffer(0));

function text(kind: i32, id: i32, capacity: i32): string | null {
  const result: OutResult = callOut(new EventsCall(kind, id, NO_NAME, null), capacity);
  return result.error == 0 ? String.UTF8.decodeUnsafe(changetype<usize>(result.bytes.dataStart), result.bytes.length) : null;
}

function value(kind: i32, id: i32, name: string, args: Uint8Array | null, initial: i32): Value | null {
  return callOut(new EventsCall(kind, id, t(name), args), initial).value();
}

/** Listens to `name`; `filters` is a finished map document or null. */
export function eventOnEx(name: string, priority: i32, flags: i32, filters: Uint8Array | null, handler: Handler): i32 {
  const n = t(name);
  return raw_events.event_on_ex(n.ptr, n.len, priority, flags, dataPtr(filters), dataLen(filters), tokenFor(handler));
}

export function eventName(ctx: i32): string | null { return text(CALL_EVENT_NAME, ctx, 64); }

export function eventGet(ctx: i32, field: string, initial: i32): Value | null {
  return value(CALL_EVENT_GET, ctx, field, null, initial);
}

export function eventSet(ctx: i32, field: string, doc: Uint8Array): i32 {
  const f = t(field);
  return raw_events.event_set(ctx, f.ptr, f.len, dataPtr(doc), dataLen(doc));
}

export function eventFields(ctx: i32, initial: i32): Value | null {
  return value(CALL_EVENT_FIELDS, ctx, "", null, initial);
}

export function eventPayload(ctx: i32, initial: i32): Value | null {
  return value(CALL_EVENT_PAYLOAD, ctx, "", null, initial);
}

export function eventDescribe(name: string, initial: i32): Value | null {
  return value(CALL_EVENT_DESCRIBE, 0, name, null, initial);
}

export function eventNames(initial: i32): Value | null {
  return value(CALL_EVENT_NAMES, 0, "", null, initial);
}

export function eventCancelled(ctx: i32): i32 { return raw_events.event_cancelled(ctx); }

export function eventCancelWith(ctx: i32, message: string, args: Args | null = null): i32 {
  const m = t(message);
  return raw_events.event_cancel_with(ctx, m.ptr, m.len, argsPtr(args), argsLen(args));
}

export function eventCancelMessage(ctx: i32): string | null { return text(CALL_EVENT_CANCEL_MESSAGE, ctx, 256); }

export function eventIsPost(ctx: i32): i32 { return raw_events.event_is_post(ctx); }

export function eventEmit(name: string, doc: Uint8Array): i32 {
  const n = t(name);
  return raw_events.event_emit(n.ptr, n.len, dataPtr(doc), dataLen(doc));
}

export function playerByUuid(uuid: string): i32 {
  const u = t(uuid);
  return raw_events.player_by_uuid(u.ptr, u.len);
}

export function playerUuid(player: i32): string | null { return text(CALL_PLAYER_UUID, player, 64); }

export function playerGet(player: i32, name: string, initial: i32): Value | null {
  return value(CALL_PLAYER_GET, player, name, null, initial);
}

export function playerSet(player: i32, name: string, doc: Uint8Array): i32 {
  const n = t(name);
  return raw_events.player_set(player, n.ptr, n.len, dataPtr(doc), dataLen(doc));
}

/** Runs a named operation; `args` is a finished list document or null. */
export function playerCall(player: i32, op: string, args: Uint8Array | null, initial: i32): Value | null {
  return value(CALL_PLAYER_CALL, player, op, args, initial);
}

export function playerDescribe(initial: i32): Value | null {
  return value(CALL_PLAYER_DESCRIBE, 0, "", null, initial);
}

/** The health in half hearts, NaN when the call failed. */
export function playerHealth(player: i32): f64 {
  const box = new ArrayBuffer(8);
  return raw_events.player_health(player, changetype<usize>(box)) < 0 ? NaN : load<f64>(changetype<usize>(box));
}

export function playerSetHealth(player: i32, health: f64): i32 { return raw_events.player_set_health(player, health); }

export function playerGamemode(player: i32): string | null { return text(CALL_PLAYER_GAMEMODE, player, 16); }

export function playerSetGamemode(player: i32, mode: string): i32 {
  const m = t(mode);
  return raw_events.player_set_gamemode(player, m.ptr, m.len);
}

export function playerTeleport(player: i32, world: string, x: f64, y: f64, z: f64, yaw: f32, pitch: f32): i32 {
  const w = t(world);
  return raw_events.player_teleport(player, w.ptr, w.len, x, y, z, yaw, pitch);
}

export function playerTitle(player: i32, title: string, subtitle: string, fadeIn: i32, stay: i32, fadeOut: i32, args: Args | null = null): i32 {
  const a = t(title);
  const s = t(subtitle);
  return raw_events.player_title(player, a.ptr, a.len, s.ptr, s.len, fadeIn, stay, fadeOut, argsPtr(args), argsLen(args));
}

export function playerKick(player: i32, message: string, args: Args | null = null): i32 {
  const m = t(message);
  return raw_events.player_kick(player, m.ptr, m.len, argsPtr(args), argsLen(args));
}

export function playerEffect(player: i32, effect: string, ticks: i32, amplifier: i32): i32 {
  const e = t(effect);
  return raw_events.player_effect(player, e.ptr, e.len, ticks, amplifier);
}

export function playerOpenInventory(player: i32, kind: string, spec: Uint8Array): i32 {
  const k = t(kind);
  return raw_events.player_open_inventory(player, k.ptr, k.len, dataPtr(spec), dataLen(spec));
}

export function playerCloseWindow(player: i32): i32 { return raw_events.player_close_window(player); }

/** Shows a form (a finished document); the handler runs once with the answer, read with completionValue. */
export function formOpen(player: i32, form: Uint8Array, handler: Handler): i32 {
  return raw_events.form_open(player, dataPtr(form), dataLen(form), tokenFor(handler));
}

export function formClose(player: i32): i32 { return raw_events.form_close(player); }

export function dialogShow(player: i32, dialog: string, handler: Handler): i32 {
  const d = t(dialog);
  return raw_events.dialog_show(player, d.ptr, d.len, tokenFor(handler));
}

export function dialogClear(player: i32): i32 { return raw_events.dialog_clear(player); }

export function completionValue(ctx: i32, initial: i32): Value | null {
  return value(CALL_RESULT, ctx, "", null, initial);
}

export * from "../basalt-internal/events";
