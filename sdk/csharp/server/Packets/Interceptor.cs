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

namespace Basalt.Server.Packets;

// Plugin channels and raw packets (abi/areas/packets.h). Calls return the import result: a count or handle, or a
// negative ABI code. The out-calls return an OutResult: Error is 0 or the negative code, Bytes the answer, and
// TryValue opens it as a document.
public static unsafe class Interceptor
{
    const int Initial = 1024;

    static byte[] Utf8(string text) => Encoding.UTF8.GetBytes(text);

    public static int ChannelListen(string name, Handler handler)
    {
        byte[] n = Utf8(name);
        int token = Native.TokenFor(handler);
        fixed (byte* np = n) return RawPackets.ChannelListen(np, n.Length, token);
    }

    public static int ChannelSend(int player, string name, byte[] body)
    {
        byte[] n = Utf8(name);
        fixed (byte* np = n)
        fixed (byte* bp = body)
            return RawPackets.ChannelSend(player, np, n.Length, bp, body.Length);
    }

    /// <summary>priority is "first", "early", "normal", "late", "last" or null for the default.</summary>
    public static int PacketListen(string pattern, string priority, bool observe, Handler handler)
    {
        byte[] document = new byte[0];
        if (priority != null || observe)
        {
            ValueWriter writer = new ValueWriter().BeginMap();
            if (priority != null) writer.Key("priority").String(priority);
            if (observe) writer.Key("observe").Bool(true);
            document = writer.End().Finish();
            if (document == null) return -3;
        }
        byte[] p = Utf8(pattern);
        int token = Native.TokenFor(handler);
        fixed (byte* pp = p)
        fixed (byte* dp = document)
            return RawPackets.PacketListen(pp, p.Length, dp, document.Length, token);
    }

    public static int PacketUnlisten(int listener) => RawPackets.PacketUnlisten(listener);

    public static int PacketSend(int player, string name, byte[] body, bool intercept)
    {
        byte[] n = Utf8(name);
        fixed (byte* np = n)
        fixed (byte* bp = body)
            return RawPackets.PacketSend(player, np, n.Length, bp, body.Length, intercept ? 1 : 0);
    }

    public static int PacketReceive(int player, string name, byte[] body, bool intercept)
    {
        byte[] n = Utf8(name);
        fixed (byte* np = n)
        fixed (byte* bp = body)
            return RawPackets.PacketReceive(player, np, n.Length, bp, body.Length, intercept ? 1 : 0);
    }

    public static OutResult PacketLookup(string name)
    {
        byte[] n = Utf8(name);
        return ValueCall.Out(Initial, (o, c) => { fixed (byte* np = n) return RawPackets.PacketLookup(np, n.Length, o, c); });
    }

    public static OutResult PacketNames(string prefix)
    {
        byte[] n = Utf8(prefix);
        return ValueCall.Out(Initial, (o, c) => { fixed (byte* np = n) return RawPackets.PacketNames(np, n.Length, o, c); });
    }

    public static OutResult PacketPlayerActions() => ValueCall.Out(Initial, (o, c) => RawPackets.PacketPlayerActions(o, c));

    /// <summary>args is an encoded list value.</summary>
    public static int PacketPlayerReceive(int player, string action, byte[] args)
    {
        byte[] a = Utf8(action);
        fixed (byte* ap = a)
        fixed (byte* gp = args)
            return RawPackets.PacketPlayerReceive(player, ap, a.Length, gp, args.Length);
    }

    public static OutResult PacketCtxInfo(int ctx) => ValueCall.Out(Initial, (o, c) => RawPackets.PacketCtxInfo(ctx, o, c));

    public static OutResult PacketCtxBody(int ctx) => ValueCall.Out(Initial, (o, c) => RawPackets.PacketCtxBody(ctx, o, c));

    public static int PacketCtxPlayer(int ctx) => RawPackets.PacketCtxPlayer(ctx);

    public static int PacketCtxCancel(int ctx) => RawPackets.PacketCtxCancel(ctx);

    public static int PacketCtxSetBody(int ctx, byte[] body)
    {
        fixed (byte* bp = body) return RawPackets.PacketCtxSetBody(ctx, bp, body.Length);
    }

    public static int PacketCtxSend(int ctx, string name, byte[] body, bool intercept)
    {
        byte[] n = Utf8(name);
        fixed (byte* np = n)
        fixed (byte* bp = body)
            return RawPackets.PacketCtxSend(ctx, np, n.Length, bp, body.Length, intercept ? 1 : 0);
    }

    public static int PacketCtxReceive(int ctx, string name, byte[] body, bool intercept)
    {
        byte[] n = Utf8(name);
        fixed (byte* np = n)
        fixed (byte* bp = body)
            return RawPackets.PacketCtxReceive(ctx, np, n.Length, bp, body.Length, intercept ? 1 : 0);
    }
}
