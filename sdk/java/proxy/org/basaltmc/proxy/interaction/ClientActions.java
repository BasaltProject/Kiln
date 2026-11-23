package org.basaltmc.proxy.interaction;

import java.nio.charset.StandardCharsets;
import org.basaltmc.proxy.core.*;
import org.basaltmc.proxy.internal.*;
import org.basaltmc.proxy.values.Documents;
import org.teavm.interop.Address;

/** Proxy interaction area (abi/areas/proxy_interaction.h): dialogs, resource packs, cookies, transfer and players. */
public final class ClientActions {
    private ClientActions() {}

    private static final int INVALID = -3;

    /** A dialog button: the label and the command the client runs as the player when it is clicked. */
    public static final class Button {
        public final String label;
        public final String command;

        public Button(String label, String command) {
            this.label = label;
            this.command = command;
        }
    }

    private interface Fill {
        int call(Address out, int cap);
    }

    private static String fetchText(Fill call) {
        int cap = 256;
        while (true) {
            byte[] out = new byte[cap];
            int n = call.call(Native.at(out), cap);
            if (n < 0) {
                return null;
            }
            if (n <= cap) {
                return new String(out, 0, n, StandardCharsets.UTF_8);
            }
            cap = n;
        }
    }

    /** Observes player.resource_pack: the value holds player, player_name, id and status. */
    public static int watchResourcePacks(Handler handler) {
        byte[] n = Native.utf8("player.resource_pack");
        return RawProxyInteraction.rawInteractionWatch(Native.at(n), n.length, Callbacks.tokenFor(handler));
    }

    /** Observes player.settings: the value holds player, player_name and settings. */
    public static int watchSettings(Handler handler) {
        byte[] n = Native.utf8("player.settings");
        return RawProxyInteraction.rawInteractionWatch(Native.at(n), n.length, Callbacks.tokenFor(handler));
    }

    /** The value of an observed event or a completion, or null. */
    public static Documents.Value valueResult(int ctx) {
        return Documents.callOut(256, (out, cap) -> RawCoreValues.rawValueResult(ctx, out, cap)).value();
    }

    /** kind is "notice", "confirmation" or "multi_action"; body may be null. */
    public static int dialogShow(int player, String kind, String title, String body, Button[] buttons, boolean closable) {
        Documents.Writer w = new Documents.Writer();
        w.beginMap().key("kind").string(kind).key("title").string(title);
        if (body != null) {
            w.key("body").string(body);
        }
        w.key("buttons").beginList();
        for (Button button : buttons) {
            w.beginMap().key("label").string(button.label).key("command").string(button.command).end();
        }
        w.end().key("can_close_with_escape").bool(closable).end();
        byte[] doc = w.finish();
        return doc == null ? INVALID : RawProxyInteraction.rawDialogShow(player, Native.at(doc), doc.length);
    }

    public static int dialogClear(int player) {
        return RawProxyInteraction.rawDialogClear(player);
    }

    /** Pushes a pack and returns its id text, or null when it failed; hash, prompt and id may be null. */
    public static String resourcePackPush(int player, String url, String hash, boolean required, String prompt, String id) {
        Documents.Writer w = new Documents.Writer();
        w.beginMap().key("url").string(url);
        if (hash != null) {
            w.key("hash").string(hash);
        }
        w.key("required").bool(required);
        if (prompt != null) {
            w.key("prompt").string(prompt);
        }
        if (id != null) {
            w.key("id").string(id);
        }
        w.end();
        byte[] doc = w.finish();
        if (doc == null) {
            return null;
        }
        return fetchText((out, cap) -> RawProxyInteraction.rawResourcePackPush(player, Native.at(doc), doc.length, out, cap));
    }

    public static int resourcePackRemove(int player, String id) {
        byte[] i = Native.utf8(id);
        return RawProxyInteraction.rawResourcePackRemove(player, Native.at(i), i.length);
    }

    public static int resourcePackClear(int player) {
        return RawProxyInteraction.rawResourcePackClear(player);
    }

    public static int cookieSet(int player, String key, byte[] data) {
        byte[] k = Native.utf8(key);
        return RawProxyInteraction.rawCookieSet(player, Native.at(k), k.length, Native.at(data), data.length);
    }

    /** Returns the request handle; the handler reads the cookie with cookieResult. */
    public static int cookieRequest(int player, String key, Handler handler) {
        byte[] k = Native.utf8(key);
        return RawProxyInteraction.rawCookieRequest(player, Native.at(k), k.length, Callbacks.tokenFor(handler));
    }

    /** The cookie of a completion, or null when the client had none. */
    public static byte[] cookieResult(int ctx) {
        Documents.Value value = valueResult(ctx);
        return value == null ? null : value.asBytes();
    }

    public static int requestCancel(int request) {
        return RawCoreValues.rawRequestCancel(request);
    }

    public static int playerTransfer(int player, String host, int port) {
        byte[] h = Native.utf8(host);
        return RawProxyInteraction.rawPlayerTransfer(player, Native.at(h), h.length, port);
    }

    public static int playersByUuid(String uuid) {
        byte[] u = Native.utf8(uuid);
        return RawProxyInteraction.rawPlayersByUuid(Native.at(u), u.length);
    }

    /** Players on the proxy, or on `server` when it is not null or empty. */
    public static int playersCount(String server) {
        byte[] s = Native.utf8(server == null ? "" : server);
        return RawProxyInteraction.rawPlayersCount(Native.at(s), s.length);
    }

    public static int playersBroadcast(String text, Args args) {
        byte[] t = Native.utf8(text);
        byte[] a = args == null ? new byte[0] : args.packed();
        return RawProxyInteraction.rawPlayersBroadcast(Native.at(t), t.length, Native.at(a), a.length);
    }

    public static String playerAddress(int player) {
        return fetchText((out, cap) -> RawProxyInteraction.rawPlayerAddress(player, out, cap));
    }

    /** The client port, a negative code when unknown. */
    public static int playerPort(int player) {
        return RawProxyInteraction.rawPlayerPort(player);
    }

    /** The client settings map, or null until the client sent it. */
    public static Documents.Value playerSettings(int player) {
        return Documents.callOut(256, (out, cap) -> RawProxyInteraction.rawPlayerSettings(player, out, cap)).value();
    }

    public static double playerDataGetF64(int player, String key, double fallback) {
        byte[] k = Native.utf8(key);
        return RawProxyInteraction.rawPlayerDataGetF64(player, Native.at(k), k.length, fallback);
    }

    public static int playerDataSetF64(int player, String key, double value) {
        byte[] k = Native.utf8(key);
        return RawProxyInteraction.rawPlayerDataSetF64(player, Native.at(k), k.length, value);
    }
}
