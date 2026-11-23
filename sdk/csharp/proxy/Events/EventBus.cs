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

// Proxy events area (abi/areas/proxy_events.h): event fields as values, chat, command and kick decisions, plugin
// identity, config arrays and typed command arguments. The *Value calls return the raw encoded value for the values
// reader, or null on a negative code.

public static unsafe partial class EventBus
{
    const byte TagFalse = 1, TagTrue = 2, TagInt = 3, TagString = 5;

    delegate int Fill(byte* output, int cap);

    static byte[] FetchValue(Fill call)
    {
        int cap = 256;
        while (true)
        {
            byte[] buffer = new byte[cap];
            int n;
            fixed (byte* p = buffer)
            {
                n = call(p, cap);
            }
            if (n < 0)
            {
                return null;
            }
            if (n > cap)
            {
                cap = n;
                continue;
            }
            byte[] value = new byte[n];
            for (int i = 0; i < n; ++i)
            {
                value[i] = buffer[i];
            }
            return value;
        }
    }

    static long ReadLeb(byte[] bytes, ref int at)
    {
        long result = 0;
        for (int shift = 0; shift < 70 && at < bytes.Length; shift += 7)
        {
            int b = bytes[at++];
            result |= (long)(b & 0x7f) << shift;
            if ((b & 0x80) == 0)
            {
                return result;
            }
        }
        return -1;
    }

    static void WriteLeb(List<byte> output, ulong value)
    {
        do
        {
            byte b = (byte)(value & 0x7f);
            value >>= 7;
            output.Add(value != 0 ? (byte)(b | 0x80) : b);
        } while (value != 0);
    }

    /// <summary>Registers an observer of the finished event; a decider is registered with Native.On.</summary>
    public static int Watch(string name, Handler handler)
    {
        byte[] n = Encoding.UTF8.GetBytes(name);
        int token = Native.TokenFor(handler);
        fixed (byte* p = n) return RawProxyEvents.EventWatch(p, n.Length, token);
    }

    /// <summary>The encoded value of a field; an empty name gives a map of every readable field.</summary>
    public static byte[] EventGetValue(int ctx, string field)
    {
        byte[] f = Encoding.UTF8.GetBytes(field);
        return FetchValue((o, c) => { fixed (byte* p = f) return RawProxyEvents.EventGet(ctx, p, f.Length, o, c); });
    }

    public static string EventGetStr(int ctx, string field)
    {
        byte[] bytes = EventGetValue(ctx, field);
        if (bytes == null || bytes.Length < 2 || bytes[0] != TagString)
        {
            return null;
        }
        int at = 1;
        long length = ReadLeb(bytes, ref at);
        return length < 0 || at + length > bytes.Length ? null : Encoding.UTF8.GetString(bytes, at, (int)length);
    }

    public static long? EventGetI64(int ctx, string field)
    {
        byte[] bytes = EventGetValue(ctx, field);
        if (bytes == null || bytes.Length < 2 || bytes[0] != TagInt)
        {
            return null;
        }
        int at = 1;
        ulong zigzag = (ulong)ReadLeb(bytes, ref at);
        return (long)(zigzag >> 1) ^ -(long)(zigzag & 1);
    }

    public static bool? EventGetBool(int ctx, string field)
    {
        byte[] bytes = EventGetValue(ctx, field);
        if (bytes == null || bytes.Length != 1 || (bytes[0] != TagTrue && bytes[0] != TagFalse))
        {
            return null;
        }
        return bytes[0] == TagTrue;
    }

    /// <summary>A list of maps (name, type, readable, writable), one per field of the event.</summary>
    public static byte[] EventFields(int ctx) => FetchValue((o, c) => RawProxyEvents.EventFields(ctx, o, c));

    public static int EventSetValue(int ctx, string field, byte[] value)
    {
        byte[] f = Encoding.UTF8.GetBytes(field);
        fixed (byte* fp = f)
        fixed (byte* vp = value)
        {
            return RawProxyEvents.EventSet(ctx, fp, f.Length, vp, value.Length);
        }
    }

    public static int EventSetText(int ctx, string field, string text)
    {
        byte[] t = Encoding.UTF8.GetBytes(text);
        List<byte> value = new List<byte> { TagString };
        WriteLeb(value, (ulong)t.Length);
        value.AddRange(t);
        return EventSetValue(ctx, field, value.ToArray());
    }

    public static int EventSetInt(int ctx, string field, long number)
    {
        List<byte> value = new List<byte> { TagInt };
        WriteLeb(value, ((ulong)number << 1) ^ (ulong)(number >> 63));
        return EventSetValue(ctx, field, value.ToArray());
    }

    public static int EventSetFlag(int ctx, string field, bool flag) =>
        EventSetValue(ctx, field, new byte[] { flag ? TagTrue : TagFalse });

    /// <summary>Cancels a decided event and shows reason (MiniMessage, or a '@' lang key) to the player.</summary>
    public static int EventDeny(int ctx, string reason = "")
    {
        if (!string.IsNullOrEmpty(reason))
        {
            int shown = EventSetText(ctx, "reason", reason);
            if (shown < 0)
            {
                return shown;
            }
        }
        return EventSetFlag(ctx, "cancelled", true);
    }

    /// <summary>"id", "name" or "version".</summary>
    public static string PluginInfo(string field)
    {
        byte[] f = Encoding.UTF8.GetBytes(field);
        byte[] bytes = FetchValue((o, c) => { fixed (byte* p = f) return RawProxyEvents.PluginInfo(p, f.Length, o, c); });
        return bytes == null ? null : Encoding.UTF8.GetString(bytes);
    }

    public static byte[] PluginManifest() => FetchValue((o, c) => RawProxyEvents.PluginManifest(o, c));

    /// <summary>The config.toml value at a dotted key (arrays are lists, tables are maps); an empty key is the whole file.</summary>
    public static byte[] ConfigValue(string key)
    {
        byte[] k = Encoding.UTF8.GetBytes(key);
        return FetchValue((o, c) => { fixed (byte* p = k) return RawProxyEvents.ConfigValue(p, k.Length, o, c); });
    }

    public static double? CommandArgF64(int ctx, string name)
    {
        byte[] n = Encoding.UTF8.GetBytes(name);
        double value = 0;
        int status;
        fixed (byte* p = n) status = RawProxyEvents.CommandArgF64(ctx, p, n.Length, &value);
        return status == 0 ? value : null;
    }

    public static bool? CommandArgBool(int ctx, string name)
    {
        byte[] n = Encoding.UTF8.GetBytes(name);
        int value = 0;
        int status;
        fixed (byte* p = n) status = RawProxyEvents.CommandArgBool(ctx, p, n.Length, &value);
        return status == 0 ? value != 0 : null;
    }

    public static byte[] CommandArgs(int ctx) => FetchValue((o, c) => RawProxyEvents.CommandArgs(ctx, o, c));
}
