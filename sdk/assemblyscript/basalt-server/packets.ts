// Plugin channels and raw packets (abi/areas/packets.h). Calls return the import result: a count or handle, or a
// negative ABI code. The out-calls return an OutResult: `error` is 0 or the negative code, `bytes` the answer, and
// `value()` opens it as a document.
import { Ctx, Handler, Player, tokenFor } from "../basalt-internal/index";
import { raw_packets as raw } from "./raw/packets";
import { OutCall, OutResult, ValueWriter, callOut } from "./values";

const PACKETS_INITIAL: i32 = 1024;

function pkU(text: string): ArrayBuffer { return String.UTF8.encode(text); }
function pkP(buffer: ArrayBuffer): usize { return changetype<usize>(buffer); }
function pkB(bytes: Uint8Array): usize { return changetype<usize>(bytes.dataStart); }

export function channelListen(name: string, handler: Handler): i32 {
  const n = pkU(name);
  return raw.channel_listen(pkP(n), n.byteLength, tokenFor(handler));
}

export function channelSend(player: Player, name: string, body: Uint8Array): i32 {
  const n = pkU(name);
  return raw.channel_send(player, pkP(n), n.byteLength, pkB(body), body.length);
}

/** `priority` is "first", "early", "normal", "late", "last" or null for the default. */
export function packetListen(pattern: string, priority: string | null, observe: bool, handler: Handler): i32 {
  let document = new Uint8Array(0);
  if (priority !== null || observe) {
    const writer = new ValueWriter().beginMap();
    if (priority !== null) writer.key("priority").string(priority);
    if (observe) writer.key("observe").boolean(true);
    const built = writer.end().finish();
    if (built === null) return -3;
    document = built;
  }
  const p = pkU(pattern);
  return raw.packet_listen(pkP(p), p.byteLength, pkB(document), document.length, tokenFor(handler));
}

export function packetUnlisten(listener: i32): i32 { return raw.packet_unlisten(listener); }

export function packetSend(player: Player, name: string, body: Uint8Array, intercept: bool): i32 {
  const n = pkU(name);
  return raw.packet_send(player, pkP(n), n.byteLength, pkB(body), body.length, intercept ? 1 : 0);
}

export function packetReceive(player: Player, name: string, body: Uint8Array, intercept: bool): i32 {
  const n = pkU(name);
  return raw.packet_receive(player, pkP(n), n.byteLength, pkB(body), body.length, intercept ? 1 : 0);
}

class LookupCall extends OutCall {
  constructor(private name: ArrayBuffer, private names: bool) { super(); }
  call(out: usize, cap: i32): i32 {
    return this.names
      ? raw.packet_names(pkP(this.name), this.name.byteLength, out, cap)
      : raw.packet_lookup(pkP(this.name), this.name.byteLength, out, cap);
  }
}

export function packetLookup(name: string): OutResult { return callOut(new LookupCall(pkU(name), false), PACKETS_INITIAL); }

export function packetNames(prefix: string): OutResult { return callOut(new LookupCall(pkU(prefix), true), PACKETS_INITIAL); }

class ActionsCall extends OutCall {
  call(out: usize, cap: i32): i32 { return raw.packet_player_actions(out, cap); }
}

export function packetPlayerActions(): OutResult { return callOut(new ActionsCall(), PACKETS_INITIAL); }

/** `args` is an encoded list value. */
export function packetPlayerReceive(player: Player, action: string, args: Uint8Array): i32 {
  const a = pkU(action);
  return raw.packet_player_receive(player, pkP(a), a.byteLength, pkB(args), args.length);
}

class CtxCall extends OutCall {
  constructor(private ctx: Ctx, private body: bool) { super(); }
  call(out: usize, cap: i32): i32 {
    return this.body ? raw.packet_ctx_body(this.ctx, out, cap) : raw.packet_ctx_info(this.ctx, out, cap);
  }
}

export function packetCtxInfo(ctx: Ctx): OutResult { return callOut(new CtxCall(ctx, false), PACKETS_INITIAL); }

export function packetCtxBody(ctx: Ctx): OutResult { return callOut(new CtxCall(ctx, true), PACKETS_INITIAL); }

export function packetCtxPlayer(ctx: Ctx): Player { return raw.packet_ctx_player(ctx); }

export function packetCtxCancel(ctx: Ctx): i32 { return raw.packet_ctx_cancel(ctx); }

export function packetCtxSetBody(ctx: Ctx, body: Uint8Array): i32 {
  return raw.packet_ctx_set_body(ctx, pkB(body), body.length);
}

export function packetCtxSend(ctx: Ctx, name: string, body: Uint8Array, intercept: bool): i32 {
  const n = pkU(name);
  return raw.packet_ctx_send(ctx, pkP(n), n.byteLength, pkB(body), body.length, intercept ? 1 : 0);
}

export function packetCtxReceive(ctx: Ctx, name: string, body: Uint8Array, intercept: bool): i32 {
  const n = pkU(name);
  return raw.packet_ctx_receive(ctx, pkP(n), n.byteLength, pkB(body), body.length, intercept ? 1 : 0);
}
