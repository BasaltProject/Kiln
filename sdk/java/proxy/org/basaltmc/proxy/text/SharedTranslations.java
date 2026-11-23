package org.basaltmc.proxy.text;

import org.basaltmc.proxy.core.*;

import static org.basaltmc.proxy.core.Level.*;
import static org.basaltmc.proxy.core.Priority.*;
import static org.basaltmc.proxy.internal.Callbacks.*;
import static org.basaltmc.proxy.internal.Native.*;
import static org.basaltmc.proxy.internal.RawCore.*;

abstract class SharedTranslations {
    public static String translate(String key) { return translate(key, null); }

    public static String translate(String key, Args args) {
        byte[] k = utf8(key);
        byte[] a = packed(args);
        return fetch((out, cap) -> rawTranslate(at(k), k.length, at(a), a.length, out, cap));
    }
}
