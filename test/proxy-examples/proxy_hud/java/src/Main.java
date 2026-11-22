import static org.basaltmc.proxy.core.Priority.NORMAL;
import static org.basaltmc.proxy.events.Events.eventPlayer;
import static org.basaltmc.proxy.events.Events.on;
import static org.basaltmc.proxy.log.Log.logInfo;
import static org.basaltmc.proxy.tasks.Tasks.after;
import org.basaltmc.proxy.hud.Hud;

public final class Main {
    private static int target;

    private static void step(String name, int code) {
        logInfo("hud " + name + (code == 0 ? " ok" : " failed"));
    }

    private static void onLater(int ctx) {
        step("bossbar_progress", Hud.bossbarProgress(target, "bar", 0.25));
        step("bossbar_title", Hud.bossbarTitle(target, "bar", "<yellow>Updated"));
        step("bossbar_style", Hud.bossbarStyle(target, "bar", "green", "notched_6"));
        step("bossbar_hide", Hud.bossbarHide(target, "bar"));
        step("sidebar_clear", Hud.sidebarClear(target));
        step("tab_entry_remove", Hud.tabEntryRemove(target, "row"));
        step("tab_list_keep", Hud.tabList(target, null, "<gray>Footer two"));
        step("chat_completions_remove", Hud.chatCompletions(target, "remove", new String[] {"alpha"}));
        step("clear_title", Hud.clearTitle(target, true));
    }

    private static void onJoin(int ctx) {
        target = eventPlayer(ctx);
        if (target == 0) {
            return;
        }
        step("clear_title", Hud.clearTitle(target, false));
        step("tab_list", Hud.tabList(target, "<gold>Header", "<gray>Footer"));
        step("bossbar_show", Hud.bossbarShow(target, "bar", "<red>Boss", 0.75, "red", "notched_10"));
        step("sidebar", Hud.sidebar(target, "<gold>Title", new String[] {"<white>first line", "<white>second line"}));
        step("tab_entry", Hud.tabEntry(target, "row", "Fake", "<green>Fake row", 25));
        step("server_links", Hud.serverLinks(target, new String[] {"website", "https://basaltmc.org", "<aqua>Custom", "https://example.org"}));
        step("chat_completions_set", Hud.chatCompletions(target, "set", new String[] {"alpha", "beta"}));
        step("chat_completions_add", Hud.chatCompletions(target, "add", new String[] {"gamma"}));
        step("invalid_id", Hud.bossbarHide(target, "") == -3 ? 0 : 1);
        step("stale_player", Hud.bossbarHide(0x7fff0000, "bar") == -2 ? 0 : 1);
        after(100, Main::onLater);
    }

    public static void init() {
        on("player.join", NORMAL, Main::onJoin);
    }
}
