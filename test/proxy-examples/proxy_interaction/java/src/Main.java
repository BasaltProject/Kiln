import static org.basaltmc.proxy.commands.Commands.command;
import static org.basaltmc.proxy.commands.Commands.commandReply;
import static org.basaltmc.proxy.core.Priority.NORMAL;
import static org.basaltmc.proxy.events.Events.eventPlayer;
import static org.basaltmc.proxy.events.Events.on;
import static org.basaltmc.proxy.log.Log.logInfo;
import static org.basaltmc.proxy.tasks.Tasks.after;
import org.basaltmc.proxy.core.Args;
import org.basaltmc.proxy.interaction.Interaction;
import org.basaltmc.proxy.interaction.Interaction.Button;
import org.basaltmc.proxy.values.Values;

public final class Main {
    private static int seen;

    private static void say(String text) {
        logInfo(text);
    }

    private static boolean ok(int result) {
        return result >= 0;
    }

    private static String textOf(Values.Value root, String key) {
        if (root == null) {
            return "";
        }
        Values.Value child = root.find(key);
        String text = child == null ? null : child.asString();
        return text == null ? "" : text;
    }

    private static void onPackStatus(int ctx) {
        Values.Value event = Interaction.valueResult(ctx);
        if (event == null) {
            say("pack event unreadable");
            return;
        }
        say("pack status:");
        say(textOf(event, "status"));
    }

    private static void onSettings(int ctx) {
        Values.Value event = Interaction.valueResult(ctx);
        if (event == null) {
            say("settings event unreadable");
            return;
        }
        say("settings locale:");
        say(textOf(event.find("settings"), "locale"));
    }

    private static void onCookie(int ctx) {
        byte[] cookie = Interaction.cookieResult(ctx);
        if (cookie == null) {
            say("cookie absent");
            return;
        }
        say("cookie received:");
        say(new String(cookie, java.nio.charset.StandardCharsets.UTF_8));
    }

    private static void onDialogClick(int ctx) {
        say("dialog clicked");
        commandReply(ctx, "<green>dialog clicked", null);
    }

    private static void onTransfer(int ctx) {
        int player = eventPlayer(ctx);
        say(ok(Interaction.playerTransfer(player, "127.0.0.1", 25970)) ? "transfer sent" : "transfer failed");
    }

    private static void onCleanup(int ctx) {
        int player = eventPlayer(ctx);
        boolean removed = ok(Interaction.resourcePackRemove(player, "demo"));
        boolean cleared = ok(Interaction.resourcePackClear(player));
        boolean closed = ok(Interaction.dialogClear(player));
        say(removed && cleared && closed ? "cleanup ok" : "cleanup failed");
    }

    private static void onStale(int ctx) {
        say(Interaction.dialogClear(seen) == -2 ? "stale after quit" : "stale not detected");
    }

    private static void onQuit(int ctx) {
        after(20, Main::onStale);
    }

    private static void onJoin(int ctx) {
        int player = eventPlayer(ctx);
        seen = player;
        say(Interaction.playersCount("") >= 1 ? "count ok" : "count failed");
        say(Interaction.playersCount("lobby") >= 1 ? "count lobby ok" : "count lobby failed");
        say(Interaction.playersCount("nowhere") == 0 ? "count nowhere ok" : "count nowhere failed");
        String address = Interaction.playerAddress(player);
        say(address != null && !address.isEmpty() && Interaction.playerPort(player) > 0 ? "address ok" : "address failed");
        String uuid = org.basaltmc.proxy.players.Players.playerUuid(player);
        say(uuid != null && Interaction.playersByUuid(uuid) == player ? "by uuid ok" : "by uuid failed");
        say(Interaction.playersByUuid("00000000-0000-0000-0000-000000000000") == 0 ? "by uuid none ok" : "by uuid none failed");

        Button[] buttons = {new Button("Accept", "pidialog"), new Button("Decline", "pidialog")};
        say(ok(Interaction.dialogShow(player, "multi_action", "<gold>Interaction", "<gray>Pick one", buttons, true))
                ? "dialog ok" : "dialog failed");
        say(Interaction.dialogShow(player, "bogus", "x", null, new Button[0], true) == -3 ? "bad kind invalid" : "bad kind accepted");

        String id = Interaction.resourcePackPush(player, "https://example.invalid/pack.zip", null, false, "<gold>Test pack", "demo");
        if (id != null) {
            say("pack pushed:");
            say(id);
        } else {
            say("pack failed");
        }
        String bad = Interaction.resourcePackPush(player, "https://example.invalid/pack.zip", "nothex", false, null, null);
        say(bad == null ? "bad hash invalid" : "bad hash accepted");

        byte[] data = {97, 98, 99, 100};
        say(ok(Interaction.cookieSet(player, "basalt:demo", data)) ? "cookie stored" : "cookie store failed");
        say(Interaction.cookieRequest(player, "basalt:demo", Main::onCookie) > 0 ? "cookie requested" : "cookie request failed");

        say(ok(Interaction.playerDataSetF64(player, "ratio", 2.5)) ? "float stored" : "float store failed");
        say(Interaction.playerDataGetF64(player, "ratio", 0.0) == 2.5 ? "float read ok" : "float read failed");
        say(Interaction.playerDataGetF64(player, "missing", 7.5) == 7.5 ? "float default ok" : "float default failed");

        say(ok(Interaction.playersBroadcast("<green>Interaction hello {who}", new Args().add("who", "everyone")))
                ? "broadcast ok" : "broadcast failed");
        say(Interaction.playerTransfer(player, "", 25565) == -3 ? "bad host invalid" : "bad host accepted");
    }

    public static void init() {
        on("player.join", NORMAL, Main::onJoin);
        on("player.quit", NORMAL, Main::onQuit);
        Interaction.watchResourcePacks(Main::onPackStatus);
        Interaction.watchSettings(Main::onSettings);
        command("pidialog", "", Main::onDialogClick);
        command("pitransfer", "", Main::onTransfer);
        command("picleanup", "", Main::onCleanup);
        say("interaction ready");
    }
}
