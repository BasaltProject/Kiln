//! The backends the proxy knows and who is on each.
const core = @import("basalt_shared").core;
const raw = @import("raw/core.zig");

/// The backend names, one per line.
pub fn list(out: []u8) ?[]const u8 {
    return core.view(out, raw.servers_list(out.ptr, core.cap(out)));
}
pub fn address(name: []const u8, out: []u8) ?[]const u8 {
    return core.view(out, raw.server_address(name.ptr, core.len(name), out.ptr, core.cap(out)));
}
pub fn players(name: []const u8, out: []core.Player) usize {
    const n = raw.players_on(name.ptr, core.len(name), out.ptr, @intCast(out.len));
    return if (n < 0) 0 else @min(@as(usize, @intCast(n)), out.len);
}
