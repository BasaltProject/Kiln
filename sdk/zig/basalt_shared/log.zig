//! Logging into the host log under the plugin's name.
const core = @import("core.zig");
const raw = @import("raw/core.zig");

pub fn write(level: i32, msg: []const u8) void {
    raw.log(level, msg.ptr, core.len(msg));
}
pub fn debug(msg: []const u8) void {
    write(core.Level.debug, msg);
}
pub fn info(msg: []const u8) void {
    write(core.Level.info, msg);
}
pub fn warn(msg: []const u8) void {
    write(core.Level.warn, msg);
}
pub fn err(msg: []const u8) void {
    write(core.Level.err, msg);
}
