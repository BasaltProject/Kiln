//! Proxy network area (abi/areas/proxy_network.h): messaging channels, raw packets and antibot control. The calls that
//! fill your buffer give the raw encoded value for `values`; the result is the length the full answer needs, or a
//! negative code.
const core = @import("core.zig");
const raw = @import("raw/proxy_network.zig").raw;

fn len(s: []const u8) i32 {
    return @intCast(s.len);
}

fn cap(out: []u8) i32 {
    return @intCast(out.len);
}

fn token(handler: core.Handler) i32 {
    const t = core.tokenFor(handler);
    return if (t < 0) -3 else t;
}

pub fn channelRegister(name: []const u8) i32 {
    return raw.network_channel_register(name.ptr, len(name));
}

pub fn channelUnregister(name: []const u8) i32 {
    return raw.network_channel_unregister(name.ptr, len(name));
}

/// Observes the messages of the plugin's channels; an empty filter takes all of them.
pub fn messageOn(filter: []const u8, handler: core.Handler) i32 {
    const t = token(handler);
    return if (t < 0) t else raw.network_message_on(filter.ptr, len(filter), t);
}

pub fn messageGet(ctx: core.Ctx, out: []u8) i32 {
    return raw.network_message_get(ctx, out.ptr, cap(out));
}

/// `toward_client` false sends to the player's backend, true to the client.
pub fn messageSend(player: core.Player, channel: []const u8, data: []const u8, toward_client: bool) i32 {
    return raw.network_message_send(player, channel.ptr, len(channel), data.ptr, len(data), if (toward_client) 1 else 0);
}

pub fn packetResolve(name: []const u8, out: []u8) i32 {
    return raw.packet_resolve(name.ptr, len(name), out.ptr, cap(out));
}

pub fn packetNames(out: []u8) i32 {
    return raw.packet_names(out.ptr, cap(out));
}

/// A decider listener; `opts` is an encoded value map (priority, fail) or empty. Returns the listener.
pub fn packetOn(name: []const u8, opts: []const u8, handler: core.Handler) i32 {
    const t = token(handler);
    return if (t < 0) t else raw.packet_on(name.ptr, len(name), opts.ptr, len(opts), t);
}

/// An observer listener: it sees the final packet and cannot cancel or change it.
pub fn packetWatch(name: []const u8, opts: []const u8, handler: core.Handler) i32 {
    const t = token(handler);
    return if (t < 0) t else raw.packet_watch(name.ptr, len(name), opts.ptr, len(opts), t);
}

pub fn packetRemove(listener: i32) i32 {
    return raw.packet_remove(listener);
}

pub fn packetGet(ctx: core.Ctx, out: []u8) i32 {
    return raw.packet_get(ctx, out.ptr, cap(out));
}

pub fn packetBody(ctx: core.Ctx, out: []u8) i32 {
    return raw.packet_body(ctx, out.ptr, cap(out));
}

pub fn packetSetBody(ctx: core.Ctx, body: []const u8) i32 {
    return raw.packet_set_body(ctx, body.ptr, len(body));
}

pub fn packetCancel(ctx: core.Ctx) i32 {
    return raw.packet_cancel(ctx);
}

pub fn packetInject(ctx: core.Ctx, name: []const u8, body: []const u8, intercept: bool) i32 {
    return raw.packet_inject(ctx, name.ptr, len(name), body.ptr, len(body), if (intercept) 1 else 0);
}

pub fn packetSend(player: core.Player, name: []const u8, body: []const u8) i32 {
    return raw.packet_send(player, name.ptr, len(name), body.ptr, len(body));
}

pub fn packetReceive(player: core.Player, name: []const u8, body: []const u8) i32 {
    return raw.packet_receive(player, name.ptr, len(name), body.ptr, len(body));
}

pub fn antibotVerify(name: []const u8) i32 {
    return raw.antibot_verify(name.ptr, len(name));
}

pub fn antibotUnverify(name: []const u8) i32 {
    return raw.antibot_unverify(name.ptr, len(name));
}

pub fn antibotIsVerified(name: []const u8) i32 {
    return raw.antibot_is_verified(name.ptr, len(name));
}

pub fn antibotBan(address: []const u8) i32 {
    return raw.antibot_ban(address.ptr, len(address));
}

pub fn antibotUnban(address: []const u8) i32 {
    return raw.antibot_unban(address.ptr, len(address));
}

pub fn antibotUnderAttack() i32 {
    return raw.antibot_under_attack();
}

pub fn antibotOn(handler: core.Handler) i32 {
    const t = token(handler);
    return if (t < 0) t else raw.antibot_on(t);
}

pub fn antibotEventGet(ctx: core.Ctx, out: []u8) i32 {
    return raw.antibot_event_get(ctx, out.ptr, cap(out));
}
