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

namespace Basalt.Proxy.Network;

// Proxy network area (abi/areas/proxy_network.h): messaging channels, raw packets and antibot control. The calls that
// answer a value return its raw encoded bytes for the values reader, or null on a negative code.

public static unsafe class Transport
{
    delegate int Fill(byte* output, int cap);

    static byte[] Fetch(Fill call)
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

    static byte[] Utf8(string text) => Encoding.UTF8.GetBytes(text);

    public static int ChannelRegister(string name)
    {
        byte[] n = Utf8(name);
        fixed (byte* p = n) return RawProxyNetwork.NetworkChannelRegister(p, n.Length);
    }

    public static int ChannelUnregister(string name)
    {
        byte[] n = Utf8(name);
        fixed (byte* p = n) return RawProxyNetwork.NetworkChannelUnregister(p, n.Length);
    }

    /// <summary>Observes the messages of the plugin's channels; an empty filter takes all of them.</summary>
    public static int MessageOn(string filter, Handler handler)
    {
        byte[] f = Utf8(filter);
        int token = Native.TokenFor(handler);
        fixed (byte* p = f) return RawProxyNetwork.NetworkMessageOn(p, f.Length, token);
    }

    /// <summary>The encoded map of the message (player_name, player_uuid, channel, data, source, server).</summary>
    public static byte[] MessageGet(int ctx) => Fetch((o, c) => RawProxyNetwork.NetworkMessageGet(ctx, o, c));

    /// <summary>Sends data on a channel; towardClient false goes to the player's backend, true to the client.</summary>
    public static int MessageSend(int player, string channel, byte[] data, bool towardClient)
    {
        byte[] ch = Utf8(channel);
        fixed (byte* cp = ch)
        fixed (byte* dp = data)
        {
            return RawProxyNetwork.NetworkMessageSend(player, cp, ch.Length, dp, data.Length, towardClient ? 1 : 0);
        }
    }

    public static byte[] PacketResolve(string name)
    {
        byte[] n = Utf8(name);
        return Fetch((o, c) => { fixed (byte* p = n) return RawProxyNetwork.PacketResolve(p, n.Length, o, c); });
    }

    public static byte[] PacketNames() => Fetch((o, c) => RawProxyNetwork.PacketNames(o, c));

    /// <summary>A decider listener; opts is an encoded value map (priority, fail) or empty. Returns the listener.</summary>
    public static int PacketOn(string name, byte[] opts, Handler handler)
    {
        byte[] n = Utf8(name);
        int token = Native.TokenFor(handler);
        fixed (byte* np = n)
        fixed (byte* op = opts)
        {
            return RawProxyNetwork.PacketOn(np, n.Length, op, opts.Length, token);
        }
    }

    /// <summary>An observer listener: it sees the final packet and cannot cancel or change it.</summary>
    public static int PacketWatch(string name, byte[] opts, Handler handler)
    {
        byte[] n = Utf8(name);
        int token = Native.TokenFor(handler);
        fixed (byte* np = n)
        fixed (byte* op = opts)
        {
            return RawProxyNetwork.PacketWatch(np, n.Length, op, opts.Length, token);
        }
    }

    public static int PacketRemove(int listener) => RawProxyNetwork.PacketRemove(listener);

    public static byte[] PacketGet(int ctx) => Fetch((o, c) => RawProxyNetwork.PacketGet(ctx, o, c));

    public static byte[] PacketBody(int ctx) => Fetch((o, c) => RawProxyNetwork.PacketBody(ctx, o, c));

    public static int PacketSetBody(int ctx, byte[] body)
    {
        fixed (byte* p = body) return RawProxyNetwork.PacketSetBody(ctx, p, body.Length);
    }

    public static int PacketCancel(int ctx) => RawProxyNetwork.PacketCancel(ctx);

    public static int PacketInject(int ctx, string name, byte[] body, bool intercept)
    {
        byte[] n = Utf8(name);
        fixed (byte* np = n)
        fixed (byte* bp = body)
        {
            return RawProxyNetwork.PacketInject(ctx, np, n.Length, bp, body.Length, intercept ? 1 : 0);
        }
    }

    public static int PacketSend(int player, string name, byte[] body)
    {
        byte[] n = Utf8(name);
        fixed (byte* np = n)
        fixed (byte* bp = body)
        {
            return RawProxyNetwork.PacketSend(player, np, n.Length, bp, body.Length);
        }
    }

    public static int PacketReceive(int player, string name, byte[] body)
    {
        byte[] n = Utf8(name);
        fixed (byte* np = n)
        fixed (byte* bp = body)
        {
            return RawProxyNetwork.PacketReceive(player, np, n.Length, bp, body.Length);
        }
    }

    public static int AntibotVerify(string name)
    {
        byte[] n = Utf8(name);
        fixed (byte* p = n) return RawProxyNetwork.AntibotVerify(p, n.Length);
    }

    public static int AntibotUnverify(string name)
    {
        byte[] n = Utf8(name);
        fixed (byte* p = n) return RawProxyNetwork.AntibotUnverify(p, n.Length);
    }

    public static int AntibotIsVerified(string name)
    {
        byte[] n = Utf8(name);
        fixed (byte* p = n) return RawProxyNetwork.AntibotIsVerified(p, n.Length);
    }

    public static int AntibotBan(string address)
    {
        byte[] a = Utf8(address);
        fixed (byte* p = a) return RawProxyNetwork.AntibotBan(p, a.Length);
    }

    public static int AntibotUnban(string address)
    {
        byte[] a = Utf8(address);
        fixed (byte* p = a) return RawProxyNetwork.AntibotUnban(p, a.Length);
    }

    public static int AntibotUnderAttack() => RawProxyNetwork.AntibotUnderAttack();

    public static int AntibotOn(Handler handler) => RawProxyNetwork.AntibotOn(Native.TokenFor(handler));

    public static byte[] AntibotEventGet(int ctx) => Fetch((o, c) => RawProxyNetwork.AntibotEventGet(ctx, o, c));
}
