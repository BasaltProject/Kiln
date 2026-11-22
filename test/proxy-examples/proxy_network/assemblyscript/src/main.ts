import * as bsConfig from "basalt-proxy/config";
import * as bsCore from "basalt-proxy/core";
import * as bsEvents from "basalt-proxy/events";
import * as bsLog from "basalt-proxy/log";
import * as bsNetwork from "basalt-proxy/network";
import * as bsValues from "basalt-proxy/values";

let rewriteMarker = "";
let cancelMarker = "";
let channel = "";
let refusedOnce = false;

function utf8(s: string): Uint8Array {
  return Uint8Array.wrap(String.UTF8.encode(s));
}

function find(hay: Uint8Array, needle: string): i32 {
  const n = utf8(needle);
  if (n.length == 0) return -1;
  for (let at = 0; at + n.length <= hay.length; at++) {
    let i = 0;
    while (i < n.length && hay[at + i] == n[i]) i++;
    if (i == n.length) return at;
  }
  return -1;
}

function text(map: bsValues.Value, key: string): string {
  const v = map.find(key);
  if (v === null) return "";
  const s = v.toStr();
  return s === null ? "" : s;
}

function open(doc: Uint8Array | null): bsValues.Value | null {
  return doc === null ? null : bsValues.Value.open(doc);
}

function chatBody(s: string): Uint8Array {
  const t = utf8(s);
  const o = new Uint8Array(t.length + 4);
  o[0] = 0x08;
  o[1] = <u8>(t.length >> 8);
  o[2] = <u8>t.length;
  o.set(t, 3);
  return o;
}

function stringPrefixed(first: string, rest: string): Uint8Array {
  const a = utf8(first);
  const b = utf8(rest);
  const o = new Uint8Array(1 + a.length + b.length);
  o[0] = <u8>a.length;
  o.set(a, 1);
  o.set(b, 1 + a.length);
  return o;
}

function onMessage(ctx: bsCore.Ctx): void {
  const map = open(bsNetwork.netMessageGet(ctx));
  if (map === null) {
    bsLog.logError("message read failed");
    return;
  }
  const d = map.find("data");
  let data = "";
  if (d !== null) {
    const raw = d.toBytes();
    if (raw !== null) data = String.UTF8.decodeUnsafe(changetype<usize>(raw.dataStart), raw.length);
  }
  bsLog.logInfo("net: message " + text(map, "source") + " " + text(map, "channel") + " " + data);
  const player = bsEvents.eventPlayer(ctx);
  if (player > 0 && bsNetwork.netMessageSend(player, "basalt:demo", utf8("pong"), true) == 0) bsLog.logInfo("net: reply sent");
}

function onDecider(ctx: bsCore.Ctx): void {
  const map = open(bsNetwork.netPacketGet(ctx));
  const body = bsNetwork.netPacketBody(ctx);
  if (map === null || body === null) {
    bsLog.logError("packet read failed");
    return;
  }
  const at = find(body, rewriteMarker);
  if (find(body, cancelMarker) >= 0) {
    if (bsNetwork.netPacketCancel(ctx) == 0) bsLog.logInfo("net: cancel for " + text(map, "player_name"));
  } else if (at >= 0) {
    const edited = body.slice(0);
    const len = utf8(rewriteMarker).length;
    for (let k = 0; k < len; k++) {
      if (edited[at + k] >= 97 && edited[at + k] <= 122) edited[at + k] = edited[at + k] - 32;
    }
    if (bsNetwork.netPacketSetBody(ctx, edited) == 0) bsLog.logInfo("net: rewrite for " + text(map, "player_name"));
  }
}

function onWatch(ctx: bsCore.Ctx): void {
  const map = open(bsNetwork.netPacketGet(ctx));
  if (map === null) return;
  if (!refusedOnce && bsNetwork.netPacketCancel(ctx) < 0) {
    refusedOnce = true;
    bsLog.logInfo("net: observer cancel refused");
  }
  const c = map.find("cancelled");
  if (c !== null && c.toBool(false)) bsLog.logInfo("net: watch saw a cancelled packet");
}

function onAntibot(ctx: bsCore.Ctx): void {
  const map = open(bsNetwork.netAntibotEventGet(ctx));
  if (map !== null) bsLog.logInfo("net: antibot " + text(map, "kind") + " " + text(map, "action"));
}

function idle(ctx: bsCore.Ctx): void {}

function onJoin(ctx: bsCore.Ctx): void {
  const player = bsEvents.eventPlayer(ctx);
  if (player <= 0) return;
  const body = chatBody("net welcome");
  if (bsNetwork.netPacketSend(player, "clientbound.system_chat", body) == 0) bsLog.logInfo("net: sent welcome");
  if (bsNetwork.netPacketSend(player, "serverbound.play.chat_command", body) < 0) bsLog.logInfo("net: wrong direction refused");
  if (bsNetwork.netMessageSend(player, channel, utf8("hello"), true) == 0) bsLog.logInfo("net: sent hello message");
  if (bsNetwork.netPacketReceive(player, "serverbound.play.custom_payload", stringPrefixed(channel, "ping")) == 0) {
    bsLog.logInfo("net: injected custom payload");
  }
  if (bsNetwork.netPacketReceive(player, "serverbound.play.chat_command", stringPrefixed("list", "")) == 0) {
    bsLog.logInfo("net: injected command");
  }
}

function onEnable(ctx: bsCore.Ctx): void {
  const names = open(bsNetwork.netPacketNames());
  if (names !== null && names.count() > 100) bsLog.logInfo("net: packets known");
  const resolved = open(bsNetwork.netPacketResolve("clientbound.system_chat"));
  if (resolved !== null) {
    const sb = resolved.find("serverbound");
    if (sb !== null && sb.kind() == bsValues.ValueKind.Bool && !sb.toBool(true)) {
      bsLog.logInfo("net: resolved a clientbound play packet");
    }
  }
  if (bsNetwork.netPacketResolve("nope.nothing") === null) bsLog.logInfo("net: unknown packet refused");
  if (bsNetwork.netChannelRegister(channel) == 0) bsLog.logInfo("net: channel registered");
  if (bsNetwork.netChannelRegister("BungeeCord") < 0) bsLog.logInfo("net: bungee channel refused");
  if (bsNetwork.netChannelRegister("basalt:temp") == 0 && bsNetwork.netChannelUnregister("basalt:temp") == 0 &&
      bsNetwork.netChannelUnregister("basalt:temp") < 0) {
    bsLog.logInfo("net: temp channel unregistered");
  }
  bsNetwork.netMessageOn("basalt:demo", onMessage);
  const opts = new bsValues.ValueWriter().beginMap().key("priority").string("late").key("fail").string("closed").end().finish();
  if (bsNetwork.netPacketOn("clientbound.system_chat", opts === null ? new Uint8Array(0) : opts, onDecider) > 0) {
    bsLog.logInfo("net: decider listening");
  }
  bsNetwork.netPacketWatch("clientbound.system_chat", new Uint8Array(0), onWatch);
  const listener = bsNetwork.netPacketOn("serverbound.play.*", new Uint8Array(0), idle);
  if (listener > 0 && bsNetwork.netPacketRemove(listener) == 0 && bsNetwork.netPacketRemove(listener) < 0) {
    bsLog.logInfo("net: listener removed");
  }
  if (bsNetwork.netPacketOn("nope.nothing", new Uint8Array(0), idle) < 0) bsLog.logInfo("net: bad listener name refused");
  if (bsNetwork.netPacketGet(9999) === null) bsLog.logInfo("net: bad context refused");
  bsNetwork.netAntibotOn(onAntibot);
  bsNetwork.netAntibotVerify("NetBot");
  bsLog.logInfo(bsNetwork.netAntibotIsVerified("NetBot") == 1 ? "net: verified yes" : "net: verified no");
  bsNetwork.netAntibotUnverify("NetBot");
  bsLog.logInfo(bsNetwork.netAntibotIsVerified("NetBot") == 0 ? "net: unverified" : "net: still verified");
  if (bsNetwork.netAntibotBan("203.0.113.9") == 0) bsLog.logInfo("net: ban ok");
  if (bsNetwork.netAntibotUnban("203.0.113.9") == 0) bsLog.logInfo("net: unban ok");
  if (bsNetwork.netAntibotUnban("203.0.113.9") < 0) bsLog.logInfo("net: unban absent");
  if (bsNetwork.netAntibotBan("not an address") < 0) bsLog.logInfo("net: bad address refused");
  bsLog.logInfo(bsNetwork.netAntibotUnderAttack() == 0 ? "net: no attack" : "net: under attack");
}

export function init(): void {
  const a = bsConfig.configStr("rewrite-marker");
  const b = bsConfig.configStr("cancel-marker");
  const c = bsConfig.configStr("channel");
  rewriteMarker = a !== null ? a : "";
  cancelMarker = b !== null ? b : "";
  channel = c !== null ? c : "";
  bsEvents.on("plugin.enable", bsCore.NORMAL, onEnable);
  bsEvents.on("player.join", bsCore.NORMAL, onJoin);
}
