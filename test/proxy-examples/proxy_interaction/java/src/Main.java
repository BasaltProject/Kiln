import java.nio.charset.StandardCharsets;
import org.basaltmc.proxy.commands.Registry;
import org.basaltmc.proxy.core.Args;
import org.basaltmc.proxy.interaction.ClientActions;
import org.basaltmc.proxy.interaction.ClientActions.Button;
import org.basaltmc.proxy.players.Directory;
import org.basaltmc.proxy.values.Documents;

import static org.basaltmc.proxy.commands.Sender.reply;
import static org.basaltmc.proxy.core.Priority.NORMAL;
import static org.basaltmc.proxy.events.EventBus.eventPlayer;
import static org.basaltmc.proxy.events.EventBus.on;
import static org.basaltmc.proxy.log.Logger.logInfo;
import static org.basaltmc.proxy.tasks.Scheduler.after;

public final class Main {
    private static int seen;

    private static void say(String text) {
        logInfo(text);
    }

    private static boolean ok(int result) {
        return result >= 0;
    }

    private static String textOf(Documents.Value root, String key) {
        if (root == null) {
            return "";
        }
        Documents.Value child = root.find(key);
        String text = child == null ? null : child.asString();
        return text == null ? "" : text;
    }

    private static void onPackStatus(int ctx) {
        Documents.Value event = ClientActions.valueResult(ctx);
        if (event == null) {
            say("pack event unreadable");
            return;
        }
        say("pack status:");
        say(textOf(event, "status"));
    }

    private static void onSettings(int ctx) {
        Documents.Value event = ClientActions.valueResult(ctx);
        if (event == null) {
            say("settings event unreadable");
            return;
        }
        say("settings locale:");
        say(textOf(event.find("settings"), "locale"));
    }

    private static void onCookie(int ctx) {
        byte[] cookie = ClientActions.cookieResult(ctx);
        if (cookie == null) {
            say("cookie absent");
            return;
        }
        say("cookie received:");
        say(new String(cookie, StandardCharsets.UTF_8));
    }

    private static void onDialogClick(int ctx) {
        say("dialog clicked");
        reply(ctx, "<green>dialog clicked", null);
    }

    private static void onTransfer(int ctx) {
        int player = eventPlayer(ctx);
        say(ok(ClientActions.playerTransfer(player, "127.0.0.1", 25970)) ? "transfer sent" : "transfer failed");
    }

    private static void onCleanup(int ctx) {
        int player = eventPlayer(ctx);
        boolean removed = ok(ClientActions.resourcePackRemove(player, "demo"));
        boolean cleared = ok(ClientActions.resourcePackClear(player));
        boolean closed = ok(ClientActions.dialogClear(player));
        say(removed && cleared && closed ? "cleanup ok" : "cleanup failed");
    }

    private static void onStale(int ctx) {
        say(ClientActions.dialogClear(seen) == -2 ? "stale after quit" : "stale not detected");
    }

    private static void onQuit(int ctx) {
        after(20, Main::onStale);
    }

    private static void onJoin(int ctx) {
        int player = eventPlayer(ctx);
        seen = player;
        say(ClientActions.playersCount("") >= 1 ? "count ok" : "count failed");
        say(ClientActions.playersCount("lobby") >= 1 ? "count lobby ok" : "count lobby failed");
        say(ClientActions.playersCount("nowhere") == 0 ? "count nowhere ok" : "count nowhere failed");
        String address = ClientActions.playerAddress(player);
        say(address != null && !address.isEmpty() && ClientActions.playerPort(player) > 0 ? "address ok" : "address failed");
        String uuid = Directory.playerUuid(player);
        say(uuid != null && ClientActions.playersByUuid(uuid) == player ? "by uuid ok" : "by uuid failed");
        say(ClientActions.playersByUuid("00000000-0000-0000-0000-000000000000") == 0 ? "by uuid none ok" : "by uuid none failed");

        Button[] buttons = {new Button("Accept", "pidialog"), new Button("Decline", "pidialog")};
        say(ok(ClientActions.dialogShow(player, "multi_action", "<gold>Interaction", "<gray>Pick one", buttons, true))
                ? "dialog ok" : "dialog failed");
        say(ClientActions.dialogShow(player, "bogus", "x", null, new Button[0], true) == -3 ? "bad kind invalid" : "bad kind accepted");

        String id = ClientActions.resourcePackPush(player, "https://example.invalid/pack.zip", null, false, "<gold>Test pack", "demo");
        if (id != null) {
            say("pack pushed:");
            say(id);
        } else {
            say("pack failed");
        }
        String bad = ClientActions.resourcePackPush(player, "https://example.invalid/pack.zip", "nothex", false, null, null);
        say(bad == null ? "bad hash invalid" : "bad hash accepted");

        byte[] data = {97, 98, 99, 100};
        say(ok(ClientActions.cookieSet(player, "basalt:demo", data)) ? "cookie stored" : "cookie store failed");
        say(ClientActions.cookieRequest(player, "basalt:demo", Main::onCookie) > 0 ? "cookie requested" : "cookie request failed");

        say(ok(ClientActions.playerDataSetF64(player, "ratio", 2.5)) ? "float stored" : "float store failed");
        say(ClientActions.playerDataGetF64(player, "ratio", 0.0) == 2.5 ? "float read ok" : "float read failed");
        say(ClientActions.playerDataGetF64(player, "missing", 7.5) == 7.5 ? "float default ok" : "float default failed");

        say(ok(ClientActions.playersBroadcast("<green>Interaction hello {who}", new Args().add("who", "everyone")))
                ? "broadcast ok" : "broadcast failed");
        say(ClientActions.playerTransfer(player, "", 25565) == -3 ? "bad host invalid" : "bad host accepted");
    }

    public static void init() {
        on("player.join", NORMAL, Main::onJoin);
        on("player.quit", NORMAL, Main::onQuit);
        ClientActions.watchResourcePacks(Main::onPackStatus);
        ClientActions.watchSettings(Main::onSettings);
        Registry.register("pidialog", "", Main::onDialogClick);
        Registry.register("pitransfer", "", Main::onTransfer);
        Registry.register("picleanup", "", Main::onCleanup);
        say("interaction ready");
    }
}
