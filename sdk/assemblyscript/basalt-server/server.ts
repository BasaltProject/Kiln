// Server area: permissions, dynamic commands, broadcast, server info and admin calls, plugin files, storage,
// services and scheduler completions. Answers that are documents come back as an OutResult.
import { raw_server as raw } from "./raw/server";
import { raw_core_values } from "../basalt-internal/raw/core_values";
import { Args, Handler, Player, Ctx, tokenFor } from "../basalt-internal/index";
import { OutCall, OutResult, ValueWriter, callOut } from "./values";

export const BROADCAST_PLAYERS: i32 = 1;
export const BROADCAST_CONSOLE: i32 = 2;
export const PROVIDE_CHECK: i32 = 1;
export const PROVIDE_META: i32 = 2;

const SERVER_BUFFER: i32 = 1024;
const ERR_INVALID: i32 = -3;

class Text {
  readonly bytes: Uint8Array;
  constructor(text: string) { this.bytes = Uint8Array.wrap(String.UTF8.encode(text)); }
  get ptr(): usize { return changetype<usize>(this.bytes.dataStart); }
  get len(): i32 { return this.bytes.length; }
}

function dp(data: Uint8Array | null): usize { return data === null ? 0 : changetype<usize>(data.dataStart); }
function dl(data: Uint8Array | null): i32 { return data === null ? 0 : data.length; }
function packed(args: Args | null): ArrayBuffer | null { return args === null ? null : args.packed(); }
function bp(buffer: ArrayBuffer | null): usize { return buffer === null ? 0 : changetype<usize>(buffer); }
function bl(buffer: ArrayBuffer | null): i32 { return buffer === null ? 0 : buffer.byteLength; }

const CALL_RESULT: i32 = 0;
const CALL_INFO: i32 = 1;
const CALL_SERVER: i32 = 2;
const CALL_STORE: i32 = 3;
const CALL_DATA: i32 = 4;
const CALL_CONFIG: i32 = 5;
const CALL_LIST: i32 = 6;
const CALL_STAT: i32 = 7;
const CALL_WALK: i32 = 8;
const CALL_SERVICES: i32 = 9;
const CALL_PERMISSION_META: i32 = 10;
const CALL_PLACEHOLDER: i32 = 11;
const CALL_COMPLETE_INPUT: i32 = 12;

class ServerCall extends OutCall {
  constructor(
    public kind: i32,
    public number: i32,
    public a: Text,
    public b: Text,
    public doc: Uint8Array | null,
  ) {
    super();
  }

  call(out: usize, cap: i32): i32 {
    switch (this.kind) {
      case CALL_RESULT: return raw_core_values.value_result(this.number, out, cap);
      case CALL_INFO: return raw.server_info(out, cap);
      case CALL_SERVER: return raw.server_call(this.a.ptr, this.a.len, dp(this.doc), dl(this.doc), out, cap);
      case CALL_STORE: return raw.store_get(this.a.ptr, this.a.len, out, cap);
      case CALL_DATA: return raw.data_read(this.a.ptr, this.a.len, out, cap);
      case CALL_CONFIG: return raw.config_value(this.a.ptr, this.a.len, out, cap);
      case CALL_LIST: return raw.files_list(this.a.ptr, this.a.len, out, cap);
      case CALL_STAT: return raw.files_stat(this.a.ptr, this.a.len, out, cap);
      case CALL_WALK: return raw.files_walk(this.a.ptr, this.a.len, out, cap);
      case CALL_SERVICES: return raw.service_list(out, cap);
      case CALL_PERMISSION_META: return raw.permission_meta(this.a.ptr, this.a.len, this.b.ptr, this.b.len, out, cap);
      case CALL_PLACEHOLDER: return raw.placeholder_fill(this.a.ptr, this.a.len, this.number, out, cap);
      case CALL_COMPLETE_INPUT: return raw.command_complete_input(this.number, out, cap);
      default: return ERR_INVALID;
    }
  }
}

const NONE: Text = new Text("");

function run(kind: i32, number: i32, a: Text, b: Text, doc: Uint8Array | null, initial: i32): OutResult {
  return callOut(new ServerCall(kind, number, a, b, doc), initial);
}

function textOf(result: OutResult): string | null {
  return result.error == 0
    ? String.UTF8.decodeUnsafe(changetype<usize>(result.bytes.dataStart), result.bytes.length)
    : null;
}

/** The document a completion callback (a wait, a lookup, a service call or a provider query) carries. */
export function valueResult(ctx: Ctx): OutResult { return run(CALL_RESULT, ctx, NONE, NONE, null, SERVER_BUFFER); }

/** Cancels a pending wait, lookup or service call by its request id. */
export function requestCancel(request: i32): i32 { return raw_core_values.request_cancel(request); }

// ---- Permissions ----

export function permissionCheck(subject: string, node: string): i32 {
  const s = new Text(subject);
  const n = new Text(node);
  return raw.permission_check(s.ptr, s.len, n.ptr, n.len);
}

export function permissionHas(player: Player, node: string): i32 {
  const n = new Text(node);
  return raw.permission_has(player, n.ptr, n.len);
}

function permissionSet(player: Player, node: string, mode: i32): i32 {
  const n = new Text(node);
  return raw.permission_set(player, n.ptr, n.len, mode);
}

export function permissionGrant(player: Player, node: string): i32 { return permissionSet(player, node, 1); }
export function permissionDeny(player: Player, node: string): i32 { return permissionSet(player, node, 0); }
export function permissionDrop(player: Player, node: string): i32 { return permissionSet(player, node, -1); }

export function permissionMeta(subject: string, key: string): string | null {
  return textOf(run(CALL_PERMISSION_META, 0, new Text(subject), new Text(key), null, 128));
}

export function permissionProvide(priority: i32, kinds: i32, handler: Handler): i32 {
  return raw.permission_provide(priority, kinds, tokenFor(handler));
}

export function permissionProviderRemove(provider: i32): i32 { return raw.permission_provider_remove(provider); }
export function permissionAnswer(ctx: Ctx, answer: i32): i32 { return raw.permission_answer(ctx, answer); }
export function permissionChanged(): i32 { return raw.permission_changed(); }

export function permissionAnswerMeta(ctx: Ctx, text: string): i32 {
  const t = new Text(text);
  return raw.permission_answer_meta(ctx, t.ptr, t.len);
}

/** The `field` ("node", "kind", ...) a provider callback is asked about, or null. */
export function permissionQuery(ctx: Ctx, field: string): string | null {
  const value = valueResult(ctx).value();
  if (value === null) return null;
  const item = value.find(field);
  return item === null ? null : item.toStr();
}

// ---- Commands ----

/** Registers a command whose spec may carry typed arguments; `complete` is optional. */
export function commandEx(spec: string, permission: string, handler: Handler, complete: Handler | null = null): i32 {
  const s = new Text(spec);
  const p = new Text(permission);
  return raw.command_register_ex(s.ptr, s.len, p.ptr, p.len, tokenFor(handler), complete === null ? -1 : tokenFor(complete));
}

export function commandAlias(name: string, alias: string): i32 {
  const n = new Text(name);
  const a = new Text(alias);
  return raw.command_alias(n.ptr, n.len, a.ptr, a.len);
}

export function commandUnregister(name: string): i32 {
  const n = new Text(name);
  return raw.command_unregister(n.ptr, n.len);
}

/** Runs a command line as the console (player 0) or as `player`. */
export function commandRun(line: string, player: Player = 0): i32 {
  const l = new Text(line);
  return raw.command_run(l.ptr, l.len, player);
}

export function completeInput(ctx: Ctx): string | null { return textOf(run(CALL_COMPLETE_INPUT, ctx, NONE, NONE, null, 256)); }

export function completeAdd(ctx: Ctx, candidate: string): i32 {
  const c = new Text(candidate);
  return raw.command_complete_add(ctx, c.ptr, c.len);
}

export function commandArgF64(ctx: Ctx, name: string, def: f64): f64 {
  const n = new Text(name);
  const out = new ArrayBuffer(8);
  return raw.command_arg_f64(ctx, n.ptr, n.len, changetype<usize>(out)) == 0 ? load<f64>(changetype<usize>(out)) : def;
}

export function commandArgBool(ctx: Ctx, name: string, def: bool): bool {
  const n = new Text(name);
  const out = new ArrayBuffer(4);
  return raw.command_arg_bool(ctx, n.ptr, n.len, changetype<usize>(out)) == 0 ? load<i32>(changetype<usize>(out)) != 0 : def;
}

// ---- Broadcast, placeholders and server information ----

/** `text` may be an "@key" lang key; `flags` is BROADCAST_PLAYERS and/or BROADCAST_CONSOLE. */
export function broadcast(text: string, args: Args | null, flags: i32): i32 {
  const t = new Text(text);
  const a = packed(args);
  return raw.server_broadcast(t.ptr, t.len, bp(a), bl(a), flags);
}

export function placeholderFill(text: string, player: Player = 0): string | null {
  return textOf(run(CALL_PLACEHOLDER, player, new Text(text), NONE, null, 256));
}

export function serverInfo(): OutResult { return run(CALL_INFO, 0, NONE, NONE, null, SERVER_BUFFER); }

/** `args` is a finished document or null. */
export function serverCall(op: string, args: Uint8Array | null): OutResult {
  return run(CALL_SERVER, 0, new Text(op), NONE, args, SERVER_BUFFER);
}

export function profileLookup(name: string, handler: Handler): i32 {
  const n = new Text(name);
  return raw.profile_lookup(n.ptr, n.len, tokenFor(handler));
}

// ---- Plugin files, storage and config ----

/** Reads into the first `cap` bytes of `out`; the result is the file size or a negative error. */
export function fileRead(path: string, out: Uint8Array, cap: i32): i32 {
  const p = new Text(path);
  return raw.files_read(p.ptr, p.len, changetype<usize>(out.dataStart), cap);
}

export function fileWrite(path: string, data: string): i32 {
  const p = new Text(path);
  const d = new Text(data);
  return raw.files_write(p.ptr, p.len, d.ptr, d.len);
}

export function fileExists(path: string): bool {
  const p = new Text(path);
  return raw.files_exists(p.ptr, p.len) == 1;
}

export function fileDelete(path: string, recursive: bool = false): i32 {
  const p = new Text(path);
  return raw.files_delete(p.ptr, p.len, recursive ? 1 : 0);
}

export function fileList(prefix: string): OutResult { return run(CALL_LIST, 0, new Text(prefix), NONE, null, SERVER_BUFFER); }
export function fileStat(path: string): OutResult { return run(CALL_STAT, 0, new Text(path), NONE, null, SERVER_BUFFER); }
export function fileWalk(path: string): OutResult { return run(CALL_WALK, 0, new Text(path), NONE, null, SERVER_BUFFER); }

function storeSetDoc(key: string, doc: Uint8Array | null): i32 {
  if (doc === null) return ERR_INVALID;
  const k = new Text(key);
  return raw.store_set(k.ptr, k.len, dp(doc), doc.length);
}

export function storeSetInt(key: string, value: i64): i32 { return storeSetDoc(key, new ValueWriter(16).int(value).finish()); }
export function storeSetStr(key: string, value: string): i32 { return storeSetDoc(key, new ValueWriter(256).string(value).finish()); }

export function storeGet(key: string): OutResult { return run(CALL_STORE, 0, new Text(key), NONE, null, SERVER_BUFFER); }

export function dataWrite(name: string, doc: Uint8Array | null): i32 {
  if (doc === null) return ERR_INVALID;
  const n = new Text(name);
  return raw.data_write(n.ptr, n.len, dp(doc), doc.length);
}

export function dataRead(name: string): OutResult { return run(CALL_DATA, 0, new Text(name), NONE, null, SERVER_BUFFER); }

/** An empty key answers the whole config table. */
export function configValue(key: string): OutResult { return run(CALL_CONFIG, 0, new Text(key), NONE, null, SERVER_BUFFER); }

// ---- Services between plugins ----

export function serviceRegister(name: string, handler: Handler): i32 {
  const n = new Text(name);
  return raw.service_register(n.ptr, n.len, tokenFor(handler));
}

/** Answers the call a service handler runs for; a null `doc` answers null. */
export function serviceReply(ctx: Ctx, doc: Uint8Array | null): i32 { return raw.service_reply(ctx, dp(doc), dl(doc)); }

export function serviceRemove(service: i32): i32 { return raw.service_remove(service); }

export function serviceHas(name: string): bool {
  const n = new Text(name);
  return raw.service_has(n.ptr, n.len) == 1;
}

export function serviceList(): OutResult { return run(CALL_SERVICES, 0, NONE, NONE, null, SERVER_BUFFER); }

export function serviceCall(name: string, args: Uint8Array | null, timeout: i32, handler: Handler): i32 {
  const n = new Text(name);
  return raw.service_call(n.ptr, n.len, dp(args), dl(args), timeout, tokenFor(handler));
}

// ---- Scheduler completions ----

export function taskWait(ticks: i32, handler: Handler): i32 { return raw.task_wait(ticks, tokenFor(handler)); }

export function taskAt(world: string, x: f64, z: f64, handler: Handler): i32 {
  const w = new Text(world);
  return raw.task_at(w.ptr, w.len, x, z, tokenFor(handler));
}
