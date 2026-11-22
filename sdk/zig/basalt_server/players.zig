//! Online players: lookup, messages, particles and sounds.
const shared = @import("basalt_shared");
const core = shared.core;
const raw = @import("raw/core.zig");

pub const byName = shared.players.byName;
pub const online = shared.players.online;
pub const name = shared.players.name;
pub const send = shared.players.send;
pub const actionBar = shared.players.actionBar;

pub fn particle(player: core.Player, id: []const u8, count: i32, spread: f64) i32 {
    return raw.player_particle(player, id.ptr, core.len(id), count, spread);
}
pub fn sound(player: core.Player, id: []const u8, volume: f32, pitch: f32) i32 {
    return raw.player_sound(player, id.ptr, core.len(id), volume, pitch);
}
