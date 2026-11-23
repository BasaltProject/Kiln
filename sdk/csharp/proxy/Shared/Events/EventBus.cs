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

namespace Basalt.Proxy.Events;

public static unsafe partial class EventBus {
    public static int On(string name, int priority, Handler handler)
    {
        byte[] n = Utf8(name);
        int token = TokenFor(handler);
        fixed (byte* p = n) return RawEventOn(p, n.Length, priority, token);
    }

    public static int EventPlayer(int ctx) => RawEventPlayer(ctx);

    public static string EventFieldStr(int ctx, string field)
    {
        byte[] f = Utf8(field);
        return Fetch((o, c) => { fixed (byte* p = f) return RawEventFieldStr(ctx, p, f.Length, o, c); });
    }

    public static long? EventFieldI64(int ctx, string field)
    {
        byte[] f = Utf8(field);
        long value = 0;
        int status;
        fixed (byte* p = f) status = RawEventFieldI64(ctx, p, f.Length, &value);
        return status == 0 ? value : null;
    }

    public static int EventCancel(int ctx, bool cancelled) => RawEventCancel(ctx, cancelled ? 1 : 0);
}
