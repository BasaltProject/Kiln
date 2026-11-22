#include <basalt/proxy.h>

static bs_player target;

static void step(const char *name, int code) {
    char line[64];
    int n = 0;
    for (const char *p = "hud "; *p; ++p) line[n++] = *p;
    for (const char *p = name; *p; ++p) line[n++] = *p;
    for (const char *p = code == 0 ? " ok" : " failed"; *p; ++p) line[n++] = *p;
    line[n] = 0;
    bs_info(line);
}

static void on_join(bs_ctx ctx) {
    const char *lines[] = {"<white>first line", "<white>second line"};
    const char *links[] = {"website", "https://basaltmc.org", "<aqua>Custom", "https://example.org"};
    const char *alpha[] = {"alpha", "beta"};
    const char *gamma[] = {"gamma"};
    target = bs_event_player(ctx);
    if (target == 0) {
        return;
    }
    step("clear_title", bs_proxy_hud_clear_title(target, 0));
    step("tab_list", bs_proxy_hud_tab_list(target, "<gold>Header", "<gray>Footer"));
    step("bossbar_show", bs_proxy_hud_bossbar_show(target, "bar", "<red>Boss", 0.75, "red", "notched_10"));
    step("sidebar", bs_proxy_hud_sidebar(target, "<gold>Title", lines, 2));
    step("tab_entry", bs_proxy_hud_tab_entry(target, "row", "Fake", "<green>Fake row", 25));
    step("server_links", bs_proxy_hud_server_links(target, links, 2));
    step("chat_completions_set", bs_proxy_hud_chat_completions(target, "set", alpha, 2));
    step("chat_completions_add", bs_proxy_hud_chat_completions(target, "add", gamma, 1));
    step("invalid_id", bs_proxy_hud_bossbar_hide(target, "") == BASALT_ERR_INVALID ? 0 : 1);
    step("stale_player", bs_proxy_hud_bossbar_hide(0x7fff0000, "bar") == BASALT_ERR_STALE ? 0 : 1);
}

static void on_later(bs_ctx ctx) {
    const char *gone[] = {"alpha"};
    (void)ctx;
    step("bossbar_progress", bs_proxy_hud_bossbar_progress(target, "bar", 0.25));
    step("bossbar_title", bs_proxy_hud_bossbar_title(target, "bar", "<yellow>Updated"));
    step("bossbar_style", bs_proxy_hud_bossbar_style(target, "bar", "green", "notched_6"));
    step("bossbar_hide", bs_proxy_hud_bossbar_hide(target, "bar"));
    step("sidebar_clear", bs_proxy_hud_sidebar_clear(target));
    step("tab_entry_remove", bs_proxy_hud_tab_entry_remove(target, "row"));
    step("tab_list_keep", bs_proxy_hud_tab_list(target, 0, "<gray>Footer two"));
    step("chat_completions_remove", bs_proxy_hud_chat_completions(target, "remove", gone, 1));
    step("clear_title", bs_proxy_hud_clear_title(target, 1));
}

static void on_enable_join(bs_ctx ctx) {
    on_join(ctx);
    bs_after(100, on_later);
}

void plugin_init(void) { bs_on("player.join", 2, on_enable_join); }
