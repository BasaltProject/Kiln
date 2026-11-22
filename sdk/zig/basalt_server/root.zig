//! Basalt server plugin SDK for Zig: `@import("basalt_server")`, one namespace per area. Declare `pub fn init() void`
//! in src/main.zig and register everything in it.
const shared = @import("basalt_shared");

pub const core = @import("core.zig");
pub const values = shared.values;
pub const log = shared.log;
pub const config = shared.config;
pub const text = shared.text;
pub const players = @import("players.zig");
pub const data = @import("data.zig");
pub const events = @import("events.zig");
pub const commands = shared.commands;
pub const tasks = shared.tasks;
pub const menus = @import("menus.zig");
pub const worlds = @import("worlds.zig");
pub const entities = @import("entities.zig");
pub const items = @import("items.zig");
pub const hud = @import("hud.zig");
pub const packets = @import("packets.zig");
pub const server = @import("server.zig");
