package org.basaltmc.server.log;
import org.basaltmc.server.core.*;
import static org.basaltmc.server.core.Level.*;
import static org.basaltmc.server.core.Priority.*;
import static org.basaltmc.server.internal.Native.*;
import static org.basaltmc.server.internal.RawCore.*;
import static org.basaltmc.server.internal.Callbacks.*;

public abstract class SharedLog {
    public static void log(int level, String msg) {
        byte[] m = utf8(msg);
        rawLog(level, at(m), m.length);
    }

    public static void logDebug(String msg) { log(DEBUG, msg); }

    public static void logInfo(String msg) { log(INFO, msg); }

    public static void logWarn(String msg) { log(WARN, msg); }

    public static void logError(String msg) { log(ERROR, msg); }
}
