package org.basaltmc.proxy.admin;
import org.basaltmc.proxy.internal.*;
import org.basaltmc.proxy.values.Values;
import org.basaltmc.proxy.core.*;


/** Proxy admin area (abi/areas/proxy_admin.h): backends, console and chat, permission providers, groups, JSON text, the
 * plugin store, data by holder and waiting. Calls that give a value answer a Values.Value, or null on a negative code. */
public final class Admin {
    private Admin() {}

    private static final int INVALID = -3;

    private static Values.Value value(Values.Out call) {
        return Values.callOut(256, call).value();
    }

    private static String text(Values.Out call) {
        Values.OutResult result = Values.callOut(256, call);
        return result.error == Values.OK ? new String(result.bytes, java.nio.charset.StandardCharsets.UTF_8) : null;
    }

    /** Result of servers_register: the status (0 when it worked) and the reason the proxy gave otherwise. */
    public static final class Registration {
        public int status;
        public String reason = "";
    }

    public static Registration serversRegister(String name, String host, int port, String like) {
        byte[] n = Native.utf8(name);
        byte[] h = Native.utf8(host);
        byte[] l = Native.utf8(like);
        byte[] reason = new byte[256];
        Registration result = new Registration();
        result.status = RawProxyAdmin.rawServersRegister(Native.at(n), n.length, Native.at(h), h.length, port,
                Native.at(l), l.length, Native.at(reason), reason.length);
        if (result.status < 0) {
            int end = 0;
            while (end < reason.length && reason[end] != 0) {
                end++;
            }
            result.reason = new String(reason, 0, end, java.nio.charset.StandardCharsets.UTF_8);
        }
        return result;
    }

    public static int serversUnregister(String name) {
        byte[] n = Native.utf8(name);
        return RawProxyAdmin.rawServersUnregister(Native.at(n), n.length);
    }

    /** Pings a backend; the handler reads the answer with completionValue. Returns the request handle. */
    public static int serversPing(String name, Handler handler) {
        byte[] n = Native.utf8(name);
        return RawProxyAdmin.rawServersPing(Native.at(n), n.length, Callbacks.tokenFor(handler));
    }

    /** Runs the handler after ticks ticks of 50 ms. Returns the request handle. */
    public static int waitTicks(int ticks, Handler handler) {
        return RawProxyAdmin.rawWait(ticks, Callbacks.tokenFor(handler));
    }

    public static int requestCancel(int request) {
        return RawCoreValues.rawRequestCancel(request);
    }

    public static Values.Value completionValue(int ctx) {
        return value((out, cap) -> RawCoreValues.rawValueResult(ctx, out, cap));
    }

    /** 1 when the line named a command, 0 when not, -4 without the capability and the operator's grant. */
    public static int consoleRun(String line) {
        byte[] l = Native.utf8(line);
        return RawProxyAdmin.rawConsoleRun(Native.at(l), l.length);
    }

    public static int playerChat(int player, String text) {
        byte[] t = Native.utf8(text);
        return RawProxyAdmin.rawPlayerChat(player, Native.at(t), t.length);
    }

    public static int playerSendJson(int player, String json) {
        byte[] j = Native.utf8(json);
        return RawProxyAdmin.rawPlayerSendJson(player, Native.at(j), j.length);
    }

    public static int playerKickJson(int player, String json) {
        byte[] j = Native.utf8(json);
        return RawProxyAdmin.rawPlayerKickJson(player, Native.at(j), j.length);
    }

    /** 1 granted, 0 denied, -1 undecided. who "" is the console; server may be empty. */
    public static int permissionCheck(String who, String node, String server) {
        byte[] w = Native.utf8(who);
        byte[] n = Native.utf8(node);
        byte[] context = new byte[0];
        if (!server.isEmpty()) {
            context = new Values.Writer(Values.MAX_BYTES).beginMap().key("server").string(server).end().finish();
            if (context == null) {
                return INVALID;
            }
        }
        return RawProxyAdmin.rawPermissionCheck(Native.at(w), w.length, Native.at(n), n.length, Native.at(context),
                context.length);
    }

    public static String permissionMeta(String who, String key) {
        byte[] w = Native.utf8(who);
        byte[] k = Native.utf8(key);
        return text((out, cap) -> RawProxyAdmin.rawPermissionMeta(Native.at(w), w.length, Native.at(k), k.length,
                Native.at(k), 0, out, cap));
    }

    /** Registers provider callbacks (null for none); returns the provider handle. */
    public static int permissionProvide(Handler check, Handler meta, int priority) {
        int checkToken = check == null ? 0 : Callbacks.tokenFor(check);
        int metaToken = meta == null ? 0 : Callbacks.tokenFor(meta);
        return RawProxyAdmin.rawPermissionProvide(checkToken, metaToken, priority);
    }

    public static int permissionUnprovide(int provider) {
        return RawProxyAdmin.rawPermissionUnprovide(provider);
    }

    /** The question of a provider callback: a map with kind, name, uuid, console, text and context. */
    public static Values.Value permissionRequest(int ctx) {
        return value((out, cap) -> RawProxyAdmin.rawPermissionRequest(ctx, out, cap));
    }

    /** 1 grants, 0 denies, negative leaves it to the next provider. */
    public static int permissionAnswer(int ctx, int answer) {
        return RawProxyAdmin.rawPermissionAnswer(ctx, answer);
    }

    public static int permissionAnswerText(int ctx, String text) {
        byte[] t = Native.utf8(text);
        return RawProxyAdmin.rawPermissionAnswerText(ctx, Native.at(t), t.length);
    }

    public static int permissionChanged() {
        return RawProxyAdmin.rawPermissionChanged();
    }

    /** 1 grants, 0 denies, -1 removes. */
    public static int permissionOverride(String name, String node, int value) {
        byte[] n = Native.utf8(name);
        byte[] o = Native.utf8(node);
        return RawProxyAdmin.rawPermissionOverride(Native.at(n), n.length, Native.at(o), o.length, value);
    }

    public static Values.Value permissionList(String name) {
        byte[] n = Native.utf8(name);
        return value((out, cap) -> RawProxyAdmin.rawPermissionList(Native.at(n), n.length, out, cap));
    }

    public static String groupGet(String name) {
        byte[] n = Native.utf8(name);
        return text((out, cap) -> RawProxyAdmin.rawGroupGet(Native.at(n), n.length, out, cap));
    }

    public static int groupSet(String name, String group) {
        byte[] n = Native.utf8(name);
        byte[] g = Native.utf8(group);
        return RawProxyAdmin.rawGroupSet(Native.at(n), n.length, Native.at(g), g.length);
    }

    /** Reads a store key (holder null) or a player data key of holder; null when unset. */
    public static Values.Value dataValue(String holder, String key) {
        byte[] k = Native.utf8(key);
        if (holder == null) {
            return value((out, cap) -> RawProxyAdmin.rawStoreGet(Native.at(k), k.length, out, cap));
        }
        byte[] h = Native.utf8(holder);
        return value((out, cap) -> RawProxyAdmin.rawPlayerDataGet(Native.at(h), h.length, Native.at(k), k.length, out,
                cap));
    }

    public static int dataSetValue(String holder, String key, byte[] doc) {
        if (doc == null || doc.length == 0) {
            return INVALID;
        }
        byte[] k = Native.utf8(key);
        if (holder == null) {
            return RawProxyAdmin.rawStoreSet(Native.at(k), k.length, Native.at(doc), doc.length);
        }
        byte[] h = Native.utf8(holder);
        return RawProxyAdmin.rawPlayerDataSet(Native.at(h), h.length, Native.at(k), k.length, Native.at(doc),
                doc.length);
    }

    public static int dataSetLong(String holder, String key, long number) {
        return dataSetValue(holder, key, new Values.Writer(Values.MAX_BYTES).integer(number).finish());
    }

    public static int dataSetDouble(String holder, String key, double number) {
        return dataSetValue(holder, key, new Values.Writer(Values.MAX_BYTES).number(number).finish());
    }

    public static int dataSetBool(String holder, String key, boolean flag) {
        return dataSetValue(holder, key, new Values.Writer(Values.MAX_BYTES).bool(flag).finish());
    }

    public static int dataSetStr(String holder, String key, String text) {
        return dataSetValue(holder, key, new Values.Writer(Values.MAX_BYTES).string(text).finish());
    }

    public static int dataRemove(String holder, String key) {
        return dataSetValue(holder, key, new Values.Writer(Values.MAX_BYTES).nul().finish());
    }

    /** The best limit numeric values of a store key: a list of maps with uuid, name and value. */
    public static Values.Value storeTop(String key, int limit) {
        byte[] k = Native.utf8(key);
        return value((out, cap) -> RawProxyAdmin.rawStoreTop(Native.at(k), k.length, limit, out, cap));
    }
}
