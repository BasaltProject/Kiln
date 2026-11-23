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

namespace Basalt.Server.Commands;

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
