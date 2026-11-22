package org.basaltmc.server.config;
import org.basaltmc.server.core.*;
import static org.basaltmc.server.core.Level.*;
import static org.basaltmc.server.core.Priority.*;
import static org.basaltmc.server.internal.Native.*;
import static org.basaltmc.server.internal.RawCore.*;
import static org.basaltmc.server.internal.Callbacks.*;

public abstract class SharedConfig {
    public static long configI64(String key, long def) {
        byte[] k = utf8(key);
        return rawConfigI64(at(k), k.length, def);
    }

    public static double configF64(String key, double def) {
        byte[] k = utf8(key);
        return rawConfigF64(at(k), k.length, def);
    }

    public static boolean configBool(String key, boolean def) {
        byte[] k = utf8(key);
        return rawConfigBool(at(k), k.length, def ? 1 : 0) != 0;
    }

    public static String configStr(String key) {
        byte[] k = utf8(key);
        return fetch((out, cap) -> rawConfigStr(at(k), k.length, out, cap));
    }
}
