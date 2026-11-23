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

public static unsafe class Arguments {
    public static string Str(int ctx, string name)
    {
        byte[] n = Utf8(name);
        return Fetch((o, c) => { fixed (byte* p = n) return RawCommandArgStr(ctx, p, n.Length, o, c); });
    }

    public static long? I64(int ctx, string name)
    {
        byte[] n = Utf8(name);
        long value = 0;
        int status;
        fixed (byte* p = n) status = RawCommandArgI64(ctx, p, n.Length, &value);
        return status == 0 ? value : null;
    }
}
