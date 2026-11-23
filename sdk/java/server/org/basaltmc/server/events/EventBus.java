package org.basaltmc.server.events;

import org.basaltmc.server.core.*;
import org.basaltmc.server.internal.*;
import org.basaltmc.server.values.Documents;
import org.basaltmc.server.values.Documents.OutResult;
import org.basaltmc.server.values.Documents.Value;
import org.teavm.interop.Address;

/** Events area: strings and byte arrays over the raw imports, documents as Values.Value (null on a failed call). */
public final class EventBus extends SharedEventBus {
    private EventBus() {}

    public static final int POST = 1;
    public static final int CANCELLED = 2;

    private static final byte[] NONE = new byte[0];

    private interface Fill {
        int call(Address out, int cap);
    }

    private static String text(int capacity, Fill fill) {
        OutResult result = Documents.callOut(capacity, fill::call);
        return result.error == Documents.OK ? Native.text(result.bytes, result.bytes.length) : null;
    }

    private static Value value(int initial, Fill fill) {
        return Documents.callOut(initial, fill::call).value();
    }

    private static byte[] bytes(byte[] data) {
        return data == null ? NONE : data;
    }

    /** Listens to `name`; `filters` is a finished map document or null. */
    public static int onEx(String name, int priority, int flags, byte[] filters, Handler handler) {
        byte[] n = Native.utf8(name);
        byte[] f = bytes(filters);
        return RawEvents.rawEventOnEx(Native.at(n), n.length, priority, flags, Native.at(f), f.length,
                Callbacks.tokenFor(handler));
    }

    public static String eventName(int ctx) {
        return text(64, (out, cap) -> RawEvents.rawEventName(ctx, out, cap));
    }

    public static Value eventGet(int ctx, String field, int initial) {
        byte[] f = Native.utf8(field);
        return value(initial, (out, cap) -> RawEvents.rawEventGet(ctx, Native.at(f), f.length, out, cap));
    }

    public static int eventSet(int ctx, String field, byte[] value) {
        byte[] f = Native.utf8(field);
        byte[] v = bytes(value);
        return RawEvents.rawEventSet(ctx, Native.at(f), f.length, Native.at(v), v.length);
    }

    public static Value eventFields(int ctx, int initial) {
        return value(initial, (out, cap) -> RawEvents.rawEventFields(ctx, out, cap));
    }

    public static Value eventPayload(int ctx, int initial) {
        return value(initial, (out, cap) -> RawEvents.rawEventPayload(ctx, out, cap));
    }

    public static Value eventDescribe(String name, int initial) {
        byte[] n = Native.utf8(name);
        return value(initial, (out, cap) -> RawEvents.rawEventDescribe(Native.at(n), n.length, out, cap));
    }

    public static Value eventNames(int initial) {
        return value(initial, (out, cap) -> RawEvents.rawEventNames(out, cap));
    }

    public static int eventCancelled(int ctx) {
        return RawEvents.rawEventCancelled(ctx);
    }

    public static int eventCancelWith(int ctx, String text, Args args) {
        byte[] t = Native.utf8(text);
        byte[] a = Native.packed(args);
        return RawEvents.rawEventCancelWith(ctx, Native.at(t), t.length, Native.at(a), a.length);
    }

    public static String eventCancelMessage(int ctx) {
        return text(256, (out, cap) -> RawEvents.rawEventCancelMessage(ctx, out, cap));
    }

    public static int eventIsPost(int ctx) {
        return RawEvents.rawEventIsPost(ctx);
    }

    public static int eventEmit(String name, byte[] value) {
        byte[] n = Native.utf8(name);
        byte[] v = bytes(value);
        return RawEvents.rawEventEmit(Native.at(n), n.length, Native.at(v), v.length);
    }

    public static int playerByUuid(String uuid) {
        byte[] u = Native.utf8(uuid);
        return RawEvents.rawPlayerByUuid(Native.at(u), u.length);
    }

    public static String playerUuid(int player) {
        return text(64, (out, cap) -> RawEvents.rawPlayerUuid(player, out, cap));
    }

    public static Value playerGet(int player, String name, int initial) {
        byte[] n = Native.utf8(name);
        return value(initial, (out, cap) -> RawEvents.rawPlayerGet(player, Native.at(n), n.length, out, cap));
    }

    public static int playerSet(int player, String name, byte[] value) {
        byte[] n = Native.utf8(name);
        byte[] v = bytes(value);
        return RawEvents.rawPlayerSet(player, Native.at(n), n.length, Native.at(v), v.length);
    }

    /** Runs a named operation; `args` is a finished list document or null. */
    public static Value playerCall(int player, String op, byte[] args, int initial) {
        byte[] o = Native.utf8(op);
        byte[] a = bytes(args);
        return value(initial,
                (out, cap) -> RawEvents.rawPlayerCall(player, Native.at(o), o.length, Native.at(a), a.length, out, cap));
    }

    public static Value playerDescribe(int initial) {
        return value(initial, (out, cap) -> RawEvents.rawPlayerDescribe(out, cap));
    }

    /** The health in half hearts, NaN when the call failed. */
    public static double playerHealth(int player) {
        byte[] box = new byte[8];
        if (RawEvents.rawPlayerHealth(player, Native.at(box)) < 0) {
            return Double.NaN;
        }
        long bits = 0;
        for (int i = 7; i >= 0; i--) {
            bits = (bits << 8) | (box[i] & 0xFF);
        }
        return Double.longBitsToDouble(bits);
    }

    public static int playerSetHealth(int player, double health) {
        return RawEvents.rawPlayerSetHealth(player, health);
    }

    public static String playerGamemode(int player) {
        return text(16, (out, cap) -> RawEvents.rawPlayerGamemode(player, out, cap));
    }

    public static int playerSetGamemode(int player, String mode) {
        byte[] m = Native.utf8(mode);
        return RawEvents.rawPlayerSetGamemode(player, Native.at(m), m.length);
    }

    public static int playerTeleport(int player, String world, double x, double y, double z, float yaw, float pitch) {
        byte[] w = Native.utf8(world);
        return RawEvents.rawPlayerTeleport(player, Native.at(w), w.length, x, y, z, yaw, pitch);
    }

    public static int playerTitle(int player, String title, String subtitle, int fadeIn, int stay, int fadeOut,
            Args args) {
        byte[] t = Native.utf8(title);
        byte[] s = Native.utf8(subtitle);
        byte[] a = Native.packed(args);
        return RawEvents.rawPlayerTitle(player, Native.at(t), t.length, Native.at(s), s.length, fadeIn, stay, fadeOut,
                Native.at(a), a.length);
    }

    public static int playerKick(int player, String text, Args args) {
        byte[] t = Native.utf8(text);
        byte[] a = Native.packed(args);
        return RawEvents.rawPlayerKick(player, Native.at(t), t.length, Native.at(a), a.length);
    }

    public static int playerEffect(int player, String effect, int ticks, int amplifier) {
        byte[] e = Native.utf8(effect);
        return RawEvents.rawPlayerEffect(player, Native.at(e), e.length, ticks, amplifier);
    }

    public static int playerOpenInventory(int player, String kind, byte[] spec) {
        byte[] k = Native.utf8(kind);
        byte[] s = bytes(spec);
        return RawEvents.rawPlayerOpenInventory(player, Native.at(k), k.length, Native.at(s), s.length);
    }

    public static int playerCloseWindow(int player) {
        return RawEvents.rawPlayerCloseWindow(player);
    }

    /** Shows a form (a finished document); the handler runs once with the answer, read with `completionValue`. */
    public static int formOpen(int player, byte[] form, Handler handler) {
        byte[] f = bytes(form);
        return RawEvents.rawFormOpen(player, Native.at(f), f.length, Callbacks.tokenFor(handler));
    }

    public static int formClose(int player) {
        return RawEvents.rawFormClose(player);
    }

    public static int dialogShow(int player, String dialog, Handler handler) {
        byte[] d = Native.utf8(dialog);
        return RawEvents.rawDialogShow(player, Native.at(d), d.length, Callbacks.tokenFor(handler));
    }

    public static int dialogClear(int player) {
        return RawEvents.rawDialogClear(player);
    }

    public static Value completionValue(int ctx, int initial) {
        return value(initial, (out, cap) -> RawCoreValues.rawValueResult(ctx, out, cap));
    }
}
