using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
using Basalt.Proxy.Core;
using Basalt.Proxy.Internal;
using Basalt.Proxy.Values;
using static Basalt.Proxy.Internal.Native;
using static Basalt.Proxy.Core.Level;
using static Basalt.Proxy.Core.Priority;
namespace Basalt.Proxy.Commands;

public static unsafe partial class Commands {
    public static int Command(string spec, string permission, Handler handler)
    {
        byte[] s = Utf8(spec);
        byte[] perm = Utf8(permission);
        int token = TokenFor(handler);
        fixed (byte* sp = s)
        fixed (byte* pp = perm)
            return RawCommandRegister(sp, s.Length, pp, perm.Length, token);
    }

    public static int CommandReply(int ctx, string text, Args args = null)
    {
        byte[] t = Utf8(text);
        byte[] a = args?.Packed();
        fixed (byte* tp = t)
        fixed (byte* ap = a)
            return RawCommandReply(ctx, tp, t.Length, ap, a == null ? 0 : a.Length);
    }

    public static string CommandArgStr(int ctx, string name)
    {
        byte[] n = Utf8(name);
        return Fetch((o, c) => { fixed (byte* p = n) return RawCommandArgStr(ctx, p, n.Length, o, c); });
    }

    public static long? CommandArgI64(int ctx, string name)
    {
        byte[] n = Utf8(name);
        long value = 0;
        int status;
        fixed (byte* p = n) status = RawCommandArgI64(ctx, p, n.Length, &value);
        return status == 0 ? value : null;
    }

    public static string CommandSourceName(int ctx) => Fetch((o, c) => RawCommandSourceName(ctx, o, c));

    public static bool CommandIsConsole(int ctx) => RawCommandIsConsole(ctx) != 0;
}
