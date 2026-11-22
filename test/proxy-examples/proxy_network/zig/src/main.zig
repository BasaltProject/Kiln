const std = @import("std");
const basalt = @import("basalt_proxy");
const net = basalt.network;
const values = basalt.values;

var rewrite_buffer: [64]u8 = undefined;
var cancel_buffer: [64]u8 = undefined;
var channel_buffer: [64]u8 = undefined;
var rewrite_marker: []const u8 = "";
var cancel_marker: []const u8 = "";
var channel: []const u8 = "";
var refused_once = false;

fn infoWith(prefix: []const u8, parts: []const []const u8) void {
    var line: [256]u8 = undefined;
    var n: usize = 0;
    for (&[_][]const u8{prefix}) |p| {
        @memcpy(line[n .. n + p.len], p);
        n += p.len;
    }
    for (parts) |p| {
        const room = @min(p.len, line.len - n);
        @memcpy(line[n .. n + room], p[0..room]);
        n += room;
    }
    basalt.log.info(line[0..n]);
}

fn openMap(status: i32, buffer: []u8) ?values.Value {
    if (status < 0 or status > buffer.len) return null;
    return values.Value.open(buffer[0..@intCast(status)]);
}

fn textOf(map: values.Value, key: []const u8) []const u8 {
    const found = map.find(key) orelse return "";
    return found.asString() orelse "";
}

fn chatBody(out: []u8, text: []const u8) usize {
    out[0] = 0x08;
    out[1] = @intCast(text.len >> 8);
    out[2] = @intCast(text.len & 0xff);
    @memcpy(out[3 .. 3 + text.len], text);
    out[3 + text.len] = 0;
    return text.len + 4;
}

fn stringPrefixed(out: []u8, first: []const u8, rest: []const u8) usize {
    out[0] = @intCast(first.len);
    @memcpy(out[1 .. 1 + first.len], first);
    @memcpy(out[1 + first.len .. 1 + first.len + rest.len], rest);
    return 1 + first.len + rest.len;
}

fn onMessage(ctx: basalt.core.Ctx) void {
    var buffer: [1024]u8 = undefined;
    const map = openMap(net.messageGet(ctx, &buffer), &buffer) orelse {
        basalt.log.err("message read failed");
        return;
    };
    const data = if (map.find("data")) |d| d.asBytes() orelse "" else "";
    infoWith("net: message ", &[_][]const u8{ textOf(map, "source"), " ", textOf(map, "channel"), " ", data });
    const player = basalt.events.playerOf(ctx);
    if (player > 0 and net.messageSend(player, "basalt:demo", "pong", true) == 0) {
        basalt.log.info("net: reply sent");
    }
}

fn onDecider(ctx: basalt.core.Ctx) void {
    var info: [512]u8 = undefined;
    var body: [2048]u8 = undefined;
    const map = openMap(net.packetGet(ctx, &info), &info) orelse {
        basalt.log.err("packet read failed");
        return;
    };
    const n = net.packetBody(ctx, &body);
    if (n < 0 or n > body.len) return;
    const bytes = body[0..@intCast(n)];
    if (std.mem.indexOf(u8, bytes, cancel_marker) != null) {
        if (net.packetCancel(ctx) == 0) infoWith("net: cancel for ", &[_][]const u8{textOf(map, "player_name")});
    } else if (std.mem.indexOf(u8, bytes, rewrite_marker)) |at| {
        for (body[at .. at + rewrite_marker.len]) |*c| {
            if (c.* >= 'a' and c.* <= 'z') c.* -= 32;
        }
        if (net.packetSetBody(ctx, bytes) == 0) infoWith("net: rewrite for ", &[_][]const u8{textOf(map, "player_name")});
    }
}

fn onWatch(ctx: basalt.core.Ctx) void {
    var info: [512]u8 = undefined;
    const map = openMap(net.packetGet(ctx, &info), &info) orelse return;
    if (!refused_once and net.packetCancel(ctx) < 0) {
        refused_once = true;
        basalt.log.info("net: observer cancel refused");
    }
    const cancelled = if (map.find("cancelled")) |c| c.asBool() orelse false else false;
    if (cancelled) basalt.log.info("net: watch saw a cancelled packet");
}

fn onAntibot(ctx: basalt.core.Ctx) void {
    var buffer: [512]u8 = undefined;
    const map = openMap(net.antibotEventGet(ctx, &buffer), &buffer) orelse return;
    infoWith("net: antibot ", &[_][]const u8{ textOf(map, "kind"), " ", textOf(map, "action") });
}

fn idle(_: basalt.core.Ctx) void {}

fn onJoin(ctx: basalt.core.Ctx) void {
    const player = basalt.events.playerOf(ctx);
    if (player <= 0) return;
    var body: [256]u8 = undefined;
    var n = chatBody(&body, "net welcome");
    if (net.packetSend(player, "clientbound.system_chat", body[0..n]) == 0) basalt.log.info("net: sent welcome");
    if (net.packetSend(player, "serverbound.play.chat_command", body[0..n]) < 0) basalt.log.info("net: wrong direction refused");
    if (net.messageSend(player, channel, "hello", true) == 0) basalt.log.info("net: sent hello message");
    n = stringPrefixed(&body, channel, "ping");
    if (net.packetReceive(player, "serverbound.play.custom_payload", body[0..n]) == 0) basalt.log.info("net: injected custom payload");
    n = stringPrefixed(&body, "list", "");
    if (net.packetReceive(player, "serverbound.play.chat_command", body[0..n]) == 0) basalt.log.info("net: injected command");
}

fn onEnable(_: basalt.core.Ctx) void {
    var buffer: [8192]u8 = undefined;
    if (openMap(net.packetNames(&buffer), &buffer)) |names| {
        if ((names.count() orelse 0) > 100) basalt.log.info("net: packets known");
    }
    if (openMap(net.packetResolve("clientbound.system_chat", &buffer), &buffer)) |map| {
        const serverbound = if (map.find("serverbound")) |s| s.asBool() orelse true else true;
        if (!serverbound) basalt.log.info("net: resolved a clientbound play packet");
    }
    if (net.packetResolve("nope.nothing", &buffer) < 0) basalt.log.info("net: unknown packet refused");
    if (net.channelRegister(channel) == 0) basalt.log.info("net: channel registered");
    if (net.channelRegister("BungeeCord") < 0) basalt.log.info("net: bungee channel refused");
    if (net.channelRegister("basalt:temp") == 0 and net.channelUnregister("basalt:temp") == 0 and net.channelUnregister("basalt:temp") < 0) {
        basalt.log.info("net: temp channel unregistered");
    }
    _ = net.messageOn("basalt:demo", onMessage);
    var storage: [96]u8 = undefined;
    var opts = values.Writer.init(&storage);
    opts.beginMap();
    opts.key("priority");
    opts.string("late");
    opts.key("fail");
    opts.string("closed");
    opts.end();
    const encoded = opts.finish() catch &[_]u8{};
    if (net.packetOn("clientbound.system_chat", encoded, onDecider) > 0) basalt.log.info("net: decider listening");
    _ = net.packetWatch("clientbound.system_chat", &[_]u8{}, onWatch);
    const listener = net.packetOn("serverbound.play.*", &[_]u8{}, idle);
    if (listener > 0 and net.packetRemove(listener) == 0 and net.packetRemove(listener) < 0) basalt.log.info("net: listener removed");
    if (net.packetOn("nope.nothing", &[_]u8{}, idle) < 0) basalt.log.info("net: bad listener name refused");
    if (net.packetGet(9999, &buffer) < 0) basalt.log.info("net: bad context refused");
    _ = net.antibotOn(onAntibot);
    _ = net.antibotVerify("NetBot");
    basalt.log.info(if (net.antibotIsVerified("NetBot") == 1) "net: verified yes" else "net: verified no");
    _ = net.antibotUnverify("NetBot");
    basalt.log.info(if (net.antibotIsVerified("NetBot") == 0) "net: unverified" else "net: still verified");
    if (net.antibotBan("203.0.113.9") == 0) basalt.log.info("net: ban ok");
    if (net.antibotUnban("203.0.113.9") == 0) basalt.log.info("net: unban ok");
    if (net.antibotUnban("203.0.113.9") < 0) basalt.log.info("net: unban absent");
    if (net.antibotBan("not an address") < 0) basalt.log.info("net: bad address refused");
    basalt.log.info(if (net.antibotUnderAttack() == 0) "net: no attack" else "net: under attack");
}

pub fn init() void {
    rewrite_marker = basalt.config.getStr("rewrite-marker", &rewrite_buffer) orelse "";
    cancel_marker = basalt.config.getStr("cancel-marker", &cancel_buffer) orelse "";
    channel = basalt.config.getStr("channel", &channel_buffer) orelse "";
    _ = basalt.events.on("plugin.enable", basalt.core.Priority.normal, onEnable);
    _ = basalt.events.on("player.join", basalt.core.Priority.normal, onJoin);
}
