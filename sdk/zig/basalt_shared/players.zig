//! Online players by name and the messages sent to them.
const core = @import("core.zig");
const raw = @import("raw/core.zig");

pub fn byName(username: []const u8) core.Player {
    return raw.player_by_name(username.ptr, core.len(username));
}
/// Fills `out` with online players and returns how many it holds; the first `out.len` when more are online.
pub fn online(out: []core.Player) usize {
    const n = raw.players_online(out.ptr, @intCast(out.len));
    if (n < 0) return 0;
    if (@as(usize, @intCast(n)) <= out.len) return @intCast(n);
    var spill: [core.spill_handles]core.Player = undefined;
    const total = raw.players_online(&spill, spill.len);
    if (total < 0 or total > spill.len) return 0;
    const kept = @min(@as(usize, @intCast(total)), out.len);
    @memcpy(out[0..kept], spill[0..kept]);
    return kept;
}
pub fn name(player: core.Player, out: []u8) ?[]const u8 {
    return core.view(out, raw.player_name(player, out.ptr, core.cap(out)));
}
pub fn send(player: core.Player, text: []const u8, args: ?*const core.Args) i32 {
    return raw.player_send(player, text.ptr, core.len(text), core.argsPtr(args), core.argsLen(args));
}
pub fn actionBar(player: core.Player, text: []const u8, args: ?*const core.Args) i32 {
    return raw.player_action_bar(player, text.ptr, core.len(text), core.argsPtr(args), core.argsLen(args));
}
