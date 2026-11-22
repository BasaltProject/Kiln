const basalt = @import("basalt_proxy");
const hud = basalt.hud;

var target: basalt.core.Player = 0;

fn step(comptime name: []const u8, code: i32) void {
    if (code == 0) basalt.log.info("hud " ++ name ++ " ok") else basalt.log.info("hud " ++ name ++ " failed");
}

fn onLater(_: basalt.core.Ctx) void {
    step("bossbar_progress", hud.bossbarProgress(target, "bar", 0.25));
    step("bossbar_title", hud.bossbarTitle(target, "bar", "<yellow>Updated"));
    step("bossbar_style", hud.bossbarStyle(target, "bar", "green", "notched_6"));
    step("bossbar_hide", hud.bossbarHide(target, "bar"));
    step("sidebar_clear", hud.sidebarClear(target));
    step("tab_entry_remove", hud.tabEntryRemove(target, "row"));
    step("tab_list_keep", hud.tabList(target, null, "<gray>Footer two"));
    step("chat_completions_remove", hud.chatCompletions(target, "remove", &.{"alpha"}));
    step("clear_title", hud.clearTitle(target, true));
}

fn onJoin(ctx: basalt.core.Ctx) void {
    target = basalt.events.playerOf(ctx);
    if (target == 0) return;
    step("clear_title", hud.clearTitle(target, false));
    step("tab_list", hud.tabList(target, "<gold>Header", "<gray>Footer"));
    step("bossbar_show", hud.bossbarShow(target, "bar", "<red>Boss", 0.75, "red", "notched_10"));
    step("sidebar", hud.sidebar(target, "<gold>Title", &.{ "<white>first line", "<white>second line" }));
    step("tab_entry", hud.tabEntry(target, "row", "Fake", "<green>Fake row", 25));
    step("server_links", hud.serverLinks(target, &.{ "website", "https://basaltmc.org", "<aqua>Custom", "https://example.org" }));
    step("chat_completions_set", hud.chatCompletions(target, "set", &.{ "alpha", "beta" }));
    step("chat_completions_add", hud.chatCompletions(target, "add", &.{"gamma"}));
    step("invalid_id", if (hud.bossbarHide(target, "") == -3) 0 else 1);
    step("stale_player", if (hud.bossbarHide(0x7fff0000, "bar") == -2) 0 else 1);
    _ = basalt.tasks.after(100, onLater);
}

pub fn init() void {
    _ = basalt.events.on("player.join", basalt.core.Priority.normal, onJoin);
}
