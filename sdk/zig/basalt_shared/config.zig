//! The plugin's resources/config.toml, read by dotted key with a default.
const core = @import("core.zig");
const raw = @import("raw/core.zig");

pub fn getI64(key: []const u8, def: i64) i64 {
    return raw.config_i64(key.ptr, core.len(key), def);
}
pub fn getF64(key: []const u8, def: f64) f64 {
    return raw.config_f64(key.ptr, core.len(key), def);
}
pub fn getBool(key: []const u8, def: bool) bool {
    return raw.config_bool(key.ptr, core.len(key), @intFromBool(def)) != 0;
}
pub fn getStr(key: []const u8, out: []u8) ?[]const u8 {
    return core.view(out, raw.config_str(key.ptr, core.len(key), out.ptr, core.cap(out)));
}
