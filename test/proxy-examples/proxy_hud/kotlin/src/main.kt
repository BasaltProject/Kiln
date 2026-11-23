import org.basaltmc.proxy.core.Ctx
import org.basaltmc.proxy.core.Priority
import org.basaltmc.proxy.events.eventPlayer
import org.basaltmc.proxy.events.on
import org.basaltmc.proxy.hud.bossbarHide
import org.basaltmc.proxy.hud.bossbarProgress
import org.basaltmc.proxy.hud.bossbarShow
import org.basaltmc.proxy.hud.bossbarStyle
import org.basaltmc.proxy.hud.bossbarTitle
import org.basaltmc.proxy.hud.chatCompletions
import org.basaltmc.proxy.hud.clearTitle
import org.basaltmc.proxy.hud.serverLinks
import org.basaltmc.proxy.hud.sidebar
import org.basaltmc.proxy.hud.sidebarClear
import org.basaltmc.proxy.hud.tabEntry
import org.basaltmc.proxy.hud.tabEntryRemove
import org.basaltmc.proxy.hud.tabList
import org.basaltmc.proxy.log.logInfo
import org.basaltmc.proxy.tasks.after

var target = 0

fun step(name: String, code: Int) {
    logInfo("hud " + name + (if (code == 0) " ok" else " failed"))
}

fun onLater(ctx: Ctx) {
    step("bossbar_progress", bossbarProgress(target, "bar", 0.25))
    step("bossbar_title", bossbarTitle(target, "bar", "<yellow>Updated"))
    step("bossbar_style", bossbarStyle(target, "bar", "green", "notched_6"))
    step("bossbar_hide", bossbarHide(target, "bar"))
    step("sidebar_clear", sidebarClear(target))
    step("tab_entry_remove", tabEntryRemove(target, "row"))
    step("tab_list_keep", tabList(target, null, "<gray>Footer two"))
    step("chat_completions_remove", chatCompletions(target, "remove", listOf("alpha")))
    step("clear_title", clearTitle(target, true))
}

fun onJoin(ctx: Ctx) {
    target = eventPlayer(ctx)
    if (target == 0) return
    step("clear_title", clearTitle(target, false))
    step("tab_list", tabList(target, "<gold>Header", "<gray>Footer"))
    step("bossbar_show", bossbarShow(target, "bar", "<red>Boss", 0.75, "red", "notched_10"))
    step("sidebar", sidebar(target, "<gold>Title", listOf("<white>first line", "<white>second line")))
    step("tab_entry", tabEntry(target, "row", "Fake", "<green>Fake row", 25))
    step("server_links", serverLinks(target, listOf("website", "https://basaltmc.org", "<aqua>Custom", "https://example.org")))
    step("chat_completions_set", chatCompletions(target, "set", listOf("alpha", "beta")))
    step("chat_completions_add", chatCompletions(target, "add", listOf("gamma")))
    step("invalid_id", if (bossbarHide(target, "") == -3) 0 else 1)
    step("stale_player", if (bossbarHide(0x7fff0000, "bar") == -2) 0 else 1)
    after(100, ::onLater)
}

fun init() {
    on("player.join", Priority.NORMAL, ::onJoin)
}
