using Basalt.Server.Core;
using Basalt.Server.Internal;
using Basalt.Server.Values;
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
using static Basalt.Server.Core.Level;
using static Basalt.Server.Core.Priority;
using static Basalt.Server.Internal.Native;

namespace Basalt.Server.Menus;

public static unsafe partial class InventoryMenu {
    public static int Placeholder(string name, Handler handler)
    {
        byte[] n = Utf8(name);
        int token = TokenFor(handler);
        fixed (byte* p = n) return RawPlaceholderRegister(p, n.Length, token);
    }

    public static int PlaceholderPlayer(int ctx) => RawPlaceholderPlayer(ctx);

    public static int PlaceholderResult(int ctx, string text)
    {
        byte[] t = Utf8(text);
        fixed (byte* p = t) return RawPlaceholderResult(ctx, p, t.Length);
    }

    public static int MenuCreate(string title, int rows)
    {
        byte[] t = Utf8(title);
        fixed (byte* p = t) return RawMenuCreate(p, t.Length, rows);
    }

    public static int MenuSet(int menu, int slot, string item, string name, string lore, Handler onClick)
    {
        byte[] i = Utf8(item);
        byte[] n = Utf8(name);
        byte[] l = Utf8(lore);
        int token = TokenFor(onClick);
        fixed (byte* ip = i)
        fixed (byte* np = n)
        fixed (byte* lp = l)
            return RawMenuSet(menu, slot, ip, i.Length, np, n.Length, lp, l.Length, token);
    }

    public static int MenuOpen(int menu, int player) => RawMenuOpen(menu, player);

    public static int MenuClickPlayer(int ctx) => RawMenuClickPlayer(ctx);

    /// <summary>Proxy plugins only (plugin.toml `target = "proxy"`): the calls of basalt_proxy_abi.h.</summary>
}
