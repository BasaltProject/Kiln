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

namespace Basalt.Proxy.Config;

public static unsafe partial class Settings {
    public static long ConfigI64(string key, long def)
    {
        byte[] k = Utf8(key);
        fixed (byte* p = k) return RawConfigI64(p, k.Length, def);
    }

    public static double ConfigF64(string key, double def)
    {
        byte[] k = Utf8(key);
        fixed (byte* p = k) return RawConfigF64(p, k.Length, def);
    }

    public static bool ConfigBool(string key, bool def)
    {
        byte[] k = Utf8(key);
        fixed (byte* p = k) return RawConfigBool(p, k.Length, def ? 1 : 0) != 0;
    }

    public static string ConfigStr(string key)
    {
        byte[] k = Utf8(key);
        return Fetch((o, c) => { fixed (byte* p = k) return RawConfigStr(p, k.Length, o, c); });
    }
}
