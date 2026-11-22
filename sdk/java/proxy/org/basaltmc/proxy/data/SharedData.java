package org.basaltmc.proxy.data;
import org.basaltmc.proxy.core.*;
import static org.basaltmc.proxy.core.Level.*;
import static org.basaltmc.proxy.core.Priority.*;
import static org.basaltmc.proxy.internal.Native.*;
import static org.basaltmc.proxy.internal.RawCore.*;
import static org.basaltmc.proxy.internal.Callbacks.*;

public abstract class SharedData {
    public static long dataGetI64(int player, String key, long def) {
        byte[] k = utf8(key);
        return rawDataGetI64(player, at(k), k.length, def);
    }

    public static int dataSetI64(int player, String key, long value) {
        byte[] k = utf8(key);
        return rawDataSetI64(player, at(k), k.length, value);
    }

    public static boolean dataGetBool(int player, String key, boolean def) {
        byte[] k = utf8(key);
        return rawDataGetBool(player, at(k), k.length, def ? 1 : 0) != 0;
    }

    public static int dataSetBool(int player, String key, boolean value) {
        byte[] k = utf8(key);
        return rawDataSetBool(player, at(k), k.length, value ? 1 : 0);
    }

    public static String dataGetStr(int player, String key) {
        byte[] k = utf8(key);
        return fetch((out, cap) -> rawDataGetStr(player, at(k), k.length, out, cap));
    }

    public static int dataSetStr(int player, String key, String value) {
        byte[] k = utf8(key);
        byte[] v = utf8(value);
        return rawDataSetStr(player, at(k), k.length, at(v), v.length);
    }

    public static int dataRemove(int player, String key) {
        byte[] k = utf8(key);
        return rawDataRemove(player, at(k), k.length);
    }
}
