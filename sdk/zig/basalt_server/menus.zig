//! Chest menus with click handlers, and placeholders that other plugins and the server fill in text.
const core = @import("basalt_shared").core;
const raw = @import("raw/core.zig");

pub fn create(title: []const u8, rows: i32) i32 {
    return raw.menu_create(title.ptr, core.len(title), rows);
}
pub fn set(menu: i32, slot: i32, item: []const u8, name: []const u8, lore: []const u8, on_click: ?core.Handler) i32 {
    const token = core.tokenFor(on_click);
    return if (token < 0) core.Status.invalid else raw.menu_set(menu, slot, item.ptr, core.len(item), name.ptr, core.len(name), lore.ptr, core.len(lore), token);
}
pub fn open(menu: i32, player: core.Player) i32 {
    return raw.menu_open(menu, player);
}
pub fn clickPlayer(ctx: core.Ctx) core.Player {
    return raw.menu_click_player(ctx);
}

pub fn placeholder(name: []const u8, handler: core.Handler) i32 {
    const token = core.tokenFor(handler);
    return if (token < 0) core.Status.invalid else raw.placeholder_register(name.ptr, core.len(name), token);
}
pub fn placeholderPlayer(ctx: core.Ctx) core.Player {
    return raw.placeholder_player(ctx);
}
pub fn placeholderResult(ctx: core.Ctx, text: []const u8) i32 {
    return raw.placeholder_result(ctx, text.ptr, core.len(text));
}
