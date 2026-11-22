// Proxy admin area (abi/areas/proxy_admin.h): backends, console and chat, permission providers, groups, component JSON,
// the plugin store, data by holder and waiting. Calls that give a value answer a Value, or null on a negative code.
import { Ctx, Handler, Player, tokenFor } from "../basalt-internal/index";
import { raw_proxy_admin as raw } from "./raw/admin";
import { raw_core_values as core } from "../basalt-internal/raw/core_values";
import { OutCall, OutResult, Value, ValueWriter, callOut } from "./values";

const PA_INVALID: i32 = -3;

function paPtr(text: ArrayBuffer): usize { return changetype<usize>(text); }

class PaCompletion extends OutCall {
  constructor(private ctx: Ctx) { super(); }
  call(out: usize, cap: i32): i32 { return core.value_result(this.ctx, out, cap); }
}

class PaRequest extends OutCall {
  constructor(private ctx: Ctx) { super(); }
  call(out: usize, cap: i32): i32 { return raw.permission_request(this.ctx, out, cap); }
}

class PaMeta extends OutCall {
  constructor(private who: ArrayBuffer, private key: ArrayBuffer) { super(); }
  call(out: usize, cap: i32): i32 {
    return raw.permission_meta(paPtr(this.who), this.who.byteLength, paPtr(this.key), this.key.byteLength, 0, 0, out, cap);
  }
}

class PaList extends OutCall {
  constructor(private name: ArrayBuffer) { super(); }
  call(out: usize, cap: i32): i32 { return raw.permission_list(paPtr(this.name), this.name.byteLength, out, cap); }
}

class PaGroup extends OutCall {
  constructor(private name: ArrayBuffer) { super(); }
  call(out: usize, cap: i32): i32 { return raw.group_get(paPtr(this.name), this.name.byteLength, out, cap); }
}

class PaData extends OutCall {
  constructor(private holder: ArrayBuffer | null, private key: ArrayBuffer) { super(); }
  call(out: usize, cap: i32): i32 {
    const holder = this.holder;
    if (holder == null) return raw.store_get(paPtr(this.key), this.key.byteLength, out, cap);
    return raw.player_data_get(paPtr(holder), holder.byteLength, paPtr(this.key), this.key.byteLength, out, cap);
  }
}

class PaTop extends OutCall {
  constructor(private key: ArrayBuffer, private limit: i32) { super(); }
  call(out: usize, cap: i32): i32 { return raw.store_top(paPtr(this.key), this.key.byteLength, this.limit, out, cap); }
}

function paText(result: OutResult): string | null {
  return result.error == 0 ? String.UTF8.decode(result.bytes.buffer) : null;
}

/** The status of proxyServersRegister (0 when it worked) and the reason the proxy gave otherwise. */
export class ProxyRegistration {
  status: i32 = 0;
  reason: string = "";
}

export function proxyServersRegister(name: string, host: string, port: i32, like: string): ProxyRegistration {
  const n = String.UTF8.encode(name);
  const h = String.UTF8.encode(host);
  const l = String.UTF8.encode(like);
  const err = new Uint8Array(256);
  const result = new ProxyRegistration();
  result.status = raw.servers_register(paPtr(n), n.byteLength, paPtr(h), h.byteLength, port, paPtr(l), l.byteLength,
    changetype<usize>(err.dataStart), 256);
  if (result.status < 0) {
    let end = 0;
    while (end < 256 && err[end] != 0) end++;
    result.reason = String.UTF8.decodeUnsafe(changetype<usize>(err.dataStart), end);
  }
  return result;
}

export function proxyServersUnregister(name: string): i32 {
  const n = String.UTF8.encode(name);
  return raw.servers_unregister(paPtr(n), n.byteLength);
}

/** Pings a backend; the handler reads the answer with proxyCompletionValue. Returns the request handle. */
export function proxyServersPing(name: string, handler: Handler): i32 {
  const n = String.UTF8.encode(name);
  return raw.servers_ping(paPtr(n), n.byteLength, tokenFor(handler));
}

/** Runs the handler after `ticks` ticks of 50 ms. Returns the request handle. */
export function proxyWait(ticks: i32, handler: Handler): i32 { return raw.wait(ticks, tokenFor(handler)); }

export function proxyRequestCancel(request: i32): i32 { return core.request_cancel(request); }

export function proxyCompletionValue(ctx: Ctx): Value | null { return callOut(new PaCompletion(ctx)).value(); }

/** 1 when the line named a command, 0 when not, -4 without the capability and the operator's grant. */
export function proxyConsoleRun(line: string): i32 {
  const l = String.UTF8.encode(line);
  return raw.console_run(paPtr(l), l.byteLength);
}

export function proxyPlayerChat(player: Player, text: string): i32 {
  const t = String.UTF8.encode(text);
  return raw.player_chat(player, paPtr(t), t.byteLength);
}

export function proxyPlayerSendJson(player: Player, json: string): i32 {
  const j = String.UTF8.encode(json);
  return raw.player_send_json(player, paPtr(j), j.byteLength);
}

export function proxyPlayerKickJson(player: Player, json: string): i32 {
  const j = String.UTF8.encode(json);
  return raw.player_kick_json(player, paPtr(j), j.byteLength);
}

/** 1 granted, 0 denied, -1 undecided. `who` "" is the console; `server` may be empty. */
export function proxyPermissionCheck(who: string, node: string, server: string = ""): i32 {
  const w = String.UTF8.encode(who);
  const n = String.UTF8.encode(node);
  let context = new Uint8Array(0);
  if (server.length > 0) {
    const doc = new ValueWriter().beginMap().key("server").string(server).end().finish();
    if (doc == null) return PA_INVALID;
    context = doc;
  }
  return raw.permission_check(paPtr(w), w.byteLength, paPtr(n), n.byteLength, changetype<usize>(context.dataStart),
    context.length);
}

export function proxyPermissionMeta(who: string, key: string): string | null {
  return paText(callOut(new PaMeta(String.UTF8.encode(who), String.UTF8.encode(key))));
}

/** Registers provider callbacks (null for none); returns the provider handle. */
export function proxyPermissionProvide(check: Handler | null, meta: Handler | null, priority: i32 = 0): i32 {
  const checkToken = check == null ? 0 : tokenFor(check);
  const metaToken = meta == null ? 0 : tokenFor(meta);
  return raw.permission_provide(checkToken, metaToken, priority);
}

export function proxyPermissionUnprovide(provider: i32): i32 { return raw.permission_unprovide(provider); }

/** The question of a provider callback: a map with kind, name, uuid, console, text and context. */
export function proxyPermissionRequest(ctx: Ctx): Value | null { return callOut(new PaRequest(ctx)).value(); }

/** 1 grants, 0 denies, negative leaves it to the next provider. */
export function proxyPermissionAnswer(ctx: Ctx, answer: i32): i32 { return raw.permission_answer(ctx, answer); }

export function proxyPermissionAnswerText(ctx: Ctx, text: string): i32 {
  const t = String.UTF8.encode(text);
  return raw.permission_answer_text(ctx, paPtr(t), t.byteLength);
}

export function proxyPermissionChanged(): i32 { return raw.permission_changed(); }

/** 1 grants, 0 denies, -1 removes. */
export function proxyPermissionOverride(name: string, node: string, value: i32): i32 {
  const n = String.UTF8.encode(name);
  const o = String.UTF8.encode(node);
  return raw.permission_override(paPtr(n), n.byteLength, paPtr(o), o.byteLength, value);
}

export function proxyPermissionList(name: string): Value | null {
  return callOut(new PaList(String.UTF8.encode(name))).value();
}

export function proxyGroupGet(name: string): string | null {
  return paText(callOut(new PaGroup(String.UTF8.encode(name)), 64));
}

export function proxyGroupSet(name: string, group: string): i32 {
  const n = String.UTF8.encode(name);
  const g = String.UTF8.encode(group);
  return raw.group_set(paPtr(n), n.byteLength, paPtr(g), g.byteLength);
}

/** Reads a store key (holder null) or a player data key of `holder`; null when unset. */
export function proxyDataValue(holder: string | null, key: string): Value | null {
  const h: ArrayBuffer | null = holder == null ? null : String.UTF8.encode(holder);
  return callOut(new PaData(h, String.UTF8.encode(key))).value();
}

export function proxyDataSetValue(holder: string | null, key: string, doc: Uint8Array | null): i32 {
  if (doc == null || doc.length == 0) return PA_INVALID;
  const k = String.UTF8.encode(key);
  const d = changetype<usize>(doc.dataStart);
  if (holder == null) return raw.store_set(paPtr(k), k.byteLength, d, doc.length);
  const h = String.UTF8.encode(holder);
  return raw.player_data_set(paPtr(h), h.byteLength, paPtr(k), k.byteLength, d, doc.length);
}

export function proxyDataSetInt(holder: string | null, key: string, number: i64): i32 {
  return proxyDataSetValue(holder, key, new ValueWriter().int(number).finish());
}

export function proxyDataSetFloat(holder: string | null, key: string, number: f64): i32 {
  return proxyDataSetValue(holder, key, new ValueWriter().float(number).finish());
}

export function proxyDataSetBool(holder: string | null, key: string, flag: bool): i32 {
  return proxyDataSetValue(holder, key, new ValueWriter().boolean(flag).finish());
}

export function proxyDataSetStr(holder: string | null, key: string, text: string): i32 {
  return proxyDataSetValue(holder, key, new ValueWriter().string(text).finish());
}

export function proxyDataRemove(holder: string | null, key: string): i32 {
  return proxyDataSetValue(holder, key, new ValueWriter().nullValue().finish());
}

/** The best `limit` numeric values of a store key: a list of maps with uuid, name and value. */
export function proxyStoreTop(key: string, limit: i32): Value | null {
  return callOut(new PaTop(String.UTF8.encode(key), limit)).value();
}
