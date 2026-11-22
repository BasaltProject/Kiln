package org.basaltmc.proxy.events;
import org.teavm.interop.Address;
import org.basaltmc.proxy.core.*;
import static org.basaltmc.proxy.core.Level.*;
import static org.basaltmc.proxy.core.Priority.*;
import static org.basaltmc.proxy.internal.Native.*;
import static org.basaltmc.proxy.internal.RawCore.*;
import static org.basaltmc.proxy.internal.Callbacks.*;

public abstract class SharedEvents {
    public static int on(String event, int priority, Handler handler) {
        byte[] e = utf8(event);
        return rawEventOn(at(e), e.length, priority, tokenFor(handler));
    }

    public static int eventPlayer(int ctx) { return rawEventPlayer(ctx); }

    public static String eventFieldStr(int ctx, String field) {
        byte[] f = utf8(field);
        return fetch((out, cap) -> rawEventFieldStr(ctx, at(f), f.length, out, cap));
    }

    public static Long eventFieldI64(int ctx, String field) {
        byte[] f = utf8(field);
        long[] out = new long[1];
        return rawEventFieldI64(ctx, at(f), f.length, Address.ofData(out)) == 0 ? Long.valueOf(out[0]) : null;
    }

    public static int eventCancel(int ctx, boolean cancelled) { return rawEventCancel(ctx, cancelled ? 1 : 0); }
}
