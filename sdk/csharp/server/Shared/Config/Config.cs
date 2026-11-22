using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
using Basalt.Server.Core;
using Basalt.Server.Internal;
using Basalt.Server.Values;
using static Basalt.Server.Internal.Native;
using static Basalt.Server.Core.Level;
using static Basalt.Server.Core.Priority;
namespace Basalt.Server.Config;

public static unsafe partial class Config {
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
