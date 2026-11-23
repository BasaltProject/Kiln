package org.basaltmc.proxy.commands;

import org.basaltmc.proxy.core.*;

import static org.basaltmc.proxy.core.Level.*;
import static org.basaltmc.proxy.core.Priority.*;
import static org.basaltmc.proxy.internal.Callbacks.*;
import static org.basaltmc.proxy.internal.Native.*;
import static org.basaltmc.proxy.internal.RawCore.*;

public final class Registry {
    private Registry() {}

    public static int register(String spec, String permission, Handler handler) {
        byte[] s = utf8(spec);
        byte[] p = utf8(permission);
        return rawCommandRegister(at(s), s.length, at(p), p.length, tokenFor(handler));
    }
}
