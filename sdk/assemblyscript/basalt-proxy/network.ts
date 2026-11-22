import * as rawProxy from "./raw/core";
import { Args, Player, Ctx, Handler } from "../basalt-internal/core";
import { tokenFor } from "../basalt-internal/handlers";
import { ap, finish, lenOf, ptrOf, u } from "../basalt-internal/buffers";
// Proxy network area (abi/areas/proxy_network.h): messaging channels, raw packets and antibot control. The calls that
// answer a value return its raw encoded bytes for the values reader, or null on a negative code.
import { Ctx, Handler, tokenFor } from "../basalt-internal/index";
import { OutCall, callOut } from "./values";
import { raw_proxy_network as raw } from "./raw/network";

function netText(text: string): ArrayBuffer {
  return String.UTF8.encode(text);
}

function netPtr(data: Uint8Array): usize {
  return data.length == 0 ? 0 : changetype<usize>(data.dataStart);
}

class NetCall extends OutCall {
  constructor(
    readonly kind: i32,
    readonly ctx: i32,
    readonly name: ArrayBuffer,
  ) {
    super();
  }

  call(out: usize, cap: i32): i32 {
    if (this.kind == 0) return raw.network_message_get(this.ctx, out, cap);
    if (this.kind == 1) return raw.packet_resolve(changetype<usize>(this.name), this.name.byteLength, out, cap);
    if (this.kind == 2) return raw.packet_names(out, cap);
    if (this.kind == 3) return raw.packet_get(this.ctx, out, cap);
    if (this.kind == 4) return raw.packet_body(this.ctx, out, cap);
    return raw.antibot_event_get(this.ctx, out, cap);
  }
}

function netFetch(kind: i32, ctx: i32, name: ArrayBuffer): Uint8Array | null {
  const result = callOut(new NetCall(kind, ctx, name));
  return result.error == 0 ? result.bytes : null;
}

export function netChannelRegister(name: string): i32 {
  const n = netText(name);
  return raw.network_channel_register(changetype<usize>(n), n.byteLength);
}

export function netChannelUnregister(name: string): i32 {
  const n = netText(name);
  return raw.network_channel_unregister(changetype<usize>(n), n.byteLength);
}

/** Observes the messages of the plugin's channels; an empty filter takes all of them. */
export function netMessageOn(filter: string, handler: Handler): i32 {
  const f = netText(filter);
  return raw.network_message_on(changetype<usize>(f), f.byteLength, tokenFor(handler));
}

/** The encoded map of the message (player_name, player_uuid, channel, data, source, server). */
export function netMessageGet(ctx: Ctx): Uint8Array | null {
  return netFetch(0, ctx, new ArrayBuffer(0));
}

/** Sends data on a channel; towardClient false goes to the player's backend, true to the client. */
export function netMessageSend(player: i32, channel: string, data: Uint8Array, towardClient: bool): i32 {
  const c = netText(channel);
  return raw.network_message_send(player, changetype<usize>(c), c.byteLength, netPtr(data), data.length, towardClient ? 1 : 0);
}

export function netPacketResolve(name: string): Uint8Array | null {
  const n = netText(name);
  return netFetch(1, 0, n);
}

export function netPacketNames(): Uint8Array | null {
  return netFetch(2, 0, new ArrayBuffer(0));
}

/** A decider listener; opts is an encoded value map (priority, fail) or empty. Returns the listener. */
export function netPacketOn(name: string, opts: Uint8Array, handler: Handler): i32 {
  const n = netText(name);
  return raw.packet_on(changetype<usize>(n), n.byteLength, netPtr(opts), opts.length, tokenFor(handler));
}

/** An observer listener: it sees the final packet and cannot cancel or change it. */
export function netPacketWatch(name: string, opts: Uint8Array, handler: Handler): i32 {
  const n = netText(name);
  return raw.packet_watch(changetype<usize>(n), n.byteLength, netPtr(opts), opts.length, tokenFor(handler));
}

export function netPacketRemove(listener: i32): i32 {
  return raw.packet_remove(listener);
}

export function netPacketGet(ctx: Ctx): Uint8Array | null {
  return netFetch(3, ctx, new ArrayBuffer(0));
}

export function netPacketBody(ctx: Ctx): Uint8Array | null {
  return netFetch(4, ctx, new ArrayBuffer(0));
}

export function netPacketSetBody(ctx: Ctx, body: Uint8Array): i32 {
  return raw.packet_set_body(ctx, netPtr(body), body.length);
}

export function netPacketCancel(ctx: Ctx): i32 {
  return raw.packet_cancel(ctx);
}

export function netPacketInject(ctx: Ctx, name: string, body: Uint8Array, intercept: bool): i32 {
  const n = netText(name);
  return raw.packet_inject(ctx, changetype<usize>(n), n.byteLength, netPtr(body), body.length, intercept ? 1 : 0);
}

export function netPacketSend(player: i32, name: string, body: Uint8Array): i32 {
  const n = netText(name);
  return raw.packet_send(player, changetype<usize>(n), n.byteLength, netPtr(body), body.length);
}

export function netPacketReceive(player: i32, name: string, body: Uint8Array): i32 {
  const n = netText(name);
  return raw.packet_receive(player, changetype<usize>(n), n.byteLength, netPtr(body), body.length);
}

export function netAntibotVerify(name: string): i32 {
  const n = netText(name);
  return raw.antibot_verify(changetype<usize>(n), n.byteLength);
}

export function netAntibotUnverify(name: string): i32 {
  const n = netText(name);
  return raw.antibot_unverify(changetype<usize>(n), n.byteLength);
}

export function netAntibotIsVerified(name: string): i32 {
  const n = netText(name);
  return raw.antibot_is_verified(changetype<usize>(n), n.byteLength);
}

export function netAntibotBan(address: string): i32 {
  const a = netText(address);
  return raw.antibot_ban(changetype<usize>(a), a.byteLength);
}

export function netAntibotUnban(address: string): i32 {
  const a = netText(address);
  return raw.antibot_unban(changetype<usize>(a), a.byteLength);
}

export function netAntibotUnderAttack(): i32 {
  return raw.antibot_under_attack();
}

export function netAntibotOn(handler: Handler): i32 {
  return raw.antibot_on(tokenFor(handler));
}

export function netAntibotEventGet(ctx: Ctx): Uint8Array | null {
  return netFetch(5, ctx, new ArrayBuffer(0));
}

export function serversList(): string[] {
  let buf = new ArrayBuffer(1024);
  let n = rawProxy.servers_list(changetype<usize>(buf), 1024);
  if (n > 1024) {
    buf = new ArrayBuffer(n);
    n = rawProxy.servers_list(changetype<usize>(buf), n);
  }
  const text = finish(buf, n);
  const names: string[] = [];
  if (text === null) return names;
  const parts = text.split("\n");
  for (let i = 0; i < parts.length; i++) if (parts[i].length > 0) names.push(parts[i]);
  return names;
}

export function serverAddress(name: string): string | null {
  const nm = u(name);
  let buf = new ArrayBuffer(256);
  let n = rawProxy.server_address(nm.ptr, nm.len, changetype<usize>(buf), 256);
  if (n > 256) {
    buf = new ArrayBuffer(n);
    n = rawProxy.server_address(nm.ptr, nm.len, changetype<usize>(buf), n);
  }
  return finish(buf, n);
}

export function playersOn(server: string): Player[] {
  const s = u(server);
  let cap = 64;
  while (true) {
    const out = new Int32Array(cap);
    const count = rawProxy.players_on(s.ptr, s.len, changetype<usize>(out.buffer), cap);
    if (count < 0) return [];
    if (count <= cap) {
      const players: Player[] = [];
      for (let i = 0; i < count; i++) players.push(out[i]);
      return players;
    }
    cap = count;
  }
}
