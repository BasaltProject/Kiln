import org.basaltmc.proxy.hud.Displays;

import static org.basaltmc.proxy.core.Priority.NORMAL;
import static org.basaltmc.proxy.events.EventBus.eventPlayer;
import static org.basaltmc.proxy.events.EventBus.on;
import static org.basaltmc.proxy.log.Logger.logInfo;
import static org.basaltmc.proxy.tasks.Scheduler.after;

public final class Main {
    private static int target;

    private static void step(String name, int code) {
        logInfo("hud " + name + (code == 0 ? " ok" : " failed"));
    }

    private static void onLater(int ctx) {
        step("bossbar_progress", Displays.bossbarProgress(target, "bar", 0.25));
        step("bossbar_title", Displays.bossbarTitle(target, "bar", "<yellow>Updated"));
        step("bossbar_style", Displays.bossbarStyle(target, "bar", "green", "notched_6"));
        step("bossbar_hide", Displays.bossbarHide(target, "bar"));
        step("sidebar_clear", Displays.sidebarClear(target));
        step("tab_entry_remove", Displays.tabEntryRemove(target, "row"));
        step("tab_list_keep", Displays.tabList(target, null, "<gray>Footer two"));
        step("chat_completions_remove", Displays.chatCompletions(target, "remove", new String[] {"alpha"}));
        step("clear_title", Displays.clearTitle(target, true));
    }

    private static void onJoin(int ctx) {
        target = eventPlayer(ctx);
        if (target == 0) {
            return;
        }
        step("clear_title", Displays.clearTitle(target, false));
        step("tab_list", Displays.tabList(target, "<gold>Header", "<gray>Footer"));
        step("bossbar_show", Displays.bossbarShow(target, "bar", "<red>Boss", 0.75, "red", "notched_10"));
        step("sidebar", Displays.sidebar(target, "<gold>Title", new String[] {"<white>first line", "<white>second line"}));
        step("tab_entry", Displays.tabEntry(target, "row", "Fake", "<green>Fake row", 25));
        step("server_links", Displays.serverLinks(target, new String[] {"website", "https://basaltmc.org", "<aqua>Custom", "https://example.org"}));
        step("chat_completions_set", Displays.chatCompletions(target, "set", new String[] {"alpha", "beta"}));
        step("chat_completions_add", Displays.chatCompletions(target, "add", new String[] {"gamma"}));
        step("invalid_id", Displays.bossbarHide(target, "") == -3 ? 0 : 1);
        step("stale_player", Displays.bossbarHide(0x7fff0000, "bar") == -2 ? 0 : 1);
        after(100, Main::onLater);
    }

    public static void init() {
        on("player.join", NORMAL, Main::onJoin);
    }
}
