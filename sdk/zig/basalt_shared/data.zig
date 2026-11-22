//! Values kept per player in the plugin database (capability players_data).
const core = @import("core.zig");
const raw = @import("raw/core.zig");

pub fn getI64(player: core.Player, key: []const u8, def: i64) i64 {
    return raw.data_get_i64(player, key.ptr, core.len(key), def);
}
pub fn setI64(player: core.Player, key: []const u8, value: i64) i32 {
    return raw.data_set_i64(player, key.ptr, core.len(key), value);
}
pub fn getBool(player: core.Player, key: []const u8, def: bool) bool {
    return raw.data_get_bool(player, key.ptr, core.len(key), @intFromBool(def)) != 0;
}
pub fn setBool(player: core.Player, key: []const u8, value: bool) i32 {
    return raw.data_set_bool(player, key.ptr, core.len(key), @intFromBool(value));
}
pub fn getStr(player: core.Player, key: []const u8, out: []u8) ?[]const u8 {
    return core.view(out, raw.data_get_str(player, key.ptr, core.len(key), out.ptr, core.cap(out)));
}
pub fn setStr(player: core.Player, key: []const u8, value: []const u8) i32 {
    return raw.data_set_str(player, key.ptr, core.len(key), value.ptr, core.len(value));
}
pub fn remove(player: core.Player, key: []const u8) i32 {
    return raw.data_remove(player, key.ptr, core.len(key));
}
