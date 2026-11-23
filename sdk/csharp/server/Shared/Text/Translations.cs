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

namespace Basalt.Server.Text;

public static unsafe partial class Translations {
    public static string Translate(string key, Args args = null)
    {
        byte[] k = Utf8(key);
        byte[] a = args?.Packed();
        return Fetch((o, c) =>
        {
            fixed (byte* kp = k)
            fixed (byte* ap = a)
                return RawTranslate(kp, k.Length, ap, a == null ? 0 : a.Length, o, c);
        });
    }
}
