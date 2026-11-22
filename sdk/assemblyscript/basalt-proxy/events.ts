import * as rawProxy from "./raw/core";
import { Args, Player, Ctx, Handler } from "../basalt-internal/core";
import { tokenFor } from "../basalt-internal/handlers";
import { ap, finish, lenOf, ptrOf, u } from "../basalt-internal/buffers";
// Proxy events area (abi/areas/proxy_events.h): event fields as values, chat, command and kick decisions, plugin
// identity, config arrays and typed command arguments. The *Value calls return the raw encoded value for the values
// reader, or null on a negative code.
import { Ctx, Handler, tokenFor } from "../basalt-internal/index";
import { raw_proxy_events as raw } from "./raw/events";

const PE_FALSE: u8 = 1;
const PE_TRUE: u8 = 2;
const PE_INT: u8 = 3;
const PE_STRING: u8 = 5;

function peReadLeb(bytes: Uint8Array, at: StaticArray<i32>): i64 {
  let result: i64 = 0;
  for (let shift = 0; shift < 70 && at[0] < bytes.length; shift += 7) {
    const b = <i32>bytes[at[0]++];
    result |= (<i64>(b & 0x7f)) << shift;
    if ((b & 0x80) == 0) return result;
  }
  return -1;
}

function peWriteLeb(out: Array<u8>, first: u64): void {
  let value = first;
  do {
    const b = <u8>(value & 0x7f);
    value >>= 7;
    out.push(value != 0 ? b | 0x80 : b);
  } while (value != 0);
}

function peBytes(values: Array<u8>): ArrayBuffer {
  const out = new Uint8Array(values.length);
  for (let i = 0; i < values.length; i++) out[i] = values[i];
  return out.buffer;
}

/** Registers an observer of the finished event; a decider is registered with `on`. */
export function proxyWatch(event: string, handler: Handler): i32 {
  const e = String.UTF8.encode(event);
  return raw.event_watch(changetype<usize>(e), e.byteLength, tokenFor(handler));
}

/** The encoded value of a field; an empty name gives a map of every readable field. */
export function proxyEventGetValue(ctx: Ctx, field: string): Uint8Array | null {
  const f = String.UTF8.encode(field);
    let cap = 256;
  while (true) {
    const buffer = new ArrayBuffer(cap);
    const needed = raw.event_get(ctx, changetype<usize>(f), f.byteLength, changetype<usize>(buffer), cap);
    if (needed < 0) return null;
    if (needed <= cap) return Uint8Array.wrap(buffer, 0, needed);
    cap = needed;
  }
  return null;
}

export function proxyEventGetStr(ctx: Ctx, field: string): string | null {
  const bytes = proxyEventGetValue(ctx, field);
  if (bytes === null || bytes.length < 2 || bytes[0] != PE_STRING) return null;
  const at = new StaticArray<i32>(1);
  at[0] = 1;
  const length = peReadLeb(bytes, at);
  if (length < 0 || <i64>at[0] + length > <i64>bytes.length) return null;
  return String.UTF8.decodeUnsafe(changetype<usize>(bytes.buffer) + bytes.byteOffset + at[0], <i32>length);
}

export function proxyEventGetI64(ctx: Ctx, field: string, def: i64): i64 {
  const bytes = proxyEventGetValue(ctx, field);
  if (bytes === null || bytes.length < 2 || bytes[0] != PE_INT) return def;
  const at = new StaticArray<i32>(1);
  at[0] = 1;
  const zigzag = <u64>peReadLeb(bytes, at);
  return <i64>(zigzag >> 1) ^ -(<i64>(zigzag & 1));
}

export function proxyEventGetBool(ctx: Ctx, field: string, def: bool): bool {
  const bytes = proxyEventGetValue(ctx, field);
  if (bytes === null || bytes.length != 1 || (bytes[0] != PE_TRUE && bytes[0] != PE_FALSE)) return def;
  return bytes[0] == PE_TRUE;
}

/** A list of maps (name, type, readable, writable), one per field of the event. */
export function proxyEventFields(ctx: Ctx): Uint8Array | null {
    let cap = 256;
  while (true) {
    const buffer = new ArrayBuffer(cap);
    const needed = raw.event_fields(ctx, changetype<usize>(buffer), cap);
    if (needed < 0) return null;
    if (needed <= cap) return Uint8Array.wrap(buffer, 0, needed);
    cap = needed;
  }
  return null;
}

export function proxyEventSetValue(ctx: Ctx, field: string, value: ArrayBuffer): i32 {
  const f = String.UTF8.encode(field);
  return raw.event_set(ctx, changetype<usize>(f), f.byteLength, changetype<usize>(value), value.byteLength);
}

export function proxyEventSetText(ctx: Ctx, field: string, text: string): i32 {
  const t = String.UTF8.encode(text);
  const out = new Array<u8>();
  out.push(PE_STRING);
  peWriteLeb(out, <u64>t.byteLength);
  const view = Uint8Array.wrap(t);
  for (let i = 0; i < view.length; i++) out.push(view[i]);
  return proxyEventSetValue(ctx, field, peBytes(out));
}

export function proxyEventSetInt(ctx: Ctx, field: string, value: i64): i32 {
  const out = new Array<u8>();
  out.push(PE_INT);
  peWriteLeb(out, (<u64>value << 1) ^ <u64>(value >> 63));
  return proxyEventSetValue(ctx, field, peBytes(out));
}

export function proxyEventSetFlag(ctx: Ctx, field: string, flag: bool): i32 {
  const out = new Array<u8>();
  out.push(flag ? PE_TRUE : PE_FALSE);
  return proxyEventSetValue(ctx, field, peBytes(out));
}

/** Cancels a decided event and shows `reason` (MiniMessage, or a '@' lang key) to the player. */
export function proxyEventDeny(ctx: Ctx, reason: string): i32 {
  if (reason.length > 0) {
    const shown = proxyEventSetText(ctx, "reason", reason);
    if (shown < 0) return shown;
  }
  return proxyEventSetFlag(ctx, "cancelled", true);
}

/** `field` is "id", "name" or "version". */
export function proxyPluginInfo(field: string): string | null {
  const f = String.UTF8.encode(field);
  let cap = 128;
  while (true) {
    const buffer = new ArrayBuffer(cap);
    const needed = raw.plugin_info(changetype<usize>(f), f.byteLength, changetype<usize>(buffer), cap);
    if (needed < 0) return null;
    if (needed <= cap) return String.UTF8.decodeUnsafe(changetype<usize>(buffer), needed);
    cap = needed;
  }
  return null;
}

export function proxyPluginManifest(): Uint8Array | null {
    let cap = 256;
  while (true) {
    const buffer = new ArrayBuffer(cap);
    const needed = raw.plugin_manifest(changetype<usize>(buffer), cap);
    if (needed < 0) return null;
    if (needed <= cap) return Uint8Array.wrap(buffer, 0, needed);
    cap = needed;
  }
  return null;
}

/** The config.toml value at a dotted key (arrays are lists, tables are maps); an empty key is the whole file. */
export function proxyConfigValue(key: string): Uint8Array | null {
  const k = String.UTF8.encode(key);
    let cap = 256;
  while (true) {
    const buffer = new ArrayBuffer(cap);
    const needed = raw.config_value(changetype<usize>(k), k.byteLength, changetype<usize>(buffer), cap);
    if (needed < 0) return null;
    if (needed <= cap) return Uint8Array.wrap(buffer, 0, needed);
    cap = needed;
  }
  return null;
}

export function proxyCommandArgF64(ctx: Ctx, name: string, def: f64): f64 {
  const n = String.UTF8.encode(name);
  const out = new ArrayBuffer(8);
  return raw.command_arg_f64(ctx, changetype<usize>(n), n.byteLength, changetype<usize>(out)) == 0 ? load<f64>(changetype<usize>(out)) : def;
}

export function proxyCommandArgBool(ctx: Ctx, name: string, def: bool): bool {
  const n = String.UTF8.encode(name);
  const out = new ArrayBuffer(4);
  return raw.command_arg_bool(ctx, changetype<usize>(n), n.byteLength, changetype<usize>(out)) == 0 ? load<i32>(changetype<usize>(out)) != 0 : def;
}

export function proxyCommandArgs(ctx: Ctx): Uint8Array | null {
    let cap = 256;
  while (true) {
    const buffer = new ArrayBuffer(cap);
    const needed = raw.command_args(ctx, changetype<usize>(buffer), cap);
    if (needed < 0) return null;
    if (needed <= cap) return Uint8Array.wrap(buffer, 0, needed);
    cap = needed;
  }
  return null;
}

export * from "../basalt-internal/events";

export function eventSetStr(ctx: Ctx, field: string, value: string): i32 {
  const f = u(field);
  const v = u(value);
  return rawProxy.event_set_str(ctx, f.ptr, f.len, v.ptr, v.len);
}

export function eventSetI64(ctx: Ctx, field: string, value: i64): i32 {
  const f = u(field);
  return rawProxy.event_set_i64(ctx, f.ptr, f.len, value);
}
