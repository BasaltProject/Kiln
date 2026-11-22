//! Players connected to the proxy: lookup, messages, their backend, settings, permissions, titles, moves and kicks.
const shared = @import("basalt_shared");
const core = shared.core;
const raw = @import("raw/core.zig");

pub const byName = shared.players.byName;
pub const online = shared.players.online;
pub const name = shared.players.name;
pub const send = shared.players.send;
pub const actionBar = shared.players.actionBar;

pub fn uuid(player: core.Player, out: []u8) ?[]const u8 {
    return core.view(out, raw.player_uuid(player, out.ptr, core.cap(out)));
}
/// The backend the player is on now.
pub fn server(player: core.Player, out: []u8) ?[]const u8 {
    return core.view(out, raw.player_server(player, out.ptr, core.cap(out)));
}
pub fn ping(player: core.Player) i32 {
    return raw.player_ping(player);
}
pub fn settingStr(player: core.Player, field: []const u8, out: []u8) ?[]const u8 {
    return core.view(out, raw.player_setting_str(player, field.ptr, core.len(field), out.ptr, core.cap(out)));
}
pub fn settingI64(player: core.Player, field: []const u8) ?i64 {
    var value: i64 = 0;
    return if (raw.player_setting_i64(player, field.ptr, core.len(field), &value) >= 0) value else null;
}
pub fn hasPermission(player: core.Player, node: []const u8) bool {
    return raw.player_has_permission(player, node.ptr, core.len(node)) == 1;
}
pub fn title(player: core.Player, heading: []const u8, subtitle: []const u8, fade_in: i32, stay: i32, fade_out: i32) i32 {
    return raw.player_title(player, heading.ptr, core.len(heading), subtitle.ptr, core.len(subtitle), fade_in, stay, fade_out);
}
pub fn connect(player: core.Player, backend: []const u8) i32 {
    return raw.player_connect(player, backend.ptr, core.len(backend));
}
pub fn kick(player: core.Player, reason: []const u8, args: ?*const core.Args) i32 {
    return raw.player_kick(player, reason.ptr, core.len(reason), core.argsPtr(args), core.argsLen(args));
}
