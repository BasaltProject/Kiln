package org.basaltmc.proxy.events;

import java.nio.charset.StandardCharsets;
import org.basaltmc.proxy.core.*;
import org.basaltmc.proxy.internal.*;
import org.basaltmc.proxy.internal.Proxy;
import org.teavm.interop.Address;

/** Proxy events area (abi/areas/proxy_events.h): event fields as values, decisions, identity, config and arguments. */
public final class EventBus extends SharedEventBus {
    private EventBus() {}

    private static final int TAG_FALSE = 1;
    private static final int TAG_TRUE = 2;
    private static final int TAG_INT = 3;
    private static final int TAG_STRING = 5;

    private interface Fetch {
        int call(Address out, int cap);
    }

    /** The encoded value a call answered, or null when it failed (see lastError). */
    private static byte[] fetchValue(Fetch call) {
        int cap = 256;
        while (true) {
            byte[] out = new byte[cap];
            int n = call.call(Native.at(out), cap);
            if (n < 0) {
                lastError = n;
                return null;
            }
            if (n <= cap) {
                byte[] value = new byte[n];
                System.arraycopy(out, 0, value, 0, n);
                return value;
            }
            cap = n;
        }
    }

    private static int lastError;

    /** The negative ABI code of the last value call that returned null. */
    public static int lastError() { return lastError; }

    private static long readLeb(byte[] bytes, int[] at) {
        long result = 0;
        for (int shift = 0; shift < 70 && at[0] < bytes.length; shift += 7) {
            int b = bytes[at[0]++] & 0xff;
            result |= (long) (b & 0x7f) << shift;
            if ((b & 0x80) == 0) {
                return result;
            }
        }
        return -1;
    }

    private static void writeLeb(java.io.ByteArrayOutputStream out, long value) {
        do {
            int b = (int) (value & 0x7f);
            value >>>= 7;
            out.write(value != 0 ? b | 0x80 : b);
        } while (value != 0);
    }

    /** Registers an observer of the finished event; a decider is registered with Native.on. */
    public static int watch(String event, Handler handler) {
        byte[] e = Native.utf8(event);
        return RawProxyEvents.rawEventWatch(Native.at(e), e.length, Callbacks.tokenFor(handler));
    }

    /** The encoded value of a field; an empty name gives a map of every readable field. Null on a negative code. */
    public static byte[] eventGetValue(int ctx, String field) {
        byte[] f = Native.utf8(field);
        return fetchValue((out, cap) -> RawProxyEvents.rawEventGet(ctx, Native.at(f), f.length, out, cap));
    }

    public static String eventGetStr(int ctx, String field) {
        byte[] bytes = eventGetValue(ctx, field);
        if (bytes == null || bytes.length < 2 || bytes[0] != TAG_STRING) {
            return null;
        }
        int[] at = {1};
        long length = readLeb(bytes, at);
        if (length < 0 || at[0] + length > bytes.length) {
            return null;
        }
        return new String(bytes, at[0], (int) length, StandardCharsets.UTF_8);
    }

    public static Long eventGetI64(int ctx, String field) {
        byte[] bytes = eventGetValue(ctx, field);
        if (bytes == null || bytes.length < 2 || bytes[0] != TAG_INT) {
            return null;
        }
        int[] at = {1};
        long zigzag = readLeb(bytes, at);
        return (zigzag >>> 1) ^ -(zigzag & 1);
    }

    public static Boolean eventGetBool(int ctx, String field) {
        byte[] bytes = eventGetValue(ctx, field);
        if (bytes == null || bytes.length != 1 || (bytes[0] != TAG_TRUE && bytes[0] != TAG_FALSE)) {
            return null;
        }
        return bytes[0] == TAG_TRUE;
    }

    /** A list of maps (name, type, readable, writable), one per field of the event. */
    public static byte[] eventFields(int ctx) {
        return fetchValue((out, cap) -> RawProxyEvents.rawEventFields(ctx, out, cap));
    }

    public static int eventSetValue(int ctx, String field, byte[] value) {
        byte[] f = Native.utf8(field);
        return RawProxyEvents.rawEventSet(ctx, Native.at(f), f.length, Native.at(value), value.length);
    }

    public static int eventSetText(int ctx, String field, String text) {
        byte[] t = Native.utf8(text);
        java.io.ByteArrayOutputStream out = new java.io.ByteArrayOutputStream();
        out.write(TAG_STRING);
        writeLeb(out, t.length);
        out.write(t, 0, t.length);
        return eventSetValue(ctx, field, out.toByteArray());
    }

    public static int eventSetInt(int ctx, String field, long number) {
        java.io.ByteArrayOutputStream out = new java.io.ByteArrayOutputStream();
        out.write(TAG_INT);
        writeLeb(out, (number << 1) ^ (number >> 63));
        return eventSetValue(ctx, field, out.toByteArray());
    }

    public static int eventSetFlag(int ctx, String field, boolean flag) {
        return eventSetValue(ctx, field, new byte[] {(byte) (flag ? TAG_TRUE : TAG_FALSE)});
    }

    /** Cancels a decided event and shows reason (MiniMessage, or a '@' lang key) to the player. */
    public static int eventDeny(int ctx, String reason) {
        if (reason != null && !reason.isEmpty()) {
            int shown = eventSetText(ctx, "reason", reason);
            if (shown < 0) {
                return shown;
            }
        }
        return eventSetFlag(ctx, "cancelled", true);
    }

    /** "id", "name" or "version". */
    public static String pluginInfo(String field) {
        byte[] f = Native.utf8(field);
        byte[] bytes = fetchValue((out, cap) -> RawProxyEvents.rawPluginInfo(Native.at(f), f.length, out, cap));
        return bytes == null ? null : new String(bytes, StandardCharsets.UTF_8);
    }

    public static byte[] pluginManifest() {
        return fetchValue((out, cap) -> RawProxyEvents.rawPluginManifest(out, cap));
    }

    /** The config.toml value at a dotted key (arrays are lists, tables are maps); an empty key is the whole file. */
    public static byte[] configValue(String key) {
        byte[] k = Native.utf8(key);
        return fetchValue((out, cap) -> RawProxyEvents.rawConfigValue(Native.at(k), k.length, out, cap));
    }

    public static Double commandArgF64(int ctx, String name) {
        byte[] n = Native.utf8(name);
        double[] out = new double[1];
        return RawProxyEvents.rawCommandArgF64(ctx, Native.at(n), n.length, Address.ofData(out)) == 0 ? Double.valueOf(out[0]) : null;
    }

    public static Boolean commandArgBool(int ctx, String name) {
        byte[] n = Native.utf8(name);
        int[] out = new int[1];
        return RawProxyEvents.rawCommandArgBool(ctx, Native.at(n), n.length, Address.ofData(out)) == 0 ? Boolean.valueOf(out[0] != 0) : null;
    }

    public static byte[] commandArgs(int ctx) {
        return fetchValue((out, cap) -> RawProxyEvents.rawCommandArgs(ctx, out, cap));
    }
    public static int eventSetStr(int ctx, String field, String value) { return Proxy.eventSetStr(ctx, field, value); }
    public static int eventSetI64(int ctx, String field, long value) { return Proxy.eventSetI64(ctx, field, value); }
}
