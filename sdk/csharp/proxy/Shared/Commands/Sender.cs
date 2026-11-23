using Basalt.Proxy.Core;
using Basalt.Proxy.Internal;
using Basalt.Proxy.Values;
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
using static Basalt.Proxy.Core.Level;
using static Basalt.Proxy.Core.Priority;
using static Basalt.Proxy.Internal.Native;

namespace Basalt.Proxy.Commands;

public static unsafe class Sender {
    public static int Reply(int ctx, string text, Args args = null)
    {
        byte[] t = Utf8(text);
        byte[] a = args?.Packed();
        fixed (byte* tp = t)
        fixed (byte* ap = a)
            return RawCommandReply(ctx, tp, t.Length, ap, a == null ? 0 : a.Length);
    }

    public static string Name(int ctx) => Fetch((o, c) => RawCommandSourceName(ctx, o, c));

    public static bool IsConsole(int ctx) => RawCommandIsConsole(ctx) != 0;
}
