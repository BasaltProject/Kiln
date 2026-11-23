import org.basaltmc.proxy.admin.Management;
import org.basaltmc.proxy.commands.Registry;
import org.basaltmc.proxy.values.Documents;

import static org.basaltmc.proxy.commands.Sender.name;
import static org.basaltmc.proxy.log.Logger.logInfo;
import static org.basaltmc.proxy.players.Directory.playerByName;

public final class Main {
    private static void sayInt(String label, long number) {
        logInfo(label + number);
    }

    private static void onCheck(int ctx) {
        Documents.Value request = Management.permissionRequest(ctx);
        if (request == null) {
            return;
        }
        String node = request.find("text").asString();
        if ("proxyadmin.granted".equals(node)) {
            Management.permissionAnswer(ctx, 1);
        } else if ("proxyadmin.denied".equals(node)) {
            Management.permissionAnswer(ctx, 0);
        }
    }

    private static void onMeta(int ctx) {
        Management.permissionAnswerText(ctx, "[Admin] ");
    }

    private static void onPing(int ctx) {
        Documents.Value root = Management.completionValue(ctx);
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
        Documents.Value visits = Management.dataValue(null, "visits");
        if (visits != null) {
            sayInt("store visits: ", visits.asLong(0));
        }
        Documents.Value top = Management.storeTop("visits", 3);
        if (top != null) {
            sayInt("store top entries: ", top.count());
        }
    }

    private static void onCommand(int ctx) {
        String name = name(ctx);
        if (name == null) {
            return;
        }
        int player = playerByName(name);
        logInfo("command by: " + name);
        Management.groupSet(name, "vip");
        String group = Management.groupGet(name);
        if (group != null) {
            logInfo("group is: " + group);
        }
        Management.permissionOverride(name, "proxyadmin.extra", 1);
        sayInt("override check: ", Management.permissionCheck(name, "proxyadmin.extra", ""));
        sayInt("granted check: ", Management.permissionCheck(name, "proxyadmin.granted", ""));
        sayInt("denied check: ", Management.permissionCheck(name, "proxyadmin.denied", ""));
        Management.permissionOverride(name, "proxyadmin.extra", -1);
        sayInt("override removed: ", Management.permissionCheck(name, "proxyadmin.extra", ""));
        Management.playerChat(player, "plain chat from the plugin");
        Management.playerSendJson(player, "{\"text\":\"json message\",\"color\":\"green\"}");
        Management.dataSetLong(name, "coins", 7);
        logInfo("command done");
    }

    private static void onConsole(int ctx) {
        logInfo("console command ran");
    }

    private static void onKickCommand(int ctx) {
        String name = name(ctx);
        if (name != null) {
            Management.playerKickJson(playerByName(name), "{\"text\":\"kicked by json\",\"color\":\"red\"}");
        }
    }

    public static void init() {
        Management.permissionProvide(Main::onCheck, Main::onMeta, 0);
        Management.permissionChanged();
        sayInt("register extra: ", Management.serversRegister("extra", "127.0.0.1", 25599, "lobby").status);
        sayInt("register again: ", Management.serversRegister("extra", "127.0.0.1", 25599, "lobby").status);
        sayInt("register bad port: ", Management.serversRegister("bad", "127.0.0.1", 0, "").status);
        sayInt("unregister extra: ", Management.serversUnregister("extra"));
        sayInt("unregister lobby: ", Management.serversUnregister("lobby"));
        sayInt("unregister none: ", Management.serversUnregister("never"));
        Management.dataSetLong(null, "visits", 5);
        Management.dataSetStr(null, "label", "hello");
        Management.dataSetLong("OfflineHolder", "coins", 42);
        Management.dataSetDouble("OfflineHolder", "ratio", 2.5);
        Documents.Value coins = Management.dataValue("OfflineHolder", "coins");
        if (coins != null) {
            sayInt("offline coins: ", coins.asLong(0));
        }
        Documents.Value ratio = Management.dataValue("OfflineHolder", "ratio");
        if (ratio != null) {
            sayInt("offline ratio x10: ", (long) (ratio.asDouble(0) * 10));
        }
        Management.dataRemove("OfflineHolder", "ratio");
        sayInt("removed ratio: ", Management.dataValue("OfflineHolder", "ratio") == null ? -1 : 0);
        Registry.register("pcon", "", Main::onConsole);
        sayInt("console run: ", Management.consoleRun("pcon"));
        Management.serversPing("lobby", Main::onPing);
        Management.waitTicks(2, Main::onWait);
        Registry.register("padmin", "proxyadmin.granted", Main::onCommand);
        Registry.register("pkick", "proxyadmin.granted", Main::onKickCommand);
        Registry.register("pdenied", "proxyadmin.denied", Main::onCommand);
    }
}
