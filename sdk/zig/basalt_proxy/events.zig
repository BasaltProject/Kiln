//! Proxy events area (abi/areas/proxy_events.h): event fields as values, chat, command and kick decisions, plugin
//! identity, config arrays and typed command arguments. The `*Value` calls fill your buffer with the raw encoded value
//! for `values`; the result is the length the full answer needs, or a negative code.
const core = @import("core.zig");
const raw = @import("raw/proxy_events.zig").raw;
const raw_core = @import("raw/core.zig");
const shared = @import("basalt_shared").events;

pub const on = shared.on;
pub const playerOf = shared.playerOf;
pub const fieldStr = shared.fieldStr;
pub const fieldI64 = shared.fieldI64;
pub const cancel = shared.cancel;

/// Sets a field of a decided event (see basalt_proxy_abi.h for the fields each event takes).
pub fn setStr(ctx: core.Ctx, field: []const u8, value: []const u8) i32 {
    return raw_core.event_set_str(ctx, field.ptr, len(field), value.ptr, len(value));
}
pub fn setI64(ctx: core.Ctx, field: []const u8, value: i64) i32 {
    return raw_core.event_set_i64(ctx, field.ptr, len(field), value);
}

const tag_false: u8 = 1;
const tag_true: u8 = 2;
const tag_int: u8 = 3;
const tag_string: u8 = 5;

fn len(s: []const u8) i32 {
    return @intCast(s.len);
}

fn cap(out: []u8) i32 {
    return @intCast(out.len);
}

fn readLeb(bytes: []const u8, at: *usize) ?u64 {
    var result: u64 = 0;
    var shift: u7 = 0;
    while (shift < 70 and at.* < bytes.len) : (shift += 7) {
        const byte = bytes[at.*];
        at.* += 1;
        result |= @as(u64, byte & 0x7f) << @intCast(shift);
        if (byte & 0x80 == 0) return result;
    }
    return null;
}

fn writeLeb(out: []u8, at: usize, value: u64) usize {
    var rest = value;
    var n = at;
    while (true) {
        const byte: u8 = @intCast(rest & 0x7f);
        rest >>= 7;
        out[n] = if (rest != 0) byte | 0x80 else byte;
        n += 1;
        if (rest == 0) return n;
    }
}

/// Registers an observer of the finished event; a decider is registered with `on`.
pub fn watch(event: []const u8, handler: core.Handler) i32 {
    const token = core.tokenFor(handler);
    return if (token < 0) -3 else raw.event_watch(event.ptr, len(event), token);
}

pub fn eventGetValue(ctx: core.Ctx, field: []const u8, out: []u8) i32 {
    return raw.event_get(ctx, field.ptr, len(field), out.ptr, cap(out));
}

/// The text of a string field written into `out`, or null when absent or longer than `out`.
pub fn eventGetStr(ctx: core.Ctx, field: []const u8, out: []u8) ?[]const u8 {
    var scratch: [4160]u8 = undefined;
    const n = eventGetValue(ctx, field, &scratch);
    if (n < 2 or n > scratch.len or scratch[0] != tag_string) return null;
    var at: usize = 1;
    const text_len: usize = @intCast(readLeb(scratch[0..@intCast(n)], &at) orelse return null);
    if (at + text_len > @as(usize, @intCast(n)) or text_len > out.len) return null;
    @memcpy(out[0..text_len], scratch[at .. at + text_len]);
    return out[0..text_len];
}

pub fn eventGetI64(ctx: core.Ctx, field: []const u8) ?i64 {
    var scratch: [16]u8 = undefined;
    const n = eventGetValue(ctx, field, &scratch);
    if (n < 2 or n > scratch.len or scratch[0] != tag_int) return null;
    var at: usize = 1;
    const zigzag = readLeb(scratch[0..@intCast(n)], &at) orelse return null;
    const half: i64 = @intCast(zigzag >> 1);
    return if (zigzag & 1 == 1) ~half else half;
}

pub fn eventGetBool(ctx: core.Ctx, field: []const u8) ?bool {
    var scratch: [4]u8 = undefined;
    const n = eventGetValue(ctx, field, &scratch);
    if (n != 1) return null;
    return switch (scratch[0]) {
        tag_true => true,
        tag_false => false,
        else => null,
    };
}

pub fn eventFields(ctx: core.Ctx, out: []u8) i32 {
    return raw.event_fields(ctx, out.ptr, cap(out));
}

pub fn eventSetValue(ctx: core.Ctx, field: []const u8, value: []const u8) i32 {
    return raw.event_set(ctx, field.ptr, len(field), value.ptr, len(value));
}

pub fn eventSetText(ctx: core.Ctx, field: []const u8, text: []const u8) i32 {
    var buf: [4120]u8 = undefined;
    if (text.len > 4096) return -3;
    buf[0] = tag_string;
    const at = writeLeb(&buf, 1, text.len);
    @memcpy(buf[at .. at + text.len], text);
    return eventSetValue(ctx, field, buf[0 .. at + text.len]);
}

pub fn eventSetInt(ctx: core.Ctx, field: []const u8, number: i64) i32 {
    var buf: [12]u8 = undefined;
    buf[0] = tag_int;
    const zigzag: u64 = (@as(u64, @bitCast(number)) << 1) ^ @as(u64, @bitCast(number >> 63));
    const end = writeLeb(&buf, 1, zigzag);
    return eventSetValue(ctx, field, buf[0..end]);
}

pub fn eventSetFlag(ctx: core.Ctx, field: []const u8, flag: bool) i32 {
    const one = [1]u8{if (flag) tag_true else tag_false};
    return eventSetValue(ctx, field, &one);
}

/// Cancels a decided event and shows `reason` (MiniMessage, or a '@' lang key) to the player.
pub fn eventDeny(ctx: core.Ctx, reason: []const u8) i32 {
    if (reason.len > 0) {
        const shown = eventSetText(ctx, "reason", reason);
        if (shown < 0) return shown;
    }
    return eventSetFlag(ctx, "cancelled", true);
}

/// `field` is "id", "name" or "version".
pub fn pluginInfo(field: []const u8, out: []u8) ?[]const u8 {
    const n = raw.plugin_info(field.ptr, len(field), out.ptr, cap(out));
    if (n < 0 or n > out.len) return null;
    return out[0..@intCast(n)];
}

pub fn pluginManifest(out: []u8) i32 {
    return raw.plugin_manifest(out.ptr, cap(out));
}

/// The config.toml value at a dotted key (arrays are lists, tables are maps); an empty key is the whole file.
pub fn configValue(key: []const u8, out: []u8) i32 {
    return raw.config_value(key.ptr, len(key), out.ptr, cap(out));
}

pub fn commandArgF64(ctx: core.Ctx, name: []const u8) ?f64 {
    var value: f64 = 0;
    return if (raw.command_arg_f64(ctx, name.ptr, len(name), @ptrCast(&value)) == 0) value else null;
}

pub fn commandArgBool(ctx: core.Ctx, name: []const u8) ?bool {
    var value: i32 = 0;
    return if (raw.command_arg_bool(ctx, name.ptr, len(name), @ptrCast(&value)) == 0) value != 0 else null;
}

pub fn commandArgs(ctx: core.Ctx, out: []u8) i32 {
    return raw.command_args(ctx, out.ptr, cap(out));
}
