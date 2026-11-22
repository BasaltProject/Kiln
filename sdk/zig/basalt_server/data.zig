//! Values kept per player in the plugin database (capability players_data), and leaderboards over them.
const shared = @import("basalt_shared");
const core = shared.core;
const raw = @import("raw/core.zig");

pub const getI64 = shared.data.getI64;
pub const setI64 = shared.data.setI64;
pub const getBool = shared.data.getBool;
pub const setBool = shared.data.setBool;
pub const getStr = shared.data.getStr;
pub const setStr = shared.data.setStr;
pub const remove = shared.data.remove;

/// The highest `limit` integer values under `key`: rows of u32 name length, name bytes and i64 little endian.
pub fn storeTop(key: []const u8, limit: i32, out: []u8) ?[]const u8 {
    return core.view(out, raw.store_top(key.ptr, core.len(key), limit, out.ptr, core.cap(out)));
}
