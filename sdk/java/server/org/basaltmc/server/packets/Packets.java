package org.basaltmc.server.packets;
import org.basaltmc.server.internal.*;
import org.basaltmc.server.values.Values;
import org.basaltmc.server.core.*;

import static org.basaltmc.server.internal.RawPackets.*;

/**
 * Plugin channels and raw packets (abi/areas/packets.h). Calls return the import result: a count or handle, or a
 * negative ABI code. The out-calls return a Values.OutResult: `error` is OK or the negative code, `bytes` the answer,
 * and `value()` opens it as a document.
 */
public final class Packets {
    private Packets() {}

    private static final int INITIAL = 1024;

    public static int channelListen(String name, Handler handler) {
        byte[] n = Native.utf8(name);
        return rawChannelListen(Native.at(n), n.length, Callbacks.tokenFor(handler));
    }

    public static int channelSend(int player, String name, byte[] body) {
        byte[] n = Native.utf8(name);
        return rawChannelSend(player, Native.at(n), n.length, Native.at(body), body.length);
    }

    /** `priority` is "first", "early", "normal", "late", "last" or null for the default. */
    public static int packetListen(String pattern, String priority, boolean observe, Handler handler) {
        byte[] document = new byte[0];
        if (priority != null || observe) {
            Values.Writer writer = new Values.Writer().beginMap();
            if (priority != null) {
                writer.key("priority").string(priority);
            }
            if (observe) {
                writer.key("observe").bool(true);
            }
            document = writer.end().finish();
            if (document == null) {
                return Values.INVALID;
            }
        }
        byte[] p = Native.utf8(pattern);
        return rawPacketListen(Native.at(p), p.length, Native.at(document), document.length, Callbacks.tokenFor(handler));
    }

    public static int packetUnlisten(int listener) { return rawPacketUnlisten(listener); }

    public static int packetSend(int player, String name, byte[] body, boolean intercept) {
        byte[] n = Native.utf8(name);
        return rawPacketSend(player, Native.at(n), n.length, Native.at(body), body.length, intercept ? 1 : 0);
    }

    public static int packetReceive(int player, String name, byte[] body, boolean intercept) {
        byte[] n = Native.utf8(name);
        return rawPacketReceive(player, Native.at(n), n.length, Native.at(body), body.length, intercept ? 1 : 0);
    }

    public static Values.OutResult packetLookup(String name) {
        byte[] n = Native.utf8(name);
        return Values.callOut(INITIAL, (out, cap) -> rawPacketLookup(Native.at(n), n.length, out, cap));
    }

    public static Values.OutResult packetNames(String prefix) {
        byte[] n = Native.utf8(prefix);
        return Values.callOut(INITIAL, (out, cap) -> rawPacketNames(Native.at(n), n.length, out, cap));
    }

    public static Values.OutResult packetPlayerActions() {
        return Values.callOut(INITIAL, (out, cap) -> rawPacketPlayerActions(out, cap));
    }

    /** `args` is an encoded list value. */
    public static int packetPlayerReceive(int player, String action, byte[] args) {
        byte[] a = Native.utf8(action);
        return rawPacketPlayerReceive(player, Native.at(a), a.length, Native.at(args), args.length);
    }

    public static Values.OutResult packetCtxInfo(int ctx) {
        return Values.callOut(INITIAL, (out, cap) -> rawPacketCtxInfo(ctx, out, cap));
    }

    public static Values.OutResult packetCtxBody(int ctx) {
        return Values.callOut(INITIAL, (out, cap) -> rawPacketCtxBody(ctx, out, cap));
    }

    public static int packetCtxPlayer(int ctx) { return rawPacketCtxPlayer(ctx); }

    public static int packetCtxCancel(int ctx) { return rawPacketCtxCancel(ctx); }

    public static int packetCtxSetBody(int ctx, byte[] body) {
        return rawPacketCtxSetBody(ctx, Native.at(body), body.length);
    }

    public static int packetCtxSend(int ctx, String name, byte[] body, boolean intercept) {
        byte[] n = Native.utf8(name);
        return rawPacketCtxSend(ctx, Native.at(n), n.length, Native.at(body), body.length, intercept ? 1 : 0);
    }

    public static int packetCtxReceive(int ctx, String name, byte[] body, boolean intercept) {
        byte[] n = Native.utf8(name);
        return rawPacketCtxReceive(ctx, Native.at(n), n.length, Native.at(body), body.length, intercept ? 1 : 0);
    }
}
