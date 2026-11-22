import static org.basaltmc.proxy.commands.Commands.command;
import static org.basaltmc.proxy.commands.Commands.commandSourceName;
import static org.basaltmc.proxy.log.Log.logInfo;
import static org.basaltmc.proxy.players.SharedPlayers.playerByName;
import org.basaltmc.proxy.admin.Admin;
import org.basaltmc.proxy.values.Values;

public final class Main {
    private static void sayInt(String label, long number) {
        logInfo(label + number);
    }

    private static void onCheck(int ctx) {
        Values.Value request = Admin.permissionRequest(ctx);
        if (request == null) {
            return;
        }
        String node = request.find("text").asString();
        if ("proxyadmin.granted".equals(node)) {
            Admin.permissionAnswer(ctx, 1);
        } else if ("proxyadmin.denied".equals(node)) {
            Admin.permissionAnswer(ctx, 0);
        }
    }

    private static void onMeta(int ctx) {
        Admin.permissionAnswerText(ctx, "[Admin] ");
    }

    private static void onPing(int ctx) {
        Values.Value root = Admin.completionValue(ctx);
        if (root == null) {
            logInfo("ping failed");
            return;
        }
        if (root.isNull()) {
            logInfo("ping offline");
            return;
        }
        logInfo("ping answered");
        sayInt("ping online: ", root.find("online").asLong(-1));
    }

    private static void onWait(int ctx) {
        logInfo("waited");
        Values.Value visits = Admin.dataValue(null, "visits");
        if (visits != null) {
            sayInt("store visits: ", visits.asLong(0));
        }
        Values.Value top = Admin.storeTop("visits", 3);
        if (top != null) {
            sayInt("store top entries: ", top.count());
        }
    }

    private static void onCommand(int ctx) {
        String name = commandSourceName(ctx);
        if (name == null) {
            return;
        }
        int player = playerByName(name);
        logInfo("command by: " + name);
        Admin.groupSet(name, "vip");
        String group = Admin.groupGet(name);
        if (group != null) {
            logInfo("group is: " + group);
        }
        Admin.permissionOverride(name, "proxyadmin.extra", 1);
        sayInt("override check: ", Admin.permissionCheck(name, "proxyadmin.extra", ""));
        sayInt("granted check: ", Admin.permissionCheck(name, "proxyadmin.granted", ""));
        sayInt("denied check: ", Admin.permissionCheck(name, "proxyadmin.denied", ""));
        Admin.permissionOverride(name, "proxyadmin.extra", -1);
        sayInt("override removed: ", Admin.permissionCheck(name, "proxyadmin.extra", ""));
        Admin.playerChat(player, "plain chat from the plugin");
        Admin.playerSendJson(player, "{\"text\":\"json message\",\"color\":\"green\"}");
        Admin.dataSetLong(name, "coins", 7);
        logInfo("command done");
    }

    private static void onConsole(int ctx) {
        logInfo("console command ran");
    }

    private static void onKickCommand(int ctx) {
        String name = commandSourceName(ctx);
        if (name != null) {
            Admin.playerKickJson(playerByName(name), "{\"text\":\"kicked by json\",\"color\":\"red\"}");
        }
    }

    public static void init() {
        Admin.permissionProvide(Main::onCheck, Main::onMeta, 0);
        Admin.permissionChanged();
        sayInt("register extra: ", Admin.serversRegister("extra", "127.0.0.1", 25599, "lobby").status);
        sayInt("register again: ", Admin.serversRegister("extra", "127.0.0.1", 25599, "lobby").status);
        sayInt("register bad port: ", Admin.serversRegister("bad", "127.0.0.1", 0, "").status);
        sayInt("unregister extra: ", Admin.serversUnregister("extra"));
        sayInt("unregister lobby: ", Admin.serversUnregister("lobby"));
        sayInt("unregister none: ", Admin.serversUnregister("never"));
        Admin.dataSetLong(null, "visits", 5);
        Admin.dataSetStr(null, "label", "hello");
        Admin.dataSetLong("OfflineHolder", "coins", 42);
        Admin.dataSetDouble("OfflineHolder", "ratio", 2.5);
        Values.Value coins = Admin.dataValue("OfflineHolder", "coins");
        if (coins != null) {
            sayInt("offline coins: ", coins.asLong(0));
        }
        Values.Value ratio = Admin.dataValue("OfflineHolder", "ratio");
        if (ratio != null) {
            sayInt("offline ratio x10: ", (long) (ratio.asDouble(0) * 10));
        }
        Admin.dataRemove("OfflineHolder", "ratio");
        sayInt("removed ratio: ", Admin.dataValue("OfflineHolder", "ratio") == null ? -1 : 0);
        command("pcon", "", Main::onConsole);
        sayInt("console run: ", Admin.consoleRun("pcon"));
        Admin.serversPing("lobby", Main::onPing);
        Admin.waitTicks(2, Main::onWait);
        command("padmin", "proxyadmin.granted", Main::onCommand);
        command("pkick", "proxyadmin.granted", Main::onKickCommand);
        command("pdenied", "proxyadmin.denied", Main::onCommand);
    }
}
