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
