package org.basaltmc.server.commands;
import org.teavm.interop.Address;
import org.basaltmc.server.core.*;
import static org.basaltmc.server.core.Level.*;
import static org.basaltmc.server.core.Priority.*;
import static org.basaltmc.server.internal.Native.*;
import static org.basaltmc.server.internal.RawCore.*;
import static org.basaltmc.server.internal.Callbacks.*;

public abstract class SharedCommands {
    public static int command(String spec, String permission, Handler handler) {
        byte[] s = utf8(spec);
        byte[] p = utf8(permission);
        return rawCommandRegister(at(s), s.length, at(p), p.length, tokenFor(handler));
    }

    public static int commandReply(int ctx, String text) { return commandReply(ctx, text, null); }

    public static int commandReply(int ctx, String text, Args args) {
        byte[] t = utf8(text);
        byte[] a = packed(args);
        return rawCommandReply(ctx, at(t), t.length, at(a), a.length);
    }

    public static String commandArgStr(int ctx, String name) {
        byte[] n = utf8(name);
        return fetch((out, cap) -> rawCommandArgStr(ctx, at(n), n.length, out, cap));
    }

    public static Long commandArgI64(int ctx, String name) {
        byte[] n = utf8(name);
        long[] out = new long[1];
        return rawCommandArgI64(ctx, at(n), n.length, Address.ofData(out)) == 0 ? Long.valueOf(out[0]) : null;
    }

    public static String commandSourceName(int ctx) { return fetch((out, cap) -> rawCommandSourceName(ctx, out, cap)); }

    public static boolean commandIsConsole(int ctx) { return rawCommandIsConsole(ctx) != 0; }
}
