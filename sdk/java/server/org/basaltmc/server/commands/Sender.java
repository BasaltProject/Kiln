package org.basaltmc.server.commands;

import org.basaltmc.server.core.*;

import static org.basaltmc.server.core.Level.*;
import static org.basaltmc.server.core.Priority.*;
import static org.basaltmc.server.internal.Callbacks.*;
import static org.basaltmc.server.internal.Native.*;
import static org.basaltmc.server.internal.RawCore.*;

public final class Sender {
    private Sender() {}

    public static int reply(int ctx, String text) { return reply(ctx, text, null); }

    public static int reply(int ctx, String text, Args args) {
        byte[] t = utf8(text);
        byte[] a = packed(args);
        return rawCommandReply(ctx, at(t), t.length, at(a), a.length);
    }

    public static String name(int ctx) { return fetch((out, cap) -> rawCommandSourceName(ctx, out, cap)); }

    public static boolean isConsole(int ctx) { return rawCommandIsConsole(ctx) != 0; }
}
