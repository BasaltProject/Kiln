use basalt_proxy::hud as hud;
use basalt_proxy::core::Ctx;
use std::cell::Cell;

thread_local! {
    static TARGET: Cell<i32> = Cell::new(0);
}

fn step(name: &str, code: i32) {
    basalt_proxy::log::info(&format!("hud {} {}", name, if code == 0 { "ok" } else { "failed" }));
}

fn on_later(_: Ctx) {
    let target = TARGET.with(|t| t.get());
    step("bossbar_progress", hud::bossbar_progress(target, "bar", 0.25));
    step("bossbar_title", hud::bossbar_title(target, "bar", "<yellow>Updated"));
    step("bossbar_style", hud::bossbar_style(target, "bar", "green", "notched_6"));
    step("bossbar_hide", hud::bossbar_hide(target, "bar"));
    step("sidebar_clear", hud::sidebar_clear(target));
    step("tab_entry_remove", hud::tab_entry_remove(target, "row"));
    step("tab_list_keep", hud::tab_list(target, None, Some("<gray>Footer two")));
    step("chat_completions_remove", hud::chat_completions(target, "remove", &["alpha"]));
    step("clear_title", hud::clear_title(target, true));
}

fn on_join(ctx: Ctx) {
    let target = basalt_proxy::events::player(ctx);
    TARGET.with(|t| t.set(target));
    if target == 0 {
        return;
    }
    step("clear_title", hud::clear_title(target, false));
    step("tab_list", hud::tab_list(target, Some("<gold>Header"), Some("<gray>Footer")));
    step("bossbar_show", hud::bossbar_show(target, "bar", "<red>Boss", 0.75, "red", "notched_10"));
    step("sidebar", hud::sidebar(target, "<gold>Title", &["<white>first line", "<white>second line"]));
    step("tab_entry", hud::tab_entry(target, "row", "Fake", "<green>Fake row", 25));
    step("server_links", hud::server_links(target, &[("website", "https://basaltmc.org"), ("<aqua>Custom", "https://example.org")]));
    step("chat_completions_set", hud::chat_completions(target, "set", &["alpha", "beta"]));
    step("chat_completions_add", hud::chat_completions(target, "add", &["gamma"]));
    step("invalid_id", if hud::bossbar_hide(target, "") == -3 { 0 } else { 1 });
    step("stale_player", if hud::bossbar_hide(0x7fff0000, "bar") == -2 { 0 } else { 1 });
    basalt_proxy::tasks::after(100, on_later);
}

pub fn init() {
    basalt_proxy::events::on("player.join", basalt_proxy::core::NORMAL, on_join);
}
