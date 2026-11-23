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

public static unsafe class Registry {
    public static int Register(string spec, string permission, Handler handler)
    {
        byte[] s = Utf8(spec);
        byte[] perm = Utf8(permission);
        int token = TokenFor(handler);
        fixed (byte* sp = s)
        fixed (byte* pp = perm)
            return RawCommandRegister(sp, s.Length, pp, perm.Length, token);
    }
}
