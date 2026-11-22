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
namespace Basalt.Proxy.Text;

public static unsafe partial class Text {
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
