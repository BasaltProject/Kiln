//! Basalt proxy plugin SDK for Zig: `@import("basalt_proxy")`, one namespace per area. Declare `pub fn init() void`
//! in src/main.zig and register everything in it; plugin.toml says `target = "proxy"`.
const shared = @import("basalt_shared");

pub const core = @import("core.zig");
pub const values = shared.values;
pub const log = shared.log;
pub const config = shared.config;
pub const text = shared.text;
pub const players = @import("players.zig");
pub const servers = @import("servers.zig");
pub const data = shared.data;
pub const events = @import("events.zig");
pub const commands = shared.commands;
pub const tasks = shared.tasks;
pub const hud = @import("hud.zig");
pub const interaction = @import("interaction.zig");
pub const admin = @import("admin.zig");
pub const network = @import("network.zig");
