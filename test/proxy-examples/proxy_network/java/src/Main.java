import static org.basaltmc.proxy.config.Config.configStr;
import static org.basaltmc.proxy.core.Priority.NORMAL;
import static org.basaltmc.proxy.events.Events.eventPlayer;
import static org.basaltmc.proxy.events.Events.on;
import static org.basaltmc.proxy.log.Log.logError;
import static org.basaltmc.proxy.log.Log.logInfo;
import org.basaltmc.proxy.network.Network;
import org.basaltmc.proxy.values.Values;
import org.basaltmc.proxy.values.Values.Value;
import java.nio.charset.StandardCharsets;

public final class Main {
    private static String rewriteMarker = "";
    private static String cancelMarker = "";
    private static String channel = "";
    private static boolean refusedOnce = false;

    private static byte[] utf8(String s) {
        return s.getBytes(StandardCharsets.UTF_8);
    }

    private static int find(byte[] hay, String needle) {
        byte[] n = utf8(needle);
        for (int at = 0; n.length > 0 && at + n.length <= hay.length; at++) {
            int i = 0;
            while (i < n.length && hay[at + i] == n[i]) {
                i++;
            }
            if (i == n.length) {
                return at;
            }
        }
        return -1;
    }

    private static String text(Value map, String key) {
        String s = map.find(key).asString();
        return s == null ? "" : s;
    }

    private static byte[] chatBody(String s) {
        byte[] t = utf8(s);
        byte[] out = new byte[t.length + 4];
        out[0] = 0x08;
        out[1] = (byte) (t.length >> 8);
        out[2] = (byte) t.length;
        System.arraycopy(t, 0, out, 3, t.length);
        return out;
    }

    private static byte[] stringPrefixed(String first, String rest) {
        byte[] a = utf8(first);
        byte[] b = utf8(rest);
        byte[] out = new byte[1 + a.length + b.length];
        out[0] = (byte) a.length;
        System.arraycopy(a, 0, out, 1, a.length);
        System.arraycopy(b, 0, out, 1 + a.length, b.length);
        return out;
    }

    private static void onMessage(int ctx) {
        byte[] doc = Network.messageGet(ctx);
        Value map = doc == null ? null : Value.open(doc);
        if (map == null) {
            logError("message read failed");
            return;
        }
        byte[] data = map.find("data").asBytes();
        logInfo("net: message " + text(map, "source") + " " + text(map, "channel") + " "
                + new String(data == null ? new byte[0] : data, StandardCharsets.UTF_8));
        int player = eventPlayer(ctx);
        if (player > 0 && Network.messageSend(player, "basalt:demo", utf8("pong"), true) == 0) {
            logInfo("net: reply sent");
        }
    }

    private static void onDecider(int ctx) {
        byte[] info = Network.packetGet(ctx);
        byte[] body = Network.packetBody(ctx);
        Value map = info == null ? null : Value.open(info);
        if (map == null || body == null) {
            logError("packet read failed");
            return;
        }
        int at = find(body, rewriteMarker);
        if (find(body, cancelMarker) >= 0) {
            if (Network.packetCancel(ctx) == 0) {
                logInfo("net: cancel for " + text(map, "player_name"));
            }
        } else if (at >= 0) {
            for (int k = 0; k < utf8(rewriteMarker).length; k++) {
                if (body[at + k] >= 'a' && body[at + k] <= 'z') {
                    body[at + k] -= 32;
                }
            }
            if (Network.packetSetBody(ctx, body) == 0) {
                logInfo("net: rewrite for " + text(map, "player_name"));
            }
        }
    }

    private static void onWatch(int ctx) {
        byte[] info = Network.packetGet(ctx);
        Value map = info == null ? null : Value.open(info);
        if (map == null) {
            return;
        }
        if (!refusedOnce && Network.packetCancel(ctx) < 0) {
            refusedOnce = true;
            logInfo("net: observer cancel refused");
        }
        if (map.find("cancelled").asBool(false)) {
            logInfo("net: watch saw a cancelled packet");
        }
    }

    private static void onAntibot(int ctx) {
        byte[] doc = Network.antibotEventGet(ctx);
        Value map = doc == null ? null : Value.open(doc);
        if (map != null) {
            logInfo("net: antibot " + text(map, "kind") + " " + text(map, "action"));
        }
    }

    private static void idle(int ctx) {}

    private static void onJoin(int ctx) {
        int player = eventPlayer(ctx);
        if (player <= 0) {
            return;
        }
        byte[] body = chatBody("net welcome");
        if (Network.packetSend(player, "clientbound.system_chat", body) == 0) {
            logInfo("net: sent welcome");
        }
        if (Network.packetSend(player, "serverbound.play.chat_command", body) < 0) {
            logInfo("net: wrong direction refused");
        }
        if (Network.messageSend(player, channel, utf8("hello"), true) == 0) {
            logInfo("net: sent hello message");
        }
        if (Network.packetReceive(player, "serverbound.play.custom_payload", stringPrefixed(channel, "ping")) == 0) {
            logInfo("net: injected custom payload");
        }
        if (Network.packetReceive(player, "serverbound.play.chat_command", stringPrefixed("list", "")) == 0) {
            logInfo("net: injected command");
        }
    }

    private static void onEnable(int ctx) {
        byte[] names = Network.packetNames();
        Value list = names == null ? null : Value.open(names);
        if (list != null && list.count() > 100) {
            logInfo("net: packets known");
        }
        byte[] resolved = Network.packetResolve("clientbound.system_chat");
        Value map = resolved == null ? null : Value.open(resolved);
        if (map != null && !map.find("serverbound").asBool(true)) {
            logInfo("net: resolved a clientbound play packet");
        }
        if (Network.packetResolve("nope.nothing") == null) {
            logInfo("net: unknown packet refused");
        }
        if (Network.channelRegister(channel) == 0) {
            logInfo("net: channel registered");
        }
        if (Network.channelRegister("BungeeCord") < 0) {
            logInfo("net: bungee channel refused");
        }
        if (Network.channelRegister("basalt:temp") == 0 && Network.channelUnregister("basalt:temp") == 0
                && Network.channelUnregister("basalt:temp") < 0) {
            logInfo("net: temp channel unregistered");
        }
        Network.messageOn("basalt:demo", Main::onMessage);
        byte[] opts = new Values.Writer().beginMap().key("priority").string("late").key("fail").string("closed").end().finish();
        if (Network.packetOn("clientbound.system_chat", opts, Main::onDecider) > 0) {
            logInfo("net: decider listening");
        }
        Network.packetWatch("clientbound.system_chat", new byte[0], Main::onWatch);
        int listener = Network.packetOn("serverbound.play.*", new byte[0], Main::idle);
        if (listener > 0 && Network.packetRemove(listener) == 0 && Network.packetRemove(listener) < 0) {
            logInfo("net: listener removed");
        }
        if (Network.packetOn("nope.nothing", new byte[0], Main::idle) < 0) {
            logInfo("net: bad listener name refused");
        }
        if (Network.packetGet(9999) == null) {
            logInfo("net: bad context refused");
        }
        Network.antibotOn(Main::onAntibot);
        Network.antibotVerify("NetBot");
        logInfo(Network.antibotIsVerified("NetBot") == 1 ? "net: verified yes" : "net: verified no");
        Network.antibotUnverify("NetBot");
        logInfo(Network.antibotIsVerified("NetBot") == 0 ? "net: unverified" : "net: still verified");
        if (Network.antibotBan("203.0.113.9") == 0) {
            logInfo("net: ban ok");
        }
        if (Network.antibotUnban("203.0.113.9") == 0) {
            logInfo("net: unban ok");
        }
        if (Network.antibotUnban("203.0.113.9") < 0) {
            logInfo("net: unban absent");
        }
        if (Network.antibotBan("not an address") < 0) {
            logInfo("net: bad address refused");
        }
        logInfo(Network.antibotUnderAttack() == 0 ? "net: no attack" : "net: under attack");
    }

    public static void init() {
        String a = configStr("rewrite-marker");
        String b = configStr("cancel-marker");
        String c = configStr("channel");
        rewriteMarker = a == null ? "" : a;
        cancelMarker = b == null ? "" : b;
        channel = c == null ? "" : c;
        on("plugin.enable", NORMAL, Main::onEnable);
        on("player.join", NORMAL, Main::onJoin);
    }
}
