package org.basaltmc.proxy.log;

import org.basaltmc.proxy.core.*;

import static org.basaltmc.proxy.core.Level.*;
import static org.basaltmc.proxy.core.Priority.*;
import static org.basaltmc.proxy.internal.Callbacks.*;
import static org.basaltmc.proxy.internal.Native.*;
import static org.basaltmc.proxy.internal.RawCore.*;

abstract class SharedLogger {
    public static void log(int level, String msg) {
        byte[] m = utf8(msg);
        rawLog(level, at(m), m.length);
    }

    public static void logDebug(String msg) { log(DEBUG, msg); }

    public static void logInfo(String msg) { log(INFO, msg); }

    public static void logWarn(String msg) { log(WARN, msg); }

    public static void logError(String msg) { log(ERROR, msg); }
}
