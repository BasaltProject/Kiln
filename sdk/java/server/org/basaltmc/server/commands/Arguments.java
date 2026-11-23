package org.basaltmc.server.commands;

import org.basaltmc.server.core.*;
import org.teavm.interop.Address;

import static org.basaltmc.server.core.Level.*;
import static org.basaltmc.server.core.Priority.*;
import static org.basaltmc.server.internal.Callbacks.*;
import static org.basaltmc.server.internal.Native.*;
import static org.basaltmc.server.internal.RawCore.*;

public final class Arguments {
    private Arguments() {}

    public static String str(int ctx, String name) {
        byte[] n = utf8(name);
        return fetch((out, cap) -> rawCommandArgStr(ctx, at(n), n.length, out, cap));
    }

    public static Long i64(int ctx, String name) {
        byte[] n = utf8(name);
        long[] out = new long[1];
        return rawCommandArgI64(ctx, at(n), n.length, Address.ofData(out)) == 0 ? Long.valueOf(out[0]) : null;
    }
}
