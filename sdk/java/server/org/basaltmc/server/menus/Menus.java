package org.basaltmc.server.menus;
import org.basaltmc.server.core.*;
import static org.basaltmc.server.core.Level.*;
import static org.basaltmc.server.core.Priority.*;
import static org.basaltmc.server.internal.Native.*;
import static org.basaltmc.server.internal.RawCore.*;
import static org.basaltmc.server.internal.Callbacks.*;
import static org.basaltmc.server.internal.RawServerCore.*;

public final class Menus {
    public static int placeholder(String name, Handler handler) {
        byte[] n = utf8(name);
        return rawPlaceholderRegister(at(n), n.length, tokenFor(handler));
    }

    public static int placeholderPlayer(int ctx) { return rawPlaceholderPlayer(ctx); }

    public static int placeholderResult(int ctx, String text) {
        byte[] t = utf8(text);
        return rawPlaceholderResult(ctx, at(t), t.length);
    }

    public static int menuCreate(String title, int rows) {
        byte[] t = utf8(title);
        return rawMenuCreate(at(t), t.length, rows);
    }

    public static int menuSet(int menu, int slot, String item, String name, String lore, Handler onClick) {
        byte[] i = utf8(item);
        byte[] n = utf8(name);
        byte[] l = utf8(lore);
        return rawMenuSet(menu, slot, at(i), i.length, at(n), n.length, at(l), l.length, tokenFor(onClick));
    }

    public static int menuOpen(int menu, int player) { return rawMenuOpen(menu, player); }

    public static int menuClickPlayer(int ctx) { return rawMenuClickPlayer(ctx); }

    /** Proxy plugins only (plugin.toml `target = "proxy"`): the calls of basalt_proxy_abi.h. */
}
