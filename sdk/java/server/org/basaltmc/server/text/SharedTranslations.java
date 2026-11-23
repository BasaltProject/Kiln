package org.basaltmc.server.text;

import org.basaltmc.server.core.*;

import static org.basaltmc.server.core.Level.*;
import static org.basaltmc.server.core.Priority.*;
import static org.basaltmc.server.internal.Callbacks.*;
import static org.basaltmc.server.internal.Native.*;
import static org.basaltmc.server.internal.RawCore.*;

abstract class SharedTranslations {
    public static String translate(String key) { return translate(key, null); }

    public static String translate(String key, Args args) {
        byte[] k = utf8(key);
        byte[] a = packed(args);
        return fetch((out, cap) -> rawTranslate(at(k), k.length, at(a), a.length, out, cap));
    }
}
