//! Plugin channels and raw packets (abi/areas/packets.h). Calls return the import result: a count or handle, or a
//! negative ABI code. The out-calls fill your buffer and return the length the full answer needs; open the slice with
//! `values.Value.open` for the value ones.
const core = @import("core.zig");
const raw = @import("raw/packets.zig").raw;
const values = @import("basalt_shared").values;

fn len(s: []const u8) i32 {
    return @intCast(s.len);
}

fn cap(out: []u8) i32 {
    return @intCast(out.len);
}

fn flag(value: bool) i32 {
    return if (value) 1 else 0;
}

pub fn channelListen(name: []const u8, handler: core.Handler) i32 {
    const token = core.tokenFor(handler);
    return if (token < 0) -3 else raw.channel_listen(name.ptr, len(name), token);
}

pub fn channelSend(player: core.Player, name: []const u8, body: []const u8) i32 {
    return raw.channel_send(player, name.ptr, len(name), body.ptr, len(body));
}

/// `priority` is "first", "early", "normal", "late", "last" or null for the default.
pub fn packetListen(pattern: []const u8, priority: ?[]const u8, observe: bool, handler: core.Handler) i32 {
    var buffer: [96]u8 = undefined;
    var document: []const u8 = buffer[0..0];
    if (priority != null or observe) {
        var writer = values.Writer.init(&buffer);
        writer.beginMap();
        if (priority) |text| {
            writer.key("priority");
            writer.string(text);
        }
        if (observe) {
            writer.key("observe");
            writer.boolean(true);
        }
        writer.end();
        document = writer.finish() catch return -3;
    }
    const token = core.tokenFor(handler);
    return if (token < 0) -3 else raw.packet_listen(pattern.ptr, len(pattern), document.ptr, len(document), token);
}

pub fn packetUnlisten(listener: i32) i32 {
    return raw.packet_unlisten(listener);
}

pub fn packetSend(player: core.Player, name: []const u8, body: []const u8, intercept: bool) i32 {
    return raw.packet_send(player, name.ptr, len(name), body.ptr, len(body), flag(intercept));
}

pub fn packetReceive(player: core.Player, name: []const u8, body: []const u8, intercept: bool) i32 {
    return raw.packet_receive(player, name.ptr, len(name), body.ptr, len(body), flag(intercept));
}

pub fn packetLookup(name: []const u8, out: []u8) i32 {
    return raw.packet_lookup(name.ptr, len(name), out.ptr, cap(out));
}

pub fn packetNames(prefix: []const u8, out: []u8) i32 {
    return raw.packet_names(prefix.ptr, len(prefix), out.ptr, cap(out));
}

pub fn packetPlayerActions(out: []u8) i32 {
    return raw.packet_player_actions(out.ptr, cap(out));
}

/// `args` is an encoded list value.
pub fn packetPlayerReceive(player: core.Player, action: []const u8, args: []const u8) i32 {
    return raw.packet_player_receive(player, action.ptr, len(action), args.ptr, len(args));
}

pub fn packetCtxInfo(ctx: core.Ctx, out: []u8) i32 {
    return raw.packet_ctx_info(ctx, out.ptr, cap(out));
}

pub fn packetCtxBody(ctx: core.Ctx, out: []u8) i32 {
    return raw.packet_ctx_body(ctx, out.ptr, cap(out));
}

pub fn packetCtxPlayer(ctx: core.Ctx) core.Player {
    return raw.packet_ctx_player(ctx);
}

pub fn packetCtxCancel(ctx: core.Ctx) i32 {
    return raw.packet_ctx_cancel(ctx);
}

pub fn packetCtxSetBody(ctx: core.Ctx, body: []const u8) i32 {
    return raw.packet_ctx_set_body(ctx, body.ptr, len(body));
}

pub fn packetCtxSend(ctx: core.Ctx, name: []const u8, body: []const u8, intercept: bool) i32 {
    return raw.packet_ctx_send(ctx, name.ptr, len(name), body.ptr, len(body), flag(intercept));
}

pub fn packetCtxReceive(ctx: core.Ctx, name: []const u8, body: []const u8, intercept: bool) i32 {
    return raw.packet_ctx_receive(ctx, name.ptr, len(name), body.ptr, len(body), flag(intercept));
}
