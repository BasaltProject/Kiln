package org.basaltmc.server.commands;

import org.basaltmc.server.core.*;

import static org.basaltmc.server.core.Level.*;
import static org.basaltmc.server.core.Priority.*;
import static org.basaltmc.server.internal.Callbacks.*;
import static org.basaltmc.server.internal.Native.*;
import static org.basaltmc.server.internal.RawCore.*;

public final class Registry {
    private Registry() {}

    public static int register(String spec, String permission, Handler handler) {
        byte[] s = utf8(spec);
        byte[] p = utf8(permission);
        return rawCommandRegister(at(s), s.length, at(p), p.length, tokenFor(handler));
    }
}
