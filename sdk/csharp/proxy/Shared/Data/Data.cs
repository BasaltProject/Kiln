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
namespace Basalt.Proxy.Data;

public static unsafe partial class Data {
    public static long DataGetI64(int player, string key, long def)
    {
        byte[] k = Utf8(key);
        fixed (byte* p = k) return RawDataGetI64(player, p, k.Length, def);
    }

    public static int DataSetI64(int player, string key, long value)
    {
        byte[] k = Utf8(key);
        fixed (byte* p = k) return RawDataSetI64(player, p, k.Length, value);
    }

    public static bool DataGetBool(int player, string key, bool def)
    {
        byte[] k = Utf8(key);
        fixed (byte* p = k) return RawDataGetBool(player, p, k.Length, def ? 1 : 0) != 0;
    }

    public static int DataSetBool(int player, string key, bool value)
    {
        byte[] k = Utf8(key);
        fixed (byte* p = k) return RawDataSetBool(player, p, k.Length, value ? 1 : 0);
    }

    public static string DataGetStr(int player, string key)
    {
        byte[] k = Utf8(key);
        return Fetch((o, c) => { fixed (byte* p = k) return RawDataGetStr(player, p, k.Length, o, c); });
    }

    public static int DataSetStr(int player, string key, string value)
    {
        byte[] k = Utf8(key);
        byte[] v = Utf8(value);
        fixed (byte* kp = k)
        fixed (byte* vp = v)
            return RawDataSetStr(player, kp, k.Length, vp, v.Length);
    }

    public static int DataRemove(int player, string key)
    {
        byte[] k = Utf8(key);
        fixed (byte* p = k) return RawDataRemove(player, p, k.Length);
    }
}
