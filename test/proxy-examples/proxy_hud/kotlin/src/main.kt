import org.basaltmc.proxy.core.Ctx

var target = 0

fun step(name: String, code: Int) {
    org.basaltmc.proxy.log.logInfo("hud " + name + (if (code == 0) " ok" else " failed"))
}

fun onLater(ctx: Ctx) {
    step("bossbar_progress", org.basaltmc.proxy.hud.bossbarProgress(target, "bar", 0.25))
    step("bossbar_title", org.basaltmc.proxy.hud.bossbarTitle(target, "bar", "<yellow>Updated"))
    step("bossbar_style", org.basaltmc.proxy.hud.bossbarStyle(target, "bar", "green", "notched_6"))
    step("bossbar_hide", org.basaltmc.proxy.hud.bossbarHide(target, "bar"))
    step("sidebar_clear", org.basaltmc.proxy.hud.sidebarClear(target))
    step("tab_entry_remove", org.basaltmc.proxy.hud.tabEntryRemove(target, "row"))
    step("tab_list_keep", org.basaltmc.proxy.hud.tabList(target, null, "<gray>Footer two"))
    step("chat_completions_remove", org.basaltmc.proxy.hud.chatCompletions(target, "remove", listOf("alpha")))
    step("clear_title", org.basaltmc.proxy.hud.clearTitle(target, true))
}

fun onJoin(ctx: Ctx) {
    target = org.basaltmc.proxy.events.eventPlayer(ctx)
    if (target == 0) return
    step("clear_title", org.basaltmc.proxy.hud.clearTitle(target, false))
    step("tab_list", org.basaltmc.proxy.hud.tabList(target, "<gold>Header", "<gray>Footer"))
    step("bossbar_show", org.basaltmc.proxy.hud.bossbarShow(target, "bar", "<red>Boss", 0.75, "red", "notched_10"))
    step("sidebar", org.basaltmc.proxy.hud.sidebar(target, "<gold>Title", listOf("<white>first line", "<white>second line")))
    step("tab_entry", org.basaltmc.proxy.hud.tabEntry(target, "row", "Fake", "<green>Fake row", 25))
    step("server_links", org.basaltmc.proxy.hud.serverLinks(target, listOf("website", "https://basaltmc.org", "<aqua>Custom", "https://example.org")))
    step("chat_completions_set", org.basaltmc.proxy.hud.chatCompletions(target, "set", listOf("alpha", "beta")))
    step("chat_completions_add", org.basaltmc.proxy.hud.chatCompletions(target, "add", listOf("gamma")))
    step("invalid_id", if (org.basaltmc.proxy.hud.bossbarHide(target, "") == -3) 0 else 1)
    step("stale_player", if (org.basaltmc.proxy.hud.bossbarHide(0x7fff0000, "bar") == -2) 0 else 1)
    org.basaltmc.proxy.tasks.after(100, ::onLater)
}

fun init() {
    org.basaltmc.proxy.events.on("player.join", org.basaltmc.proxy.core.Priority.NORMAL, ::onJoin)
}
