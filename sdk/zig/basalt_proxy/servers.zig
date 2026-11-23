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
/// Fills `out` with the players on a backend and returns how many it holds; the first `out.len` when more are there.
pub fn players(name: []const u8, out: []core.Player) usize {
    const n = raw.players_on(name.ptr, core.len(name), out.ptr, @intCast(out.len));
    if (n < 0) return 0;
    if (@as(usize, @intCast(n)) <= out.len) return @intCast(n);
    var spill: [core.spill_handles]core.Player = undefined;
    const total = raw.players_on(name.ptr, core.len(name), &spill, spill.len);
    if (total < 0 or total > spill.len) return 0;
    const kept = @min(@as(usize, @intCast(total)), out.len);
    @memcpy(out[0..kept], spill[0..kept]);
    return kept;
}
